#!/usr/bin/env bash
# ============================================================
# java-cli Uninstaller
# Removes global link and node_modules
# ============================================================

set -e

echo "======================================"
echo "java-cli Uninstaller"
echo "======================================"
echo ""

cd "$(dirname "$0")"

echo "This will:"
echo "  1. Remove the global \"java-cli\" command link"
echo "  2. Remove node_modules directory"
echo ""
echo "Your SDK installations at ~/.java-cli/ will NOT be deleted."
echo ""

read -p "Continue? (y/N): " CONFIRM
if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 0
fi

echo ""
echo "Step 1: Removing global link..."
npm unlink -g 2>/dev/null || true
echo "[OK] Global link removed."

echo ""
echo "Step 2: Removing node_modules..."
rm -rf node_modules
echo "[OK] node_modules removed."

echo ""
echo "======================================"
echo "Uninstall complete!"
echo "======================================"
echo ""
echo "To restore, run: ./setup_build.sh"
echo ""
