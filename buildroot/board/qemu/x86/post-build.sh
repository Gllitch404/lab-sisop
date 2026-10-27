#!/bin/sh

set -u
set -e

# Add a console on tty1
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    grep -qE '^tty1::' ${TARGET_DIR}/etc/inittab || \
	sed -i '/GENERIC_SERIAL/a\
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # QEMU graphical window' ${TARGET_DIR}/etc/inittab
fi

BUILDROOT_DIR=$BASE_DIR/..

# Localizar o compilador cruzado apropriado (uclibc, gnu ou symlink)
if [ -x "$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-uclibc-gcc" ]; then
    COMPILER="$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-uclibc-gcc"
elif [ -x "$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-gnu-gcc" ]; then
    COMPILER="$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-gnu-gcc"
elif [ -x "$BUILDROOT_DIR/output/host/bin/i686-linux-gcc" ]; then
    COMPILER="$BUILDROOT_DIR/output/host/bin/i686-linux-gcc"
else
    COMPILER=$(find "$BUILDROOT_DIR/output/host/bin" -name "*gcc" 2>/dev/null | head -n 1)
fi

# Tutorial 2.2: Compile the syscall_test.c
if [ -n "$COMPILER" ] && [ -x "$COMPILER" ] && [ -f "$BUILDROOT_DIR/custom-scripts/syscall_test.c" ]; then
    mkdir -p $BUILDROOT_DIR/output/target/bin
    $COMPILER -o $BUILDROOT_DIR/output/target/bin/syscall_test $BUILDROOT_DIR/custom-scripts/syscall_test.c
fi

# Tutorial 2.4: Compile the disk_test.c
if [ -n "$COMPILER" ] && [ -x "$COMPILER" ] && [ -f "$BUILDROOT_DIR/disk-test/disk_test.c" ]; then
    mkdir -p $BUILDROOT_DIR/output/target/usr/bin
    mkdir -p $BUILDROOT_DIR/output/target/bin
    $COMPILER -o $BUILDROOT_DIR/output/target/usr/bin/disk_test $BUILDROOT_DIR/disk-test/disk_test.c
    ln -sf /usr/bin/disk_test $BUILDROOT_DIR/output/target/bin/disk_test 2>/dev/null || cp $BUILDROOT_DIR/output/target/usr/bin/disk_test $BUILDROOT_DIR/output/target/bin/disk_test
fi

# Tutorial 3.1 (Scheduler): Compile setpriority, deadline, thread_runner
if [ -n "$COMPILER" ] && [ -x "$COMPILER" ] && [ -d "$BUILDROOT_DIR/setpriority" ]; then
    mkdir -p $BUILDROOT_DIR/output/target/usr/bin
    mkdir -p $BUILDROOT_DIR/output/target/bin
    $COMPILER -Wall -O2 -pthread -o $BUILDROOT_DIR/output/target/usr/bin/setpriority $BUILDROOT_DIR/setpriority/setpriority.c -lpthread || true
    $COMPILER -Wall -O2 -pthread -o $BUILDROOT_DIR/output/target/usr/bin/deadline $BUILDROOT_DIR/setpriority/deadline.c -lpthread || true
    $COMPILER -Wall -O2 -pthread -o $BUILDROOT_DIR/output/target/usr/bin/thread_runner $BUILDROOT_DIR/setpriority/thread_runner.c -lpthread || true
    ln -sf /usr/bin/setpriority $BUILDROOT_DIR/output/target/bin/setpriority 2>/dev/null || cp $BUILDROOT_DIR/output/target/usr/bin/setpriority $BUILDROOT_DIR/output/target/bin/setpriority 2>/dev/null || true
    ln -sf /usr/bin/deadline $BUILDROOT_DIR/output/target/bin/deadline 2>/dev/null || cp $BUILDROOT_DIR/output/target/usr/bin/deadline $BUILDROOT_DIR/output/target/bin/deadline 2>/dev/null || true
    ln -sf /usr/bin/thread_runner $BUILDROOT_DIR/output/target/bin/thread_runner 2>/dev/null || cp $BUILDROOT_DIR/output/target/usr/bin/thread_runner $BUILDROOT_DIR/output/target/bin/thread_runner 2>/dev/null || true
fi

# Tutorial 3.1 (Ftrace): Instala script de teste e garante montagem do tracefs em /etc/fstab
if [ -f "$BUILDROOT_DIR/custom-scripts/ftrace_test.sh" ]; then
    mkdir -p $BUILDROOT_DIR/output/target/usr/bin
    cp $BUILDROOT_DIR/custom-scripts/ftrace_test.sh $BUILDROOT_DIR/output/target/usr/bin/ftrace_test
    chmod +x $BUILDROOT_DIR/output/target/usr/bin/ftrace_test
fi

mkdir -p "$BUILDROOT_DIR/output/target/sys/kernel/tracing" 2>/dev/null || true
if [ -f "$BUILDROOT_DIR/output/target/etc/fstab" ]; then
    if ! grep -q "tracefs /sys/kernel/tracing tracefs" "$BUILDROOT_DIR/output/target/etc/fstab"; then
        echo "tracefs /sys/kernel/tracing tracefs 0 0" >> "$BUILDROOT_DIR/output/target/etc/fstab"
    fi
fi

# Tutorial 2.3: Garante que os modulos estejam compilados e instalados no rootfs
if [ -d "$BASE_DIR/build/linux-custom" ]; then
    if [ ! -e "$BUILDROOT_DIR/target" ]; then
        ln -sf output/target "$BUILDROOT_DIR/target" 2>/dev/null || true
    fi
    [ -d "$BUILDROOT_DIR/modules/hello" ] && make -C "$BUILDROOT_DIR/modules/hello" || true
    [ -d "$BUILDROOT_DIR/modules/simple_driver" ] && make -C "$BUILDROOT_DIR/modules/simple_driver" || true
    [ -d "$BUILDROOT_DIR/modules/xtea_driver" ] && make -C "$BUILDROOT_DIR/modules/xtea_driver" || true
fi
