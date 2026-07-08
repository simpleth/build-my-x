#!/usr/bin/env bash
# ============================================================
# java-cli Dependency Installer
# Checks and installs Node.js if missing
# ============================================================

set -e

echo "======================================"
echo "java-cli Dependency Installer"
echo "======================================"
echo ""

# Check Node.js
echo "Checking Node.js..."
if command -v node &> /dev/null; then
    NODE_VER=$(node --version)
    echo "[OK] Node.js is installed: $NODE_VER"
else
    echo "[INFO] Node.js not found."
    echo ""
    echo "Please install Node.js from: https://nodejs.org/"
    echo "Or use a package manager:"
    echo "  Ubuntu/Debian: sudo apt-get install nodejs npm"
    echo "  macOS:         brew install node"
    echo "  Fedora:        sudo dnf install nodejs"
    echo ""
    exit 1
fi

# Check npm
echo "Checking npm..."
if command -v npm &> /dev/null; then
    NPM_VER=$(npm --version)
    echo "[OK] npm is installed: $NPM_VER"
else
    echo "[ERROR] npm not found. Please reinstall Node.js."
    exit 1
fi

echo ""
echo "======================================"
echo "All dependencies are installed!"
echo "======================================"
echo ""
echo "Run ./setup_build.sh to install the CLI."
echo ""
