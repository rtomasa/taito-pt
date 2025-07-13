#!/bin/bash

set -e

PKG="taito_pt"
VER="1.0"
SRCDIR="."    # use current directory
USR_SRC="/usr/src/$PKG-$VER"

usage() {
    echo "Usage: $0 {add|build|install|remove|all}"
    exit 1
}

add() {
    echo "Copying source to $USR_SRC..."
    sudo rm -rf "$USR_SRC"
    sudo mkdir -p "$USR_SRC"
    sudo cp -r * "$USR_SRC/"
    echo "Adding module to DKMS..."
    sudo dkms add -m "$PKG" -v "$VER"
}

build() {
    echo "Building module with DKMS..."
    sudo dkms build -m "$PKG" -v "$VER"
}

install() {
    echo "Installing module with DKMS..."
    sudo dkms install -m "$PKG" -v "$VER"
}

remove() {
    echo "Removing module from DKMS (all kernel versions)..."
    sudo dkms remove -m "$PKG" -v "$VER" --all
    sudo rm -rf "$USR_SRC"
}

case "$1" in
    add) add ;;
    build) build ;;
    install) install ;;
    remove) remove ;;
    all) add; build; install ;;
    *) usage ;;
esac

echo "Done."
