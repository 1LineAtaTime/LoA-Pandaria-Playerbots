#!/usr/bin/env python3
"""Upstream auto-sync helper for .github/workflows/upstream-sync.yml.

Looks at the Legends-of-Azeroth master commits that arrived since the last check, decides which ones can be pulled into
playerbot-beta without a human, and applies those on a candidate branch. Everything else is reported for review.

  plan   list the new upstream commits and the verdict for each (no changes)
  apply  same, then apply the leading run of "auto" commits onto the current branch of --work

Verdicts:
  already  the commit is in playerbot-beta already (ancestor, or a "(cherry picked from commit <sha>)" trailer)
  auto     safe to pull: only SQL updates and/or scripts, small, touches no rows or files this fork changed
  review   needs a human (the reasons are listed); this and every later commit are left out of the run

The window: --since <sha> (the upstream commit the previous run ended on). When it is empty or not an ancestor of the
upstream head (first run, or upstream rewrote its history), the commits of the last --window-hours hours are taken.
"""
import argparse
import datetime
import json
import os
import re
import subprocess
import sys

AUTO_PATHS = re.compile(r"^(sql/updates/(world|characters|auth)/[^/]+\.sql|src/server/scripts/.+)$")
MAX_FILES = 15
MAX_LINES = 600
OVERLAP_DAYS = 90
SQL_NAME = re.compile(r"^(\d{4}_\d{2}_\d{2})_(\d+)(.*\.sql)$")
TRAILER = re.compile(r"\(cherry picked from commit ([0-9a-f]{40})\)")
PR_SUFFIX = re.compile(r"\s*\(#\d+\)\s*$")
KEY_COLUMNS = {"guid", "id", "entry", "creatureid", "questid", "entryorguid", "sourceentry", "menuid", "spawnid",
               "spellid", "spell_id", "item", "itemid", "areatriggerid", "npc_entry", "pathid", "gameobjectid"}


def git(*args, cwd, check=True, binary=False):
    r = subprocess.run(["git", *args], cwd=cwd, capture_output=True)
    if check and r.returncode:
        sys.exit("git %s failed: %s" % (" ".join(args), r.stderr.decode(errors="replace")))
    return r.stdout if binary else r.stdout.decode(errors="replace")


# --- SQL row targets -----------------------------------------------------------------------------------------------

def strip_comments(sql):
    out, i, n, q = [], 0, len(sql), None
    while i < n:
        c = sql[i]
        if q:
            out.append(c)
            if c == "\\" and i + 1 < n:
                out.append(sql[i + 1]); i += 2; continue
            if c == q:
                q = None
        elif c in "'\"`":
            q = c; out.append(c)
        elif sql.startswith("--", i) or c == "#":
            while i < n and sql[i] != "\n":
                i += 1
            continue
        elif sql.startswith("/*", i):
            j = sql.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        else:
            out.append(c)
        i += 1
    return "".join(out)


def split_statements(sql):
    stmts, cur, q, i, n = [], [], None, 0, len(sql)
    while i < n:
        c = sql[i]
        cur.append(c)
        if q:
            if c == "\\" and i + 1 < n:
                cur.append(sql[i + 1]); i += 2; continue
            if c == q:
                q = None
        elif c in "'\"`":
            q = c
        elif c == ";":
            stmts.append("".join(cur)); cur = []
        i += 1
    if "".join(cur).strip():
        stmts.append("".join(cur))
    return stmts


def first_values(values_sql):
    """First field of every (...) tuple of an INSERT ... VALUES list."""
    out, depth, q, field, i, n, start = [], 0, None, None, 0, len(values_sql), None
    while i < n:
        c = values_sql[i]
        if q:
            if c == "\\" and i + 1 < n:
                i += 2; continue
            if c == q:
                q = None
        elif c in "'\"":
            q = c
        elif c == "(":
            depth += 1
            if depth == 1:
                start = i + 1; field = None
        elif c == "," and depth == 1 and field is None:
            field = values_sql[start:i].strip()
        elif c == ")":
            if depth == 1:
                if field is None:
                    field = values_sql[start:i].strip()
                out.append(field)
            depth -= 1
        i += 1
    return out


