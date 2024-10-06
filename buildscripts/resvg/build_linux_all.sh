#!/bin/bash -e

SOURCE_DIR="$(cd "$(dirname "${0}")" && pwd)"
DOCKER_USER="user"
DOCKER_WORKSACE="/home/${DOCKER_USER}/workspace"
FORWARD_SCRIPT="build_linux.sh"
for DOCKER_ARCH in amd64 arm64 i386 ; do
    DOCKER_IMAGE="aliencoweatcake/${DOCKER_ARCH}-trusty-qt5projects:qt5.15.15"
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
        "${DOCKER_WORKSACE}/${FORWARD_SCRIPT}"
done
