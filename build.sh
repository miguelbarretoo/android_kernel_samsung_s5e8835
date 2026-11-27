#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

START=$SECONDS

format_time() {
    local total=$1
    local h=$((total / 3600))
    local m=$(((total % 3600) / 60))
    local s=$((total % 60))

    if (( h > 0 )); then
        echo "${h}h ${m}m ${s}s"
    elif (( m > 0 )); then
        echo "${m}m ${s}s"
    else
        echo "${s}s"
    fi
}

abort()
{
    cd -
    echo "-----------------------------------------------"
    echo "Build failed in $(format_time $((SECONDS - START)))!"
    echo "-----------------------------------------------"
    exit -1
}

usage() {
	cat <<EOF
Usage: $(basename "$0") [options]

Options:
  -m, --model <name>       Target model (default: x1slte)
  -k, --ksu <y|n>          Include KernelSU-Next (default: n)
  -r, --recovery <y|n>     Build only recovery-side artifacts
  -d, --dtbs <y|n>         Build only Image/DTB artifacts, no boot ZIP
	  -j, --jobs <count>       Parallel jobs (default: nproc --all)
EOF
}

MODEL="${MODEL:-x1slte}"
KSU_OPTION="${KSU_OPTION:-n}"
RECOVERY_OPTION="${RECOVERY_OPTION:-n}"
DTB_OPTION="${DTB_OPTION:-n}"
JOBS="${JOBS:-$(nproc --all)}"

while [[ $# -gt 0 ]]; do
	case "$1" in
		-m|--model) MODEL="$2"; shift 2 ;;
		-k|--ksu) KSU_OPTION="$2"; shift 2 ;;
		-r|--recovery) RECOVERY_OPTION="$2"; shift 2 ;;
		-d|--dtbs) DTB_OPTION="$2"; shift 2 ;;
		-j|--jobs) JOBS="$2"; shift 2 ;;
		-h|--help) usage; exit 0 ;;
		*) usage >&2; exit 2 ;;
	esac
done

case "$MODEL" in
	x1slte) BOARD=SRPSJ28B018KU ;;
	x1s) BOARD=SRPSI19A018KU ;;
	y2slte) BOARD=SRPSJ28A018KU ;;
	y2s) BOARD=SRPSG12A018KU ;;
	z3s) BOARD=SRPSI19B018KU ;;
	c1slte) BOARD=SRPTC30B009KU ;;
	c1s) BOARD=SRPTB27D009KU ;;
	c2slte) BOARD=SRPTC30A009KU ;;
	c2s) BOARD=SRPTB27C009KU ;;
	r8s) BOARD=SRPTF26B014KU ;;
	*) echo "Unsupported model: $MODEL" >&2; usage >&2; exit 2 ;;
esac

MODEL_CONFIG="arch/arm64/configs/${MODEL}.config"
[[ -f "$MODEL_CONFIG" ]] || { echo "Missing model config: $MODEL_CONFIG" >&2; exit 1; }
[[ -f "build/dtconfigs/${MODEL}.cfg" ]] || { echo "Missing DTBO config for $MODEL" >&2; exit 1; }

if [[ "$RECOVERY_OPTION" == y ]]; then
	KSU_OPTION=n
fi

if [[ "$KSU_OPTION" == y && ! -e drivers/kernelsu ]]; then
	[[ -x ./setup.sh ]] || { echo "KernelSU-Next setup.sh is missing" >&2; exit 1; }
	./setup.sh "$(git -C KernelSU-Next rev-parse HEAD)"
fi

if [[ "$KSU_OPTION" == y ]]; then
	KSU_PATCH="$ROOT_DIR/patches/KernelSU-Next-5.15.patch"
	if git -C KernelSU-Next apply --check "$KSU_PATCH" >/dev/null 2>&1; then
		git -C KernelSU-Next apply "$KSU_PATCH"
	elif ! git -C KernelSU-Next apply --reverse --check "$KSU_PATCH" >/dev/null 2>&1; then
		echo "KernelSU-Next compatibility patch cannot be applied" >&2
		exit 1
	fi
fi

CLANG_DIR="${CLANG_DIR:-$ROOT_DIR/toolchain/clang_14}"
if [[ -d "$CLANG_DIR/bin" ]]; then
	export PATH="$CLANG_DIR/bin:$PATH"
fi

if command -v ld.lld >/dev/null 2>&1; then
	LLVM_BIN="$(dirname "$(readlink -f "$(command -v ld.lld)")")"
	export PATH="$LLVM_BIN:$PATH"
fi

