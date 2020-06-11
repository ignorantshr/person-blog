### nginx 启动失败,日志里面报错信息如下:

> Starting nginx: nginx: [emerg] bind() to 0.0.0.0:8095 failed (13: Permission denied)

权限拒绝，经检查发现是开启selinux 导致的。 直接关闭。

 `getenforce` 这个命令可以查看当前是否开启了selinux 如果输出 `disabled` 或 `permissive` 那就是关闭了，
如果输出 `enforcing` 那就是开启了 selinux。

### 临时关闭selinux
* setenforce 0    ##设置SELinux 成为permissive模式
* setenforce 1    ##设置SELinux 成为enforcing模式

### 永久关闭selinux,
* 修改/etc/selinux/config 文件
* 将SELINUX=enforcing改为SELINUX=disabled
* 重启机器即可

Tags: selinux , nginx绑定端口失败
