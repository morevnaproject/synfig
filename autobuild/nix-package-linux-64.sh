set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)
export OUTPUT="${SCRIPTPATH}/../_packages"
export OUTPUT_TMP="${OUTPUT}/linux-64"
cd ${SCRIPTPATH}

# Requirements: patchelf

parse()
{
    #echo "$1"
    DRVFILE=`basename "$1"`
    if [ ! -f "${OUTPUT_TMP}/${DRVFILE}" ]; then
	chmod +w "${OUTPUT_TMP}"
	cp "$1" "${OUTPUT_TMP}/${DRVFILE}"
	for DIR in `nix-store --query --outputs "$DEP"`; do
	    if [[ $DIR != *-dev ]] && [[ $DIR != *-man ]] \
	    && [[ $DIR != *-doc ]] && [[ $DIR != *-devdoc ]] \
	    && [ -d $DIR ] \
	    ; then
	    echo "--${DIR}"
	    #mkdir -p "${OUTPUT_TMP}/${DRVFILE}-c/" || true
	    #rsync -av "${DIR}/" "${OUTPUT_TMP}/${DRVFILE}-c/" || true
	    rsync -av "${DIR}/" "${OUTPUT_TMP}/" || true
	    fi
	done
	for DEP in `nix-store --query --references $1`; do
	    if [[ $DEP == *\.drv ]] \
	    && [[ $DEP != *\.patch\.drv ]] \
	    && [[ $DEP != *-hook\.drv ]] \
	    && [[ $DEP != *-acl-* ]] \
	    && [[ $DEP != *-alsa-lib-* ]] \
	    && [[ $DEP != *-attr-* ]] \
	    && [[ $DEP != *-audit-* ]] \
	    && [[ $DEP != *-autoconf-* ]] \
	    && [[ $DEP != *-automake-* ]] \
	    && [[ $DEP != *-avahi-* ]] \
	    && [[ $DEP != *-bash-* ]] \
	    && [[ $DEP != *-binutils-* ]] \
	    && [[ $DEP != *-bootstrap-tools-* ]] \
	    && [[ $DEP != *-busybox-* ]] \
	    && [[ $DEP != *-celt-* ]] \
	    && [[ $DEP != *-coreutils-* ]] \
	    && [[ $DEP != *-cups-* ]] \
	    && [[ $DEP != *-curl-* ]] \
	    && [[ $DEP != *-dbus-* ]] \
	    && [[ $DEP != *-dconf-* ]] \
	    && [[ $DEP != *-default-builder* ]] \
	    && [[ $DEP != *-dejavu-fonts* ]] \
	    && [[ $DEP != *-dns-root-data-* ]] \
	    && [[ $DEP != *-gawk-* ]] \
	    && [[ $DEP != *-glu-* ]] \
	    && [[ $DEP != *-gnugrep-* ]] \
	    && [[ $DEP != *-gnum4-* ]] \
	    && [[ $DEP != *-gnumake-* ]] \
	    && [[ $DEP != *-gnused-* ]] \
	    && [[ $DEP != *-gnutls-* ]] \
	    && [[ $DEP != *-gtk\+-* ]] \
	    && [[ $DEP != *-gtk-doc* ]] \
	    && [[ $DEP != *-intltool-* ]] \
	    && [[ $DEP != *-iptables-* ]] \
	    && [[ $DEP != *-kexec-tools-* ]] \
	    && [[ $DEP != *-kmod-* ]] \
	    && [[ $DEP != *-libapparmor-* ]] \
	    && [[ $DEP != *-libdrm-* ]] \
	    && [[ $DEP != *-libelf-* ]] \
	    && [[ $DEP != *-libICE-* ]] \
	    && [[ $DEP != *-libGL-* ]] \
	    && [[ $DEP != *-libglvnd-* ]] \
	    && [[ $DEP != *-libjack2-* ]] \
	    && [[ $DEP != *-libpciaccess-* ]] \
	    && [[ $DEP != *-libpulseaudio-* ]] \
	    && [[ $DEP != *-libtasn-* ]] \
	    && [[ $DEP != *-libusb-* ]] \
	    && [[ $DEP != *-libX* ]] \
	    && [[ $DEP != *-libXdmcp-* ]] \
	    && [[ $DEP != *-libxcb-* ]] \
	    && [[ $DEP != *-libxshmfence-* ]] \
	    && [[ $DEP != *-linux-4* ]] \
	    && [[ $DEP != *-linux-headers-* ]] \
	    && [[ $DEP != *-linux-pam-* ]] \
	    && [[ $DEP != *-m4-* ]] \
	    && [[ $DEP != *-mesa-* ]] \
	    && [[ $DEP != *-ncurses-* ]] \
	    && [[ $DEP != *-openssl-* ]] \
	    && [[ $DEP != *-p11-kit-* ]] \
	    && [[ $DEP != *-pcre2-* ]] \
	    && [[ $DEP != *-perl-* ]] \
	    && [[ $DEP != *-perl5\.* ]] \
	    && [[ $DEP != *-pkg-config-* ]] \
	    && [[ $DEP != *-python-* ]] \
	    && [[ $DEP != *-readline-* ]] \
	    && [[ $DEP != *-shadow-* ]] \
	    && [[ $DEP != *-shared-mime-info-* ]] \
	    && [[ $DEP != *-stdenv-* ]] \
	    && [[ $DEP != *-systemd-* ]] \
	    && [[ $DEP != *-util-linux-* ]] \
	    && [[ $DEP != *-wayland-* ]] \
	    && [[ $DEP != *-wheel-* ]] \
	    && [[ $DEP != *-which-* ]] \
	    && [[ $DEP != *-xcb-util-* ]] \
	    && [[ $DEP != *-xkeyboard-config-* ]] \
	    && [[ $DEP != *-xproto-* ]] \
	    ; then
		parse "$DEP"
	    fi
	done
    fi
}

if [ -d "${OUTPUT_TMP}" ]; then
    chmod -R +w "${OUTPUT_TMP}"
    rm -rf "${OUTPUT_TMP}/"
fi
mkdir -p "${OUTPUT_TMP}"
#nix-build
ITEM=`nix-instantiate default.nix`
parse "$ITEM"

rsync -av "${SCRIPTPATH}/_production/build/" "${OUTPUT_TMP}/" || true

chmod -R +w "${OUTPUT_TMP}"

mkdir ${OUTPUT_TMP}/bin.new
for i in animate convert ffmpeg synfig; do
mv ${OUTPUT_TMP}/bin/$i ${OUTPUT_TMP}/bin.new/
done
mv ${OUTPUT_TMP}/bin/.synfigstudio-wrapped ${OUTPUT_TMP}/bin.new/synfigstudio
rm -rf ${OUTPUT_TMP}/bin
mv ${OUTPUT_TMP}/bin.new ${OUTPUT_TMP}/bin

pushd ${OUTPUT_TMP}/bin/
for i in `ls -1`; do
patchelf --set-interpreter /lib64/ld-linux-x86-64.so.2 $i
done

rm -rf ${OUTPUT_TMP}/include
rm -rf ${OUTPUT_TMP}/lib64
rm -rf ${OUTPUT_TMP}/nix-support
rm -rf ${OUTPUT_TMP}/share/doc
rm -rf ${OUTPUT_TMP}/share/man
rm -rf ${OUTPUT_TMP}/sbin

cp ${SCRIPTPATH}/linux/synfigstudio.sh ${OUTPUT_TMP}/synfigstudio
chmod +x ${OUTPUT_TMP}/synfigstudio
