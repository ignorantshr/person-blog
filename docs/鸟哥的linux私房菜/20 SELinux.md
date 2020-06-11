[red_hat selinux_users_and_administrators_guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html-single/selinux_users_and_administrators_guide/index)

## 为什么要使用SELinux

当某个进程想要对文件进行存取时，系统就会根据该进程的拥有者/群组，并比对文件的权限，若通过权限检查，就可以存取该文件了。这种存取文件系统的方式被称为**自主访问控制 (Discretionary Access Control， DAC)**。

Linux系统中有两种自主访问控制策略，一种是9位权限码（User-Group-Other），另一种是访问控制列表ACL（Access Control List）。缺陷就是：

1. root 具有最高的权限
2. 使用者可以变更文件资源的访问权限

总之，DAC的不足是用户容易随意设置权限。因此 SELinux 导入了**强制访问控制 (Mandatory Access Control，MAC) **的方法。该方式可以针对特定的**进程**与特定的**文件资源**来进行权限的控管。SELinux 是透过 MAC 的方式来控管进程，他控制的*主体*是*进程*，而*目标*则是该进程能否读取的*文件资源*。

![DAC与MAC](img/DAC与MAC.png)

## SELinux 的运作模式

- 主体 (Subject)

  进程或线程。

- 目标 (Object)

  一般就是文件系统。

- 策略 (Policy)

  由于进程与文件数量庞大，会预设一些策略，这些策略内还会有详细的规则 (rule) 来指定不同的服务开放某些资源的存取与否。CentOS 7.x 里面仅有提供三个主要的策略：

	- targeted：针对网络服务限制较多，针对本机限制较少，是预设的策略；
	- minimum：由 target 修订而来，仅针对选择的进程来保护；
	- mls：完整的 SELinux 限制，限制方面较为严格。

- 安全性本文 (security context)

  主体能不能存取目标除了策略指定之外，主体与目标的安全性本文必须一致才能够顺利存取。安全性本文 (security context) 有点类似文件系统的 rwx。

工作流程：

![](./img/SELinux_Decision_Process.png)

