#!/bin/bash -e

SOURCE_DIR="$(cd "$(dirname "${0}")/.." && pwd)"
DOCKER_ARCH="$(echo "${0##*/docker_}" | sed 's|_.*||')"
DOCKER_IMAGE="aliencoweatcake/${DOCKER_ARCH}-lucid-qt5projects:qt5.6.3u2"
DOCKER_USER="user"
DOCKER_WORKSACE="/home/${DOCKER_USER}/workspace"
FORWARD_SCRIPT="${0##*/docker_${DOCKER_ARCH}_}"

ADDITIONAL_DOCKER_ARGS=
if [ -t 1 ]; then
    if [ -z "$(uname -s | grep -E -i 'cygwin|mingw|msys')" ]; then
        ADDITIONAL_DOCKER_ARGS="${ADDITIONAL_DOCKER_ARGS} --tty"
    fi
fi

docker pull "${DOCKER_IMAGE}"
MSYS_NO_PATHCONV=1 docker run \
    --rm \
    --interactive \
    --user "${DOCKER_USER}" \
    --volume "${SOURCE_DIR}:${DOCKER_WORKSACE}:delegated" \
    ${ADDITIONAL_DOCKER_ARGS} \
    "${DOCKER_IMAGE}" \
    "${DOCKER_WORKSACE}/buildscripts/${FORWARD_SCRIPT}"
