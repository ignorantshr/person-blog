## 相关文件

### /etc/passwd 文件结构

每行都代表了一个账号。

```bash
[root@dev communication]# head /etc/passwd
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
daemon:x:2:2:daemon:/sbin:/sbin/nologin
sink:x:1000:1000:sink:/home/sink:/bin/bash
user1:x:1001:1002::/home/user1:/bin/bash
```

这7个字段的意义：

1. 账号名称，即用户名。
2. 密码。早期Unix存放密码的地方（已转移到`/etc/shadow`存放），现在用x代替。
3. UID，用户的ID。0：系统管理员，可以有多个账号都是系统管理员（不建议）；1~999：系统账号，保留给系统使用的ID；1000~60000：一般使用者的可登录账号。
4. GID，用户组ID。与`/etc/group`对应。
5. 用户信息说明。
6. 家目录。
7. shell。登录时的预设shell。

`/sbin/nologin`表示无法登录系统：

```bash
# 不论是login shell 还是 non-login shell 都不能登录
[root@dev ~]# su - bin
Last login: Thu Oct 17 08:46:10 CST 2019 on pts/0
This account is currently not available.
[root@dev ~]# su bin
This account is currently not available.
```

可通过编辑`/etc/nologin.txt`来修改提示信息：

```bash
[root@dev ~]# cat /etc/nologin.txt
The account is not allowed to login.
[root@dev ~]# su bin
The account is not allowed to login.
```



### /etc/shadow 文件结构

```bash
[root@dev vitest]# cat /etc/shadow
root:$6$dEmXqJj8IH07TF97$sD08U6KQTP.Y.2Wr.Fe27m.b7Hq4M0LjIgeNoLI5GYU1e/qvaT/bxWxgF5F8tz5O.Zd4KayZfvnBae/hV2QbA1::0:99999:7:::
bin:*:17110:0:99999:7:::
daemon:*:17110:0:99999:7:::
user3:!!:18181:0:99999:7:::
```

这7个字段的意义：

1. 账号名称，与`/etc/passwd`对应。
2. 加密之后的密码。`!!`表示没有合法密码。
3. 最后一次修改密码的时间。从`1970.01.01`开始计算的**天数**。
4. 能够几天之后再次修改密码。距离最后一次修改之后不可再修改的天数。
5. 几天之内需要重新修改密码。距离最后一次修改之后需要再次修改的天数。
6. 需要修改密码前的警告天数。密码快到期时，给账号发出即将过期警告。
7. 密码过期后的宽限天数。密码有效期=最后一次修改密码的时间+需要重新修改的天数；过了有效期之后进入密码过期状态，当账号登录之后会强制要求更改账号密码。若密码过期之后的宽限时间内没有登录系统修改密码，那么账号处于密码失效状态，再也无法使用该密码登录了。
8. 账号失效日期。账号在此日期之后无法使用（但是root仍可切换）。从`1970.01.01`开始计算的**天数**。
9. 保留字段。

例子：

```
user2:$6$mjcTl/W0$h01E8KRCadMIjOMbj.pVg.xQjGdDG4899FSQvItlTqRVo520TzI.BeTspDSXkNaS/YATo00IzrAD.fUoRQGPQ0:18181:1:7:2:5:20000:

这行配置代表什么呢：
在 18181 （2019-10-12）这天修改了密码，之后1天不能再次修改密码，8（1+7)天之后就需要修改密码了，在需要修改密码之前的2天会发出警告，8天之后仍然有5天的宽限时间，到 20000 （2024-10-04）这天账号失效，该账号无法再使用。
```

#### 日期的计算

```bash
# 从1970年1月1日起之后17843天的日期
[root@dev vitest]# date --date="19700101 + 17843 day" +%Y-%m-%d
2018-11-08
# 当前日期后的99999天
[root@dev vitest]# date --date="99999 day" +%Y-%m-%d
2293-07-26
# 从1970年1月1日起之后17843天的日期
[root@dev vitest]# date --date="@$((17843*60*60*24))" +%Y-%m-%d
2018-11-08
```

