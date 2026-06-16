#!/usr/bin/env bash
# =============================================================================
# Reconner — Example run script
#
# Demonstrates common usage patterns.
# WARNING: Only scan targets you own or have explicit written permission to test.
# =============================================================================

set -euo pipefail

OUTPUT_DIR="./example-results"
TARGET="example.com"   # <-- Replace with an authorized target

echo "=========================================="
echo "  Reconner — Example Run"
echo "=========================================="
echo ""
echo "Target : $TARGET"
echo "Output : $OUTPUT_DIR"
echo ""

# Clean previous example results (optional)
if [ -d "$OUTPUT_DIR" ]; then
    echo "Removing previous results..."
    rm -rf "$OUTPUT_DIR"
fi

# ---------------------------------------------------------------------------
# Example 1: Basic scan
# ---------------------------------------------------------------------------
echo "--- Example 1: Basic Scan ---"
python -m reconner \
    --target "$TARGET" \
    --output-dir "$OUTPUT_DIR" \
    --threads 20 \
    || echo "Note: scan will fail if tools are not installed or target is not authorized."

echo ""
echo "=========================================="
echo "  Scan complete"
echo "=========================================="
echo ""
echo "Results saved to: $OUTPUT_DIR"
echo "  summary.json   — structured scan data"
echo "  report.md      — Markdown report"
echo "  report.pdf     — PDF report"
echo "  highlights.txt — executive summary"
echo "  raw/           — raw tool outputs"
echo ""

# ---------------------------------------------------------------------------
# Uncomment the examples below to try other modes
# ---------------------------------------------------------------------------

# Example 2: Fast mode (small wordlists, skip nuclei)
# python -m reconner \
#     --target "$TARGET" \
#     --output-dir "${OUTPUT_DIR}-fast" \
#     --fast \
#     --threads 10

# Example 3: Stealth mode (slow, low-noise)
# python -m reconner \
#     --target "$TARGET" \
#     --output-dir "${OUTPUT_DIR}-stealth" \
#     --stealth \
#     --threads 5

# Example 4: With Burp Suite proxy
# python -m reconner \
#     --target "$TARGET" \
#     --output-dir "${OUTPUT_DIR}-proxy" \
#     --proxy "http://127.0.0.1:8080" \
#     --threads 20

# Example 5: Bulk targets from file
# cat > targets.txt <<EOF
# example.com
# test.example.com
# demo.example.com
# EOF
# python -m reconner \
#     --input-file targets.txt \
#     --output-dir "${OUTPUT_DIR}-multi" \
#     --threads 20

# Example 6: Regenerate reports from cached data
# python -m reconner \
#     --export-only \
#     --output-dir "$OUTPUT_DIR"
