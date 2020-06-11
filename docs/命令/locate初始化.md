安装`locate`：

```shell
yum install -y mlocate
```

新安装的`locate`执行失败：

```shell
locate: can not stat () `/var/lib/mlocate/mlocate.db': No such file or directory
```

需要执行`updatedb`更新数据库。

