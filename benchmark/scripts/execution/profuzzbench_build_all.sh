#!/bin/bash

export NO_CACHE="--no-cache"
export MAKE_OPT="-j4"

#cd $PFBENCH
#cd subjects/FTP/LightFTP
#docker build . -t lightftp --build-arg MAKE_OPT $NO_CACHE
#
#cd $PFBENCH
#cd subjects/FTP/BFTPD
#docker build . -t bftpd --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/FTP/ProFTPD
docker build . -t ${IMAGE_PREFIX}proftpd --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/FTP/PureFTPD
docker build . -t ${IMAGE_PREFIX}pure-ftpd --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/SMTP/Exim
docker build . -t ${IMAGE_PREFIX}exim --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/RTSP/Live555
docker build . -t ${IMAGE_PREFIX}live555 --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/SIP/Kamailio
docker build . -t ${IMAGE_PREFIX}kamailio --build-arg MAKE_OPT $NO_CACHE

cd $PFBENCH
cd subjects/DAAP/forked-daapd
docker build . -t ${IMAGE_PREFIX}forked-daapd --build-arg MAKE_OPT $NO_CACHE

#cd $PFBENCH
#cd subjects/HTTP/Lighttpd1
#docker build . -t lighttpd1 --build-arg MAKE_OPT $NO_CACHE