#### 忘记root密码

此时可以使用各种方法进入linux去修改：

1. 进入单人维护模式，使用`passwd`修改。
2. 用镜像启动，挂载根目录之后修改`/etc/shadow`文件，将root密码字段清空，再次重启即可无需密码登录。

#### 加密机制

linux版本差距很大，以下命令可以查询是使用了哪种加密机制：

```bash
[root@dev vitest]# authconfig --test | grep hashing
 password hashing algorithm is sha512
```

### /etc/group 文件结构

```bash
[root@dev vitest]# cat /etc/group
root:x:0:
bin:x:1:
daemon:x:2:
projects:x:1001:user1,user2
user1:x:1002:
user2:x:1003:
```

这4个字段的意义：

1. 组名。
2. 组密码。已转移到`/etc/gshadow`存放了，以`x`代替。
3. 组ID，GID。
4. 在群组中的账号。新版的linux已不会将账号加入到该字段中了（例：user1组中并没有user1这个用户）。

### /etc/gshadow 文件结构

```bash
[root@dev vitest]# cat /etc/gshadow
root:::
bin:::
daemon:::
user3:!::
```

这4个字段的意义：

1. 组名。
2. 密码。`!`表示没有合法密码。
3. 群组管理员的账号。很少使用。
4. 加入该群组的账号。

## 账号管理

### 添加

```
useradd [options] LOGIN
useradd -D
useradd -D [options]
create a new user or update default new user information

OPTIONS
	-u, --uid UID					指定UID
	-c, --comment COMMENT			对应 /etc/passwd 中的说明字段
	-d, --home-dir HOME_DIR			家目录的绝对路径
	-g, --gid GROUP					初始群组
	-G, --groups GROUP1[,GROUP2,...]]		附加群组
	-m, --create-home				创建家目录。一般账号默认值
	-M, --no-create-home			不创建家目录。系统账号默认值
	-N, --no-user-group				不创建和账号名一致的群组，而是添加到 -g 指定的群组中
	-r, --system					创建一个系统账号，通常会有一些限制
	-e, --expiredate EXPIRE_DATE	指定账号失效日期，格式为 YYYY-MM-DD
	-f, --inactive INACTIVE			指定密码几天之后会过期。默认值为 -1 ，表示不启用该功能
```

```bash
[root@dev vitest]# useradd -c "useradd test" -e 2019-10-13 user3
[root@dev vitest]# ll -d /home/user3/
drwx------ 3 user3 user3 78 Oct 12 14:52 /home/user3/
[root@dev vitest]# grep user3 /etc/passwd /etc/shadow /etc/group
/etc/passwd:user3:x:1003:1004:useradd test:/home/user3:/bin/bash
/etc/shadow:user3:!!:18181:0:99999:7::18182:
/etc/group:user3:x:1004:
```

`/etc/default/useradd`配置有默认的创建值，可以通过`useradd -D`打印。

```bash
[root@dev vitest]# useradd -D
GROUP=100
HOME=/home
INACTIVE=-1
EXPIRE=
SHELL=/bin/bash
SKEL=/etc/skel		# 家目录参考的基准目录，初始家目录下的东西都是从这里复制的
CREATE_MAIL_SPOOL=yes
```

其它的配置在`/etc/login.defs`文件中。

### 密码的修改

账号新增完毕之后是锁定状态，还不能登录，需要修改密码才可。

```
passwd [-k] [-l] [-u [-f]] [-d] [-e] [-n mindays] [-x maxdays] [-w warndays] [-i inactivedays] [-S] [--stdin] [username]

-l, --lock				会将 /etc/shadow 第2栏最前面加上 ! ， 使密码失效，使账号只能被root使用
-u, --unlock			
-d, --delete			
-e, --expire			使密码过期(root only)
-f, --force
-x, --maximum=DAYS      多长时间之内需要修改密码 (root only)
-n, --minimum=DAYS      多长时间之内不能修改密码 (root only)
-w, --warning=DAYS      几天之前收到密码过期的警告 (root only)
-i, --inactive=DAYS     密码过期几天后账号变为不可用状态(root only)
-S, --status			root only
```

