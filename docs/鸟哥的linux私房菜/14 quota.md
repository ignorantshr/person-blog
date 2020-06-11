quota（配额）：设置linux下的磁盘使用配额。

quota使用限制：

1. EXT 文件系统仅针对整个文件系统

    > ext 在进行quota限制的时候不能对单一目录进行设置。

2. 内核必须支持 quota

3. 只对一般用户有效。root不能，因为它拥有几乎整个系统的数据。

4. 若启用了 selinux，不是所有的目录都可设置 quota。

XFS 的主要限制项：

1. 针对用户（user）、群组（group）或目录（project）进行限制。后两者不能共存。

2. 通过限制 inode 来限制文件数量；通过限制 block 来限制磁盘容量

3. soft、hard限制

    > soft 是产生警告的界限，hard 是不能再使用磁盘的界限（比soft高），inode/block都可以设置。位于两者之间时，每次用户登录都会发出警告，若在宽限时间（grace time）内不予处理，那么将soft值将取代hard值作为quota的限制，此时就无法再新增文件了。

4. 宽限时间

## 实验(XFS)

### 搭建实验环境

```bash
# 先创建实验用的用户
[root@dev ~]# groupadd qus
[root@dev ~]# for i in {1..3}; do useradd -g qus qu$i; echo "123" | passwd --stdin qu$i; done
Changing password for user qu1.
passwd: all authentication tokens updated successfully.
Changing password for user qu2.
passwd: all authentication tokens updated successfully.
Changing password for user qu3.
passwd: all authentication tokens updated successfully.

# 检查文件系统
[root@dev tmp]# df -hT disk-sdb/
Filesystem     Type  Size  Used Avail Use% Mounted on
/dev/sdb       xfs   2.0G   33M  2.0G   2% /tmp/disk-sdb
# xfs 开启 quota 必须在 /etc/fstab 中写入或在挂载之初就指明，否则不会生效（remount无用）
[root@dev tmp]# vim /etc/fstab
……
/dev/sdb     /disk-sdb                  xfs    defaults,usrquota,grpquota        0 0
# 这里一定要先卸载才能生效
[root@dev tmp]# umount disk-sdb/
[root@dev tmp]# mount -a
[root@dev tmp]# mount | grep sdb
/dev/sdb on /tmp/disk-sdb type xfs (rw,relatime,seclabel,attr2,inode64,usrquota,grpquota)
[root@dev tmp]# cd disk-sdb/
[root@dev disk-sdb]# chgrp qus quota-test/; chmod 2770 quota-test/; ll -d quota-test
drwxrws---. 2 root qus 6 Oct 22 03:18 quota-test
```

`/etc/fstab`的字段中针对 quota 的限制主要有以下几种：

1. uquota/usrquota/quota：针对账号的设定
2. gquota/grpquota：针对群组的设定
3. pquota/prjquota：针对单一目录的设定，**不能与群组的设定并存**

### 通用命令

```
xfs_quota -x -c cmd ... [ -d project ] ... [ path ... ]
管理 XFS 文件系统上的配额使用

-x        启用专家模式。只有在此模式下才可使用 ADMINISTRATOR COMMANDS
-c cmd    非交互模式执行命令。可以指定多个 -c 命令，按顺序执行。

下列命令的选项说明统一适用于所有同样的选项
			-h：	human-readable
			-b：	blocks used
			-i： inodes used
			-r：	realtime blocks used
			-f：	输出到文件
```

### 信息获取命令

```
USER COMMANDS：只有信息获取命令中有该种指令，其它的都是ADMINISTRATOR COMMANDS
	print
	quota [ -g | -p | -u ] [ -bir ] [ -h ] [ -f file ] [ ID | name ] ...		显示单个用户的使用与限制。
	df/free [ -bir ] [ -h ] [ -f file ]											报告fs使用情况，与df(1)工具类似（但是更快）
	
ADMINISTRATOR COMMANDS：
	report [ -gpu ] [ -bir ] [ -ahntlLNU ] [ -f file ]							报告fs的quota信息，默认是block
	state [ -gpu ] [ -av ] [ -f file ]											报告总体quota状态信息。
```

