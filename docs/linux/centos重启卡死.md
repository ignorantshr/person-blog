运行`fix_systemd.sh`文件：
```shell
#!/bin/bash

#only centos need fix
if [ ! -f /etc/centos-release ]; then
    echo "this is not centos linux"
    exit 1
fi

#only centos7.0 7.1 7.2 need fix
osversion=`awk '{print $4}' /etc/centos-release`
if [ ${osversion} != '7.0.1406' ] && 
   [ ${osversion} != '7.1.1503' ] && 
   [ ${osversion} != '7.2.1511' ]; then
    echo "this is not centos7.0 7.1 7.2"
    exit 1
fi 

#create rc-local.service
cat > /etc/systemd/system/rc-local.service <<EOF
#  This file is part of systemd.
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.

# This unit gets pulled automatically into multi-user.target by
# systemd-rc-local-generator if /etc/rc.d/rc.local is executable.
[Unit]
Description=/etc/rc.d/rc.local Compatibility
ConditionFileIsExecutable=/etc/rc.d/rc.local
After=network.target

[Service]
Type=forking
ExecStart=/etc/rc.d/rc.local start
TimeoutSec=5
RemainAfterExit=yes
EOF

#modify the default timeout of systemd
sed -i 's/#DefaultTimeoutStopSec=90s/DefaultTimeoutStopSec=30s/g' /etc/systemd/system.conf

systemctl daemon-reload

exit 0
```

