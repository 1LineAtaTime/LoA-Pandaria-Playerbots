"""Summarize existing logs or capture a fresh, bounded observation interval.

Only reads the server's logs. Reports may contain private runtime data; keep them
in the ignored runtime logs directory, never commit or upload them automatically.
"""
import argparse
import collections
import datetime
from pathlib import Path
import re
import time


def summarize(path, severities=None):
    counts = collections.Counter()
    examples = {}
    lines = 0
    with path.open(encoding="utf-8-sig", errors="replace") as stream:
        for line in stream:
            lines += 1
            text = line.strip()
            if not text:
                continue
            if severities and not re.match(r"^\d{4}-\d\d-\d\d.*?\s(?:" + severities + r")\s+", text):
                continue
            # Prefixes are available in newer configurations; older logs omit them.
            text = re.sub(r"^\d{4}-\d\d-\d\d.*?\s(?:WARN|ERROR|FATAL)\s+", "", text)
            key = re.sub(r"0x[\da-fA-F]+|\b\d+(?:\.\d+)?\b", "#", text)
            # Names differ per row but have the same actionable failure class.
            key = re.sub(r"ScriptName '[^']*' exists in database", "ScriptName '<name>' exists in database", key)
            key = re.sub(r"Creature\([^)]*\)", "Creature(<name>)", key)
            counts[key] += 1
            examples.setdefault(key, text)
    return lines, counts, examples


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--logs", type=Path, required=True)
    parser.add_argument("--watch-seconds", type=int, default=0)
    parser.add_argument("--source", choices=("database", "all"), default="database",
                        help="all scans WARN/ERROR/FATAL across every category in Server.log")
    parser.add_argument("--scope", choices=("historical", "current"), default="historical",
                        help="Use current only when the source log begins at the current boot")
    args = parser.parse_args()
    if args.watch_seconds < 0:
        parser.error("watch-seconds must be nonnegative")
    logs = args.logs.resolve(strict=True)
    output = logs / "diagnostics"
    output.mkdir(exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    if args.watch_seconds:
        # Start at EOF so historical errors cannot be mistaken for new failures.
        sources = [logs / "Server.log", logs / "DBErrors.log"]
        positions = {p: (p.stat().st_ino, p.stat().st_size) for p in sources if p.exists()}
        deadline = time.monotonic() + args.watch_seconds
        with (output / f"live-{stamp}.txt").open("w", encoding="utf-8", newline="") as target:
            target.write(f"Observation started {datetime.datetime.now().astimezone().isoformat()}\n")
            while time.monotonic() < deadline:
                for path in sources:
                    if not path.exists():
                        continue
                    stat = path.stat()
                    inode, offset = positions.get(path, (stat.st_ino, 0))
                    if inode != stat.st_ino or stat.st_size < offset:
                        offset = 0
                    with path.open("rb") as stream:
                        stream.seek(offset)
                        data = stream.read()
                        positions[path] = (stat.st_ino, stream.tell())
                    if data:
                        target.write(f"\n--- {path.name} ---\n")
                        target.write(data.decode("utf-8", errors="replace"))
                        target.flush()
                time.sleep(min(1, max(0, deadline - time.monotonic())))
        print(output / f"live-{stamp}.txt")
        return
    # SQL has a dedicated destination and does not necessarily reach Server.log.
    # Errors.txt is the canonical all-category ERROR/FATAL stream; WARN is only
    # in Server.log. Combining these disjoint severities avoids double-counting.
    errors = logs / ("Errors.txt" if args.source == "all" else "DBErrors.log")
    lines, counts, examples = summarize(errors)
    sources = [errors]
    if args.source == "all":
        warnings = logs / "Server.log"
        warn_lines, warn_counts, warn_examples = summarize(warnings, severities="WARN")
        lines += warn_lines
        counts.update(warn_counts)
        for pattern, example in warn_examples.items():
            examples.setdefault(pattern, example)
        sources.append(warnings)
    report = output / f"error-summary-{args.source}-{stamp}.txt"
    with report.open("w", encoding="utf-8") as target:
        target.write(f"Generated {datetime.datetime.now().astimezone().isoformat()}\n")
        target.write(f"Sources: {', '.join(map(str, sources))}\n{lines} lines scanned; {sum(p.stat().st_size for p in sources)} bytes\n")
        target.write(f"Source selection: {args.source}; {sum(counts.values())} diagnostic messages.\n")
        target.write("CURRENT SESSION LOG (scope selected by caller).\n" if args.scope == "current" else
                     "CUMULATIVE LOG: counts may span multiple boots and previously fixed issues.\n")
        target.write("Counts are message patterns, not independent defects or current-run failures.\n\n")
        for pattern, count in counts.most_common(40):
            target.write(f"{count:>8}  {pattern}\n  Example: {examples[pattern]}\n\n")
    print(report)
    for pattern, count in counts.most_common(12):
        print(f"{count:>8}  {pattern[:180]}")


if __name__ == "__main__":
    main()
