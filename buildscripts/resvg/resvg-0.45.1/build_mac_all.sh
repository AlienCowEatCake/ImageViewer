#!/bin/bash -e

RESVG_VERSION="0.45.1"
if [[ $(sysctl -n machdep.cpu.brand_string) =~ "Apple" ]] ; then
    RESVG_HOST="aarch64-apple-darwin"
else
    RESVG_HOST="x86_64-apple-darwin"
fi
export MACOSX_DEPLOYMENT_TARGET=10.7

# Mac OS X 10.7 support is dropped since 1.74.0. See changelog here:
# https://releases.rs/docs/1.74.0/
RUST_VERSION="1.73.0"

cd "$(dirname $0)"

export RUSTUP_HOME="${PWD}/RUSTUP_HOME"
export CARGO_HOME="${PWD}/CARGO_HOME"
export PATH="${CARGO_HOME}/bin:${PATH}"

curl -Lo rustup-init.sh "https://raw.githubusercontent.com/rust-lang/rustup/refs/heads/main/rustup-init.sh"
chmod +x rustup-init.sh
./rustup-init.sh --default-host "${RESVG_HOST}" --target "x86_64-apple-darwin" --target "aarch64-apple-darwin" --default-toolchain "${RUST_VERSION}" --profile default --no-modify-path -y

curl -Lo "resvg-${RESVG_VERSION}.tar.gz" "https://github.com/linebender/resvg/archive/refs/tags/v${RESVG_VERSION}.tar.gz"
tar -xvpf "resvg-${RESVG_VERSION}.tar.gz"
for RESVG_TARGET in "x86_64-apple-darwin" "aarch64-apple-darwin" ; do
    pushd "resvg-${RESVG_VERSION}" > /dev/null
    cd "crates/c-api"
    cargo build --release --target "${RESVG_TARGET}"
    popd > /dev/null

    mkdir -p "${RESVG_TARGET}"
    cp -a "resvg-${RESVG_VERSION}/crates/c-api/resvg.h" "${RESVG_TARGET}/"
    cp -a "resvg-${RESVG_VERSION}/target/${RESVG_TARGET}/release/libresvg.dylib" "${RESVG_TARGET}/"
    strip -x "${RESVG_TARGET}/libresvg.dylib"
done

rm -rf "${RUSTUP_HOME}" "${CARGO_HOME}" rustup-init.sh "resvg-${RESVG_VERSION}.tar.gz" "resvg-${RESVG_VERSION}"
