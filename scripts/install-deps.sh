#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  cmake \
  qt6-base-dev \
  qt6-tools-dev-tools \
  build-essential

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  cmake \
  qt6-base-dev \
  qt6-tools-dev-tools \
  build-essential \
  python3 \
  python3-pip \
  curl

echo "Dependências instaladas."
echo "  make install-reconner  # instalar reconner"
echo "  make build             # compilar zap-desk"
