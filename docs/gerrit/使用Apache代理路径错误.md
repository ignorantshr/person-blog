使用Apache代理gerrit的情况下，使用test/test这种带有‘/’的路径会有404错误。

在 `/etc/httpd/config.d/gerrit.config` 修改如下：

> AllowEncodedSlashes On
> 
> ProxyPass / http://192.168.216.12:8082/ nocanon

Nocanon会禁止字符转义，直接传递给后台
