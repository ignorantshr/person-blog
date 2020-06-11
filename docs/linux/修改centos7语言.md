# 修改centos7语言

* 临时解决方法
使用LANG=“zh_CN.UTF-8”，这个命令来实现，不过在重新登录的时候又会变回英文。这个不是长久的方法。

* 更改系统参数
如果系统没有中文支持，可以通过网上下载安装中文语言包,使用命令：
```shell
yum groupinstall Chinese-support

vim /etc/locale.conf
```

进入以后只有简单的一句LANG="en_US.UTF-8" 这个配置，把"en_US.UTF-8"替换成"zh_CN.UTF-8" 保存退出即可。
