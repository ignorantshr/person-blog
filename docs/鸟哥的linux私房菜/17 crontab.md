- `crontab`：定期执行命令。需要启动`crond`服务。

- `at`：执行一次命令。需要启动`atd`服务。

## at

`at`可以通过配置文件来限制可以使用它的用户：

1. 先找寻 `/etc/at.allow` 这个文件，写在这个文件中的使用者才能使用 at ，没有在这个文件中的使用者则不能使用 at (即使没有写在 at.deny 当中)；

2. 如果 `/etc/at.allow` 不存在，就寻找 `/etc/at.deny` 这个文件，若写在这个 at.deny 的使用者则不能使用 at ，而没有在这个 at.deny 文件中的使用者，就可以使用 at 咯；

3. 如果两个文件都不存在，那么只有 root 可以使用 at 这个指令。

at 创建的日程会在`/var/spool/at/`中。

### 使用

```
at [-q queue] [-f file] [-mMlv] timespec...
at [-q queue] [-f file] [-mMkv] [-t time]
at -c job [job...]
atq [-q queue]
atrm job [job...]

OPTIONS
	-q queue
	-m      	当job完成时，即使没有输出也发送邮件给用户。
	-M      	Never send mail to the user.
    -f file 	从文件读取job
    -t time 	格式： [[CC]YY]MMDDhhmm[.ss]
    -l      	Is an alias for atq. 查询job
    -r,-d      	Is an alias for atrm. 删除job
    -b      	is an alias for batch. 空闲时执行job
    -c			cats the jobs listed on the command line to standard output. 从 /var/spool/at/ 下读取
    
timespec
	HH:MM		在某个时刻执行指令。如果今天该时间已经过去了，那么就明天执行
	am or pm	in the morning or the evening
	MMDD[CC]YY, MM/DD/[CC]YY, DD.MM.[CC]YY or [CC]YY-MM-DD	年月日，年份可选
	now + count time-units		time-units：minutes, hours, days, or weeks
```

```bash
[root@dev ~]# at now + 10 minutes
at> echo "hi!" > /dev/pts/3
at> <EOT>
job 3 at Wed Oct 30 16:11:00 2019
[root@dev ~]# atq
3	Wed Oct 30 16:11:00 2019 a root
[root@dev ~]# at -c 3
#!/bin/sh
# atrun uid=0 gid=0
# mail root 0
umask 22
……
${SHELL:-/bin/sh} << 'marcinDELIMITER15abe4af'
echo "hi!" > /dev/pts/3

marcinDELIMITER15abe4af

# 删除 job
[root@dev ~]# atrm 3
```

一些需要注意的地方

- at 在运作时，会跑到当时下达 at 指令的那个工作目录。所以job中最好使用**绝对路径**。
- at 的执行与终端机环境无关，而所有 standard output/standard error output 都会传送到执行者的 mailbox 去。假如你在 tty1 登入，则可以使用` echo "Hello" > /dev/tty1`来取代。
- 如果在 at shell 内的指令并没有任何的讯息输出，那么 at 默认不会发 email 给执行者的。可以使用*-m*选项来发送邮件，告诉用户是否执行了指令。
- 系统会将该项 at 工作独立出你的 bash 环境中，直接交给系统的 atd 程序来接管。
- 当用户争用资源时，当前使用 at  和 batch 不适合，此时可以考虑其他的工具，比如 nqs。

#### batch

在 CPU 的工作负载（CPU 在单一时间点所负责的工作数量，不是 CPU 的使用率!）小于 0.8 的时候，才进行你所下达的工作任务。

不支持时间参数，感觉没什么用处啊。

## crontab

与`at`一样，也可以通过配置文件来限制用户：

1. /etc/cron.allow:
   将可以使用 crontab 的账号写入其中，若不在这个文件内的使用者则不可使用 crontab;
2. /etc/cron.deny:
   将不可以使用 crontab 的账号写入其中，若未记录到这个文件当中的使用者，就可以使用 crontab 。

3. /etc/cron.allow 比 /etc/cron.deny 优先级高。

当用户使用 crontab 这个指令来建立工作排程之后，该项工作就会被纪录到 `/var/spool/cron/` 里面去
了，而且是以账号来作为判别（比如sink用户就是/var/spool/cron/sink）。cron 执行的每一项工作都会被记录到 `/var/log/cron`。

### 使用

```
crontab [-u user] file
crontab [-u user] [-l | -r | -e]
服务于守护进程 cron(8)，来 install a crontab table file, remove or list the existing tables

OPTIONS
	-u     要修改其 crontab 的用户的名称。
	-l     Displays the current crontab on standard output.
    -r     Removes the current crontab. 会删除全部的 crontab，只删除一部分的话使用编辑选项
    -e     Edits the current crontab using the editor
```

