## 变量

使用`env`列出变量；

使用`export`列出与设置环境变量；

使用`set`、`declare`或`typeset`列出变量；

使用`unset`取消变量。



环境变量`PS1`设置命令提示符。

```shell
[sink@dev ~]$ set | grep PS1
PS1='[\u@\h \W]\$ '
[sink@dev ~]$ PS1='[\u@\h \t \W]\$ '
[sink@dev 08:37:24 ~]$ 

# 查询使用说明
$ man bash
```

### 获取键盘输入

```
 read [-p prompt] [-t timeout] [name ...]
 将键盘输入存到变量中
 
 -p				提示信息
 -t				等待时长
 name			变量名
```

```bash
[sink@dev ~]$ read -p "input> " -t 5 tmp_str
input> 123456
[sink@dev ~]$ echo $tmp_str
123456
```

### 声明变量类型

```
declare/typeset [-aAfFgilrtux] [-p] [name[=value] ...]

-a     	定义数组类型
-i		定义整数类型
-x		与 export 一样，将变量声明为环境变量
-r		定义 readonly 类型，不可修改，不可 unset。注销重新登录才开消除该临时变量
-p		列出变量的值和属性

使用“+”来取消声明的属性。例外： +a 不能用于销毁数组变量和 +r 不会删除只读属性。
```

```bash
[sink@dev ~]$ sum=123-234
[sink@dev ~]$ echo $sum
123-234
[sink@dev ~]$ declare -i sum=123-234
[sink@dev ~]$ echo $sum
-111
[sink@dev ~]$ declare -x sum
[sink@dev ~]$ export | grep sum
declare -ix sum="-111"
[sink@dev ~]$ declare -r sum
[sink@dev ~]$ sum=123
-bash: sum: readonly variable
[sink@dev ~]$ unset sum
-bash: unset: sum: cannot unset: readonly variable
[sink@dev ~]$ declare -p sum
declare -irx sum="-111"
```

### 数组变量

可以不写declare语句直接声明

```bash
[sink@dev ~]$ var[1]="a1"
[sink@dev ~]$ var[2]="b2"
[sink@dev ~]$ var[3]=3
[sink@dev ~]$ declare -a var[4]='45'
[sink@dev ~]$ declare -p var
declare -a var='([1]="a1" [2]="b2" [3]="3" [4]="45")'
# 数组的获取使用 ${} 形式
[sink@dev ~]$ echo $var[1]
[1]
[sink@dev ~]$ echo ${var[1]}
a1
```

## 系统限制

```
ulimit [-HSTabcdefilmnpqrstuvx [limit]]
限制可创建的文件大小、CPU等。

-H     不能超过设定值
-S	   超过时发出警告
-a     All current limits are reported
-b     The maximum socket buffer size
-c     The maximum size of core files created。核心文件：系统将某些出错的程序在内存中的信息写出的文件
-d     The maximum size of a process's data segment
-f     The maximum size of files written by the shell and its children。没有选项时的默认选项
-n     The maximum number of open file descriptors (most systems do not allow this value to be set)
-t     The maximum amount of cpu time in seconds
-u     The maximum number of processes available to a single user
-T     The maximum number of threads
```



```bash
[sink@dev ~]$ ulimit -a
core file size          (blocks, -c) 0			# 0表示无限制
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 63335
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1024
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 4096
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited

[sink@dev ~]$ ulimit -f 10240
[sink@dev ~]$ dd if=/dev/zero of=limit-file bs=4k count=10000
File size limit exceeded
[sink@dev ~]$ ll -h limit-file
-rw-rw-r-- 1 sink sink 10M Oct  9 09:37 limit-file
```

注销重新登录可恢复原先的限制。

```bash
[sink@dev ~]$ exit
logout
> ssh sink@172.16.2.237
[sink@dev ~]$ ulimit -f
unlimited
```

## bash的操作环境

### 指令的搜寻顺序

指令按照以下顺序来搜索：

1. 以绝对/相对路径执行指令，例如`/bin/ls`、`./ls`
2. 由alias找到该指令并执行
3. 由bash内置的指令来执行
4. 通过`$PATH`的顺序搜寻到的第一个指令来执行

`type -a <command>`会按顺序将找到的指令列出。

### bash的进入时的信息

登录之前显示的信息在`/etc/issue`（`/etc/issue.net`是通过telnet登录时显示的信息）中配置，可在`man 8 agetty`的`ISSUE ESCAPES`章节查阅都支持哪些信息。

```bash
[root@dev vitest]# cat /etc/issue
\S
Kernel \r on an \m

```

登录之后显示的信息在`/etc/motd`中配置：

```bash
[root@dev vitest]# cat /etc/motd
Warning, the server will be maintained at 2019.10.10 15:00~19:00
```

### bash的环境配置文件（`login shell`与`non-login shell`）

- `login shell`：取得 bash 时需要完整的登陆流程的，就称为 login shell。
- `non-login shell`：不需要重复登陆举动的取得 bash 。比如子程序。

#### login shell

读取`/etc/profile`和`~/.bash_profile 或 ~/.bash_login 或 ~/.profile`文件。

/etc/profile 是设置整体环境变量的地方，控制者所有登录用户的变量。主要有：

- PATH
- USER
- LOGNAME
- MAIL
- HOSTNAME
- HISTSIZE：历史命令数量。
- umask

除了设置一些全局变量，还会去执行一些bash环境设置的脚本：

```shell
for i in /etc/profile.d/*.sh /etc/profile.d/sh.local ; do
    if [ -r "$i" ]; then
        if [ "${-#*i}" != "$-" ]; then
            . "$i"
        else
            . "$i" >/dev/null
        fi
    fi
done
```

在执行完毕`/etc/profile`之后，就会按顺序读取用户的个人偏好设置文件（只读取找到的第一个文件）：

1. ~/.bash_profile
2. ~/.bash_login
3. ~/.profile

```shell
[root@dev ~]# cat ~/.bash_profile
# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
        . ~/.bashrc
fi

# 用户指定的环境和启动程序
# User specific environment and startup programs

PATH=$PATH:$HOME/bin

export PATH
```

这里会执行 `~/.bashrc` 文件，将里面的设置读取到当前的环境中。而`~/.bashrc`里面设置了变量别名，然后将`/etc/bashrc`的设置读取到当前shell环境中，至于`/etc/bashrc`做了什么，请看下面的`non-login shell`小节。

#### non-login shell

读取`~/.bashrc`文件。

上文提到`~/.bashrc`主要是设置变量别名与执行`/etc/bashrc`，`/etc/bashrc`的主要作用有以下几点：

1. 根据不同的UID规定unmask的值
2. 根据不同的UID规定 PS1（命令行提示符）变量
3. 调用`/etc/profile.d/*.sh`的设置

## 杂项

读入环境配置文件的指令是`source`或`.`。

注销之后执行的动作在`~/.bash_logout`中配置。

`stty`设置终端的按键等。

<kbd>Ctrl+U</kbd>：在命令行上向前删除命令；<kbd>Ctrl+K</kbd>：向后删除命令。

bash的通配符：`*`、`?`、`[-]`（例：[0-9]）、`[]`（例：[ABCD]）、`[^]`（例：[^abcd]）。

