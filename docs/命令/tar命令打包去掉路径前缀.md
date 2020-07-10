1. 所有的文件都在一个文件夹下进行打包：
```bash
$ tar -cpvf <tar-name> -C <dir> <files>
```
**<files>参考的是当前目录**，而不是你要切换的目录，比如这样是**不行**的：

```bash
$ tar -Jpc -f compress.tar.xz -C compress/ services*
```



2. 文件在不同的文件夹下，就需要使用`-r`选项来把文件逐个添加到压缩包中（没有压缩包的话会创建）：
```bash
$ tar -rpf <tar-name> -C <dir-1> <dir-1/file(s)-1>
        ...
$ tar -rpf <tar-name> -C <dir1-N> <dir-N/file(s)-N>
```

