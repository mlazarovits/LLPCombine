import os
import sys
import argparse
import ROOT as rt
from pathlib import Path

# Get the directory where this script is located
SCRIPT_DIR = Path(__file__).parent.absolute()
PROJECT_ROOT = SCRIPT_DIR.parent  # Assuming script is in macro/ subdirectory
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "output"

def print_help():
    """Print usage help"""
    print("CollectSignificance.py - Extract significance values from CMS Combine results")
    print()
    print("Usage: python CollectSignificance.py [OPTIONS]")
    print()
    print("Options:")
    print("  -d, --datacard-dir DIR    Directory containing datacard subdirectories (default: ../datacards_eos)")
    print("  -o, --output FILE         Output file for significance results (default: auto-generated)")
    print("  -f, --format FORMAT       Output format: txt, csv, json (default: txt)")
    print("  -s, --sort                Sort results by significance value")
    print("  -v, --verbose             Enable verbose output")
    print("  -q, --quiet               Suppress all output except errors")
    print("  --filter-min VALUE        Only include results with significance >= VALUE")
    print("  --filter-max VALUE        Only include results with significance <= VALUE")
    print("  --root-file PATTERN       ROOT file pattern (default: higgsCombine.Test.Significance.mH120.root)")
    print("  -h, --help                Show this help message and exit")
    print()
    print("Examples:")
    print("  python CollectSignificance.py -d datacards -o results.txt")
    print("  python CollectSignificance.py --datacard-dir datacards_run2 --format csv --sort")
    print("  python CollectSignificance.py -d datacards --filter-min 2.0 --verbose")
    print("  python CollectSignificance.py -q -f json -o results.json")

def get_directories(path, verbose=False):
    """
    Returns a list of directory names within the specified path.
    """
    directories = []
    try:
        if verbose:
            print(f"Scanning directory: {path}")
        
        # Get all entries in the directory
        if(len(sigs) == 0):
            all_entries = os.listdir(path)
        else:
            all_entries = sigs
        # Iterate through entries and check if they are directories
        for entry in all_entries:
            full_path = os.path.join(path, entry)
            if os.path.isdir(full_path):
                directories.append(entry)
                if verbose:
                    print(f"  Found subdirectory: {entry}")
                    
    except FileNotFoundError:
        print(f"Error: The path '{path}' was not found.", file=sys.stderr)
        return []
    except Exception as e:
        print(f"An error occurred while reading directory: {e}", file=sys.stderr)
        return []
        
    if verbose:
        print(f"Found {len(directories)} subdirectories")
        
    return directories

#datacard_dir = "datacards"
#datacard_dir = "../datacards_22j"
#datacard_dir = "../datacards_11j"
#datacard_dir = "../datacards_2GLLL"
#datacard_dir = "../datacards_eos"
datacard_dir = "../datacards_sq"
datacard_subdir_list = get_directories(datacard_dir)

args = parser.parse_args()

datacard_subdir_list = get_directories(args.directory, args.sigs)

#print(datacard_subdir_list)
match = "datacards_"
endidx = -1
if(args.directory.find("/") != -1):
    endidx = args.directory.find("/")
else:
    endidx = len(args.directory)
oname = args.directory[args.directory.find(match)+len(match):endidx]
ofile = "output/Significances_"+oname+".txt"

with open("../output/Significance_sq.txt", "w") as file:
    sig=-1;
    for subdir in datacard_subdir_list:
        f = rt.TFile.Open(datacard_dir+"/"+subdir+"/higgsCombine.Test.Significance.mH120.root")
        tree = f.Get("limit")
        if not tree:
            if verbose:
                print(f"  Error: 'limit' tree not found in {root_file_path}")
            f.Close()
            return None
        
        sig = -1
        for entry in tree:
            sig = entry.limit
            break  # Only need the first entry
            
        f.Close()
        
        if verbose:
            print(f"  Extracted significance: {sig}")
            
        return sig
        
    except Exception as e:
        if verbose:
            print(f"  Error reading ROOT file {root_file_path}: {e}")
        return None

def write_results(results, output_file, output_format, verbose=False):
    """Write results to file in specified format"""
    
    if verbose:
        print(f"Writing {len(results)} results to {output_file} in {output_format} format")
    
    try:
        with open(output_file, "w") as file:
            if output_format == "csv":
                file.write("Signal,Significance\n")
                for signal, sig in results:
                    file.write(f"{signal},{sig}\n")
                    
            elif output_format == "json":
                import json
                data = {signal: sig for signal, sig in results}
                json.dump(data, file, indent=2)
                
            else:  # txt format
                for signal, sig in results:
                    file.write(f"{signal} {sig}\n")
                    
        if verbose:
            print(f"Successfully wrote results to {output_file}")
            
    except Exception as e:
        print(f"Error writing to output file: {e}", file=sys.stderr)
        return False
        
    return True

