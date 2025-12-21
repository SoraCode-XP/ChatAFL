#!/bin/bash

# 设置缓存模式 - 默认使用缓存，除非明确指定不使用
# 可以通过环境变量 NO_CACHE=1 来禁用缓存
# 在debug模式下，建议使用缓存以加快构建速度
if [ "$NO_CACHE" = "1" ]; then
    export NO_CACHE="--no-cache"
    echo "禁用Docker构建缓存"
else
    export NO_CACHE=""
    echo "启用Docker构建缓存"
fi

# 优化构建参数 - 使用所有可用的CPU核心
export MAKE_OPT="-j$(nproc)"

# 并行构建函数
build_image_parallel() {
    local target=$1
    local image_name=$2
    local path=$3

    # 如果BUILD_TARGET未设置或匹配目标，则构建镜像
    if [ -z "$BUILD_TARGET" ] || [ "$BUILD_TARGET" = "$target" ]; then
        echo "Building image: ${IMAGE_PREFIX}${image_name}"
        cd $PFBENCH
        cd $path
        docker build . -t ${IMAGE_PREFIX}${image_name} --build-arg MAKE_OPT=$MAKE_OPT $NO_CACHE &
        BUILD_PIDS="$BUILD_PIDS $!"
    else
        echo "Skipping image: ${IMAGE_PREFIX}${image_name} (not in BUILD_TARGET)"
    fi
}

# 等待所有后台构建进程完成
wait_for_builds() {
    for pid in $BUILD_PIDS; do
        if wait $pid; then
            echo "构建进程 $pid 成功完成"
        else
            echo "构建进程 $pid 失败"
            exit 1
        fi
    done
}

# 清理变量
BUILD_PIDS=""

# 根据BUILD_TARGET并行构建镜像
build_image_parallel "proftpd" "proftpd" "subjects/FTP/ProFTPD"
build_image_parallel "pure-ftpd" "pure-ftpd" "subjects/FTP/PureFTPD"
build_image_parallel "exim" "exim" "subjects/SMTP/Exim"
build_image_parallel "live555" "live555" "subjects/RTSP/Live555"
build_image_parallel "kamailio" "kamailio" "subjects/SIP/Kamailio"
build_image_parallel "forked-daapd" "forked-daapd" "subjects/DAAP/forked-daapd"

# 等待所有构建完成
wait_for_builds

# 可选：添加额外的镜像构建
# build_image_parallel "lightftp" "lightftp" "subjects/FTP/LightFTP"
# build_image_parallel "bftpd" "bftpd" "subjects/FTP/BFTPD"
# build_image_parallel "lighttpd1" "lighttpd1" "subjects/HTTP/Lighttpd1"

echo "所有镜像构建完成"
