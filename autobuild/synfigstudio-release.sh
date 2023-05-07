#!/bin/bash

# Usage notes
#
# Running this script will creates release tarballs for all synfig 
# modules and testing them by installing into ~/local-synfig/.
#
# It is also possible to run procedure for each module separately by 
# passing specific arguments to the script:
# * Run procedures for ETL:
#    synfigstudio-release.sh etl
# * Run procedures for synfig-core:
#    synfigstudio-release.sh core
# * Run procedures for synfig-studio:
#    synfigstudio-release.sh studio
# * Run check for localization files:
#    synfigstudio-release.sh l10n
#

set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)
export SRCPREFIX=`dirname "$SCRIPTPATH"`

BUILD_RELEASE_DIR=${SRCPREFIX}/_release/

export PREFIX="$BUILD_RELEASE_DIR/build"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$PREFIX/lib/pkgconfig"
export PATH="$PREFIX/bin:$PATH"
export ETL_VERSION=`cat $SRCPREFIX/ETL/configure.ac |egrep "AC_INIT\(\[Extended Template Library\],"| sed "s|.*Library\],\[||" | sed "s|\],\[.*||"`
echo "ETL_VERSION=$ETL_VERSION"
export CORE_VERSION=`cat $SRCPREFIX/synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
echo "CORE_VERSION=$CORE_VERSION"
export STUDIO_VERSION=`cat $SRCPREFIX/synfig-studio/configure.ac |egrep "AC_INIT\(\[Synfig Studio\],"| sed "s|.*Studio\],\[||" | sed "s|\],\[.*||"`
echo "STUDIO_VERSION=$STUDIO_VERSION"

# Colored output
RED='\033[0;31m'
YLW='\033[1;33m'
GRN='\033[0;32m'
NC='\033[0m' # No Color

if [[ `uname` == "MINGW"* ]]; then # MacOS doesn't support `uname -o` flag
	PATH="${MINGW_PREFIX}/lib/ccache/bin:${PATH}"
	PKG_CONFIG_PATH="/opt/mlt-7.2.0/lib/pkgconfig:${PKG_CONFIG_PATH}"
	echo "ccache enabled"
fi

if [ -z $THREADS ]; then
	export THREADS=4
fi

start_stage()
{
	echo -e "Starting ${YLW}$1${NC} stage"
}

end_stage() {
	echo "$1 complete."
}

l10n_check()
{
	cd $2
	OUTPUT=`intltool-update -m 2>&1`
	
	if [ ! -z "${OUTPUT}" ]; then
		echo -e "Checking $1 translations... ${RED}Error${NC}"
		echo "${OUTPUT}"
		exit 1
	fi
	echo -e "Checking $1 translations... ${GRN}Done${NC}"
}

l10n()
{
	start_stage "l10n"
	l10n_check "Synfig Core" "$SRCPREFIX/synfig-core/po"
	l10n_check "Synfig Studio" "$SRCPREFIX/synfig-studio/po"
	end_stage "l10n"
}

pack_etl()
{
	start_stage "Pack ETL"
	cd $SRCPREFIX/ETL
	autoreconf -if
	mkdir -p ${BUILD_RELEASE_DIR}/ETL && cd $BUILD_RELEASE_DIR/ETL
	$SRCPREFIX/ETL/configure --prefix="$PREFIX"
	make V=0 CXXFLAGS="-w" distcheck -j${THREADS}
	make V=0 CXXFLAGS="-w" install -j${THREADS}
	mv ETL-${ETL_VERSION}.tar.gz ${BUILD_RELEASE_DIR}
	end_stage "Pack ETL"
}

pack_core()
{
	start_stage "Pack Synfig Core"
	cd $SRCPREFIX/synfig-core
	./bootstrap.sh
	mkdir -p ${BUILD_RELEASE_DIR}/synfig-core && cd $BUILD_RELEASE_DIR/synfig-core
	$SRCPREFIX/synfig-core/configure --prefix="$PREFIX"
	echo "------------------------------------- pack-core make"
	make V=0 CXXFLAGS="-w" distcheck -j${THREADS}
	make V=0 CXXFLAGS="-w" install -j${THREADS}
	mv synfig-${CORE_VERSION}.tar.gz ${BUILD_RELEASE_DIR}
	end_stage "Pack Synfig Core"
}

pack_studio()
{
	start_stage "Pack Synfig Studio"
	cd $SRCPREFIX/synfig-studio
	./bootstrap.sh
	mkdir -p ${BUILD_RELEASE_DIR}/synfig-studio && cd $BUILD_RELEASE_DIR/synfig-studio
	$SRCPREFIX/synfig-studio/configure --prefix="$PREFIX"
	make V=0 CXXFLAGS="-w" distcheck -j${THREADS}
	mv synfigstudio-${STUDIO_VERSION}.tar.gz ${BUILD_RELEASE_DIR}
	end_stage "Pack Synfig Studio"
}

mkall()
{
	l10n
	pack_etl
	pack_core
	pack_studio
}

do_cleanup()
{
	start_stage "Clean up"
	#echo "Cleaning up..."
	if [ "${PREFIX}" != "${DEPSPREFIX}" ]; then
		[ ! -e ${DEPSPREFIX} ] || mv ${DEPSPREFIX} ${DEPSPREFIX}.off
	fi
	[ ! -e ${SYSPREFIX} ] || mv ${SYSPREFIX} ${SYSPREFIX}.off
	exit
}

#trap do_cleanup INT SIGINT SIGTERM EXIT

if [ -z $1 ]; then
	rm -rf "$PREFIX" || true
	mkall
else
	echo "Executing custom user command..."

	$@
fi

#do_cleanup