```bash
[root@dev vitest]# passwd -S user2
user2 PS 2019-10-12 0 1 1 5 (Password set, SHA512 crypt.)
[root@dev vitest]# passwd -l user2
Locking password for user user2.
passwd: Success
[root@dev vitest]# passwd -S user2
user2 LK 2019-10-12 0 1 1 5 (Password locked.)
[root@dev vitest]# grep /etc/shadow

[root@dev vitest]# grep user2 /etc/shadow
user2:!!$6$mjcTl/W0$h01E8KRCadMIjOMbj.pVg.xQjGdDG4899FSQvItlTqRVo520TzI.BeTspDSXkNaS/YATo00IzrAD.fUoRQGPQ0:18181:0:1:1:5:10000:
```

还有一个指令也可以修改密码信息：

```
chage [options] LOGIN

OPTIONS
	-l, --list					
	-d, --lastday LAST_DAY        更改密码的修改日期，格式：YYYY-MM-DD。如果 LAST_DAY 设置为 0，则用户在下一次登录时会被强制更改密码。
	-m, --mindays MIN_DAYS        设置密码可修改之前的最短天数
	-M, --maxdays MAX_DAYS        设置密码修改的最长天数
	-W, --warndays WARN_DAYS      设置过期前提醒天数
	-I, --inactive INACTIVE       设置密码失效天数
	-E, --expiredate EXPIRE_DATE  设置账号失效日期，格式：YYYY-MM-DD
```

```bash
# 普通用户之能查看自己的信息
[sink@dev vitest]$ chage -l sink
Last password change                                    : never
Password expires                                        : never
Password inactive                                       : never
Account expires                                         : never
Minimum number of days between password change          : 0
Maximum number of days between password change          : 99999
Number of days of warning before password expires       : 7

# 多么的直观方便啊
[root@dev vitest]# chage -m 2 -M 30 -W 7 -I 7 -E 2020-01-01 user1
[root@dev vitest]# chage -l user1
Last password change                                    : Oct 12, 2019
Password expires                                        : Nov 11, 2019
Password inactive                                       : Nov 18, 2019
Account expires                                         : Jan 01, 2020
Minimum number of days between password change          : 2
Maximum number of days between password change          : 30
Number of days of warning before password expires       : 7
# 强制用户下次登录时更改密码
[root@dev vitest]# chage -d 0 user1
[root@dev vitest]# chage -l user1
Last password change                                    : password must be changed
Password expires                                        : password must be changed
Password inactive                                       : password must be changed
Account expires                                         : Jan 01, 2020
Minimum number of days between password change          : 2
Maximum number of days between password change          : 30
Number of days of warning before password expires       : 7
```

### 账号的修改

```
usermod [options] LOGIN

options
	-L, --lock                    lock the user account
	-U, --unlock                  unlock the user account
	
	-c, --comment COMMENT
	-d, --home HOME_DIR
	-u, --uid UID                 new UID for the user account
	-l, --login NEW_LOGIN         new value of the login name
	-g, --gid GROUP
	-G, --groups GROUPS
	-a, --append					append the user to the supplemental GROUPS
	
	-e, --expiredate EXPIRE_DATE			账号失效
	-f, --inactive INACTIVE					密码过期
	-p, --password PASSWORD       use encrypted password for the new password
	
```

### 删除

```
userdel [options] LOGIN

-f, --force
-r, --remove                  把家目录和邮箱一起删掉
```

若真正地想要删除用户则可以使用改指令，否则可以设置账号失效而保留所有的相关数据。

### 用户信息查看

