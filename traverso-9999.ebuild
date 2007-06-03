# Copyright 1999-2007 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/ingmar/development/exherbo/temp/traverso/traverso/traverso-9999.ebuild,v 1.2 2007/06/03 23:15:52 ingmar Exp $

inherit eutils qt4 toolchain-funcs cvs

DESCRIPTION="Professional Audio Tools for GNU/Linux"
HOMEPAGE="http://traverso-daw.org/"
SRC_URI=""

IUSE="alsa jack lv2 sse"
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~amd64 ~ppc ~x86"

RDEPEND="$(qt4_min_version 4.2.3)
	alsa? ( media-libs/alsa-lib )
	jack? ( media-sound/jack-audio-connection-kit )
	>=media-libs/libsndfile-1.0.12
	media-libs/libsamplerate
	>=sci-libs/fftw-3
	lv2? ( dev-libs/rasqal dev-libs/redland )"

DEPEND="${RDEPEND}
	sys-apps/sed"

S="${WORKDIR}/${PN}"

src_unpack() {

	ECVS_SERVER="cvs.savannah.nongnu.org:/sources/traverso"
	ECVS_USER="anonymous"
	ECVS_PASS=""
	ECVS_AUTH="pserver"
	ECVS_MODULE="traverso"

	cvs_src_unpack

	cd "${S}"

	sed -ie "s:^\(\#define\ RESOURCES_DIR\) \(.*\):\1 \"/usr/share/traverso\":" src/config.h
	sed -ie "s:^\(target.path\ =\) \(.*\):\1 /usr/bin:" src/traverso/traverso.pro
	sed -ie "s:^\(DESTDIR_TARGET\ =\) \(.*\):\1 /usr/bin:" src/traverso/traverso.pro
	#  Removing forced cxxflags
	sed -ie "s:^\(.*QMAKE_CXXFLAGS_RELEASE.*\):#\1:" src/base.pri
	# adding our cxxflags
	sed -ie "s:^\(.*release\ {.*\):\1\n QMAKE_CXXFLAGS_RELEASE\ =\ ${CXXFLAGS}:" src/base.pri
}

src_compile() {
	use jack || echo "DEFINES -= JACK_SUPPORT" >> src/base.pri
	use alsa || echo "DEFINES -= ALSA_SUPPORT" >> src/base.pri
	use sse || echo "DEFINES -= SSE_OPTIMIZATIONS" >> src/base.pri
	use lv2 || echo "DEFINES -= LV2_SUPPORT" >> src/base.pri

	QMAKE="/usr/bin/qmake"
	$QMAKE traverso.pro -after "QMAKE_STRIP=\"/usr/bin/true\"" || die "qmake failed"
	emake CC=$(tc-getCC) CXX=$(tc-getCXX) LINK=$(tc-getCXX) || die "emake failed"
}

src_install() {
	emake INSTALL_ROOT="${D}" install || die "emake install failed"
	dodoc AUTHORS ChangeLog README

	doicon resources/images/traverso-logo.svg
	make_desktop_entry ${PN} Traverso /usr/share/pixmaps/traverso-logo.svg
}
