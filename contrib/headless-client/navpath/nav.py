"""Navmesh path queries for the headless client (Python 3, standard library only).

Talks to the native ``navpath`` CLI in ``--serve`` mode: one JSON request per line on its stdin,
one JSON answer per line on its stdout. The subprocess is started lazily on first use and reused,
so tiles stay cached between queries.

    import nav
    path = nav.find_path(0, (-8914.0, -133.0, 81.0), (-8775.0, -185.0, 84.0))
    if path:                       # list of (x, y, z), first point = start snapped to the mesh
        for x, y, z in path: ...

Configuration (environment):
    MOP_NAV_DATA     server data directory that contains mmaps/ (the worldserver's DataDir)
    MOP_NAVPATH_EXE  path of the navpath executable (default: build/ next to this file)
"""

import json
import os
import subprocess
import sys
import threading

__all__ = [
    "NavError", "NavClient", "default_data_dir", "default_exe",
    "find_path", "query_path", "nearest", "height", "close",
]

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO = os.path.normpath(os.path.join(_HERE, "..", "..", ".."))

# Data directories tried when MOP_NAV_DATA is not set, relative to the repository root.
_DATA_CANDIDATES = (
    "data",
)


class NavError(RuntimeError):
    """The navpath process could not be started or stopped answering."""


def default_data_dir():
    env = os.environ.get("MOP_NAV_DATA")
    if env:
        return env
    for rel in _DATA_CANDIDATES:
        cand = os.path.join(_REPO, rel)
        if os.path.isdir(os.path.join(cand, "mmaps")):
            return cand
    return None


def default_exe():
    env = os.environ.get("MOP_NAVPATH_EXE")
    if env:
        return env
    name = "navpath.exe" if os.name == "nt" else "navpath"
    for sub in (("build",), ("build", "Release"), ("build", "RelWithDebInfo")):
        cand = os.path.join(_HERE, *sub, name)
        if os.path.isfile(cand):
            return cand
    return os.path.join(_HERE, "build", name)


def _vec(p, n=3):
    vals = [float(v) for v in p]
    if len(vals) != n:
        raise ValueError("expected %d coordinates, got %r" % (n, p))
    return vals


