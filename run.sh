#!/usr/bin/env bash
# Quick QEMU launcher for CoreKernel (no ISO required)
set -euo pipefail

KERNEL="corekernel.elf"

if [[ ! -f "$KERNEL" ]]; then
  echo "Kernel not built. Run: make"
  exit 1
fi

echo "Booting CoreKernel in QEMU..."
echo "  Serial output: this terminal"
echo "  Press Ctrl-A X to exit QEMU"
echo ""

qemu-system-i386 \
  -kernel "$KERNEL"      \
  -serial stdio          \
  -m 128M                \
  -display sdl           \
  -no-reboot             \
  "$@"
