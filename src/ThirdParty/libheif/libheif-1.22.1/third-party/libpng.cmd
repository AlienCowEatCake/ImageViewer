: # If you want to use a local libpng build, please check that the WITH_LIBPNG_INTERNAL CMake variable is set correctly.

: # The odd choice of comment style in this file is to try to share this script between *nix and win32.

git clone --single-branch https://github.com/pnggroup/libpng.git

cd libpng
mkdir build
cd build
cmake -G Ninja -DCMAKE_INSTALL_PREFIX="$(pwd)/dist" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DPNG_TESTS=OFF -DPNG_TOOLS=OFF ..
ninja
ninja install
cd ../..
