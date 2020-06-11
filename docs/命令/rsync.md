[TOC]

参考：https://www.linuxtechi.com/rsync-command-examples-linux/

## 简易配置

使用`rsync damon`模式的配置：

```
# /etc/rsyncd: configuration file for rsync daemon mode

# See rsyncd.conf man page for more options.

# configuration example:

uid = nobody
gid = nobody
# use chroot = yes
# max connections = 4
# pid file = /var/run/rsyncd.pid
# exclude = lost+found/
# transfer logging = yes
# timeout = 900
# ignore nonreadable = yes
# dont compress   = *.gz *.tgz *.zip *.z *.Z *.rpm *.deb *.bz2
log file = /var/log/rsyncd.log

[ovirt_iso]
path = /home/pub/ovirt_iso/
# [ftp]
#        path = /home/ftp
#        comment = ftp export area
```

详细配置执行：`man rsyncd.conf`

然后服务端以daemon的形式运行：`rsync --daemon`，同时检查防火墙端口是否开放。客户端即可通过rsync进行远程同步。

## 语法

```bash
# 本地同步语法： 
rsync [OPTION...] SRC... [DEST]

# 通过远程shell同步： 
# 拉取 
rsync [OPTION...] [USER@]HOST:SRC... [DEST]
# 推送
rsync [OPTION...] SRC... [USER@]HOST:DEST

# 通过rsync daemon同步： 
# 拉取
rsync [OPTION...] [USER@]HOST::SRC... [DEST]
rsync [OPTION...] rsync://[USER@]HOST[:PORT]/SRC... [DEST]
# 推送
rsync [OPTION...] SRC... [USER@]HOST::DEST
rsync [OPTION...] SRC... rsync://[USER@]HOST[:PORT]/DEST

# 只有SRC没有DEST会列出文件而不会复制
```

常用选项：

| 选项                | 说明                                           |
| ------------------- | ---------------------------------------------- |
| -v, –verbose        | 详细输出                                       |
| -q, –quiet          | 抑制信息输出                                   |
| -a, –archive        | 同步的时候归档文件和文件夹 ( -a 等于 -rlptgoD) |
| -r, –recursive      | 递归地同步文件和文件夹                         |
| -b, –backup         | 在同步时进行备份                               |
| -u, –update         | 如何目标文件比源文件更新那么不要从源复制文件   |
| -l, –links          | 在同步时将符号链接复制为符号链接               |
| -n, –dry-run        | 执行非同步的试运行，可以用来预先查看效果       |
| -e, –rsh=COMMAND    | 同步时指定远程shell                            |
| -z, –compress       | 在传输过程中压缩文件数据                       |
| -h, –human-readable | 人类友好性地显示输出数字                       |
| -progress           | 在转换时展示同步进程                           |
| -p, --perms         | 保留权限                                       |
| -t, --times         | 保留修改时间                                   |
| -g, --group         | 保留组                                         |
| -o, --owner         | 保留用户（只用于超级用户）                     |
| -D                  | 保留设备文件（只用于超级用户）、特殊文件       |

## 示例

### 同步文件夹

```shell
$ ll test_lv1/
total 20
-rw-r--r-- 1 root root 1575 May 10 16:07 echo.py
-rwxr-xr-x 1 root root 8592 May 13 19:37 test
-rw-r--r-- 1 root root  156 May 13 19:38 test.c
drwxr-xr-x 2 root root   18 May 14 10:50 ttt
```

当目录末尾不带`/`时，会复制文件夹：

```shell
$ rsync -avh test_lv1 test_lv2/

$ ll test_lv2/
total 0
drwxr-xr-x 3 root root 58 May 14 10:50 test_lv1
```

带`/`时，只会复制文件夹中的内容：

```shell
$ rsync -avh test_lv1/ test_lv2/

$ ll test_lv2/
total 20
-rw-r--r-- 1 root root 1575 May 10 16:07 echo.py
-rwxr-xr-x 1 root root 8592 May 13 19:37 test
-rw-r--r-- 1 root root  156 May 13 19:38 test.c
drwxr-xr-x 2 root root   18 May 14 10:50 ttt
```