```bash
[sink@dev study]$ xfs_quota -x -c "print"
Filesystem          Pathname
/                   /dev/mapper/cl-root
/boot               /dev/sda1
/tmp/disk-sdb       /dev/sdb (uquota, gquota)
[sink@dev study]$ xfs_quota -x -c "df -h" /tmp/disk-sdb/
Filesystem     Size   Used  Avail Use% Pathname
/dev/sdb       2.0G  32.2M   2.0G   2% /tmp/disk-sdb
[sink@dev tmp]$ xfs_quota -x -c "state"
User quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #67 (2 blocks, 2 extents)
Group quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #68 (2 blocks, 2 extents)
Project quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: OFF
  Enforcement: OFF
  Inode: #68 (2 blocks, 2 extents)
Blocks grace time: [7 days]
Inodes grace time: [7 days]
Realtime Blocks grace time: [7 days]
```

### 限制命令

```
ADMINISTRATOR COMMANDS
	limit [ -g | -p | -u ] bsoft=N | bhard=N | isoft=N | ihard=N | rtbsoft=N | rtbhard=N -d | id | name						   设置 block limits (bhard/bsoft), inode count limits (ihard/isoft) and/or realtime block limits (rtbhard/rtbsoft)。-d 表示对默认的对象进行设置，否则使用指定的 user/group/project
	timer [ -g | -p | -u ] [ -bir ] value		设置宽限时间。单位默认是 秒，其它可识别单位：'minutes', 'hours', 'days', and 'weeks'，可以使用简写形式
```

```bash
# 限制 inode
[root@dev tmp]# xfs_quota -x -c "limit -u isoft=5 ihard=10 qu1" disk-sdb/
# 限制 block
[root@dev tmp]# for i in {1..3}; do xfs_quota -x -c "limit -u bsoft=10M bhard=20M qu$i" disk-sdb/; done
# 限制 群组
[root@dev tmp]# xfs_quota -x -c "limit -g bsoft=20M bhard=50M qus" disk-sdb/
[root@dev tmp]# xfs_quota -x -c "report -u -bih" /tmp/disk-sdb/
User quota on /tmp/disk-sdb (/dev/sdb)
                        Blocks                            Inodes
User ID      Used   Soft   Hard Warn/Grace     Used   Soft   Hard Warn/Grace
---------- --------------------------------- ---------------------------------
root            0      0      0  00 [0 days]      4      0      0  00 [------]
qu1           20M    10M    20M  00 [2 days]      1      5     10  00 [------]
qu2             0    10M    20M  00 [------]      0      0      0  00 [------]
qu3             0    10M    20M  00 [------]      0      0      0  00 [------]
# 调整宽限时间
[root@dev tmp]# xfs_quota -x -c "timer -g -b 3d" disk-sdb/
[root@dev tmp]# xfs_quota -x -c "state -gu"
User quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #67 (2 blocks, 2 extents)
Group quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #68 (2 blocks, 2 extents)
Blocks grace time: [3 days]
Inodes grace time: [7 days]
Realtime Blocks grace time: [7 days]
```