!!! warning
	千万不要直接键入 crontab 并回车，因为默认动作是替换掉现有的 crontab 的内容。如果你回车之后直接退出了，那么配置过的 crontab 就全部被删除了！所以在你想好之前不要执行此动作。

```bash
[root@dev ~]# crontab -u sink -e 
no crontab for sink - using an empty one	# 若用户没有 crontab 会出现此提示
crontab: installing new crontab
[root@dev ~]# crontab -u sink -l
# 这个就是编辑时写入的内容，一共分为六个字段
0 12 * * * mail -s "dinner time" sink < /tmp/dinner.remind
```

这六个字段按照从左到右的顺序的意义分别是：

|   意义   | 分钟 | 小时 | 日期 | 月份 |         周几          | 指令 |
| :------: | :--: | :--: | :--: | :--: | :-------------------: | :--: |
| 数字范围 | 0-59 | 0-23 | 1-31 | 1-12 | 0-7<br>0和7都代表周日 |  ——  |

辅助的特殊字符：

| 字符 |                             意义                             |
| :--: | :----------------------------------------------------------: |
|  *   |                        任何时刻都接受                        |
|  ,   |  多个时段。比如小时字段的 4,8,9 表示这三个时间都会触发日程   |
|  -   | 时间范围。比如 15 2-8 * * * command，代表2-8点的15分时刻都执行指令 |
| */N  | N是数字，表示每隔多长时间。比如 \*/10 \* * * * command，代表每隔10分钟执行一次指令 |



### 系统的配置文件

`crontab -e`是给使用者设计的，**系统性的日程**就需要编辑`/etc/crontab`文件啦。

```bash
[root@dev ~]# cat /etc/crontab 
SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin
# 发送邮件给系统中的账户
MAILTO=root

# For details see man 4 crontabs
# 指明了七个字段的意义
# Example of job definition:
# .---------------- minute (0 - 59)
# |  .------------- hour (0 - 23)
# |  |  .---------- day of month (1 - 31)
# |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
# |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
# |  |  |  |  |
# *  *  *  *  * user-name  command to be executed
```

cron 这个服务的最低侦测限制是**分钟**，所以 cron 会每分钟去读取一次 `/etc/crontab` 与`/var/spool/cron` 里面的数据内容。

!!! note
	在某些情况下，crontab是读到内存中的，所以修改完 `/etc/crontab`可能不会立即执行，需要重启服务。

`/etc/cron.d`目录则是用于存放不同用户的系统性 cron。

```bash
[root@dev ~]# cat /etc/cron.d/raid-check 
# Run system wide raid-check once a week on Sunday at 1am by default
0 1 * * Sun root /usr/sbin/raid-check

# 每小时执行一次 run-parts /etc/cron.hourly
[root@dev ~]# cat /etc/cron.d/0hourly 
# Run the hourly jobs
SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root
01 * * * * root run-parts /etc/cron.hourly
# run-parts 脚本会在大约 5 分钟内随机选一个时间来执行 /etc/cron.hourly 目录内的所有可执行文件!
[root@dev ~]# ll /etc/cron.hourly/
total 4
# 此文件中存放的是 anacron 的执行方式，anacron 执行 cron.{daily,weekly,monthly} 三个目录，防止 anacron 与 cron 冲突
-rwxr-xr-x. 1 root root 392 Aug  9 07:07 0anacron
```

`cron.{hourly,daily,weekly,monthly}`都是`crontabs`（也即是 `run-parts`）的配置文件：

```bash
[root@dev ~]# ll -d /etc/cron.*ly
# 存放着需要每小时执行的脚本
drwxr-xr-x. 2 root root  22 Oct 22 16:03 /etc/cron.hourly

# 下面的是 anacron 直接执行的脚本
# 存放着需要每天执行的脚本
drwxr-xr-x. 2 root root  57 Oct 31 11:06 /etc/cron.daily
# 存放着需要每月执行的脚本
drwxr-xr-x. 2 root root   6 Jun 10  2014 /etc/cron.monthly
# 存放着需要每周执行的脚本
drwxr-xr-x. 2 root root   6 Jun 10  2014 /etc/cron.weekly

# 存放的都是一些可执行脚本，由 anacron 执行
[root@dev ~]# ll /etc/cron.daily
total 12
-rwx------. 1 root root 219 Oct 31  2018 logrotate
-rwxr-xr-x. 1 root root 618 Oct 30  2018 man-db.cron
-rwx------. 1 root root 208 Apr 11  2018 mlocate
```

