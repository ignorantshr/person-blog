低版本的vdsm在高版本的libvirt环境下编译安装之后启动失败，因为libvirt的验证方式发生了改变，所以在配置文件中写入以下配置：
```shell
#libvirt configure
echo '
mech_list: digest-md5
sasldb_path: /etc/libvirt/passwd.db ' > /etc/sasl2/libvirt.conf
```
重启libvirt服务；重新生成vdsm验证配置文件
