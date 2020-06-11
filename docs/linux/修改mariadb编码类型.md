### 第一种：

vi /etc/my.conf.d/server.cnf

```
[server]
character_set_server = utf8
[mysqld]
character_set_server = utf8
Service mysql restart
```



### 第二种：

1. 登录MySQL，使用
`SHOW VARIABLES LIKE 'character%';`
查看当前使用的字符集，应该有好几个不是UTF-8格式。

2. 要修改的配置文件位于`/etc/my.cnf.d`目录下：
```
client.cnf
在[client]字段里加入
default-character-set=utf8
server.cnf
在[mysqld]字段里加入
character-set-server=utf8
```