#!/usr/bin/env bash
# ============================================================
# java-cli Build Script
# Installs dependencies and links the CLI globally
# ============================================================

set -e

echo "======================================"
echo "Building java-cli"
echo "======================================"
echo ""

# Navigate to script directory
cd "$(dirname "$0")"

# Install dependencies
echo "Installing npm dependencies..."
npm install
echo "[OK] Dependencies installed."

# Link globally
echo ""
echo "Linking CLI globally..."
npm link
echo "[OK] CLI linked globally."

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo ""
echo 'The "java-cli" command is now available globally.'
echo ""
echo "Quick start:"
echo "  java-cli init my-app"
echo "  cd my-app"
echo "  java-cli build"
echo "  java-cli run"
echo ""
