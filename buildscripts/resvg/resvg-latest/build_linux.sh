#!/bin/bash -e

RESVG_VERSION="0.46.0"
RESVG_TARGET="$(gcc -dumpmachine | sed 's|-.*||')-unknown-linux-gnu"

RUST_VERSION="1.87.0"

cd "$(dirname $0)"

export RUSTUP_HOME="${PWD}/RUSTUP_HOME"
export CARGO_HOME="${PWD}/CARGO_HOME"
export PATH="${CARGO_HOME}/bin:${PATH}"

wget --no-check-certificate "https://raw.githubusercontent.com/rust-lang/rustup/refs/heads/main/rustup-init.sh" -O rustup-init.sh
chmod +x rustup-init.sh
./rustup-init.sh --default-host "${RESVG_TARGET}" --default-toolchain "${RUST_VERSION}" --profile default --no-modify-path -y

wget --no-check-certificate "https://github.com/linebender/resvg/archive/refs/tags/v${RESVG_VERSION}.tar.gz" -O "resvg-${RESVG_VERSION}.tar.gz"
tar -xvpf "resvg-${RESVG_VERSION}.tar.gz"
pushd "resvg-${RESVG_VERSION}" > /dev/null
cd "crates/c-api"
cargo build --release
popd > /dev/null

mkdir -p "${RESVG_TARGET}"
cp -a "resvg-${RESVG_VERSION}/crates/c-api/resvg.h" "${RESVG_TARGET}/"
cp -a "resvg-${RESVG_VERSION}/target/release/libresvg.so" "${RESVG_TARGET}/"
strip --strip-all "${RESVG_TARGET}/libresvg.so"

rm -rf "${RUSTUP_HOME}" "${CARGO_HOME}" rustup-init.sh "resvg-${RESVG_VERSION}.tar.gz" "resvg-${RESVG_VERSION}"
