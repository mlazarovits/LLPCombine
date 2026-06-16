#!/usr/bin/env bash
# run_ctau_scan_bfi.sh
# ---------------------
# Run BFI.x for a ctau scan using a single base AnalysisConfig YAML.
#
# BFI.x accepts --source-ctau and --target-ctau directly on the CLI.
# When --target-ctau is passed and the YAML has no lifetimeWeights block,
# BFI.x automatically appends _ctauN to the output JSON filename so each
# scan point writes a distinct file (no generated per-ctau YAMLs needed).
#
# --target-zrate and --target-grate override the Z/photon decay BRs without
# needing a separate YAML per BR point.  When set, BFI.x appends _Z{n}G{n}
# to the output JSON (e.g. MyAnalysis_ctau30_Z100G0.json).
#
# Usage:
#   bash macro/run_ctau_scan_bfi.sh [options] BASE_CONFIG SOURCE_CTAU TARGET_CTAU [TARGET_CTAU ...]
#
# Options:
#   --continue          Skip any ctau point whose output JSON already exists
#   --workers N         Parallel worker count (default: 4)
#   --json-dir DIR      Directory for output JSONs (default: json/)
#   --target-zrate VAL  Target Z decay rate passed to BFI.x (0-1)
#   --target-grate VAL  Target photon decay rate passed to BFI.x (0-1)
#
# Examples:
#   bash macro/run_ctau_scan_bfi.sh \
#       config_master/BigGuy_NonCompressed_FullRegions_AnalysisConfig.yaml \
#       50  5 10 20 30 75 100
#
#   bash macro/run_ctau_scan_bfi.sh --continue --workers 8 \
#       config_master/BigGuy_NonCompressed_FullRegions_AnalysisConfig.yaml \
#       50  10 30 100
#
#   bash macro/run_ctau_scan_bfi.sh --target-zrate 1 --target-grate 0 \
#       config_master/LittleGuy_Compressed_FullRegions_MixDelShape_AnalysisConfig.yaml \
#       50  5 10 20 30 75 100
#
# Run from the LLPCombine root directory (where BFI.x lives).
#
# -------------------------------------------------------------------------
# IMPORTANT: directory rename required before RunAndCollectLimits.py
# -------------------------------------------------------------------------
# BFI.x keys signal processes from the MC filename (e.g. gogoGZ_2300_..._50)
# even when reweighted to a different ctau.  BF.x names datacard directories
# after those keys, and RunAndCollectLimits.py reads the ctau from the
# directory name — so without renaming, every limit lands under ctau=50.
#
# After running BF.x for each ctau point, rename the subdirectories:
#   python3 macro/rename_ctau_datacards.py \
#       --dc-dir datacards_<fitname> \
#       --source-ctau <source_ctau> \
#       --target-ctau <target_ctau> \
#       --apply
# Run this once per target ctau before RunAndCollectLimits.py.
# -------------------------------------------------------------------------

set -euo pipefail

# Convert a ctau float to the Xp filename tag used by BFI.x:
#   50 -> "50",  0.01 -> "0p01",  1.5 -> "1p5"
ctau_to_tag() {
    echo "$1" | awk '{ if ($1 == int($1)) printf "%d\n", int($1)
                       else { gsub(/\./, "p"); print } }'
}

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
CONTINUE_MODE=false
NWORKERS=4
JSON_DIR="json"
SOURCE_ZRATE=""
SOURCE_GRATE=""
TARGET_ZRATE=""
TARGET_GRATE=""
POSITIONAL=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --continue)       CONTINUE_MODE=true; shift ;;
        --workers)        NWORKERS="$2"; shift 2 ;;
        --json-dir)       JSON_DIR="$2"; shift 2 ;;
        --source-zrate)   SOURCE_ZRATE="$2"; shift 2 ;;
        --source-grate)   SOURCE_GRATE="$2"; shift 2 ;;
        --target-zrate)   TARGET_ZRATE="$2"; shift 2 ;;
        --target-grate)   TARGET_GRATE="$2"; shift 2 ;;
        -*)               echo "Unknown option: $1"; exit 1 ;;
        *)                POSITIONAL+=("$1"); shift ;;
    esac
done

