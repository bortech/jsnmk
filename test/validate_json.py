#!/usr/bin/env python3
"""Validate documents emitted by jsnmk_json_dump with a real JSON parser.

Usage: validate_json.py EXECUTABLE [EXPECTED_DOCS]

The dump program prints one JSON document per line. Each line is parsed
with json.loads(), so this catches malformed JSON that byte-for-byte C
assertions may not cover (numbers, UTF-8, punctuation, etc.).
"""

import json
import subprocess
import sys


def fail(message):
    print("jsnmk_json_valid: " + message, file=sys.stderr)
    return 1


def main(argv):
    if len(argv) not in (2, 3):
        print(__doc__, file=sys.stderr)
        return 2

    executable = argv[1]
    expected = int(argv[2]) if len(argv) == 3 else None

    try:
        proc = subprocess.run(
            [executable],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            check=False,
        )
    except OSError as exc:
        return fail("cannot run %s: %s" % (executable, exc))

    if proc.returncode != 0:
        return fail(
            "%s exited with %d\n%s"
            % (executable, proc.returncode, proc.stderr.rstrip())
        )

    lines = [line for line in proc.stdout.splitlines() if line.strip()]
    if not lines:
        return fail("no documents were emitted")

    if expected is not None and len(lines) != expected:
        return fail("expected %d documents, got %d" % (expected, len(lines)))

    for number, line in enumerate(lines, start=1):
        try:
            json.loads(line)
        except json.JSONDecodeError as exc:
            return fail(
                "document %d is not valid JSON: %s\n  %s"
                % (number, exc, line)
            )

    print("jsnmk_json_valid: parsed %d JSON documents" % len(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
