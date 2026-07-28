#!/usr/bin/env python3
"""Filter .beads/issues.jsonl on stdin: one line per issue id, newest wins.

`.beads/issues.jsonl` uses git's `union` merge driver (see .gitattributes),
which appends the other side's unique lines AFTER ours. `bd import` (1.0.3)
upserts last-line-wins with no updated_at comparison — so a merged-in branch's
stale snapshot silently overwrites newer issue states. That reopened a closed
issue in kube-lvl47 on 2026-07-22; this repo runs the identical union driver
and the identical import, so it has always had the same exposure.

Feeding import through this filter keeps only the newest line per id, so line
order stops mattering.

Ties on updated_at keep the later line (matches plain import behavior).
Memory records (_type=memory) and unparseable lines pass through untouched:
memory records have no issue id to collapse on, and silently dropping a line
we failed to parse would be worse than importing it as-is.

Ported from kube-lvl47 scripts/beads-dedupe-jsonl.py (ansible-cxu). Keep the
two copies behaviourally identical.
"""

import json
import sys


def main() -> None:
    records = []  # (idx, raw, issue_id, timestamp)
    best = {}  # issue_id -> (idx, timestamp)
    for idx, raw in enumerate(sys.stdin):
        raw = raw.rstrip("\n")
        if not raw.strip():
            continue
        try:
            doc = json.loads(raw)
        except ValueError:
            records.append((idx, raw, None, None))
            continue
        issue_id = doc.get("id") if doc.get("_type") != "memory" else None
        # ISO-8601 UTC timestamps compare correctly as strings
        ts = doc.get("updated_at") or doc.get("updated") or ""
        records.append((idx, raw, issue_id, ts))
        if issue_id:
            cur = best.get(issue_id)
            if cur is None or ts >= cur[1]:
                best[issue_id] = (idx, ts)

    for idx, raw, issue_id, _ in records:
        if issue_id and best[issue_id][0] != idx:
            continue
        print(raw)


if __name__ == "__main__":
    main()