command -v clang >/dev/null || {
	echo "clang was not found; set CLANG_DIR or add LLVM to PATH" >&2
	exit 1
}

CORES="${JOBS:-$(nproc --all)}"
BUILD_OUT="$ROOT_DIR/out"
PACKAGE_OUT="$ROOT_DIR/build/out/$MODEL"
rm -rf -- "$PACKAGE_OUT"
mkdir -p "$PACKAGE_OUT/zip/files" \
	"$PACKAGE_OUT/zip/META-INF/com/google/android"

MAKE_ARGS=(
	make
	O="$BUILD_OUT"
	ARCH=arm64
	LLVM=1
	LLVM_IAS=1
	TARGET_SOC=exynos9830
	PLATFORM_VERSION_FOR_GPU=13
)

echo "-----------------------------------------------"
echo "Defconfig: exynos990-x1slte_defconfig + $MODEL.config"
echo "KernelSU-Next: $KSU_OPTION"
echo "Jobs: $CORES"
echo "Recovery-only: $RECOVERY_OPTION"
echo "DTBs-only: $DTB_OPTION"
echo "bomb by @miguelbarretoo"
echo "-----------------------------------------------"

"${MAKE_ARGS[@]}" exynos990-x1slte_defconfig
FRAGMENTS=("$MODEL_CONFIG")
if [[ "$KSU_OPTION" == y ]]; then
	FRAGMENTS+=("arch/arm64/configs/ksu.config")
fi
scripts/kconfig/merge_config.sh -m -O "$BUILD_OUT" "$BUILD_OUT/.config" "${FRAGMENTS[@]}"
if [[ "$KSU_OPTION" == n ]]; then
	# Do not inherit KSU from a previous build in the reusable O= directory.
	scripts/config --file "$BUILD_OUT/.config" --disable KSU
fi
"${MAKE_ARGS[@]}" olddefconfig
if [[ "$MODEL" == x1slte ]]; then
        scripts/config --file "$BUILD_OUT/.config" --enable EXYNOS_EL2 --enable EXYNOS_S2MPU --enable EXYNOS_PM --enable EXYNOS_CHIPID_V2 --enable PINCTRL_SAMSUNG
        scripts/config --file "$BUILD_OUT/.config" --disable CP_PMUCAL --disable GNSS_PMUCAL --disable CHUB_PMUCAL --disable SCHED_EMS --disable EXYNOS_ARM64_COMMON_CLK --disable SND_SOC_SAMSUNG_ABOX --disable SND_SOC_SAMSUNG_VTS --disable SND_SOC_SAMSUNG_EXYNOS8835_AUDIO
fi


if [[ "$DTB_OPTION" == y ]]; then
	BUILD_TARGETS=(dtbs)
else
	BUILD_TARGETS=(Image modules dtbs)
fi

#echo "Building: ${BUILD_TARGETS[*]}"
echo "-----------------------------------------------"
echo "Building kernel..."
"${MAKE_ARGS[@]}" -j"$CORES" "${BUILD_TARGETS[@]}" || abort

DTB_CONFIG=build/dtconfigs/exynos9830.cfg
[[ "$MODEL" == r8s ]] && DTB_CONFIG=build/dtconfigs/exynos9830-r8s.cfg

# Build dtbo
echo "-----------------------------------------------"
echo "Building Device Tree Blob Output Image for "$MODEL"..."
./toolchain/mkdtimg cfg_create "$PACKAGE_OUT/dtb.img" "$DTB_CONFIG" \
	-d "$BUILD_OUT/arch/arm64/boot/dts/exynos"
./toolchain/mkdtimg cfg_create "$PACKAGE_OUT/dtbo.img" \
	"build/dtconfigs/$MODEL.cfg" \
	-d "$BUILD_OUT/arch/arm64/boot/dts/samsung"
echo "-----------------------------------------------"

if [[ "$DTB_OPTION" == y ]]; then
	echo "DTB build finished: $PACKAGE_OUT"
	echo "-----------------------------------------------"
	exit 0
fi

cp "$BUILD_OUT/arch/arm64/boot/Image" "$PACKAGE_OUT/Image"

