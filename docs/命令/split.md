分割命令。

```
split [OPTION]... [INPUT [PREFIX]]
Output fixed-size pieces of INPUT to PREFIXaa, PREFIXab, ...; default
size is 1000 lines, and default PREFIX is 'x'.  With no INPUT, or when INPUT
is -, read standard input.

-a, --suffix-length=N   生成长度为 N (default 2)的后缀
--additional-suffix=SUFFIX  附加额外的后缀
-b, --bytes=SIZE        按每个输出文件的大小分割。单位：K, M, G, T, P, E, Z, Y (powers of 1024) or KB, MB, ... (powers of 1000)
-d, --numeric-suffixes[=FROM]  使用数字代替字母作为后缀；FROM 是起始数字(default 0)
-l, --lines=NUMBER      按每个输出文件的行数分割
-n, --number=CHUNKS     生成 CHUNKS 个输出文件
```

```bash
[root@dev vitest]# ll man_db.conf
-rw-r--r-- 1 sink sink 4561 Oct  8 14:06 man_db.conf
# 根据文件大小分割
[root@dev vitest]# split -a 1 --additional-suffix x -b 1K -d man_db.conf man_db.conf
[root@dev vitest]# ll man_db.conf??
-rw-r--r-- 1 root root 1024 Oct 10 14:35 man_db.conf0x
-rw-r--r-- 1 root root 1024 Oct 10 14:35 man_db.conf1x
-rw-r--r-- 1 root root 1024 Oct 10 14:35 man_db.conf2x
-rw-r--r-- 1 root root 1024 Oct 10 14:35 man_db.conf3x
-rw-r--r-- 1 root root  465 Oct 10 14:35 man_db.conf4x
# 根据文件数量分割
[root@dev vitest]# split -a 1 -n 4 -d man_db.conf man_db.conf
[root@dev vitest]# ll man_db.conf?
-rw-r--r-- 1 root root 1140 Oct 10 14:40 man_db.conf0
-rw-r--r-- 1 root root 1140 Oct 10 14:40 man_db.conf1
-rw-r--r-- 1 root root 1140 Oct 10 14:40 man_db.conf2
-rw-r--r-- 1 root root 1141 Oct 10 14:40 man_db.conf3
```

使用流来复原文件:

```bash
[root@dev vitest]# cat man_db.conf? >> man_db.conf.bak
[root@dev vitest]# diff man_db.conf man_db.conf.bak
```

