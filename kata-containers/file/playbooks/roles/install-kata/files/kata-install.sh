#!/bin/bash

source /etc/os-release

ARCH=$(arch)
BRANCH="${BRANCH:-master}"
sudo -E yum-config-manager --add-repo "http://download.opensuse.org/repositories/home:/katacontainers:/releases:/${ARCH}:/${BRANCH}/CentOS_${VERSION_ID}/home:katacontainers:releases:${ARCH}:${BRANCH}.repo"
sudo -E yum -y install kata-runtime kata-proxy kata-shim
