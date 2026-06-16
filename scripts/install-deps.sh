#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  cmake \
  qt6-base-dev \
  qt6-tools-dev-tools \
  build-essential \
  python3 \
  python3-pip \
  curl

# Optional: secure API key storage in system keyring
# sudo apt install libsecret-tools

echo "Dependencies installed."
echo "  make install-reconner  # install reconner"
echo "  make build             # build zap-desk"
