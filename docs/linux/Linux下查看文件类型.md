## 查看文件

使用`file`命令查看：

```shell
$ touch t{1..2}
$ tar -z -cf test.tar.gz t1 t2
$ file test.tar.gz
test.tar.gz: gzip compressed data, from Unix, last modified: Tue Jul  9 09:33:41 2019
```

## 查看压缩文件类型

[参考问答](https://stackoverflow.com/questions/19120676/how-to-detect-type-of-compression-used-on-the-file-if-no-file-extension-is-spe)

使用`xxd`命令查看压缩文件的前几个字节：

```shell
$ xxd -l 10 test.tar.gz
0000000: 1f8b 0800 f5ee 235d 0003                 ......#]..
```

根据字节判断压缩文件类型：

- Zip（.zip）格式：0x50, 0x4b, 0x03, 0x04（除非为空，那么最后两个是0x05, 0x06 或 0x06, 0x06）
- Gzip（.gz）格式：0x1f, 0x8b, 0x08
- xz（.xz）格式：0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00
- 压缩（.Z）：0x1f, 0x9d
- bzip2（.bz2）格式：0x42, 0x5a, 0x68
- zlib（.zz）格式：没看懂，*file*也检测不出来。不过发现前3个字节都是0x1f，0x9d，0x90，勉强算检测出来吧。