class NavClient:
    """One navpath --serve subprocess. Thread-safe; requests are serialised."""

    def __init__(self, data_dir=None, exe=None, extra_args=()):
        self.data_dir = data_dir or default_data_dir()
        self.exe = exe or default_exe()
        self.extra_args = list(extra_args)
        self._proc = None
        self._lock = threading.Lock()
        self._next_id = 1

    # -- process management -------------------------------------------------
    def _start(self):
        if not self.data_dir:
            raise NavError("no navmesh data dir: set MOP_NAV_DATA to the server DataDir (holds mmaps/)")
        if not os.path.isfile(self.exe):
            raise NavError("navpath executable not found at %s (build it with build.cmd or set MOP_NAVPATH_EXE)" % self.exe)
        flags = 0
        if os.name == "nt":
            flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
        self._proc = subprocess.Popen(
            [self.exe, "--data", self.data_dir, "--serve"] + self.extra_args,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
            universal_newlines=True, bufsize=1, creationflags=flags)

    def close(self):
        with self._lock:
            proc, self._proc = self._proc, None
            if proc is None:
                return
            try:
                if proc.poll() is None:
                    proc.stdin.write('{"op":"quit"}\n')
                    proc.stdin.flush()
                    proc.wait(timeout=5)
            except (OSError, ValueError, subprocess.TimeoutExpired):
                proc.kill()
            finally:
                for f in (proc.stdin, proc.stdout):
                    try:
                        f.close()
                    except OSError:
                        pass

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def request(self, req):
        """Send one request dict, return the answer dict. Restarts a dead process once."""
        with self._lock:
            req = dict(req)
            req["id"] = self._next_id
            self._next_id += 1
            line = json.dumps(req, separators=(",", ":")) + "\n"
            for attempt in (1, 2):
                if self._proc is None or self._proc.poll() is not None:
                    self._start()
                try:
                    self._proc.stdin.write(line)
                    self._proc.stdin.flush()
                    answer = self._proc.stdout.readline()
                except (OSError, ValueError):
                    answer = ""
                if answer:
                    resp = json.loads(answer)
                    if resp.get("id") != req["id"]:
                        raise NavError("navpath answer out of order: %r" % (resp,))
                    return resp
                # process died mid-request: restart and retry once
                try:
                    self._proc.kill()
                except OSError:
                    pass
                self._proc = None
            raise NavError("navpath stopped answering")

    # -- queries ------------------------------------------------------------
    def query_path(self, map_id, start, end, step=None, **opts):
        """Full answer dict: ok, partial, path, length, end, polys, ms, ... (see README)."""
        req = {"op": "path", "map": int(map_id), "from": _vec(start), "to": _vec(end)}
        if step is not None:
            req["step"] = float(step)
        req.update(opts)
        return self.request(req)

    def find_path(self, map_id, start, end, step=None, allow_partial=False, **opts):
        """List of (x, y, z) from start to end, or None when there is no walkable path.

        A partial path (destination not reachable, or more than 7 yd off the mesh) is returned
        only with allow_partial=True; it then ends at the reachable point closest to 'end'.
        """
        r = self.query_path(map_id, start, end, step=step, **opts)
        if not r.get("ok"):
            return None
        if r.get("partial") and not allow_partial:
            return None
        return [tuple(p) for p in r["path"]]

    def nearest(self, map_id, xyz, h_ext=None, v_ext=None):
        """(x, y, z) of the closest walkable mesh point, or None. Default search box is the server's."""
        req = {"op": "nearest", "map": int(map_id), "pos": _vec(xyz)}
        if h_ext is not None:
            req["h_ext"] = float(h_ext)
        if v_ext is not None:
            req["v_ext"] = float(v_ext)
        r = self.request(req)
        return tuple(r["pos"]) if r.get("ok") else None

    def height(self, map_id, x, y, z=None, all_levels=False):
        """Mesh height at (x, y): the level nearest z (or the highest), or every level with all_levels."""
        pos = [float(x), float(y)] + ([float(z)] if z is not None else [])
        r = self.request({"op": "height", "map": int(map_id), "pos": pos})
        if not r.get("ok"):
            return [] if all_levels else None
        return r["heights"] if all_levels else r["z"]

    def stats(self):
        return self.request({"op": "stats"})


_default = None
_default_lock = threading.Lock()


def _client():
    global _default
    with _default_lock:
        if _default is None:
            _default = NavClient()
        return _default


def find_path(map_id, start, end, step=None, allow_partial=False, **opts):
    return _client().find_path(map_id, start, end, step=step, allow_partial=allow_partial, **opts)


def query_path(map_id, start, end, step=None, **opts):
    return _client().query_path(map_id, start, end, step=step, **opts)


def nearest(map_id, xyz, h_ext=None, v_ext=None):
    return _client().nearest(map_id, xyz, h_ext=h_ext, v_ext=v_ext)


def height(map_id, x, y, z=None, all_levels=False):
    return _client().height(map_id, x, y, z=z, all_levels=all_levels)


def close():
    global _default
    with _default_lock:
        if _default is not None:
            _default.close()
            _default = None


def _main(argv):
    import argparse
    ap = argparse.ArgumentParser(description="navmesh path query via navpath --serve")
    ap.add_argument("--map", type=int, required=True)
    ap.add_argument("--from", dest="start", required=True, help="x,y,z")
    ap.add_argument("--to", dest="end", required=True, help="x,y,z")
    ap.add_argument("--step", type=float)
    a = ap.parse_args(argv)
    parse = lambda s: [float(v) for v in s.split(",")]
    r = query_path(a.map, parse(a.start), parse(a.end), step=a.step)
    print(json.dumps(r))
    close()
    return 0 if r.get("ok") else 1


if __name__ == "__main__":
    sys.exit(_main(sys.argv[1:]))