COND = re.compile(r"`?(\w+)`?\s*(=\s*(-?\d+)|IN\s*\(([^)]*)\)|BETWEEN\s+(-?\d+)\s+AND\s+(-?\d+))", re.I)


def sql_targets(text):
    """(table, key) pairs a SQL file writes, plus a list of statements whose keys could not be read."""
    targets, unknown = set(), []
    for stmt in split_statements(strip_comments(text)):
        s = stmt.strip()
        m = re.match(r"(INSERT(?:\s+IGNORE)?|REPLACE)\s+INTO\s+`?(\w+)`?\s*(\([^)]*\))?\s*VALUES\s*(.*)$", s, re.I | re.S)
        if m:
            table = m.group(2).lower()
            for v in first_values(m.group(4)):
                v = v.strip("'\"")
                if re.fullmatch(r"-?\d+", v):
                    targets.add((table, v))
                else:
                    unknown.append("%s: INSERT key %r" % (table, v[:40]))
            continue
        m = re.match(r"(DELETE\s+(?:\w+\s+)?FROM|UPDATE)\s+`?(\w+)`?(.*)$", s, re.I | re.S)
        if m:
            table, rest = m.group(2).lower(), m.group(3)
            where = re.split(r"\bWHERE\b", rest, maxsplit=1, flags=re.I)
            keys = 0
            if len(where) == 2:
                for cm in COND.finditer(where[1]):
                    if cm.group(1).lower() not in KEY_COLUMNS:
                        continue
                    if cm.group(3):
                        targets.add((table, cm.group(3))); keys += 1
                    elif cm.group(4) is not None:
                        for v in re.findall(r"-?\d+", cm.group(4)):
                            targets.add((table, v)); keys += 1
                    elif cm.group(5):
                        lo, hi = int(cm.group(5)), int(cm.group(6))
                        if hi - lo <= 5000:
                            for v in range(lo, hi + 1):
                                targets.add((table, str(v)))
                            keys += 1
            if not keys:
                unknown.append("%s: %s without a readable key" % (table, s.split(None, 1)[0].upper()))
            continue
        if re.match(r"(SET|START|COMMIT|BEGIN|ROLLBACK)\b", s, re.I) or not s.strip(" ;\n\r\t"):
            continue
        unknown.append("statement %r" % s[:50])
    return targets, unknown


# --- selection -----------------------------------------------------------------------------------------------------

def pulled_shas(work, branch):
    return set(TRAILER.findall(git("log", "--format=%B", branch, cwd=work)))


def our_sql_targets(work, branch, upstream):
    """Rows this fork's own SQL updates write: every update file of the branch whose path upstream does not have."""
    ours = set(git("ls-tree", "-r", "--name-only", branch, "--", "sql/updates", cwd=work).split())
    theirs = set(git("ls-tree", "-r", "--name-only", upstream, "--", "sql/updates", cwd=work).split())
    targets = set()
    for path in sorted(ours - theirs):
        if not path.endswith(".sql") or "/playerbots/" in path:
            continue
        t, _ = sql_targets(git("show", "%s:%s" % (branch, path), cwd=work))
        targets |= t
    return targets


def recently_ours(work, branch, path):
    """True when a commit of this fork (not an upstream pick) changed path in the last OVERLAP_DAYS days."""
    log = git("log", "--no-merges", "--since=%d.days" % OVERLAP_DAYS, "--format=%H%x00%B%x01", branch, "--", path, cwd=work)
    for entry in log.split("\x01"):
        if entry.strip() and not TRAILER.search(entry):
            return True
    return False


