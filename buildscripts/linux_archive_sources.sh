#!/bin/bash -e
PROJECT="ImageViewer"
BUILDDIR="build_src"
LITE_ARCHIVE_SUFFIX="_system_thirdparty"

cd "$(dirname $0)"/..
SOURCE_PATH="${PWD}"
VERSION="$(grep setApplicationVersion "src/ImageViewer/src/main.cpp" | sed 's|.*setApplicationVersion([^"]*"\([^"]*\)".*|\1|')"
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"
fakeroot git archive --format=tar --prefix="${PROJECT}-${VERSION}/" HEAD > "${BUILDDIR}/${PROJECT}-${VERSION}.tar"
fakeroot xz -z9ev --threads=0 --memlimit-compress=75% "${BUILDDIR}/${PROJECT}-${VERSION}.tar"
cd "${BUILDDIR}"
tar -xvpf "${PROJECT}-${VERSION}.tar.xz"

# Remove ThirdParty projects and sources
find "${PROJECT}-${VERSION}/src/ThirdParty" -mindepth 2 -maxdepth 2 -type d -exec rm -rf \{\} \;
find "${PROJECT}-${VERSION}/src/ThirdParty" -mindepth 2 -maxdepth 2 -type f -name '*.pro' -delete
rm -rf "${PROJECT}-${VERSION}/src/ThirdParty/ThirdParty.pro"
# Remove fallback ICC profiles
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/iccprofiles"
# Remove buildscripts
rm -rf "${PROJECT}-${VERSION}/buildscripts"
# Remove manual testing data
rm -rf "${PROJECT}-${VERSION}/tests"
# Remove icon sources and convertation scripts
find "${PROJECT}-${VERSION}/src/ImageViewer/resources/icon" -type f -name '*.sh' -delete
find "${PROJECT}-${VERSION}/src/ImageViewer/resources/icon" -type f -name 'drawing*.svg' -delete
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/icon/oldiconutil"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/style/dark/original"
rm -rf "${PROJECT}-${VERSION}/src/QtUtils/resources/icons/original"
# Remove build scripts for duti
find "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/macosx" -type f -name 'make_duti*.sh' -delete
# Remove autogeneration scripts
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/src/GUI/Dialogs/generate_contributors.py"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/update_translations.sh"
# Remove MSI installer files
find "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows" -type f -name '*.wxs' -delete
find "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows" -type f -name '*.wxi' -delete
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows/generate_associations.py"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows/gpl-3.0.rtf"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows/make_ui.sh"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows/ui_banner.bmp"
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/windows/ui_dialog.bmp"
# Remove DEB packaging files
rm -rf "${PROJECT}-${VERSION}/src/ImageViewer/resources/platform/debian"
# Remove GIT and CI files
rm -rf "${PROJECT}-${VERSION}/.appveyor.yml"
rm -rf "${PROJECT}-${VERSION}/.gitignore"

fakeroot tar -cvpf "${PROJECT}-${VERSION}${LITE_ARCHIVE_SUFFIX}.tar" "${PROJECT}-${VERSION}"
fakeroot xz -z9ev --threads=0 --memlimit-compress=75% "${PROJECT}-${VERSION}${LITE_ARCHIVE_SUFFIX}.tar"
rm -rf "${PROJECT}-${VERSION}"
