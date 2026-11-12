#!/bin/bash
echo "=== Parando servicos noVNC e KernelShark ==="
pkill -9 -f "Xvfb :99" 2>/dev/null || true
pkill -9 -f "x11vnc.*:99" 2>/dev/null || true
pkill -9 -f "websockify.*6080" 2>/dev/null || true
pkill -9 -f "kernelshark" 2>/dev/null || true
rm -f /tmp/.X99-lock /tmp/.X11-unix/X99
echo "✓ Todos os servicos foram finalizados."
