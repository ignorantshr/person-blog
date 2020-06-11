```shell
yum install -y epel-release
```

libvirt_storage_backend_rbd.so缺失
缺失的链接库文件在librbd1包里，解决方法如下：

```shell
yum update librbd1
systemctl restart libvirtd
```

