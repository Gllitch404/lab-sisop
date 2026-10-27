#!/bin/sh

mkdir -p $BASE_DIR/target/etc/init.d
if [ -f "$BASE_DIR/../custom-scripts/S41network-config" ]; then
    cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
    chmod +x $BASE_DIR/target/etc/init.d/S41network-config
fi

# Tutorial 3.1: Montagem do tracefs em /sys/kernel/tracing no /etc/fstab
mkdir -p "$BASE_DIR/target/sys/kernel/tracing" 2>/dev/null || true
if [ -f "$BASE_DIR/target/etc/fstab" ]; then
    if ! grep -q "tracefs /sys/kernel/tracing tracefs" "$BASE_DIR/target/etc/fstab"; then
        echo "tracefs /sys/kernel/tracing tracefs 0 0" >> "$BASE_DIR/target/etc/fstab"
    fi
fi

# Cria symlink buildroot/target -> output/target para compatibilidade com INSTALL_MOD_PATH=../../target
if [ ! -e "$BASE_DIR/../target" ]; then
    ln -sf output/target "$BASE_DIR/../target" 2>/dev/null || true
fi

# Se os fontes do kernel ja estiverem prontos, compila os modulos
if [ -d "$BASE_DIR/build/linux-custom" ]; then
    make -C $BASE_DIR/../modules/hello/ || true
    make -C $BASE_DIR/../modules/simple_driver/ || true
    make -C $BASE_DIR/../modules/xtea_driver/ || true
fi
