## trap介绍

trap命令是linux中用来在命令执行过程中捕获信号的。好像只能在脚本中使用？？？

```bash
trap [-lp] [[arg] sigspec ...]
```

当shell接收到了信号`sigspec`（可以是多个），就会执行`arg`命令。

- 若`arg`不存在或者是`-`（但是为了区别于选项，会写成`--`），并且有至少一个指定的信号，那么就按照指定信号原来的动作行事（即清除指定信号的自定义动作）。
- 若`arg`是空字符串，那么指定的`sigspec`会被忽视。
- 若`arg`不存在且提供了选项`-p`，则显示与每个`sigspec`关联的trap命令。
- 若没有提供任何参数或只有选项`-p`，那么打印和每个信号关联的命令列表。
- `-l`选项使 shell 打印信号名称及其相应编号的列表。
- `sigspec`可以是信号号码或者信号名称（通过`man 7 signal`查看）。信号名称不区分大小写，SIG 前缀是可选的。

`sigspec`其它的值：

- `EXIT (0)`：表示从shell退出后执行`arg`命令。
- `DEBUG`：在每个 *简单命令（for、变种for、case、select命令）和shell函数的第一条命令* 执行之前执行`arg`命令。
- `RETURN`：每次执行完一个shell函数或脚本
- `ERR`：每当简单命令具有非零退出状态时，执行`arg`命令，同时也会执行`EXIT`的命令。如果失败的命令是以下情况则不会执行`arg`命令：
    - 是紧跟在`while或until`关键字后面的命令列表的一部分；
    - `if`语句中的test的一部分；
    - 在`&&或||`命令列表中的一部分；
    - 命令的返回值通过`!`反转。

信号被忽略后，再进入shell时不会被捕获、重置或列出。创建未被忽略的捕获信号将重置为子shell或子shell环境中的原始值。

如果有任何`sigspec`是不合格的，那么trap命令的返回值是`false`，否则是`true`。

## 常见信号

参考：

https://www.jianshu.com/p/b26d4e520385

https://dsa.cs.tsinghua.edu.cn/oj/static/unix_signal.html

`man 7 signal`

| 信号 | 值      | 描述                                |
| ---- | ------- | ----------------------------------- |
| 1    | SIGHUP  | 挂起进程，见下方详细解释            |
| 2    | SIGINT  | 终止进程（打断进程？？？）Ctrl+C    |
| 3    | SIGQUIT | 停止进程                            |
| 9    | SIGKILL | 无条件终止进程                      |
| 15   | SIGTERM | 尽可能终止进程                      |
| 17   | SIGSTOP | 无条件停止进程，但不是终止进程      |
| 18   | SIGTSTP | 停止或暂停进程，但不终止进程 Ctrl+Z |
| 19   | SIGCONT | 继续运行停止的进程                  |

- SIGHUP：控制终端被关闭时该信号会被发送到进程。最初被设计成通知挂起的过程。在现代操作系统中，该信号通常意味着伪终端或虚拟终端被关闭了。许多 daemons 会 reload 它们的配置文件和 reopen 他们的日志文件，而不是退出。`nohup`可以让指令忽略该信号，与`&`搭配使用可让你在脱机状态下继续执行指令，[参考在后台执行命令](../shell/在后台执行命令.md)。

## 举例

当发生信号时忽略该信号

```shell
#!/bin/sh
trap "" 2
```

清除指定信号的指定动作，按原计划行事

```shell
#!/bin/sh
trap "" SIGINT
do something...

# 下面的代码不会再忽略SIGINT信号
trap -- SIGINT
do other something...
```

退出时执行命令

```shell
#!/bin/sh
# 即使中途打断程序(Ctrl+C)执行也会执行
trap "echo 'goodbye!'" 0
do something...
```

