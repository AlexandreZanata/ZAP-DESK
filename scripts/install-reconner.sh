#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/reconner"

echo ">> Installing Reconner Python dependencies..."
python3 -m pip install --user -e ".[dev]" 2>/dev/null || python3 -m pip install --user -e .

echo ">> Reconner installed. Test with: python3 -m reconner --help"