```bash
# 限制测试
# 超过 soft 界限
[qu1@dev quota-test]$ dd if=/dev/zero of=qu1-f bs=1M count=11
11+0 records in
11+0 records out
11534336 bytes (12 MB) copied, 0.0320814 s, 360 MB/s
# 超过 hard 界限
[qu1@dev quota-test]$ dd if=/dev/zero of=qu1-f bs=1M count=22
dd: error writing ‘qu1-f’: Disk quota exceeded
21+0 records in
20+0 records out
20971520 bytes (21 MB) copied, 0.128578 s, 163 MB/s
[qu1@dev quota-test]$ ll -h
total 20M
-rw-r--r--. 1 qu1 qus 20M Oct 22 04:24 qu1-f
# qu2也创建一个
[qu2@dev quota-test]$ dd if=/dev/zero of=qu2-f bs=1M count=20
# qu3再创建时已经达到了群组的限制
[qu3@dev quota-test]$ dd if=/dev/zero of=qu3-f bs=1M count=20
dd: error writing ‘qu3-f’: Disk quota exceeded
11+0 records in
10+0 records out
10485760 bytes (10 MB) copied, 0.0137186 s, 764 MB/s
# 开始倒计时
[root@dev tmp]# xfs_quota -x -c "report -ug -bih" /tmp/disk-sdb/
User quota on /tmp/disk-sdb (/dev/sdb)
                        Blocks                            Inodes
User ID      Used   Soft   Hard Warn/Grace     Used   Soft   Hard Warn/Grace
---------- --------------------------------- ---------------------------------
root            0      0      0  00 [0 days]      4      0      0  00 [------]
qu1           20M    10M    20M  00 [2 days]      1      5     10  00 [------]
qu2           20M    10M    20M  00 [2 days]      1      0      0  00 [------]
qu3           10M    10M    20M  00 [------]      1      0      0  00 [------]

Group quota on /tmp/disk-sdb (/dev/sdb)
                        Blocks                            Inodes
Group ID     Used   Soft   Hard Warn/Grace     Used   Soft   Hard Warn/Grace
---------- --------------------------------- ---------------------------------
root            0      0      0  00 [0 days]      3      0      0  00 [------]
qus           50M    20M    50M  00 [2 days]      4      0      0  00 [------]
```

#### project的限制设置

```
# 命令
project [ -cCs [ -d depth ] [ -p path ] id | name ]
	-c			检查树的设置
	-s			初始化设置树
	-C			递归地清除限制
	-d			限制递归层级。-1 是无限的，0 是顶层的，1 是第一级， 以此类推...
	-p			在命令行指定 project 的路径，而不是通过 /etc/projects 文件。但也会读取配置文件
```

由于grpquota与prjquota不能同时存在，所以需要先卸载再重新挂载。

```bash
[root@dev ~]# umount /dev/sdb
[root@dev ~]# vi /etc/fstab
/dev/sdb     /tmp/disk-sdb                xfs    defaults,usrquota,prjquota        0 0
[root@dev ~]# mount -a

[sink@dev tmp]$ xfs_quota -x -c "state"
User quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #67 (2 blocks, 2 extents)
Group quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: OFF
  Enforcement: OFF
  Inode: #68 (2 blocks, 2 extents)
Project quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  Enforcement: ON
  Inode: #68 (2 blocks, 2 extents)
Blocks grace time: [3 days]
Inodes grace time: [7 days]
Realtime Blocks grace time: [7 days]
```

project的限制需要用到两个配置文件：

```
/etc/projects       Mapping of numeric project identifiers to directories trees.
/etc/projid         Mapping of numeric project identifiers to project names.
```