### 同步目录结构

使用`-f`选项添加过滤规则实现：

```shell
$ rsync -avh -f "+ */" -f "- *" test_lv1 test_lv2/

$ tree test_lv1/
test_lv1/
├── echo.py
├── test
├── test.c
└── ttt
    └── tstg
    
$ tree test_lv2/
test_lv2/
└── test_lv1
    └── ttt
```

### 恢复scp文件传输

如果因为某些原因在使用scp传输文件时停止/终止了，那么可以使用rsync从停止/终止的地方继续复制文件。

使用`-P`（--partial --progress）选项实现：

```shell
$ scp root@192.168.1.29:/root/ubuntu-18.04-desktop-amd64.iso /opt
root@192.168.1.29's password:
ubuntu-18.04-desktop-amd64.iso                   28%  526MB  61.5MB/s   00:21 ETA
^CKilled by signal 2.

$ rsync -P --rsh=ssh root@192.168.1.29:/root/ubuntu-18.04-desktop-amd64.iso /opt
root@192.168.1.29's password:
ubuntu-18.04-desktop-amd64.iso
  1,921,843,200 100%   18.47MB/s    0:01:39 (xfr#1, to-chk=0/1)
```

### 在目标中删除源中不存在的文件

使用`--delete`选项

### 限制文件传输大小

超过大小限制的文件不予转换。使用`--max-size`选项。

```shell
$ rsync -avz --max-size='500K' /opt/rpms_db root@192.168.1.28:/tmp
```

单位：

> 1024进制："K"  (or  "KiB") is a kibibyte  (1024), "M" (or "MiB") is a mebibyte (1024\*1024),  and  "G"  (or   "GiB")  is  a gibibyte (1024\*1024*1024).
>
> 1000进制："KB",  "MB",  or  "GB"

### 不予复制修改过的目标文件

不覆盖在目标中修改过文件。使用`-u`选项

### 同步完成后删除源文件

使用`--remove-source-files`选项

### 限制传输速率

使用`–bwlimit=<KB/s>`选项：

```shell
$ rsync -avz --progress --bwlimit=600 /home/pkumar/techi root@192.168.1.29:/opt
```

### 显示源与目标的不同

使用`-i`选项

```shell
$ tree test_lv1/
test_lv1/
├── echo.py
├── test
├── test.c
└── ttt
    └── tstg

$ tree sdb1/
sdb1/
└── echo.py

$ rsync -ani test_lv1/ sdb1/
.d..t...... ./
>f+++++++++ test
>f+++++++++ test.c
cd+++++++++ ttt/
>f+++++++++ ttt/tstg
```

输出的十一个字符长度代表的是：`YXcstpoguax`。

- Y：指示将要完成的更新类型
    - <：表示文件将被转换到远程主机
    - \>：表示文件将被转换到本地
    - c：意味着本地将会发生一个改变/创建
    - h：表示是一个硬链接（要求有选项`--hard-links`）
    - .：表示不会更新
    - *：表示剩下的字符包含一个信息。例如"deleting"。
- X：指示文件类型
    - f：文件
    - d：文件夹
    - L：链接
    - D：设备
    - S：特殊文件。比如socket。

剩下的字符显示：属性值没有更新显示`.`；有更新则显示：

- 新创建的条目每个都显示`+`
- 相同的条目用空格` `代替`.`
- 未知的属性每个都用`?`代替

相关的属性如下：

- c：表示常规文件具有不同的校验和（要求`--checksum`选项）或者是链接、设备或特殊文件的值已更改。
- s：表示文件的大小不一致，将会更新
- t：表示更改时间不同，将会更新成发送方的时间（要求`--times`选项）
- p：表示权限不同，将更新成发送方的权限（要求`--perms`选项）
- o：表示所属用户不同，将更新成发送方的用户（要求`--owner`选项和超级用户权限）
- g：表示所属组不同，将更新成发送方的组（要求`--group`选项和设置组的权限）
- u：保留做未来使用
- a：ACL信息改变了
- x：表示扩展属性信息改变了