def classify(work, branch, upstream, sha, our_rows):
    reasons = []
    parents = git("rev-list", "--parents", "-n", "1", sha, cwd=work).split()
    if len(parents) != 2:
        return ["merge commit"]
    files = [l.split("\t") for l in git("diff-tree", "--no-commit-id", "-r", "--name-status", sha, cwd=work).splitlines() if l]
    paths = [f[-1] for f in files]
    bad = [p for p in paths if not AUTO_PATHS.match(p)]
    if bad:
        reasons.append("touches paths outside SQL updates/scripts: %s" % ", ".join(bad[:6]))
    lines = 0
    for l in git("diff-tree", "--no-commit-id", "-r", "--numstat", sha, cwd=work).splitlines():
        a, d, _ = l.split("\t", 2)
        lines += (int(a) if a != "-" else 0) + (int(d) if d != "-" else 0)
    if len(paths) > MAX_FILES or lines > MAX_LINES:
        reasons.append("too large: %d files, %d lines (limit %d / %d)" % (len(paths), lines, MAX_FILES, MAX_LINES))
    for status, *rest in files:
        path = rest[-1]
        if path.startswith("sql/updates/") and path.endswith(".sql") and status != "D":
            t, unknown = sql_targets(git("show", "%s:%s" % (sha, path), cwd=work))
            hit = sorted(t & our_rows)
            if hit:
                reasons.append("%s writes rows this fork also changes: %s" % (path, ", ".join("%s %s" % h for h in hit[:8])))
            if unknown:
                reasons.append("%s has statements whose rows cannot be read: %s" % (path, "; ".join(unknown[:3])))
        elif path.startswith("src/") and recently_ours(work, branch, path):
            reasons.append("%s was changed by this fork in the last %d days" % (path, OVERLAP_DAYS))
    return reasons


def window(work, upstream, since, hours):
    head = git("rev-parse", upstream, cwd=work).strip()
    if since and subprocess.run(["git", "merge-base", "--is-ancestor", since, head], cwd=work,
                                capture_output=True).returncode == 0:
        return head, "since %s" % since[:8], git("rev-list", "--reverse", "%s..%s" % (since, head), cwd=work).split()
    cutoff = (datetime.datetime.now(datetime.timezone.utc) - datetime.timedelta(hours=hours)).strftime("%Y-%m-%dT%H:%M:%SZ")
    return head, "last %d h (no usable previous check)" % hours, \
        git("rev-list", "--reverse", "--since=%s" % cutoff, head, cwd=work).split()


# --- apply ---------------------------------------------------------------------------------------------------------

def free_name(work, directory, day, rest, taken):
    for nn in range(100):
        name = "%s_%02d%s" % (day, nn, rest)
        path = "%s/%s" % (directory, name)
        if path not in taken and not os.path.exists(os.path.join(work, path)):
            taken.add(path)
            return path
    sys.exit("no free update file name for %s in %s" % (day, directory))


def apply_commit(work, sha, day, taken):
    """Apply sha onto the work tree's HEAD. Added SQL updates get today's name (fresh installs then apply them after
    everything the branch has, the order existing installs see). Returns (ok, renames, error)."""
    files = [l.split("\t") for l in git("diff-tree", "--no-commit-id", "-r", "--name-status", sha, cwd=work).splitlines() if l]
    renames, rest_paths = [], []
    for status, *names in files:
        path = names[-1]
        m = SQL_NAME.match(os.path.basename(path))
        if status == "A" and path.startswith("sql/updates/") and m:
            new = free_name(work, os.path.dirname(path), day, m.group(3), taken)
            with open(os.path.join(work, new), "wb") as fh:
                fh.write(git("show", "%s:%s" % (sha, path), cwd=work, binary=True))
            git("add", "--", new, cwd=work)
            renames.append((path, new))
        else:
            rest_paths.append(path)
    if rest_paths:
        patch = git("diff", "--binary", "%s^" % sha, sha, "--", *rest_paths, cwd=work, binary=True)
        r = subprocess.run(["git", "apply", "--index", "--3way"], cwd=work, input=patch, capture_output=True)
        if r.returncode:
            git("reset", "--hard", "-q", "HEAD", cwd=work)
            for _, new in renames:
                p = os.path.join(work, new)
                if os.path.exists(p):
                    os.remove(p)
            return False, renames, r.stderr.decode(errors="replace").strip()[:800]
    subject = PR_SUFFIX.sub("", git("log", "-1", "--format=%s", sha, cwd=work).strip())
    body = git("log", "-1", "--format=%b", sha, cwd=work).strip()
    notes = ["Update file renamed from %s." % os.path.basename(old) for old, _ in renames]
    msg = subject + ("\n\n" + body if body else "") + ("\n\n" + "\n".join(notes) if notes else "") + \
        "\n\n(cherry picked from commit %s)" % sha
    env = dict(os.environ,
               GIT_AUTHOR_NAME=git("log", "-1", "--format=%an", sha, cwd=work).strip(),
               GIT_AUTHOR_EMAIL=git("log", "-1", "--format=%ae", sha, cwd=work).strip(),
               GIT_AUTHOR_DATE=git("log", "-1", "--format=%aD", sha, cwd=work).strip())
    r = subprocess.run(["git", "commit", "-q", "-F", "-"], cwd=work, input=msg.encode(), env=env, capture_output=True)
    if r.returncode:
        return False, renames, r.stderr.decode(errors="replace")[:800]
    return True, renames, ""


