#!/usr/bin/env bash
# =============================================================================
# Reconner — External tool installer
# =============================================================================
# Installs: Go, subfinder, httpx, whatweb, gobuster, nuclei, SecLists
#
# Usage:
#   chmod +x install-tools.sh
#   ./install-tools.sh
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Color helpers
# ---------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()    { echo -e "${BLUE}[INFO]${NC}  $1"; }
success() { echo -e "${GREEN}[OK]${NC}    $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; }

echo ""
echo "=============================================="
echo "  Reconner — Tool Installer"
echo "=============================================="
echo ""

# ---------------------------------------------------------------------------
# 1. Go
# ---------------------------------------------------------------------------
install_go() {
    if command -v go &>/dev/null; then
        success "Go already installed: $(go version)"
        return
    fi

    info "Installing Go via apt..."
    sudo apt-get update -qq
    sudo apt-get install -y golang-go

    # Persist GOPATH/GOBIN in .bashrc if not already set
    if ! grep -q 'export GOPATH' ~/.bashrc; then
        {
            echo 'export GOPATH=$HOME/go'
            echo 'export GOBIN=$HOME/go/bin'
            echo 'export PATH=$PATH:$HOME/go/bin'
        } >> ~/.bashrc
    fi

    export GOPATH="$HOME/go"
    export GOBIN="$HOME/go/bin"
    export PATH="$PATH:$HOME/go/bin"

    success "Go installed: $(go version)"
}

# ---------------------------------------------------------------------------
# 2. Go-based tools (subfinder, httpx, gobuster, nuclei)
# ---------------------------------------------------------------------------
install_go_tool() {
    local name="$1"
    local pkg="$2"
    local bin="$3"

    if command -v "$name" &>/dev/null; then
        success "$name already installed: $(which "$name")"
        return
    fi

    info "Installing $name..."
    go install -v "$pkg"

    local bin_path="$HOME/go/bin/$bin"
    if [ -f "$bin_path" ]; then
        sudo mv "$bin_path" /usr/local/bin/
        success "$name installed at /usr/local/bin/$bin"
    else
        warn "$name binary not found in ~/go/bin — check your GOBIN setting"
    fi
}

# ---------------------------------------------------------------------------
# 3. WhatWeb
# ---------------------------------------------------------------------------
install_whatweb() {
    if command -v whatweb &>/dev/null; then
        success "whatweb already installed: $(which whatweb)"
        return
    fi

    info "Installing whatweb via apt..."
    sudo apt-get install -y whatweb
    success "whatweb installed"
}

# ---------------------------------------------------------------------------
# 4. SecLists
# ---------------------------------------------------------------------------
install_seclists() {
    if [ -d "/usr/share/seclists" ]; then
        success "SecLists already installed at /usr/share/seclists"
        return
    fi

    info "Attempting to install SecLists via apt..."
    if sudo apt-get install -y seclists 2>/dev/null; then
        success "SecLists installed via apt"
        return
    fi

    warn "SecLists not available via apt. Cloning from GitHub (~1 GB)..."
    sudo git clone --depth 1 https://github.com/danielmiessler/SecLists.git /usr/share/seclists
    success "SecLists installed at /usr/share/seclists"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

# Ensure GOPATH is set in the current shell session
export GOPATH="${GOPATH:-$HOME/go}"
export GOBIN="${GOBIN:-$HOME/go/bin}"
export PATH="$PATH:$GOBIN"

install_go

# Reload Go binary path in case it was just installed
export PATH="$PATH:$(go env GOPATH 2>/dev/null)/bin" || true

install_go_tool "subfinder" \
    "github.com/projectdiscovery/subfinder/v2/cmd/subfinder@latest" \
    "subfinder"

install_go_tool "httpx" \
    "github.com/projectdiscovery/httpx/cmd/httpx@latest" \
    "httpx"

install_go_tool "gobuster" \
    "github.com/OJ/gobuster/v3@latest" \
    "gobuster"

install_go_tool "nuclei" \
    "github.com/projectdiscovery/nuclei/v3/cmd/nuclei@latest" \
    "nuclei"

install_whatweb
install_seclists

# ---------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------
echo ""
echo "=============================================="
echo "  Verification"
echo "=============================================="

all_ok=true
for tool in subfinder httpx whatweb gobuster nuclei; do
    if command -v "$tool" &>/dev/null; then
        success "$tool → $(which "$tool")"
    else
        error "$tool → NOT FOUND"
        all_ok=false
    fi
done

if [ -d "/usr/share/seclists" ]; then
    success "SecLists → /usr/share/seclists"
else
    error "SecLists → NOT FOUND"
    all_ok=false
fi

echo ""
if $all_ok; then
    echo "All tools installed successfully."
    echo "Run: reconner --target example.com --output-dir ./results"
else
    echo "Some tools could not be installed. Check the errors above."
fi
echo ""
