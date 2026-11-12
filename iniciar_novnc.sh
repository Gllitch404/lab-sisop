#!/bin/bash
set -e

echo "=== Limpando processos anteriores ==="
pkill -9 -f "Xvfb :99" 2>/dev/null || true
pkill -9 -f "x11vnc.*:99" 2>/dev/null || true
pkill -9 -f "websockify.*6080" 2>/dev/null || true
pkill -9 -f "kernelshark" 2>/dev/null || true
rm -f /tmp/.X99-lock /tmp/.X11-unix/X99
sleep 1

echo "=== Iniciando Xvfb (display :99) ==="
setsid Xvfb :99 -screen 0 1600x900x24 -ac +extension GLX +render -noreset >/tmp/xvfb.log 2>&1 &
sleep 2

echo "=== Iniciando x11vnc ==="
x11vnc -display :99 -rfbport 5999 -nopw -forever -bg -quiet >/tmp/x11vnc.log 2>&1
sleep 1

echo "=== Iniciando websockify (noVNC na porta 6080) ==="
websockify -D --web /usr/share/novnc/ 6080 localhost:5999 >/tmp/websockify.log 2>&1
sleep 1

echo "=== Iniciando KernelShark ==="
export DISPLAY=:99
TRACE_FILE="${1:-/workspaces/lab-sisop/trace_rr.dat}"
if [ -f "$TRACE_FILE" ]; then
    echo "Carregando trace: $TRACE_FILE"
    setsid kernelshark "$TRACE_FILE" >/tmp/kernelshark.log 2>&1 &
else
    setsid kernelshark >/tmp/kernelshark.log 2>&1 &
fi
sleep 1

echo ""
echo "=== Verificando portas ativas ==="
ss -tlnp | grep -E "6080|5999" || true
echo ""
echo "Pronto! Acesse http://localhost:6080/vnc.html no navegador."
