#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/reconner"

echo ">> Instalando dependências Python do Reconner..."
python3 -m pip install --user -e ".[dev]" 2>/dev/null || python3 -m pip install --user -e .

echo ">> Reconner instalado. Teste com: python3 -m reconner --help"
