# Builds Heath for an ARM target, then solves problem/fforum-20-39.obf with QEMU and compares the results with
# the scores of the file (tests/check-solve.py). Run it from src.
# usage: sh qemu.sh [arm32|neon|sve|armv8]   (armv8 by default)
# The exit code is not 0 when the build fails, when the program fails under QEMU, or when a result differs.
# The cross compilers are those of C++ (all.cpp needs the C++ library): g++-arm-linux-gnueabi, g++-aarch64-linux-gnu.
set -e
case "$1" in
arm32)
	arch=arm; cc=arm-linux-gnueabi-g++
	qemu="qemu-arm -L /usr/arm-linux-gnueabi -cpu max,neon=off";;
neon)
	arch=arm-neon; cc=arm-linux-gnueabi-g++
	qemu="qemu-arm -L /usr/arm-linux-gnueabi";;
sve)
	arch=arm-sve256; cc=aarch64-linux-gnu-g++
	qemu="qemu-aarch64 -L /usr/aarch64-linux-gnu -cpu max,sve256=on";;
""|armv8)
	arch=armv8; cc=aarch64-linux-gnu-g++
	qemu="qemu-aarch64 -L /usr/aarch64-linux-gnu";;
*)
	echo "usage: sh qemu.sh [arm32|neon|sve|armv8]" >&2
	exit 2;;
esac
make build OS=linux ARCH=$arch COMP=gcc CC=$cc
cd ../bin
python3 ../tests/check-solve.py ./lEdax-$arch ../problem/fforum-20-39.obf --prefix "$qemu"
# C:\Android\android-ndk-r21\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe --target=aarch64-linux-gnu -O2 -S flip_neon_bitscan.c