### 注意事项

- 要将日程的执行时间错开，并考虑到系统的资源分配问题，合理地规划日程。
- 取消不要的输出项。当有执行成果或者是执行的项目中有输出的数据时，该数据将会 mail 给 MAILTO 设定的账号，不想要的话就利用好数据重定向吧。
- 安全的检验。`/var/log/cron`中记录了所有的 cron 活动，查看此文件可以检查一些非本人设定的 cron。

## anacron

anacron 并不是用来取代 crontab 的，而是对 crontab 的补充。anacron 存在的目的就在于：处理非 24 小时一直启动的 Linux 系统的 crontab； 以及因为某些原因导致的超过时间而没有被执行的日程。

其实 anacron 也是每个小时被 crond 执行一次，然后 anacron 再去检测相关的排程任务有没有被执行，如果有超过期限的工作在，就执行该排程任务，执行完毕或无须执行任何排程时，anacron 就停止了。

anacron 会去分析现在的时间与时间记录文件`/var/spool/anacron/cron.daily`所记载的上次执行 anacron 的时间，两者比较后若发现有差异， 那就是在某些时刻没有进行 crontab 啰!此时 anacron 就会开始执行未进行的 crontab 任务了。

```bash
# 这里文件名加了个 0 的目的是有多个脚本在该目录时首先执行 anacron，让时间戳先更新，避免误判 crontab 尚未执行任何任务
[root@dev ~]# cat /etc/cron.hourly/0anacron 
#!/bin/sh
# Check whether 0anacron was run today already
if test -r /var/spool/anacron/cron.daily; then
    day=`cat /var/spool/anacron/cron.daily`
fi
if [ `date +%Y%m%d` = "$day" ]; then
    exit 0;
fi

# 测试是否连接了电源
#       0 (true)： System is on line power.
#       1 (false)：System is not on line power.
# Do not run jobs when on battery power
if test -x /usr/bin/on_ac_power; then
    /usr/bin/on_ac_power >/dev/null 2>&1
    if test $? -eq 1; then
    exit 0
    fi
fi
# 执行
/usr/sbin/anacron -s
```

```
anacron [options] [job] ...
Options:
	-s         开始连续的执行各项日程 (job)，会依据时间记录文件的数据判断是否进行
	-f         强制执行日程，忽略所有的时间戳
	-n         立即执行日程，没有延迟，隐士包含了 -s 选项
	-u         更新所有的时间戳到当前日期，但不执行任何日程
```

anacron 从配置文件（说明可通过 `man anacrontab` 查看）中读取任务：

```bash
[root@dev ~]# cat /etc/anacrontab 
# /etc/anacrontab: configuration file for anacron

# See anacron(8) and anacrontab(5) for details.

SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root
# 在基础延迟时间之上添加的最大随机延迟时间，单位：分钟
RANDOM_DELAY=45
# 日程会在3:00-22:00开始
START_HOURS_RANGE=3-22

# 执行的频率			延迟时间			日程id，可自定义			指令
#period in days   delay in minutes   job-identifier   command
1	5	cron.daily		nice run-parts /etc/cron.daily
# cron.weekly 的延迟是 25分钟 + RANDOM_DELAY
7	25	cron.weekly		nice run-parts /etc/cron.weekly
@monthly 45	cron.monthly		nice run-parts /etc/cron.monthly
```

- `period in days`：若当前时间与时间戳（`/var/spool/anacron/cron.{daily,weekly,monthly}`）的相差天数大于等于此时间，则执行该日程
- `delay in minutes`：基础延迟时间，即决定执行日程之后延迟执行的时间（防止资源冲突？）
- `job-identifier`：取个名字啦，会在 `/var/log/cron` 中记录
- `command`：执行的指令

### crond 与 anacron 的关系

1. crond 会主动去读取 `/etc/crontab, /var/spool/cron/*, /etc/cron.d/*` 等配置文件，并依据 *分、时、日、月、周* 的时间设定去各项工作排程；
2. 根据 `/etc/cron.d/0hourly` 的设定，主动去 `/etc/cron.hourly/` 目录下，执行所有在该目录下的执行文件；
3. 因为 `/etc/cron.hourly/0anacron` 这个脚本文件的缘故，主动的每小时执行 anacron ，并呼叫`/etc/anacrontab`的配置文件;
4. 根据` /etc/anacrontab` 的设定，依据每天、每周、每月去分析 `/etc/cron.daily/, /etc/cron.weekly/, /etc/cron.monthly/` 内的执行文件，以进行固定周期需要执行的指令。