```
finger [-lmsp] [user ...] [user@host ...]
用户信息查询工具

Options
-s    Finger displays the user's login name, real name, terminal name and write status (as a ``*'' after the terminal name if write permission is denied), idle time, login time, office loca‐tion and office phone number.
-l    Produces a multi-line format displaying all of the information described for the -s option as well as the user's home directory, home phone number, login shell, mail status, and the contents of the files “.plan”, “.project”, “.pgpkey” and “.forward” from the user's home directory.
```

```bash
[root@dev libguestfs]# finger root
Login: root                             Name: root
Directory: /root                        Shell: /bin/bash
On since Tue Oct  8 09:06 (CST) on :0 from :0 (messages off)
On since Wed Oct  9 17:09 (CST) on pts/0 from 172.16.2.27
   4 seconds idle
On since Tue Oct  8 09:08 (CST) on pts/1 from :0
   4 days 22 hours idle
On since Thu Oct 10 09:48 (CST) on pts/5 from :0
   2 days 5 hours idle
On since Mon Oct 14 11:19 (CST) on pts/8 from :0
   1 hour 47 minutes idle
Last login Sat Oct 12 16:28 (CST) on pts/2
New mail received Mon Oct 14 03:20 2019 (CST)
     Unread since Thu Sep 12 13:46 2019 (CST)
No Plan.
```

#### 信息修改

```
chfn [-f full-name] [-o office] ,RB [ -p office-phone] [-h home-phone]] [username]
change your finger information
```

其实就是修改了`/etc/passwd`的信息说明字段：

```bash
[sink@dev vitest]$ grep sink /etc/passwd
sink:x:1000:1000:sink,xxx JiangXi,,xxx:/home/sink:/bin/bash
```

### 改变登录 shell

```
chsh [options] [username]
change your login shell
Options:
 -s, --shell <shell>  specify login shell
 -l, --list-shells    print list of shells and exit
```

```bash
[sink@dev vitest]$ chsh -s /bin/tcsh
Changing shell for sink.
Password:
Shell changed.
[sink@dev 15:31:28 vitest]$ grep sink /etc/passwd
sink:x:1000:1000:sink,xxx JiangXi,,xxx:/home/sink:/bin/tcsh
```

## 群组

初始群组：initial group。创建账号时加入的群组；

有效群组：effietive group。创建文件之类的东西时该文件所属的群组。

查看有效群组：

```bash
# 两个命令列出的第一个群组即为 有效群组
[sink@dev vitest]$ id
uid=1000(sink) gid=1000(sink) groups=1000(sink),10(wheel),980(docker)
[sink@dev vitest]$ groups
sink wheel docker
```

切换有效群组（只能在所在的群组中切换）：

```bash
[sink@dev vitest]$ newgrp docker
[sink@dev vitest]$ id
uid=1000(sink) gid=980(docker) groups=980(docker),10(wheel),1000(sink)
[sink@dev vitest]$ groups
docker wheel sink
[sink@dev vitest]$ touch group-test
[sink@dev vitest]$ ll group-test
-rw-r--r-- 1 sink docker 0 Oct 12 14:23 group-test
[sink@dev vitest]$ exit
exit
```

由于`newgrp`是通过新的shell来提供该功能的，所以完毕之后需要`exit`。

### 相关文件

```
FILES
       /etc/group
           Group account information.

       /etc/gshadow
           Secure group account information.

       /etc/login.defs
           Shadow password suite configuration.
           
	   /etc/passwd
           User account information.
```

### 添加

与`useradd`类似，自然有`groupadd`。

```
groupadd [options] group

OPTIONS
	-f, --force
	-g, --gid GID
	-r, --system				Create a system group.
```

### 修改

```
groupmod [options] GROUP

OPTIONS
	-g, --gid GID
	-n, --new-name NEW_GROUP
```

### 删除

```
groupdel GROUP
```

若有账号将此群组作为有效群组，那么不能删除该群组。

```bash
[root@dev libguestfs]# groupdel user2
groupdel: cannot remove the primary group of user 'user2'
```

### 群组管理员（不常用）

管理某个群组，负责用户的添加、删除。

