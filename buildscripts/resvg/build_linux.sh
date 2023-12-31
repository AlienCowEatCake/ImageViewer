#!/bin/bash -e

RESVG_VERSION="0.37.0"
RESVG_TARGET="$(gcc -dumpmachine | sed 's|-.*||')-unknown-linux-gnu"

cd "$(dirname $0)"

export RUSTUP_HOME="${PWD}/RUSTUP_HOME"
export CARGO_HOME="${PWD}/CARGO_HOME"
export PATH="${CARGO_HOME}/bin:${PATH}"

wget --no-check-certificate "https://sh.rustup.rs" -O rustup-init.sh
chmod +x rustup-init.sh
./rustup-init.sh --default-host "${RESVG_TARGET}" --default-toolchain stable --profile default --no-modify-path -y

wget "https://github.com/RazrFalcon/resvg/releases/download/v${RESVG_VERSION}/resvg-${RESVG_VERSION}.tar.xz"
tar -xvpf "resvg-${RESVG_VERSION}.tar.xz"
pushd "resvg-${RESVG_VERSION}" > /dev/null
cd "crates/c-api"
cargo build --release
popd > /dev/null

mkdir -p "${RESVG_TARGET}"
cp -a "resvg-${RESVG_VERSION}/crates/c-api/resvg.h" "${RESVG_TARGET}/"
cp -a "resvg-${RESVG_VERSION}/target/release/libresvg.so" "${RESVG_TARGET}/"
strip --strip-all "${RESVG_TARGET}/libresvg.so"

rm -rf "${RUSTUP_HOME}" "${CARGO_HOME}" rustup-init.sh "resvg-${RESVG_VERSION}.tar.xz" "resvg-${RESVG_VERSION}"
