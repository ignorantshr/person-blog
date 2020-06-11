目录结构：
`/var/www/html/pub -> /home/pub/ #软连接`

`/home/pub/`下面存放了真正的文件，把`_h5ai`目录放到`/home/pub/`目录下。

```
vi /etc/httpd/conf/httpd.conf
    DirectoryIndex  index.html  index.php  /pub/_h5ai/public/index.php
```


