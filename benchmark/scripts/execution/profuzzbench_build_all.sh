#!/bin/bash

export NO_CACHE="--no-cache"
export MAKE_OPT="-j4"

# Function to build a specific image if it matches BUILD_TARGET
build_image() {
    local target=$1
    local image_name=$2
    local path=$3

    # If BUILD_TARGET is not set or matches the target, build the image
    if [ -z "$BUILD_TARGET" ] || [ "$BUILD_TARGET" = "$target" ]; then
        echo "Building image: ${IMAGE_PREFIX}${image_name}"
        cd $PFBENCH
        cd $path
        docker build . -t ${IMAGE_PREFIX}${image_name} --build-arg MAKE_OPT $NO_CACHE
    else
        echo "Skipping image: ${IMAGE_PREFIX}${image_name} (not in BUILD_TARGET)"
    fi
}

# Build images based on BUILD_TARGET
build_image "proftpd" "proftpd" "subjects/FTP/ProFTPD"
build_image "pure-ftpd" "pure-ftpd" "subjects/FTP/PureFTPD"
build_image "exim" "exim" "subjects/SMTP/Exim"
build_image "live555" "live555" "subjects/RTSP/Live555"
build_image "kamailio" "kamailio" "subjects/SIP/Kamailio"
build_image "forked-daapd" "forked-daapd" "subjects/DAAP/forked-daapd"

# Uncomment and add to build_image function if needed
# build_image "lightftp" "lightftp" "subjects/FTP/LightFTP"
# build_image "bftpd" "bftpd" "subjects/FTP/BFTPD"
# build_image "lighttpd1" "lighttpd1" "subjects/HTTP/Lighttpd1"