```
gpasswd [option] group
administer /etc/group and /etc/gshadow

OPTIONS
	# 没有指定选项时会为该群组设置密码
	# 系统管理员的操作
	-A, --administrators user,...		Set the list of administrative users.
	-M, --members user,...				Set the list of group members.
	-r, --remove-password
	-R, --restrict						禁用 /etc/gshadow 的密码栏
	# 群组管理员的操作
    -a, --add user
	-d, --delete user
```

## 登录账号信息查询

```
w [options] user [...]
展示谁在登录和正在做什么

OPTIONS
	-i, --ip-addr		Display IP address instead of hostname for from field.
```

```bash
[sink@dev study]$ w
 22:52:58 up  2:16,  5 users,  load average: 0.14, 0.05, 0.06
USER     TTY      FROM             LOGIN@   IDLE   JCPU   PCPU WHAT
root     :0       :0               20:38   ?xdm?   5:52   0.38s gdm-session-worker [pam/gdm-password]
root     pts/0    :0               20:39    2:08m  7:11   0.66s /home/pycharm-2016.2/bin/fsnotifier64
root     pts/1    :0               20:45    2:05m  4:13   0.64s /home/clion-2018.3/bin/fsnotifier64
root     pts/2    192.168.216.1    21:39    1:06   0.52s  0.52s -bash
sink     pts/3    192.168.216.1    22:50    2.00s  0.05s  0.00s w
```



```
who [OPTION]
展示谁在登录
```

```bash
[sink@dev study]$ who
root     :0           2019-10-20 20:38 (:0)
root     pts/0        2019-10-20 20:39 (:0)
root     pts/1        2019-10-20 20:45 (:0)
root     pts/2        2019-10-20 21:39 (192.168.216.1)
sink     pts/3        2019-10-20 22:50 (192.168.216.1)
```



```
lastlog [options]
报告账号最近一次的登录信息

Options:
	-u, --user LOGIN|RANGE		RANGE：UID_MIN-UID_MAX
```

```bash
[sink@dev study]$ lastlog
Username         Port     From             Latest
root             pts/2    192.168.216.1    Sun Oct 20 21:39:09 -0400 2019
bin                                        **Never logged in**
daemon                                     **Never logged in**
……

```



```
last 
显示最近的用户登录信息

-F     Print full login and logout times and dates.
```

```bash
[sink@dev study]$ last
sink     pts/3        192.168.216.1    Sun Oct 20 22:50   still logged in
user1    pts/3        192.168.216.1    Sun Oct 20 22:08 - 22:09  (00:00)
root     pts/2        192.168.216.1    Sun Oct 20 21:39   still logged in
root     pts/1        :0               Sun Oct 20 20:45   still logged in
root     pts/0        :0               Sun Oct 20 20:39   still logged in
root     :0           :0               Sun Oct 20 20:38   still logged in
(unknown :0           :0               Sun Oct 20 20:37 - 20:38  (00:00)
reboot   system boot  3.10.0-514.el7.x Sun Oct 20 20:36 - 23:06  (02:29)
(unknown :0           :0               Sun Oct 20 20:35 - 20:35  (00:00)
reboot   system boot  3.10.0-514.el7.x Sun Oct 20 20:34 - 23:06  (02:31)
root     pts/0        :0               Fri Oct 18 04:19 - 20:31 (2+16:11)
root     pts/0        :0               Fri Oct 18 03:27 - 03:28  (00:01)
root     pts/2        :0               Fri Oct 18 03:18 - 04:19  (01:00)
root     pts/1        :0               Fri Oct 18 02:29 - 20:31 (2+18:01)
root     pts/0        :0               Fri Oct 18 09:57 - 03:24  (-6:-33)
root     :0           :0               Fri Oct 18 09:56 - 20:31 (2+10:34)
(unknown :0           :0               Fri Oct 18 09:56 - 09:56  (00:00)
reboot   system boot  3.10.0-514.el7.x Fri Oct 18 09:54 - 20:31 (2+10:36)

wtmp begins Fri Oct 18 09:54:36 2019
```

