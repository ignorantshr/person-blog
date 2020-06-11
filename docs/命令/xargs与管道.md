管道是将前者的标准输出作为后者的**标准输入**。

xargs 是给命令传递参数的一个过滤器，也是组合多个命令的一个工具。可以将管道或标准输入（stdin）数据转换成**命令行参数**，也能够从文件的输出中读取数据。一般和管道一起使用。

xargs[教程](https://www.runoob.com/linux/linux-comm-xargs.html)

对于不支持管道的命令，可以使用xargs来为其提供参数：

```bash
[root@dev vitest]# find . -perm /7000 | xargs ls -l
-rwSr--r-- 1 root root 18 Oct 10 14:15 ./num1
```

此法也可以解决命令行参数过长的问题：

```shell
[root@dev vitest]# find / -type f 2> /dev/null | xargs -n 10 grep -l '\*' 2> /dev/null
```