# GKI-style modules: first-stage init loads /lib/modules/modules.load from the
# boot ramdisk. Only the modules listed for this model, plus what they depend
# on, are packed, so the boot image keeps fitting its partition.
MODULES_STAGE=""
MODULES_LIST="build/modules/$MODEL.load"
if [[ "$RECOVERY_OPTION" != y && -s "$MODULES_LIST" ]]; then
	echo "Staging kernel modules from $MODULES_LIST..."
	MODULES_STAGE="$(mktemp -d)"
	trap 'rm -rf -- "$MODULES_STAGE"' EXIT
	INSTALL="$MODULES_STAGE/install"
	"${MAKE_ARGS[@]}" INSTALL_MOD_PATH="$INSTALL" INSTALL_MOD_STRIP=1 \
		DEPMOD=true modules_install > /dev/null || abort
	KREL="$(cat "$BUILD_OUT/include/config/kernel.release")"
	depmod -b "$INSTALL" "$KREL" || abort
	python3 build/modules/stage.py "$INSTALL/lib/modules/$KREL" "$MODULES_LIST" \
		"$MODULES_STAGE/lib/modules" || abort
	echo "Modules: $(grep -c . "$MODULES_STAGE/lib/modules/modules.load") listed," \
		"$(ls "$MODULES_STAGE/lib/modules" | grep -c '\.ko$') packed"
	echo "-----------------------------------------------"
fi

if [[ "$RECOVERY_OPTION" != y ]]; then
	# Build ramdisk
    echo "Building RAMDisk..."
	RAMDISK="$PACKAGE_OUT/ramdisk.cpio.gz"
	(
		RAMDISK_ROOT="$(mktemp -d)"
		trap 'rm -r -- "$RAMDISK_ROOT"' EXIT
		FSTAB=build/ramdisk/fstab.exynos990
		[[ "$MODEL" == x1slte ]] && FSTAB=build/ramdisk/fstab.x1slte

		install -d -m 0755 "$RAMDISK_ROOT"/{dev,proc,sys,mnt,debug_ramdisk}
		install -m 0750 build/ramdisk/init "$RAMDISK_ROOT/init"
		install -m 0640 "$FSTAB" "$RAMDISK_ROOT/fstab.exynos990"
		[[ -n "$MODULES_STAGE" ]] && cp -a "$MODULES_STAGE/lib" "$RAMDISK_ROOT/"
		cd "$RAMDISK_ROOT"
		find . -mindepth 1 -exec touch -h -d '@0' {} +
		find . -mindepth 1 -print0 | LC_ALL=C sort -z |
			cpio --null --reproducible -o -H newc -R root:root |
			gzip -n > "$RAMDISK"
	)
    echo "-----------------------------------------------"
    # Create boot image
    echo "Creating boot image..."
    echo "-----------------------------------------------"
	./toolchain/mkbootimg \
		--base 0x10000000 \
		--board "$BOARD" \
		--cmdline 'androidboot.hardware=exynos990 loop.max_part=7' \
		--dtb "$PACKAGE_OUT/dtb.img" \
		--dtb_offset 0x00000000 \
		--hashtype sha1 \
		--header_version 2 \
		--kernel "$PACKAGE_OUT/Image" \
		--kernel_offset 0x00008000 \
		--os_patch_level 2026-05 \
		--os_version 16.0.0 \
		--pagesize 2048 \
		--ramdisk "$RAMDISK" \
		--ramdisk_offset 0x01000000 \
		--second_offset 0xF0000000 \
		--tags_offset 0x00000100 \
		-o "$PACKAGE_OUT/boot.img"

    # Build zip
    echo "Building zip..."
    echo "-----------------------------------------------"
	cp "$PACKAGE_OUT/boot.img" "$PACKAGE_OUT/zip/files/boot.img"
	cp "$PACKAGE_OUT/dtbo.img" "$PACKAGE_OUT/zip/files/dtbo.img"
	cp build/update-binary "$PACKAGE_OUT/zip/META-INF/com/google/android/update-binary"
	cp build/updater-script "$PACKAGE_OUT/zip/META-INF/com/google/android/updater-script"

	VERSION="$(sed -n 's/^CONFIG_LOCALVERSION="\(.*\)"/\1/p' "$BUILD_OUT/.config")"
	VERSION="${VERSION:-MIG29_krenol}"
	DATE="$(date +%d-%m-%Y_%H-%M-%S)"
	SUFFIX="UNOFFICIAL"
	[[ "$KSU_OPTION" == y ]] && SUFFIX="UNOFFICIAL_KSU"
	ZIP_NAME="${VERSION}_${MODEL}_${SUFFIX}_${DATE}.zip"
	(
		cd "$PACKAGE_OUT/zip"
		zip -r -qq "../$ZIP_NAME" .
	)
	echo "Flashable ZIP: $PACKAGE_OUT/$ZIP_NAME"
fi

echo "-----------------------------------------------"
echo "Build finished successfully in $(format_time $((SECONDS - START)))!"
echo "-----------------------------------------------"