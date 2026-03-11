# LLPCombine Configuration Examples

This directory contains example configuration files for the LLPCombine analysis framework.

## Configuration Files

### `example_analysis.yaml`
Basic configuration that reproduces the original hardcoded analysis setup. Good starting point for most analyses.

### `minimal_example.yaml`
Simplest possible configuration with just one analysis bin. Useful for testing and learning the configuration format.

### `comprehensive_analysis.yaml`
Full-featured example showing:
- Multiple signal and control regions
- YAML anchors and references for code reuse
- Comprehensive systematic uncertainties
- Advanced configuration options

## Configuration Schema

### Required Sections

#### `analysis`
- `name`: Analysis name (string)
- `luminosity`: Integrated luminosity in fb^-1 (number)
- `output_json`: Output JSON filename (string)
- `output_dir`: Output directory path (string, optional, default: "./json/")

#### `samples`
- `backgrounds`: List of background sample names (list of strings)
- `signals`: List of signal sample names (list of strings)

#### `bins`
Each bin requires:
- `description`: Human-readable description (string)
- `cuts`: List of cut strings that will be combined with AND logic (list of strings)

### Optional Sections

#### `systematics`
- `experimental`: List of experimental systematic uncertainties
- `theoretical`: List of theoretical systematic uncertainties

Each systematic has:
- `name`: Systematic name (string)
- `type`: Type (usually "lnN" for log-normal)
- `value`: Uncertainty value (number, e.g., 1.05 for 5% uncertainty)
- `processes`: Which processes to apply to (list, or "all")

#### `options`
- `verbosity`: Output verbosity level 0-3 (number, default: 1)
- `parallel`: Enable parallel processing (boolean, default: false)
- `dry_run`: Validate config without running (boolean, default: false)

## Usage Examples

```bash
# Run with basic configuration
./BFI.x config/example_analysis.yaml

# Run with command-line overrides
./BFI.x --config config/example_analysis.yaml --luminosity 500 --verbosity 2

# Dry run to validate configuration
./BFI.x --dry-run config/comprehensive_analysis.yaml

# Get help
./BFI.x --help
```

## YAML Features Used

### Comments
Use `#` for comments, both full-line and inline.

### Anchors and References
Use `&anchor_name` to define reusable content and `*anchor_name` to reference it:

```yaml
common_cuts: &photon_base
  - "nSelPhotons >= 1"
  - "photon_pt > 200"

bins:
  region1:
    cuts:
      - *photon_base
      - "additional_cut > 100"
```

### Lists
Two formats supported:
```yaml
# Bracket format
samples: ["Wjets", "Zjets", "Gjets"]

# Dash format  
samples:
  - "Wjets"
  - "Zjets"
  - "Gjets"
```

## Tips

1. Start with `minimal_example.yaml` for testing
2. Use `--dry-run` to validate new configurations
3. Use anchors (`&` and `*`) to avoid repeating common cuts
4. Keep descriptions meaningful for documentation
5. Test with increasing verbosity levels to debug issues