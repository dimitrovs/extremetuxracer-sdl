#!/bin/bash
# Cross-build Extreme Tux Racer for the Trimui (tg5040, aarch64, PowerVR GLES2).
# Run inside the trimui-etr-build container with:
#   /work/etr   = this repository
#   /work/gl4es = a checkout of https://github.com/ptitSeb/gl4es
set -e
SYSROOT=/sdk/usr
CROSS=aarch64-linux-gnu-
OUT=/work/etr/build-trimui
mkdir -p "$OUT/obj"

# --- gl4es: desktop GL 1.x/2.x on top of GLES2, static, context comes from SDL2 ---
if [ ! -f "$OUT/gl4es/lib/libGL.a" ]; then
	mkdir -p "$OUT/gl4es" && cd "$OUT/gl4es"
	cmake /work/gl4es -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
		-DCMAKE_C_COMPILER=${CROSS}gcc -DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$OUT/gl4es/lib" -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$OUT/gl4es/lib" \
		-DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON -DNO_LOADER=ON -DNO_INIT_CONSTRUCTOR=ON -DDEFAULT_ES=2 > cmake.log
	make -j"$(nproc)" GL > make.log 2>&1 || { tail -40 make.log; exit 1; }
fi
GL4ES_LIB=$(find "$OUT/gl4es" /work/gl4es/lib -name 'libGL.a' 2>/dev/null | head -1)

# --- the game ---
CXXFLAGS="-O2 -g -std=gnu++11 -fsigned-char -DOS_LINUX -w \
	-I/work/gl4es/include -I$SYSROOT/include/SDL2 -I$SYSROOT/include/freetype2 -I$SYSROOT/include"
LDFLAGS="-L$SYSROOT/lib -Wl,-rpath-link=$SYSROOT/lib -Wl,--allow-shlib-undefined"
LIBS="$GL4ES_LIB -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype -lGLESv2 -lEGL -lz -lm -ldl -lpthread"

cd /work/etr/src
pids=()
for f in *.cpp; do
	o="$OUT/obj/${f%.cpp}.o"
	if [ ! -f "$o" ] || [ "$f" -nt "$o" ] || [ -n "$(find . -name '*.h' -newer "$o" | head -1)" ]; then
		echo "CXX $f"
		${CROSS}g++ $CXXFLAGS -c "$f" -o "$o" &
		pids+=($!)
		if [ ${#pids[@]} -ge "$(nproc)" ]; then wait "${pids[0]}" || exit 1; pids=("${pids[@]:1}"); fi
	fi
done
for p in "${pids[@]}"; do wait "$p" || exit 1; done
${CROSS}g++ -o "$OUT/etr" "$OUT"/obj/*.o $LDFLAGS $LIBS
echo "built $OUT/etr"
