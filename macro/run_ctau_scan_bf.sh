#!/usr/bin/env bash
# run_ctau_scan_bf.sh
# --------------------
# Run BF.x + combineTool.py for a ctau scan, then rename the signal
# subdirectory inside each datacard directory so RunAndCollectLimits.py
# reads the correct target ctau from the directory name.
#
# BF.x is called with --extra ctau<N> so each ctau point writes to its own
# top-level datacard directory (datacards_<fitname>_ctau<N>/) with no risk
# of overwriting between scan points.
#
# After BF.x, the signal subdirectory inside still carries the source ctau
# from the MC filename (e.g. gogoGZ_2300_1300_1000_50).  rename_ctau_datacards.py
# renames it to the target ctau (e.g. gogoGZ_2300_1300_1000_30) so that
# RunAndCollectLimits.py records limits under the correct ctau value.
#
# Must be run on LPC el9 inside the CMSSW + Combine environment (cmsenv).
#
# Usage:
#   bash macro/run_ctau_scan_bf.sh [options] FIT_CONFIG TARGET_CTAU [TARGET_CTAU ...]
#
# Options:
#   --continue        Skip any ctau point whose Combine result already exists
#   --workers N       Parallel worker count (default: 4)
#   --json-dir DIR    Directory containing BFI output JSONs (default: json/)
#   --method M        Combine method: AsymptoticLimits or Significance (default: AsymptoticLimits)
#   --source-ctau N   Source ctau passed explicitly to rename_ctau_datacards.py.
#                     Required when the BFI JSON key already ends in the target ctau
#                     (e.g. native-sample scans where source == target, or where the
#                     ct0p1 sample was used as source for target=10).  Without this,
#                     auto-detection fails and Combine is never run for those points.
#
# Examples:
#   bash macro/run_ctau_scan_bf.sh --source-ctau 50 \
#       config_master/BigGuy_NonCompressed_FullRegions_FitConfig.yaml \
#       5 10 20 30 75 100
#
#   bash macro/run_ctau_scan_bf.sh --continue --workers 8 --source-ctau 50 \
#       config_master/BigGuy_NonCompressed_FullRegions_FitConfig.yaml \
#       10 30 100
#
# Run from the LLPCombine root directory (where BF.x lives).

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
METHOD="AsymptoticLimits"
SOURCE_CTAU=""
TARGET_ZRATE=""
TARGET_GRATE=""
POSITIONAL=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --continue)       CONTINUE_MODE=true; shift ;;
        --workers)        NWORKERS="$2"; shift 2 ;;
        --json-dir)       JSON_DIR="$2"; shift 2 ;;
        --method)         METHOD="$2"; shift 2 ;;
        --source-ctau)    SOURCE_CTAU="$2"; shift 2 ;;
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