```bash
# id、project-name都是自己指定
[root@dev ~]# echo 1:/tmp/disk-sdb/quota-test >> /etc/projects
[root@dev ~]# echo quota-test-files:1 >> /etc/projid
# 初始化
[root@dev ~]# xfs_quota -x -c "project -s quota-test-files" /tmp/disk-sdb/
Setting up project quota-test-files (path /tmp/disk-sdb/quota-test)...
Processed 1 (/etc/projects and cmdline) paths for project quota-test-files with recursion depth infinite (-1).
[root@dev ~]# xfs_quota -x -c "limit -p bsoft=2M bhard=10M 1" /tmp/disk-sdb/
[root@dev ~]# xfs_quota -x -c "report -p -h" /tmp/disk-sdb/
Project quota on /tmp/disk-sdb (/dev/sdb)
                        Blocks
Project ID   Used   Soft   Hard Warn/Grace
---------- ---------------------------------
#0              0      0      0  00 [------]
quota-test-files    30M     2M    10M  00 [-none-]
# 可以看到 /tmp/disk-sdb/quota-test 已经超出限制了
[root@dev ~]# xfs_quota -x -c "df -h" /tmp/disk-sdb/
Filesystem     Size   Used  Avail Use% Pathname
/dev/sdb       2.0G  62.2M   1.9G   3% /tmp/disk-sdb
/dev/sdb         2M    30M 8192.0E 1500% /tmp/disk-sdb/quota-test
# 检查树的设置
# 已设置
[root@dev ~]# xfs_quota -x -c "project -c 1" /tmp/disk-sdb/
Checking project 1 (path /tmp/disk-sdb/quota-test)...
Processed 1 (/etc/projects and cmdline) paths for project 1 with recursion depth infinite (-1).
# 未设置
[root@dev ~]# xfs_quota -x -c "project -c 2" /tmp/disk-sdb/
Processed 0 (/etc/projects and cmdline) paths for project 2 with recursion depth infinite (-1).

# 直接在命令行配置树，但是许多命令都检测不到
[root@dev ~]# xfs_quota -x -c 'project -s -p /tmp/disk-sdb/quota-test2 3' /tmp/disk-sdb/
[root@dev disk-sdb]# xfs_quota -x -c "limit -p bhard=30M 3" /tmp/disk-sdb/
[root@dev quota-test2]# cp qu1-f qu1-f-1
cp: error writing ‘qu1-f-1’: No space left on device
cp: failed to extend ‘qu1-f-1’: No space left on device
[root@dev quota-test2]# ll -h
total 30M
-rw-r--r--. 1 root root 20M Oct 23 10:29 qu1-f
-rw-r--r--. 1 root root 10M Oct 23 10:29 qu1-f-1
# 检测
[root@dev quota-test2]# xfs_quota -x -c "df -bh" /tmp/disk-sdb/
Filesystem     Size   Used  Avail Use% Pathname
/dev/sdb       2.0G  92.4M   1.9G   5% /tmp/disk-sdb
/dev/sdb         2M    30M 8192.0E 1500% /tmp/disk-sdb/quota-test
[root@dev quota-test2]# xfs_quota -x -c "print" /tmp/disk-sdb/
Filesystem          Pathname
/tmp/disk-sdb       /dev/sdb (uquota, pquota)
/tmp/disk-sdb/quota-test /dev/sdb (project 1, quota-test-files)
[root@dev quota-test2]# xfs_quota -x -c "project -c 3" /tmp/disk-sdb/
Processed 0 (/etc/projects and cmdline) paths for project 3 with recursion depth infinite (-1).
# 必须指定路径才可
[root@dev quota-test2]# xfs_quota -x -c "project -c -p /tmp/disk-sdb/quota-test2 3" /tmp/disk-sdb/
Checking project 3 (path /tmp/disk-sdb/quota-test2)...
Processed 1 (/etc/projects and cmdline) paths for project 3 with recursion depth infinite (-1).
```

### 取消限制命令

```
disable [ -gpu ] [ -v ]				暂时关闭 quota 的限制
enable [ -gpu ] [ -v ]				打开 quota 的限制
off [ -gpu ] [ -v ]					永久关闭 quota 的限制。只能umount之后再次mount恢复
remove [ -gpu ] [ -v ]				删除 quota 的限制。只能在 off 状态下执行
```

```bash
# 暂时关闭
[root@dev tmp]# xfs_quota -x -c "disable -p" /tmp/disk-sdb
[root@dev tmp]# xfs_quota -x -c "state" /tmp/disk-sdb
……
Project quota state on /tmp/disk-sdb (/dev/sdb)
  Accounting: ON
  # 这里是关闭状态
  Enforcement: OFF
  Inode: #68 (2 blocks, 2 extents)
……
# 彻底关闭
[root@dev tmp]# xfs_quota -x -c "off -p" /tmp/disk-sdb
[root@dev tmp]# xfs_quota -x -c "state" /tmp/disk-sdb
……
Project quota state on /tmp/disk-sdb (/dev/sdb)
# 全部是关闭状态
  Accounting: OFF
  Enforcement: OFF
  Inode: #68 (2 blocks, 2 extents)
……
# 报告也没有任何结果
[root@dev tmp]# xfs_quota -x -c "report -pbh" /tmp/disk-sdb

# 此时再删除策略，结果居然报错，而且id删不掉！！！，真是折磨！！！ FIXME
[root@dev tmp]# xfs_quota -x -c 'remove -p' /tmp/disk-sdb
XFS_QUOTARM: Invalid argument
```

