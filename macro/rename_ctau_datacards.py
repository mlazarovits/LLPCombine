#!/usr/bin/env python3
"""
rename_ctau_datacards.py
--------------------------
Rename signal subdirectories inside a BF.x datacard directory so that the
embedded ctau token reflects the *target* ctau used in lifetime reweighting,
rather than the source ctau baked into the MC filename.

Background
----------
BFI.x derives signal process keys from the MC filename via GetSignalTokens
(BuildFitTools.h).  For a sample generated at ctau=50 cm the key is e.g.

    gogoGZ_2300_1300_1000_50

regardless of any --target-ctau reweighting applied.  BF.x names the
datacard subdirectory after this key, and RunAndCollectLimits.py extracts
the ctau from the directory name — so without renaming, every reweighted
ctau point would be recorded under ctau=50.

This script replaces the source ctau token with the target ctau token in
every matching subdirectory name.

Usage
-----
# Dry-run (default): show what would be renamed, make no changes
python3 macro/rename_ctau_datacards.py \\
    --dc-dir datacards_BigGuy_NonCompressed_FullRegions_CtauScan_ctau30 \\
    --source-ctau 50 \\
    --target-ctau 30

# Actually rename
python3 macro/rename_ctau_datacards.py \\
    --dc-dir datacards_BigGuy_NonCompressed_FullRegions_CtauScan_ctau30 \\
    --source-ctau 50 \\
    --target-ctau 30 \\
    --apply

The script only renames immediate subdirectories of --dc-dir (one level deep),
which is where BF.x writes one directory per signal mass/ctau point.

The ctau token is matched as a whole number field delimited by underscores or
end-of-string, so e.g. source_ctau=50 matches "_50" but not "_500".
"""

import os
import re
import sys
import argparse


def detect_source_ctau(dc_dir: str, target_ctau: int) -> int:
    """
    Infer the source ctau by reading the trailing numeric token from the
    first signal subdirectory whose name does not already end with target_ctau.
    Signal dirs follow the pattern: <process>_<mS>_<mN2>_<mN1>_<ctau>
    """
    try:
        entries = sorted(os.listdir(dc_dir))
    except FileNotFoundError:
        print(f"Error: directory not found: {dc_dir}", file=sys.stderr)
        sys.exit(1)

    tgt_str = str(target_ctau)
    for name in entries:
        if not os.path.isdir(os.path.join(dc_dir, name)):
            continue
        m = re.search(r'_(\d+)$', name)
        if m and m.group(1) != tgt_str:
            return int(m.group(1))

    return None


def find_renames(dc_dir: str, source_ctau: int, target_ctau: int):
    """
    Return list of (old_path, new_path) pairs for subdirectories of dc_dir
    whose names contain the source_ctau token as a whole field.
    """
    src_tok = str(source_ctau)
    tgt_tok = str(target_ctau)

    # Match the ctau token as a whole field: preceded by '_', followed by '_' or end-of-string.
    pattern = re.compile(r'(?<=_)' + re.escape(src_tok) + r'(?=_|$)')

    renames = []
    try:
        entries = sorted(os.listdir(dc_dir))
    except FileNotFoundError:
        print(f"Error: directory not found: {dc_dir}", file=sys.stderr)
        sys.exit(1)

    for name in entries:
        old_path = os.path.join(dc_dir, name)
        if not os.path.isdir(old_path):
            continue
        if not pattern.search(name):
            continue
        new_name = pattern.sub(tgt_tok, name)
        if new_name == name:
            continue
        new_path = os.path.join(dc_dir, new_name)
        renames.append((old_path, new_path))

    return renames


def main():
    parser = argparse.ArgumentParser(
        description="Rename ctau token in BF.x datacard subdirectory names",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("--dc-dir",      required=True,
                        help="Path to the datacard root directory (contains signal subdirs)")
    parser.add_argument("--source-ctau", type=int, default=None,
                        help="ctau value to replace (integer cm); auto-detected if omitted")
    parser.add_argument("--target-ctau", required=True, type=int,
                        help="Replacement ctau value (integer cm), e.g. 30")
    parser.add_argument("--apply",       action="store_true",
                        help="Actually rename directories (default: dry-run only)")
    args = parser.parse_args()

    source_ctau = args.source_ctau
    if source_ctau is None:
        source_ctau = detect_source_ctau(args.dc_dir, args.target_ctau)
        if source_ctau is None:
            print(f"Error: could not detect source ctau in '{args.dc_dir}'. "
                  f"Pass --source-ctau explicitly.", file=sys.stderr)
            sys.exit(1)
        print(f"Auto-detected source_ctau={source_ctau}")

    if source_ctau == args.target_ctau:
        print("source-ctau and target-ctau are identical — nothing to do.")
        sys.exit(0)

    renames = find_renames(args.dc_dir, source_ctau, args.target_ctau)

    if not renames:
        print(f"No subdirectories in '{args.dc_dir}' contain ctau token '{args.source_ctau}'.")
        sys.exit(0)

    mode_label = "APPLY" if args.apply else "DRY-RUN"
    print(f"[{mode_label}] {len(renames)} rename(s) in '{args.dc_dir}':")
    for old, new in renames:
        old_name = os.path.basename(old)
        new_name = os.path.basename(new)
        print(f"  {old_name}  ->  {new_name}")

    if not args.apply:
        print("\nDry-run complete. Pass --apply to execute the renames.")
        return

    n_ok = 0
    n_skip = 0
    n_fail = 0
    for old, new in renames:
        if os.path.exists(new):
            print(f"  Skipped (target exists — native sample already present): {os.path.basename(new)}")
            n_skip += 1
            continue
        try:
            os.rename(old, new)
            n_ok += 1
        except OSError as e:
            print(f"  Error renaming {os.path.basename(old)}: {e}")
            n_fail += 1

    parts = [f"renamed {n_ok}"]
    if n_skip:
        parts.append(f"skipped {n_skip} (native sample exists)")
    if n_fail:
        parts.append(f"FAILED {n_fail}")
    print(f"\n{', '.join(parts)} out of {len(renames)} directories.")
    if n_fail:
        sys.exit(1)


if __name__ == "__main__":
    main()
