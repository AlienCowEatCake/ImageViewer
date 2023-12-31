#!/bin/bash -e

RESVG_VERSION="0.37.0"
RESVG_HOST="x86_64-apple-darwin"
export MACOSX_DEPLOYMENT_TARGET=10.7

cd "$(dirname $0)"

export RUSTUP_HOME="${PWD}/RUSTUP_HOME"
export CARGO_HOME="${PWD}/CARGO_HOME"
export PATH="${CARGO_HOME}/bin:${PATH}"

curl -Lo rustup-init.sh "https://sh.rustup.rs"
chmod +x rustup-init.sh
./rustup-init.sh --default-host "${RESVG_HOST}" --target "x86_64-apple-darwin" --target "aarch64-apple-darwin" --default-toolchain stable --profile default --no-modify-path -y

curl -LO "https://github.com/RazrFalcon/resvg/releases/download/v${RESVG_VERSION}/resvg-${RESVG_VERSION}.tar.xz"
tar -xvpf "resvg-${RESVG_VERSION}.tar.xz"
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

rm -rf "${RUSTUP_HOME}" "${CARGO_HOME}" rustup-init.sh "resvg-${RESVG_VERSION}.tar.xz" "resvg-${RESVG_VERSION}"