if [[ ${#POSITIONAL[@]} -lt 2 ]]; then
    echo "Usage: $0 [--continue] [--workers N] [--json-dir DIR] [--method M] [--source-ctau N] [--target-zrate VAL] [--target-grate VAL] FIT_CONFIG TARGET_CTAU [TARGET_CTAU ...]"
    exit 1
fi

FIT_CONFIG="${POSITIONAL[0]}"
TARGET_CTAUS=("${POSITIONAL[@]:1}")
BF_BIN="./BF.x"
RENAME_SCRIPT="macro/rename_ctau_datacards.py"
RESULT_FILE="higgsCombine.Test.${METHOD}.mH120.root"

if [[ ! -f "$FIT_CONFIG" ]]; then
    echo "Error: fit config not found: $FIT_CONFIG"
    exit 1
fi

# Extract fitname from the YAML so consolidation only touches directories from
# this config, not leftover dirs from earlier runs with a different config.
FITNAME=$(grep -m1 '^fitname:' "$FIT_CONFIG" | sed "s/fitname:[[:space:]]*[\"']//;s/[\"'].*//"  | tr -d '[:space:]')
if [[ -z "$FITNAME" ]]; then
    echo "Error: could not extract 'fitname' from $FIT_CONFIG"
    exit 1
fi
if [[ ! -x "$BF_BIN" ]]; then
    echo "Error: BF.x not found or not executable."
    exit 1
fi
if [[ ! -f "$RENAME_SCRIPT" ]]; then
    echo "Error: rename script not found: $RENAME_SCRIPT"
    exit 1
fi

# ---------------------------------------------------------------------------
# Worker function — BF.x + rename + combineTool.py for one ctau point.
# The --extra ctau<N> flag gives each ctau its own top-level datacard
# directory, so scan points never overwrite each other.
# ---------------------------------------------------------------------------
process_bf_point() {
    local target_ctau="$1"
    local json_dir="$2"
    local fit_cfg="$3"
    local method="$4"
    local result_file="$5"
    local source_ctau="$6"

    local tag
    tag=$(ctau_to_tag "$target_ctau")
    local extra="${BR_EXTRA:+${BR_EXTRA}_}ctau${tag}"

    # Locate the BFI JSON for this ctau point.
    local json_file
    json_file=$(compgen -G "${json_dir}/*_ctau${tag}${BR_SUFFIX}.json" 2>/dev/null | head -1 || true)
    if [[ -z "$json_file" ]]; then
        echo "  ERROR: no BFI JSON found for ctau=${target_ctau} in '${json_dir}/'"
        echo "         (expected a file matching *_ctau${tag}${BR_SUFFIX}.json)"
        return 1
    fi

    echo "=== BF: target_ctau=${target_ctau}  json=${json_file} ==="

    if ! $BF_BIN --config "$fit_cfg" --BFI "$json_file" --extra "$extra"; then
        echo "  ERROR: BF.x failed for ctau=${target_ctau}"
        return 1
    fi

    # Identify the top-level datacard directory BF.x just created.
    # It follows the pattern: datacards_<fitname>_<extra>
    local dc_dir
    dc_dir=$(compgen -G "datacards_${FITNAME}_${extra}" 2>/dev/null | head -1 || true)
    if [[ -z "$dc_dir" ]]; then
        echo "  ERROR: could not find datacard directory matching 'datacards_${FITNAME}_${extra}'"
        return 1
    fi

    # Rename signal subdirectory: gogoGZ_..._<source_ctau> -> gogoGZ_..._<target_ctau>
    # When source_ctau == target_ctau (native-sample point) the rename script exits 0
    # with "nothing to do", so Combine still runs correctly for those points.
    echo "  Renaming signal subdirs to target_ctau=${target_ctau} in ${dc_dir}/"
    local rename_args=(--dc-dir "$dc_dir" --target-ctau "$target_ctau" --apply)
    [[ -n "$source_ctau" ]] && rename_args+=(--source-ctau "$source_ctau")
    if ! python3 "$RENAME_SCRIPT" "${rename_args[@]}"; then
        echo "  ERROR: rename failed for ctau=${target_ctau}"
        return 1
    fi

    # Run combineTool.py inside each (now-renamed) signal subdirectory.
    pushd "$dc_dir" > /dev/null
    for sig_dir in */; do
        sig_dir="${sig_dir%/}"
        [[ ! -d "$sig_dir" ]] && continue
        echo "  Combine (${method}): ${dc_dir}/${sig_dir}"
        pushd "$sig_dir" > /dev/null

        combineTool.py -M T2W -i *.txt -o ws.root --parallel 1 2>/dev/null

        if [[ "$method" == "Significance" ]]; then
            combineTool.py -M Significance -t -1 --expectSignal=1 \
                -d ws.root --parallel 1 2>/dev/null
        else
            combineTool.py -M AsymptoticLimits -t -1 \
                -d ws.root --parallel 1 2>/dev/null
        fi

        popd > /dev/null
    done
    popd > /dev/null

    echo "  Done: ctau=${target_ctau} -> ${dc_dir}/"
}

export -f process_bf_point
export BF_BIN RENAME_SCRIPT SOURCE_CTAU FITNAME

BR_EXTRA=""
BR_SUFFIX=""
if [[ -n "$TARGET_ZRATE" || -n "$TARGET_GRATE" ]]; then
    zpct=$(awk "BEGIN { printf \"%d\", ${TARGET_ZRATE:-0} * 100 }")
    gpct=$(awk "BEGIN { printf \"%d\", ${TARGET_GRATE:-0} * 100 }")
    BR_EXTRA="Z${zpct}G${gpct}"
    BR_SUFFIX="_${BR_EXTRA}"
fi
export BR_EXTRA BR_SUFFIX

# ---------------------------------------------------------------------------
# Build work list, optionally skipping already-done points
# ---------------------------------------------------------------------------
todo_tgt=()
n_done=0

for tgt in "${TARGET_CTAUS[@]}"; do
    tag=$(ctau_to_tag "$tgt")
    if $CONTINUE_MODE; then
        if compgen -G "datacards_${FITNAME}_${BR_EXTRA:+${BR_EXTRA}_}ctau${tag}/*/${RESULT_FILE}" > /dev/null 2>&1; then
            echo "  Already done: ctau=${tgt} — skipping"
            (( n_done++ )) || true
            continue
        fi
    fi
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
    echo "Running BF + Combine for $n_total ctau point(s) — up to $NWORKERS parallel workers [$METHOD]"
fi
echo "Fit config: $FIT_CONFIG"
echo "JSON dir:   $JSON_DIR/"
echo ""

# ---------------------------------------------------------------------------
# Parallel worker pool (bash 3.x compatible)
# ---------------------------------------------------------------------------
pids=()
n_ok=0

for (( i=0; i<n_total; i++ )); do
    process_bf_point "${todo_tgt[$i]}" "$JSON_DIR" \
                     "$FIT_CONFIG" "$METHOD" "$RESULT_FILE" "$SOURCE_CTAU" &
    pids+=($!)

    if (( ${#pids[@]} >= NWORKERS )); then
        if wait "${pids[0]}"; then (( n_ok++ )) || true; fi
        pids=("${pids[@]:1}")
    fi
done

for pid in "${pids[@]}"; do
    if wait "$pid"; then (( n_ok++ )) || true; fi
done

echo ""
echo "Done. $n_ok / $n_total succeeded, $n_done already complete."

# ---------------------------------------------------------------------------
# Consolidate signal subdirs from all per-ctau directories into one combined
# directory so RunAndCollectLimits.py can be called once on it as normal.
# ---------------------------------------------------------------------------
# Derive the base fitname by stripping _ctau<tag> from the first matching dir.
combined_dir=""
for d in datacards_${FITNAME}_${BR_EXTRA:+${BR_EXTRA}_}ctau*/; do
    [[ -d "$d" ]] || continue
    [[ "$d" == *ctauScan* ]] && continue
    base="${d%_ctau*/}"          # strip from last _ctau onwards
    base="${base%/}"
    combined_dir="${base}_ctauScan"
    break
done

if [[ -n "$combined_dir" ]]; then
    mkdir -p "$combined_dir"
    n_moved=0
    for ctau_dir in datacards_${FITNAME}_${BR_EXTRA:+${BR_EXTRA}_}ctau*/; do
        [[ -d "$ctau_dir" ]] || continue
        [[ "$ctau_dir" == *ctauScan* ]] && continue
        for sig_dir in "$ctau_dir"*/; do
            [[ -d "$sig_dir" ]] || continue
            sig_name=$(basename "$sig_dir")
            dest="$combined_dir/$sig_name"
            [[ -d "$dest" ]] && rm -rf "$dest"
            mv "$sig_dir" "$combined_dir/"
            (( n_moved++ )) || true
        done
    done
    echo ""
    echo "Consolidated $n_moved signal dir(s) into '$combined_dir/'."
    echo ""
    echo "Next step:"
    if [[ -n "$BR_EXTRA" ]]; then
        echo "  python3 macro/RunAndCollectLimits.py -d ${combined_dir}/ -s gogoGZ --extra ${BR_EXTRA}"
        echo "  python3 macro/PlotLimits.py -i gogoGZ_limits_${BR_EXTRA}.json --lumi <lumi>"
    else
        echo "  python3 macro/RunAndCollectLimits.py -d ${combined_dir}/ -s gogoGZ"
        echo "  python3 macro/PlotLimits.py -i gogoGZ_limits.json --lumi <lumi>"
    fi
else
    echo ""
    echo "Next step:"
    if [[ -n "$BR_EXTRA" ]]; then
        echo "  python3 macro/RunAndCollectLimits.py -d datacards_<fitname>/ -s gogoGZ --extra ${BR_EXTRA}"
        echo "  python3 macro/PlotLimits.py -i gogoGZ_limits_${BR_EXTRA}.json --lumi <lumi>"
    else
        echo "  python3 macro/RunAndCollectLimits.py -d datacards_<fitname>/ -s gogoGZ"
        echo "  python3 macro/PlotLimits.py -i gogoGZ_limits.json --lumi <lumi>"
    fi
fi