# Default to native SMS sample BRs (50/50) when not explicitly set.
if [[ -z "$TARGET_ZRATE" && -z "$TARGET_GRATE" ]]; then
    TARGET_ZRATE="0.5"
    TARGET_GRATE="0.5"
fi

if [[ ${#POSITIONAL[@]} -lt 3 ]]; then
    echo "Usage: $0 [--continue] [--workers N] [--json-dir DIR] [--source-zrate VAL] [--source-grate VAL] [--target-zrate VAL] [--target-grate VAL] BASE_CONFIG SOURCE_CTAU TARGET_CTAU [...]"
    exit 1
fi

BASE_CONFIG="${POSITIONAL[0]}"
SOURCE_CTAU="${POSITIONAL[1]}"
TARGET_CTAUS=("${POSITIONAL[@]:2}")
BFI_BIN="./BFI.x"

if [[ ! -f "$BASE_CONFIG" ]]; then
    echo "Error: base config not found: $BASE_CONFIG"
    exit 1
fi
if [[ ! -x "$BFI_BIN" ]]; then
    echo "Error: BFI.x not found or not executable. Run 'make' first."
    exit 1
fi

mkdir -p "$JSON_DIR"

# Limit ROOT thread count per worker to avoid oversubscription.
NCPUS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
THREADS_PER_WORKER=$(( (NCPUS + NWORKERS - 1) / NWORKERS ))
THREADS_PER_WORKER=$(( THREADS_PER_WORKER < 1 ? 1 : THREADS_PER_WORKER ))
export ROOT_THREADS=$THREADS_PER_WORKER

# ---------------------------------------------------------------------------
# Worker function
# ---------------------------------------------------------------------------
process_bfi_point() {
    local source_ctau="$1"
    local target_ctau="$2"
    local json_dir="$3"
    local cfg="$4"
    local source_zrate="$5"
    local source_grate="$6"
    local zrate="$7"
    local grate="$8"

    local br_args=()
    [[ -n "$source_zrate" ]] && br_args+=(--source-zrate "$source_zrate")
    [[ -n "$source_grate" ]] && br_args+=(--source-grate "$source_grate")
    [[ -n "$zrate" ]] && br_args+=(--target-zrate "$zrate")
    [[ -n "$grate" ]] && br_args+=(--target-grate "$grate")

    echo "=== BFI: source_ctau=${source_ctau}  target_ctau=${target_ctau} ${br_args[*]} ==="
    if ! $BFI_BIN --config "$cfg" \
                  --source-ctau "$source_ctau" \
                  --target-ctau "$target_ctau" \
                  --output-dir  "$json_dir" \
                  "${br_args[@]}"; then
        echo "  ERROR: BFI.x failed for target_ctau=${target_ctau}"
        return 1
    fi
    echo "  Done: target_ctau=${target_ctau}"
}

export -f process_bfi_point
export BFI_BIN SOURCE_ZRATE SOURCE_GRATE

# ---------------------------------------------------------------------------
# Build work list, optionally skipping already-done points
# ---------------------------------------------------------------------------

# Compute the BR suffix that BFI.x will append when --target-zrate/grate are set,
# so --continue can match the correct output filenames.
BR_EXTRA=""
BR_SUFFIX=""
if [[ -n "$TARGET_ZRATE" || -n "$TARGET_GRATE" ]]; then
    zpct=$(awk "BEGIN { printf \"%d\", ${TARGET_ZRATE:-0} * 100 }")
    gpct=$(awk "BEGIN { printf \"%d\", ${TARGET_GRATE:-0} * 100 }")
    BR_EXTRA="Z${zpct}G${gpct}"
    BR_SUFFIX="_${BR_EXTRA}"
fi

todo_src=()
todo_tgt=()
n_done=0

for tgt in "${TARGET_CTAUS[@]}"; do
    tag=$(ctau_to_tag "$tgt")

    if $CONTINUE_MODE; then
        if compgen -G "${JSON_DIR}/*_ctau${tag}${BR_SUFFIX}.json" > /dev/null 2>&1; then
            echo "  Already done: target_ctau=${tgt} — skipping"
            (( n_done++ )) || true
            continue
        fi
    fi

    todo_src+=("$SOURCE_CTAU")
    todo_tgt+=("$tgt")
done

n_total=${#todo_tgt[@]}
if [[ $n_total -eq 0 && $n_done -eq 0 ]]; then
    echo "No target ctau values to process."
    exit 1
fi

if $CONTINUE_MODE; then
    echo "Continue mode: $n_done already complete, running $n_total remaining"
else
    echo "Running BFI for $n_total ctau point(s) — up to $NWORKERS workers, ${ROOT_THREADS} ROOT thread(s) each"
fi
echo "Config:  $BASE_CONFIG"
echo "Output:  $JSON_DIR/"
echo ""

# ---------------------------------------------------------------------------
# Parallel worker pool (bash 3.x compatible, no wait -n)
# ---------------------------------------------------------------------------
pids=()
pid_tgts=()
failed=()
n_ok=0

for (( i=0; i<n_total; i++ )); do
    process_bfi_point "${todo_src[$i]}" "${todo_tgt[$i]}" "$JSON_DIR" "$BASE_CONFIG" "$SOURCE_ZRATE" "$SOURCE_GRATE" "$TARGET_ZRATE" "$TARGET_GRATE" &
    pids+=($!)
    pid_tgts+=("${todo_tgt[$i]}")

    if (( ${#pids[@]} >= NWORKERS )); then
        if wait "${pids[0]}"; then (( n_ok++ )) || true
        else failed+=("${pid_tgts[0]}"); fi
        pids=("${pids[@]:1}")
        pid_tgts=("${pid_tgts[@]:1}")
    fi
done

for (( i=0; i<${#pids[@]}; i++ )); do
    if wait "${pids[$i]}"; then (( n_ok++ )) || true
    else failed+=("${pid_tgts[$i]}"); fi
done

echo ""
echo "Done. $n_ok / $n_total succeeded, $n_done already complete."
if [[ ${#failed[@]} -gt 0 ]]; then
    echo "Failed ctau points:"
    for tgt in "${failed[@]}"; do
        echo "  ctau=${tgt}"
    done
fi
echo "JSON files written to '$JSON_DIR/'."
echo ""
echo "Next steps — choose one of the two approaches below:"
echo ""
echo "  ── Batch (recommended) ──────────────────────────────────────────────────"
echo "  bash macro/run_ctau_scan_bf.sh \\"
echo "      --source-ctau ${SOURCE_CTAU} \\"
echo "      --json-dir ${JSON_DIR} \\"
[[ -n "$TARGET_ZRATE" ]] && echo "      --target-zrate ${TARGET_ZRATE} \\"
[[ -n "$TARGET_GRATE" ]] && echo "      --target-grate ${TARGET_GRATE} \\"
echo "      <FitConfig.yaml> \\"
echo "      ${TARGET_CTAUS[*]}"
echo "  (handles BF.x + rename + combineTool.py for all ctau points in parallel,"
echo "   then consolidates into a single datacards_*_ctauScan/ directory)"
echo ""
echo "  ── Individual ───────────────────────────────────────────────────────────"
echo "  for tgt in ${TARGET_CTAUS[*]}; do"
echo "    tag=\$(echo \$tgt | awk '{ if (\$1==int(\$1)) printf \"%d\",int(\$1); else { gsub(/\./,\"p\"); print } }')"
echo "    ./BF.x --config <FitConfig.yaml> --BFI ${JSON_DIR}/*_ctau\${tag}${BR_SUFFIX}.json --extra ctau\${tag}"
echo "    python3 macro/rename_ctau_datacards.py \\"
echo "        --dc-dir datacards_<fitname>_ctau\${tag} \\"
echo "        --source-ctau ${SOURCE_CTAU} --target-ctau \${tgt} --apply"
echo "    pushd datacards_<fitname>_ctau\${tag}/<signal_dir>/"
echo "    combineTool.py -M T2W -i *.txt -o ws.root --parallel 1"
echo "    combineTool.py -M AsymptoticLimits -t -1 -d ws.root --parallel 1"
echo "    popd"
echo "  done"
echo ""
echo "  ── Collect and plot (after either approach) ─────────────────────────────"
echo "  python3 macro/RunAndCollectLimits.py -d datacards_<fitname>_ctauScan/ -s gogoGZ"
echo "  python3 macro/PlotLimits.py -i gogoGZ_limits.json --lumi <lumi>"