# --- main ----------------------------------------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["plan", "apply"])
    ap.add_argument("--work", required=True, help="checkout of playerbot-beta (apply commits onto its HEAD)")
    ap.add_argument("--branch", default="HEAD", help="ref of playerbot-beta in --work")
    ap.add_argument("--upstream", default="upstream/master")
    ap.add_argument("--since", default="")
    ap.add_argument("--window-hours", type=int, default=6)
    ap.add_argument("--out", default="upstream-sync.json")
    a = ap.parse_args()

    work = os.path.abspath(a.work)
    head, how, commits = window(work, a.upstream, a.since, a.window_hours)
    pulled = pulled_shas(work, a.branch)
    our_rows = our_sql_targets(work, a.branch, a.upstream) if commits else set()
    day = datetime.datetime.now(datetime.timezone.utc).strftime("%Y_%m_%d")
    taken, results, blocked = set(), [], False
    for sha in commits:
        subject = git("log", "-1", "--format=%s", sha, cwd=work).strip()
        entry = {"sha": sha, "subject": subject}
        if sha in pulled or subprocess.run(["git", "merge-base", "--is-ancestor", sha, a.branch], cwd=work).returncode == 0:
            entry["verdict"] = "already"
        elif blocked:
            entry.update(verdict="review", reasons=["held: comes after a commit that needs review"])
        else:
            reasons = classify(work, a.branch, a.upstream, sha, our_rows)
            if reasons:
                entry.update(verdict="review", reasons=reasons); blocked = True
            elif a.mode == "apply":
                ok, renames, err = apply_commit(work, sha, day, taken)
                if ok:
                    entry.update(verdict="auto", applied=git("rev-parse", "HEAD", cwd=work).strip(),
                                 renames=["%s -> %s" % r for r in renames])
                else:
                    entry.update(verdict="review", reasons=["does not apply cleanly: " + err]); blocked = True
            else:
                entry["verdict"] = "auto"
        results.append(entry)

    report = {"upstream_head": head, "window": how, "commits": results,
              "auto": sum(1 for r in results if r["verdict"] == "auto"),
              "review": sum(1 for r in results if r["verdict"] == "review"),
              "already": sum(1 for r in results if r["verdict"] == "already")}
    with open(a.out, "w") as fh:
        json.dump(report, fh, indent=2)
    print("upstream head %s, window %s: %d new, %d auto, %d review, %d already in"
          % (head[:8], how, len(results), report["auto"], report["review"], report["already"]))
    for r in results:
        print("  %s %-7s %s" % (r["sha"][:8], r["verdict"], r["subject"]))
        for reason in r.get("reasons", []):
            print("           - " + reason)
        for rn in r.get("renames", []):
            print("           renamed " + rn)


if __name__ == "__main__":
    main()