def main():
    parser = argparse.ArgumentParser(description="Extract significance values from CMS Combine results", add_help=False)
    
    parser.add_argument("-d", "--datacard-dir", default=str(PROJECT_ROOT / "datacards_eos"),
                       help="Directory containing datacard subdirectories")
    parser.add_argument("-o", "--output", default=None,
                       help="Output file for significance results")
    parser.add_argument("-f", "--format", choices=["txt", "csv", "json"], default="txt",
                       help="Output format")
    parser.add_argument("-s", "--sort", action="store_true",
                       help="Sort results by significance value (descending)")
    parser.add_argument("-v", "--verbose", action="store_true",
                       help="Enable verbose output")
    parser.add_argument("-q", "--quiet", action="store_true",
                       help="Suppress all output except errors")
    parser.add_argument("--filter-min", type=float, default=None,
                       help="Only include results with significance >= VALUE")
    parser.add_argument("--filter-max", type=float, default=None,
                       help="Only include results with significance <= VALUE")
    parser.add_argument("--root-file", default="higgsCombine.Test.Significance.mH120.root",
                       help="ROOT file pattern to look for")
    parser.add_argument("-h", "--help", action="store_true",
                       help="Show help message and exit")
    
    args = parser.parse_args()
    
    if args.help:
        print_help()
        return 0
    
    # Handle quiet/verbose flags
    verbose = args.verbose and not args.quiet
    quiet = args.quiet
    
    # Generate output filename if not provided
    if args.output is None:
        datacard_basename = os.path.basename(os.path.abspath(args.datacard_dir))
        DEFAULT_OUTPUT_DIR.mkdir(exist_ok=True)  # Create output directory if it doesn't exist
        args.output = str(DEFAULT_OUTPUT_DIR / f"Significance_{datacard_basename}.{args.format}")
    
    if not quiet:
        print("=== CMS Combine Significance Collector ===")
        print(f"Datacard directory: {args.datacard_dir}")
        print(f"Output file: {args.output}")
        print(f"Output format: {args.format}")
        if args.filter_min is not None:
            print(f"Minimum significance filter: {args.filter_min}")
        if args.filter_max is not None:
            print(f"Maximum significance filter: {args.filter_max}")
        print()
    
    # Get list of signal directories
    datacard_subdir_list = get_directories(args.datacard_dir, verbose)
    
    if not datacard_subdir_list:
        print("Error: No subdirectories found in datacard directory", file=sys.stderr)
        return 1
    
    # Extract significance values
    results = []
    skipped = 0
    
    for subdir in datacard_subdir_list:
        if verbose:
            print(f"Processing signal: {subdir}")
        
        sig = extract_significance(args.datacard_dir, subdir, args.root_file, verbose)
        
        if sig is not None:
            # Apply filters
            if args.filter_min is not None and sig < args.filter_min:
                if verbose:
                    print(f"  Skipped: significance {sig} < {args.filter_min}")
                skipped += 1
                continue
                
            if args.filter_max is not None and sig > args.filter_max:
                if verbose:
                    print(f"  Skipped: significance {sig} > {args.filter_max}")
                skipped += 1
                continue
                
            results.append((subdir, sig))
        else:
            if verbose:
                print(f"  Skipped: could not extract significance")
            skipped += 1
    
    if not results:
        print("Error: No valid significance values found", file=sys.stderr)
        return 1
    
    # Sort results if requested
    if args.sort:
        results.sort(key=lambda x: x[1], reverse=True)  # Sort by significance, descending
    
    # Write results
    success = write_results(results, args.output, args.format, verbose)
    
    if not quiet:
        print(f"\nProcessed {len(datacard_subdir_list)} signal directories")
        print(f"Successfully extracted {len(results)} significance values")
        if skipped > 0:
            print(f"Skipped {skipped} entries")
        print(f"Results written to: {args.output}")
        
        if verbose and results:
            print("\nTop 5 results:")
            sorted_results = sorted(results, key=lambda x: x[1], reverse=True)
            for i, (signal, sig) in enumerate(sorted_results[:5]):
                print(f"  {i+1}. {signal}: {sig:.3f}")
    
    return 0 if success else 1

if __name__ == "__main__":
    # Suppress ROOT messages unless verbose
    if "--verbose" not in sys.argv and "-v" not in sys.argv:
        rt.gROOT.SetBatch(True)
        rt.gErrorIgnoreLevel = rt.kWarning
    
    sys.exit(main())