## 实验(EXT4)

大致流程

```bash
# 使用前文的批量创建账户脚本创建用户
[root@centos-server-1 users]# groupadd friends
[root@centos-server-1 users]# cat accounts
friend1
friend2
friend3
[root@centos-server-1 users]# sh users.sh create friends
# 编辑配置文件
[root@centos-server-1 users]# cat /etc/fstab | grep home
# 注意，ext4 不支持uquota等简写
/dev/mapper/cl_server-myhome /home                   ext4    defaults,usrquota,grpquota        1 2
# 重新挂载
[root@centos-server-1 users]# umount /home/ ; mount -a
[root@centos-server-1 users]# mount | grep home
/dev/mapper/cl_server-myhome on /home type ext4 (rw,relatime,seclabel,quota,usrquota,grpquota,data=ordered)
# 制作 Quota 数据文件,并启动 Quota 支持
[root@centos-server-1 ~]# quotacheck -v -a -ug
# 输出只要有这一行就OK了
quotacheck: Scanning /dev/mapper/cl_server-myhome [/home] done
[root@centos-server-1 users]# quotaon -avug
/dev/mapper/cl_server-myhome [/home]: group quotas turned on
/dev/mapper/cl_server-myhome [/home]: user quotas turned on
# 设置限额，单位是KB
[root@centos-server-1 users]# edquota -u friend1
Disk quotas for user friend1 (uid 1000):
# soft 1.8 GB，hard 2 GB
  Filesystem                   blocks       soft       hard     inodes     soft     hard
  /dev/mapper/cl_server-myhome         16          1800000          200000          4        0        0
# 复制
[root@centos-server-1 users]# edquota -p friend1 friend2
[root@centos-server-1 users]# edquota -p friend1 friend3
# 查看
[root@centos-server-1 users]# repquota -a
*** Report for user quotas on device /dev/mapper/cl_server-myhome
Block grace time: 7days; Inode grace time: 7days
                        Block limits                File limits
User            used    soft    hard  grace    used  soft  hard  grace
----------------------------------------------------------------------
root      --      24       0       0              3     0     0
friend1   --      16 1800000  200000              4     0     0
friend2   --      16 1800000  200000              4     0     0
friend3   --      16 1800000  200000              4     0     0
```



## XFS与EXT对比

|      设置流程项目       |            **XFS**文件系统             |     **EXT**家族     |
| :---------------------: | :------------------------------------: | :-----------------: |
|   /etc/fstab参数设置    |       usrquota/grpquota/prjquota       |  usrquota/grpquota  |
|     quota 配置文件      |                   无                   |     quotacheck      |
|   设置用户/群组限制值   |   xfs_quota -x -c "limit -u -g ..."    | edquota 或 setquota |
|     设置 grace time     |       xfs_quota -x -c "timer..."       |       edquota       |
|     设置目录限制值      |     xfs_quota -x -c "limit -p ..."     |         无          |
|        观察报告         |      xfs_quota -x -c "report..."       |  repquota 或 quota  |
| 启 动与 关闭 quota 限制 | xfs_quota -x -c "[disable\|enable]..." |  quotaoff, quotaon  |
|    发送警告信给用 户    |           目 前版本尚未支持            |      warnquota      |

实际上，后者的命令对于quota的限制几乎是对所有的FS都是通用的。具体的使用方法：https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/storage_administration_guide/ch-disk-quotas

## 不改变现有的 quota 情况下对其它路径添加限制

该做法的前提是有独立的分区，比如 /home。

情景：若某一目录在规划的时候并未考虑对其做出限制，后来又想对其设置 quota。

解决方法是将该目录移动到设置了 quota 的独立分区下，然后做一个软链接，再对独立分区进行配置。

!!! note
	在移动目录的时候可能需要更改 SELinux。