!!! important
	先判断的是文件系统的 rwx 权限设定，然后再进入selinux的流程。如果DAC的权限都不满足，那么也不会进行SELinux policy rules的检查了。鸟哥在这里写错了，详见[redhat说明 Note 部分](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html-single/selinux_users_and_administrators_guide/index#chap-Security-Enhanced_Linux-Introduction)

### 安全性本文 (Security Context)

安全性本文存在于主体进程中与目标文件资源中。进程在内存内，安全性本文可以存入内存；文件的安全性本文是放置到文件的 inode 内的。

查看进程的安全性文本：

```bash
[sink@dev ~]$ ps -Z
LABEL                              PID TTY          TIME CMD
unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 18876 pts/5 00:00:00 bash
unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 18923 pts/5 00:00:00 ps
```

查看文件的安全性文本：

```bash
[sink@dev ~]$ ls -Z
-rw-rw-r--. sink sink unconfined_u:object_r:user_home_t:s0 f1
```

内容格式为*SELinux user:role:type:level*，主要分为三个字段：

- SELinux user (Identify)：相当于账号方面的身份识别。类型：
	- unconfined_u：不受限的用户，也就是说，该文件来自于**不受限的进程**所产生的。一般情况下，预设的bash是不受SELinux管控的。
	- system_u：系统用户，大部分就是系统自己产生的文件。
- 角色 (Role)：说明资料是属于进程、文件资源还是代表使用者。类型：
	- object_r：代表的是文件或目录等文件资源。
	- system_r：代表的就是进程啦；不过，一般使用者也会被指定成为 system_r。
- 类型 (Type)：定义进程的域（domain）和文件的类型（type）。SELinux 策略规则定义Type如何相互访问，无论是访问type的domain，还是访问另一个domain的domain。**在 targeted 策略下，只有该字段会影响存取。**

```bash
# 可执行文件的 type 是 crond_exec_t，在执行之后进程的 domain 会变成 crond_t（通过 entrypoint）
# entrypoint 权限在SELinux policy中使用，控制哪个程序可以被用于进入一个 domain
[root@dev ~]# ls -Z /usr/sbin/crond 
-rwxr-xr-x. root root system_u:object_r:crond_exec_t:s0 /usr/sbin/crond

# domain 已经变成了 crond_t
[root@dev ~]# ps -Z -C crond
LABEL                              PID TTY          TIME CMD
system_u:system_r:crond_t:s0-s0:c0.c1023 1886 ? 00:00:01 crond

# SELinux policy rule 规定，允许 crond_t 域中运行的进程读取和写入标记为 system_cron_spool_t、user_cron_spool_t 类型的文件
[root@dev ~]# ls -Z /etc/crontab /etc/cron.d /var/spool/cron/
-rw-r--r--. root root system_u:object_r:system_cron_spool_t:s0 /etc/crontab

/etc/cron.d:
-rw-r--r--. root root system_u:object_r:system_cron_spool_t:s0 0hourly
-rw-r--r--. root root system_u:object_r:system_cron_spool_t:s0 raid-check
-rw-------. root root system_u:object_r:system_cron_spool_t:s0 sysstat

/var/spool/cron/:
-rw-------. root root unconfined_u:object_r:user_cron_spool_t:s0 root
-rw-------. sink sink unconfined_u:object_r:user_cron_spool_t:s0 sink
```

SELinux实验：

```bash
[root@dev ~]# vi selinux_crond_test
[root@dev cron.d]# cat selinux_crond_test 
1 * * * * root echo "selinux test"
[root@dev ~]# ls -Z selinux_crond_test 
-rw-r--r--. root root unconfined_u:object_r:admin_home_t:s0 selinux_crond_test

[root@dev ~]# mv selinux_crond_test /etc/cron.d/
[root@dev ~]# sc restart crond

# 这个 cron 失败了
[root@dev ~]# tail /var/log/cron
Nov 12 17:26:32 dev crond[22054]: ((null)) Unauthorized SELinux context=system_u:system_r:system_cronjob_t:s0-s0:c0.c1023 file_context=unconfined_u:object_r:admin_home_t:s0 (/etc/cron.d/selinux_crond_test)
Nov 12 17:26:32 dev crond[22054]: (root) FAILED (loading cron table)
```

## SELinux 三种模式的启动、关闭与观察

并非所有的 Linux distributions 都支持 SELinux 的。

- enforcing：强制模式，代表 SELinux 运作中，且已经正确的开始限制 domain/type 了；
- permissive：宽容模式，代表 SELinux 运作中，不过仅会有警告讯息并不会实际限制 domain/type 的存取。
  这种模式可以运来作为 SELinux 的 debug 之用；
- disabled：关闭。

使用`getenforce`来查看当前模式：

```bash
[sink@dev ~]$ getenforce 
Enforcing
```

使用`setenforce`来临时设置当前模式，要想永久生效就修改`/etc/selinux/config`文件：

```bash
# permissive mode
[root@dev ~]# setenforce 0
[root@dev ~]# getenforce 
Permissive
# enforcing mode
[root@dev ~]# setenforce 1
[root@dev ~]# getenforce 
Enforcing
```

也可以使用`sestatus`来查看SELinux信息：

```
sestatus [-v] [-b]

-v  Displays the context of files and processes listed in the /etc/sestatus.conf file.
-b  将目前策略的规则布尔值列出，即某些规则 (rule) 是否启动
```

```bash
[sink@dev ~]$ sestatus 
SELinux status:                 enabled
SELinuxfs mount:                /sys/fs/selinux		# SELinux 的相关文件数据挂载点
SELinux root directory:         /etc/selinux
Loaded policy name:             targeted
Current mode:                   enforcing
Mode from config file:          enforcing
Policy MLS status:              enabled		# 是否含有 MLS 的模式机制
Policy deny_unknown status:     allowed		# 是否预设抵挡未知的主体进程
Max kernel policy version:      31
```

### 启动

!!! note
		如果改变了策略则需要重新启动；如果由 enforcing 或 permissive 改成 disabled，或由 disabled 改成其他两个，那也必须要重新启动。这是因为 SELinux 是整合到内核里面去的，你只可以在 SELinux 运作下切换成为强制 (enforcing) 或宽容 (permissive) 模式，不能够直接关闭 SELinux 的。

​		如果从 disable 转到启动 SELinux 的模式时， 由于系统必须要针对文件写入安全性本文的信息，因此开机过程会花费不少时间在等待重新写入 SELinux 安全性本文 (有时也称为SELinux Label) ，而且在写完之后还得要再次的重新启动一次。

#### 启动失败的情况

​		在某些特殊的情况下，会出现一些服务无法启动的情况，类似于在 /lib/xxx 里面的数据没有权限读取，大多是由于在重新写入 SELinux type (Relabel) 出错之故。

​		最简单的方法就是在 Permissive 的状态下，使用`restorecon -Rv /` 重新还原所有 SELinux 的安全性文本，就能够处理这个错误。

## SELinux 策略内的规则管理

### 查询 SELinux 规则的布尔值

使用上文提到的`sestatus -b`与`getsebool -a`均可：

```
getsebool [-a]|[boolean] ...
boolean:规则名称

-a     Show all SELinux booleans.
```

```bash
[sink@dev ~]$ getsebool -a
abrt_anon_write --> off
abrt_handle_event --> off
abrt_upload_watch_anon_write --> on
……
[sink@dev ~]$ getsebool abrt_anon_write
abrt_anon_write --> off
```

### 修改 SELinux 规则的布尔值

```
setsebool [ -P ] boolean value | bool1=val1 bool2=val2 ...
Without the -P option， only the current boolean value is affected; the boot-time default settings are not changed

-P		直接将设定值写入配置文件，永久生效
```

```bash
# 关闭规则
[root@dev ~]# setsebool -P httpd_enable_homedirs 0
```

### SELinux 策略查询

首先安装软件：`yum install setools-console`

```
seinfo [OPTIONS] [EXPRESSION] [POLICY ...]
SELinux 策略查询工具

OPTIONS:
	-x， --expand                     show more info for specified components
	--stats                          print useful policy statistics，默认选项
EXPRESSIONS:
	-u[NAME]， --user[=NAME]			Print a list of users or， if NAME is provided， print the user NAME.
	-r[NAME]， --role[=NAME]
	-t[NAME]， --type[=NAME]
	-b[NAME]， --bool[=NAME]
	--all  							Print all components.
```

```bash
# 统计信息
[sink@dev ~]$ seinfo 

Statistics for policy file: /sys/fs/selinux/policy
Policy Version & Type: v.31 (binary， mls)

   Classes:           130    Permissions:       272
   Sensitivities:       1    Categories:       1024
   Types:            4792    Attributes:        253
   Users:               8    Roles:              14
   Booleans:          316    Cond. Expr.:       362
   ……
# 用户信息
[sink@dev ~]$ seinfo -u

Users: 8
   sysadm_u
   system_u
   xguest_u
   root
   guest_u
   staff_u
   user_u
   unconfined_u
[sink@dev ~]$ seinfo -u -x

Users: 8
   sysadm_u
      default level: s0
      range: s0 - s0:c0.c1023
      roles:
         object_r
         sysadm_r
   system_u
   ……
```



```
sesearch [OPTIONS] RULE_TYPE [RULE_TYPE ...] [EXPRESSION] [POLICY ...]
在 SELinux 策略中搜索规则

RULE TYPE OPTIONS
	-A， --allow		Search for allow rules.
	--all  			Search all rule types.
	-T， --type		Search for type_transition， type_member， and type_change rules.
EXPRESSIONS
	-s NAME， --source=NAME		Find rules with type/attribute NAME as their source.即主体
	-t NAME， --target=NAME		Find rules with type/attribute NAME as their target.即目标
	-b NAME， --bool=NAME		Find conditional rules with NAME in their conditional expression.即规则名字
```

```bash
[sink@dev ~]$ ls -Z /usr/bin/passwd
-rwsr-xr-x. root root system_u:object_r:passwd_exec_t:s0 /usr/bin/passwd
[sink@dev ~]$ ps -Z -C passwd
LABEL                              PID TTY          TIME CMD
unconfined_u:unconfined_r:passwd_t:s0-s0:c0.c1023 28764 pts/5 00:00:00 passwd
[sink@dev ~]$ ls -Z /etc/shadow
----------. root root system_u:object_r:shadow_t:s0    /etc/shadow
# 列出 passwd_t 这个主体进程能否读取 SELinux type 为 shadow_t 的文件
[sink@dev ~]$ sesearch --allow -s passwd_t | grep shadow_t
   allow passwd_t shadow_t : file { ioctl read write create getattr setattr lock relabelfrom relabelto append map unlink link rename open } ;
```
查询一个规则内定义了哪些东西：

```bash
# 看一下规则的说明
[root@dev ~]# semanage boolean -l | grep httpd_enable_homedirs
httpd_enable_homedirs          (off  ，  off)  Allow httpd to enable homedirs
# 规则内定义了什么
[root@dev ~]# sesearch --allow -b httpd_enable_homedirs
   ……
   allow httpd_t cifs_t : dir { getattr search open } ; 
   allow httpd_t cifs_t : dir { ioctl read getattr lock search open } ; 
   allow httpd_sys_script_t nfs_t : lnk_file { read getattr } ;
   ……
```

## SELinux 安全本文的修改

SELinux 对受限的主体进程有没有影响，第一关考虑 SELinux 的三种模式，第二关考虑 SELinux 的策略规则是否放行，第三关则是开始比对 SELinux type。

### 使用 chcon 手动修改文件的 SELinux type

```
chcon [OPTION]... CONTEXT FILE...
chcon [OPTION]... [-u USER] [-r ROLE] [-l RANGE] [-t TYPE] FILE...
chcon [OPTION]... --reference=RFILE FILE...

OPTION
	-u, --user=USER		设置目标 security context 的 user 字段
	--reference=RFILE	使用 RFILE 的安全性文本来设置目标
	
	-R, --recursive
	-v, --verbose
```

```bash
# hello.selinux 是用 vi 编写的 cron 文件
[root@dev cron.d]# ls -Z 0hourly hello.selinux 
-rw-r--r--. root root system_u:object_r:system_cron_spool_t:s0 0hourly
-rw-r--r--. root root unconfined_u:object_r:admin_home_t:s0 hello.selinux
# 改变其 type 
[root@dev cron.d]# chcon -vt shadow_t hello.selinux 
changing security context of ‘hello.selinux’
[root@dev cron.d]# ls -Z hello.selinux 
-rw-r--r--. root root unconfined_u:object_r:shadow_t:s0 hello.selinux

# 复制另一个文件的 安全性文本
[root@dev cron.d]# chcon -v --reference=/etc/shadow hello.selinux 
changing security context of ‘hello.selinux’
[root@dev cron.d]# ls -Z hello.selinux 
-rw-r--r--. root root system_u:object_r:shadow_t:s0    hello.selinux
[root@dev cron.d]# ls -Z hello.selinux /etc/shadow
----------. root root system_u:object_r:shadow_t:s0    /etc/shadow
-rw-r--r--. root root system_u:object_r:shadow_t:s0    hello.selinux
```

虽然`chcon`可以手动修改，但是会出现一些问题，比如修改值不一定符合要求，权限不满足等情况：

```bash
[root@dev cron.d]# chcon -vt crond_t hello.selinux 
changing security context of ‘hello.selinux’
chcon: failed to change context of ‘hello.selinux’ to ‘unconfined_u:object_r:crond_t:s0’: Permission denied
```

这时就需要`restorecon`登场了，它可以让 SELinux 自己解决默认目录下的 SELinux type。

### 恢复默认的 SELinux type

```
restorecon [-R] [-n] [-p] [-v] [-e directory] pathname...

-R, -r change files and directories file labels recursively (descend directories).
-v     show changes in file labels, if type or role are going to be changed.
```

```bash
[root@dev cron.d]# restorecon -Rv /etc/cron.d
restorecon reset /etc/cron.d/hello.selinux context unconfined_u:object_r:net_conf_t:s0->unconfined_u:object_r:system_cron_spool_t:s0
```

可以看到已经被修改过为默认值了，此时就可以成功重启`crond`啦。那么默认值如何查看修改呢，使用`semanage`就OK啦！

### 默认目录的安全性本文查询与修改

```
semanage <mode> <options>

每种mode的具体选项可使用 man semanage-<mode> 查询
MODE
	fcontext            Manage file context mapping definitions

fcontext 的 OPTIONS
	-l, --list          List records of the specified object type
	-a, --add           Add a record of the specified object type
    -d, --delete        Delete a record of the specified object type
    -m, --modify        Modify a record of the specified object type
```

```bash
# 查询 /root 目录下的安全性文本
[root@dev ~]# semanage fcontext -l | grep "^/root"
SELinux fcontext                                   type               Context
/root(/.*)?                                        all files          system_u:object_r:admin_home_t:s0 
/root/.ppprc                                       regular file       system_u:object_r:pppd_etc_t:s0 
/root/.manpath                                     regular file       system_u:object_r:mandb_home_t:s0 
/root/bin(/.*)?                                    all files          system_u:object_r:bin_t:s0
……
# 查询 /etc/cron\.d 目录下的安全性文本
[root@dev ~]# semanage fcontext -l | grep "/etc/cron\.d"
SELinux fcontext                                   type               Context
/etc/cron.daily(/.*)?                              all files          system_u:object_r:bin_t:s0
```

```bash
# 新增默认安全性文本
[root@dev ~]# mkdir /my-selinux
[root@dev ~]# ls -dZ /my-selinux/
drwxr-xr-x. root root unconfined_u:object_r:default_t:s0 /my-selinux/
# 这里填写的必须是已存在的 user、role、type 等
[root@dev ~]# semanage fcontext -a -t shadow_t "/my-selinux(/.*)?"
[root@dev ~]# semanage fcontext -l | grep my-selinux
/my-selinux(/.*)?                                  all files          system_u:object_r:shadow_t:s0

# 在新增完毕之后不会立即生效
[root@dev ~]# ls -dZ /my-selinux/
drwxr-xr-x. root root unconfined_u:object_r:default_t:s0 /my-selinux/
# 使用 restorecon 之后才会生效
[root@dev ~]# restorecon -Rv /my-selinux/
restorecon reset /my-selinux context unconfined_u:object_r:default_t:s0->unconfined_u:object_r:shadow_t:s0
[root@dev ~]# ls -dZ /my-selinux/
drwxr-xr-x. root root unconfined_u:object_r:shadow_t:s0 /my-selinux/
```

!!! important
	在使用`semanage fcontext`执行修改动作之后，不会立即生效，需要执行`restorecon`来生效。

## SELinux 错误侦测

怎么知道哪个时候才需要执行上述指令？怎么知道系统因为 SELinux 的问题导致网络服务不对劲？CentOS 7.x 提供几支侦测的服务在登录 SELinux 产生的错误——`auditd` 和`setroubleshootd`。

### setroubleshoot

这个服务会将关于 SELinux 的错误讯息与解决方法记录到 `/var/log/messages` 与 `/var/log/setroubleshoot/*` 里头。

原本的 SELinux 信息本来是以两个服务来记录的，分别是 auditd 与 setroubleshootd。既然是同样的信息，因此 CentOS 6.x (含 7.x) 以后将两者整合在 auditd 当中啦！所以，并没有setroubleshootd 的服务存在了。

先检查一下是否安装了以下rpm包：

```bash
[sink@dev ~]$ rpm -qa | grep setroubleshoot
setroubleshoot-plugins-3.0.67-4.el7.noarch
setroubleshoot-3.2.30-7.el7.x86_64
setroubleshoot-server-3.2.30-7.el7.x86_64
```

安装好了 setroubleshoot-server 之后，请记得要重新启动 auditd，否则 setroubleshootd 的功能不会被启动的。

### 使用 FTP 实践

按鸟哥说的做。

安装ftp服务端：

```bash
[root@dev ~]# yum install -y vsftpd
[root@dev ~]# systemctl start vsftpd
[root@dev ~]# netstat -tpan | grep ftp
tcp6       0      0 :::21                  :::*                    LISTEN      48351/vsftpd
```

创建FTP账户：

```bash
[root@dev ~]# useradd -s /sbin/nologin ftp_u
[root@dev ~]# passwd ftp_u
```

#### 看看匿名访问者能否获取

```bash
# 丢两个文件进去
[root@dev ~]# cp -a /etc/securetty /etc/sysctl.conf /var/ftp/pub/
[root@dev ~]# ll /var/ftp/pub/
total 8
-rw-------. 1 root root 221 Oct 31  2018 securetty
-rw-r--r--. 1 root root 449 Aug  9 07:52 sysctl.conf

[root@dev ~]# curl ftp://localhost
drwxr-xr-x    2 0        0              42 Nov 15 01:22 pub
# 读取目录时目录末尾必须加上 "/"
[root@dev ~]# curl ftp://localhost/pub/
-rw-------    1 0        0             221 Oct 30  2018 securetty
-rw-r--r--    1 0        0             449 Aug 08 23:52 sysctl.conf
# 可以读取
[root@dev ~]# curl ftp://localhost/pub/sysctl.conf
……
[root@dev ~]# curl ftp://localhost/pub/securetty
curl: (78) RETR response: 550
[root@dev ~]# chmod a+r /var/ftp/pub/securetty 
# 修改权限之后就可以啦
[root@dev ~]# curl ftp://localhost/pub/securetty
……

# 可选步骤：恢复默认的 SELinux type
[root@dev ~]# restorecon -Rv /var/ftp/pub/
```

#### 普通账户访问/下载自己的家目录

```bash
# 丢两个文件进去
# ～<user> 会替换每个帐号的家目录，比如 ～ftp 就是 /var/ftp
[root@dev ~]# echo "abcdefg" > ~ftp_u/f1.txt
[root@dev ~]# cp -a /etc/hosts /etc/sysctl.conf ~ftp_u/
[root@dev ~]# ll ~ftp_u/
total 12
-rw-r--r--. 1 root root   8 Nov 15 09:39 f1.txt
-rw-r--r--. 1 root root 158 Jun  7  2013 hosts
-rw-r--r--. 1 root root 449 Aug  9 07:52 sysctl.conf
# 访问一下，xxx是账户密码
[root@dev ~]# curl ftp://ftp_u:xxx@localhost/~/
-rw-r--r--    1 0        0               8 Nov 15 01:39 f1.txt
-rw-r--r--    1 0        0             158 Jun 07  2013 hosts
-rw-r--r--    1 0        0             449 Aug 08 23:52 sysctl.conf
# 鸟哥这里是获取不到的，但是我却可以获取，呃呃呃，可能是版本的问题？
[root@dev ~]# curl ftp://ftp_u:xxx@localhost/~/f1.txt
abcdefg

# 虽然成功获取到了文件，但还是记录一下出现错误时的后续步骤吧
1. 切换SELinux的模式为 Permissive
2. 再次执行一下失败的步骤，如果可以成功执行则说明是SELinux导致的
3. 切换SELinux的模式为 Enforcing
4. 查看 /var/log/messages 文件了解可能的失败原因及解决方案
5. 解决问题
6. 再次执行失败的步骤，看能否成功
```

#### 普通账户访问/下载其他目录

```bash
# 老样子，先造数据
[root@dev ~]# mkdir /ftp-other
[root@dev ~]# chgrp ftp_u /ftp-other/
[root@dev ~]# echo "ccm" > /ftp-other/other.txt
# 访问一下
# 注意路径要写完整
[root@dev ~]# curl ftp://ftp_u:abcd1234@localhost//ftp-other/other.txt
curl: (78) RETR response: 550
# 哈哈，终于出错了
[root@dev ~]# vi /var/log/messages
……
Nov 15 10:35:26 dev dbus[1330]: [system] Activating service name='org.fedoraproject.Setroubleshootd' (using servicehelper)
Nov 15 10:35:26 dev dbus[1330]: [system] Successfully activated service 'org.fedoraproject.Setroubleshootd'
Nov 15 10:35:26 dev setroubleshoot: failed to retrieve rpm info for /ftp-other/other.txt
Nov 15 10:35:26 dev setroubleshoot: SELinux is preventing /usr/sbin/vsftpd from getattr access on the file /ftp-other/other.txt. For complete SELinux messages run: sealert -l 68b4938c-32d9-4a07-b3c4-0eead65887c4
……
[root@dev ~]# sealert -l 8f2821f6-889a-40a8-8ea8-8c4fd85c1d7c
SELinux is preventing vsftpd from read access on the file other.txt.
# 意思是说有57.6%的概率是这个原因引起的
*****  Plugin catchall_boolean (57.6 confidence) suggests   ******************

If you want to allow ftpd to full access
Then you must tell SELinux about this by enabling the 'ftpd_full_access' boolean.

Do
setsebool -P ftpd_full_access 1		# 这是解决方法
# 同上
*****  Plugin catchall_labels (36.2 confidence) suggests   *******************

If you want to allow vsftpd to have read access on the other.txt file
Then you need to change the label on other.txt
Do
# semanage fcontext -a -t FILE_TYPE 'other.txt'
……
Then execute:
restorecon -v 'other.txt'
# 其他详细信息
Additional Information:
Source Context                system_u:system_r:ftpd_t:s0-s0:c0.c1023
Target Context                unconfined_u:object_r:default_t:s0
Target Objects                other.txt [ file ]
Source                        vsftpd
Source Path                   vsftpd
Port                          <Unknown>
Host                          dev
Source RPM Packages           
Target RPM Packages           
Policy RPM                    selinux-policy-3.13.1-252.el7.noarch
Selinux Enabled               True
Policy Type                   targeted
Enforcing Mode                Enforcing
Host Name                     dev
Platform                      Linux dev 3.10.0-1062.el7.x86_64 #1 SMP Wed Aug 7
                              18:08:02 UTC 2019 x86_64 x86_64
Alert Count                   2
First Seen                    2019-11-15 10:35:26 CST
Last Seen                     2019-11-15 10:35:41 CST
Local ID                      8f2821f6-889a-40a8-8ea8-8c4fd85c1d7c

# 因为第一个开启的权限太多，所以这里选择第二种解决方案
[root@dev ~]# ls -dZ ~ftp
drwxr-xr-x. root root system_u:object_r:public_content_t:s0 /var/ftp
[root@dev ~]# semanage fcontext -a -t public_content_t "/ftp-other(/.*)?"
# 别忘了这一步哦
[root@dev ~]# restorecon -Rv /ftp-other/
# 成功获取！
[root@dev ~]# curl ftp://ftp_u:abcd1234@localhost//ftp-other/other.txt
ccm
```

#### 修改 FTP 服务端口

```bash
# 先直接修改端口看看
# 在文件的最后一行添加
[root@dev ~]# vi /etc/vsftpd/vsftpd.conf
……
listen_port=555
# 啊哦，出错了
[root@dev ~]# systemctl restart vsftpd
Job for vsftpd.service failed because the control process exited with error code. See "systemctl status vsftpd.service" and "journalctl -xe" for details.
# 是 SELinux 出了问题
[root@dev ~]# journalctl -xe
……
setroubleshoot[48235]: SELinux is preventing vsftpd from name_bind access on the tcp_socket port 555. For complete SELinux messages run: sealert -l ac1e407c-d892-445b-9cc5-82337a504a39
……
# 解决之道
[root@dev ~]# sealert -l ac1e407c-d892-445b-9cc5-82337a504a39
SELinux is preventing vsftpd from name_bind access on the tcp_socket port 555.

*****  Plugin bind_ports (92.2 confidence) suggests   ************************

If you want to allow vsftpd to bind to network port 555
Then you need to modify the port type.
Do
# semanage port -a -t PORT_TYPE -p tcp 555
    where PORT_TYPE is one of the following: certmaster_port_t, cluster_port_t, ephemeral_port_t, ftp_data_port_t, ftp_port_t, hadoop_datanode_port_t, hplip_port_t, isns_port_t, port_t, postgrey_port_t, unreserved_port_t.
    ……
# 查看修改的端口属于哪个 type
[root@dev ~]# semanage port -l | grep ftp
ftp_data_port_t                tcp      20
ftp_port_t                     tcp      21, 989, 990		# 21 在这里，就是你啦
ftp_port_t                     udp      989, 990
tftp_port_t                    udp      69
[root@dev ~]# semanage port -a -t ftp_port_t -p tcp 555
# OK，已经添加完毕啦
[root@dev ~]# semanage port -l | grep ftp_port_t
ftp_port_t                     tcp      555, 21, 989, 990
ftp_port_t                     udp      989, 990
tftp_port_t                    udp      69
# 修改完成！
[root@dev ~]# systemctl restart vsftpd
[root@dev ~]# netstat -tpan | grep ftp
tcp6       0      0 :::555                  :::*                    LISTEN      48351/vsftpd
[root@dev ~]# curl ftp://localhost:555/pub/
-rw-r--r--    1 0        0             221 Oct 30  2018 securetty
-rw-r--r--    1 0        0             449 Aug 08 23:52 sysctl.conf
```

### 设置报错提醒

虽然`setroubleshoot`会将错误信息写入到`/var/log/messages`里面，但每次去查找就显得有点麻烦！我们可以设置一个邮件提醒：

修改`/etc/setroubleshoot/setroubleshoot.cfg`文件，添加或修改：

```
recipients_filepath = /var/lib/setroubleshoot/email_alert_recipients
```

然后添加邮件列表：

```bash
$> vim /var/lib/setroubleshoot/email_alert_recipients
root@localhost
your@email.address
$> systemctl restart auditd
```

