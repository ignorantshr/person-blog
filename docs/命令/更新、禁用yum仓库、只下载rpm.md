*	更新
```shell
createrepo --update-md-path=/home/pub/vms-sec/rpm/ /home/pub/vms-sec/rpm/
```
*	禁用、开启
```shell
yum-config-manager --disable base epel epel/x86_64/metalink extras updates
yum-config-manager --enable base epel epel/x86_64/metalink extras updates
```

*	只下载不安装rpm
```shell
yum install --downloadonly --downloaddir=/tmp <package-name>
# 需要先安装yum-utils
yumdownloader --resolve --downloadonly --downloaddir=/tmp <package-name>
```

- 查看仓库下所有的rpm

```shell
# 列出所有仓库的rpm
yum list
# 列出某一仓库的rpm
yum repo-pkgs <repo-name> list
```

