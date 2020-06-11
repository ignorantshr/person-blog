shell自动化输入的三种方式：

- 重定向
- 管道
- expect

前两种的前提是指令需要有参数来指定密码输入方式，具体使用可参考博客：

https://blog.csdn.net/zhangjikuan/article/details/51105166

下面讲一下expect方式的使用

当不想要手动输入时（如：密码），可使用`expect`来实现脚本自动输入。

安装

```shell
yum install expect -y
```

在shell脚本内使用

```shell
if [[ "x${use_v2v_local_cmd}" == "xtrue" ]]; then
    display "${v2v_local_cmd}"
    if [[ "x${password_file}" != "x" && "x${mode}" == "xESXi" ]]; then
        passwd=`cat ${password_file}`
        expect <<-EOF
        set timeout -1

        spawn ${v2v_local_cmd}
        expect {
            "*password*" {
                send "${passwd}\r"
                exp_continue
            }
            "*error*" {
                exit 1
            }
        }
EOF
    else
        eval ${v2v_local_cmd}
    fi
fi
```

这种方式虽好，但是在实际使用中发现了一个**限制**：

​		传入的命令**不能带有引号**，否则执行失败。比如：`spawn virsh -c 'esx://root@172.16.2.179?no_verify=1' list --all`，会报错：`no such file or directory`。实际工作中免不了要将一些带有特殊字符的参数用引号包起来，这时可以绕一下，将命令写到文件中再执行：

expect执行脚本：**pass.sh**

```shell
#!/bin/expect
set command [lindex $argv 0]
set passwd [lindex $argv 1]
spawn ${command}
expect {
    "*password*" {
        send "${passwd}\r"
        exp_continue
    }
    "*error*" {
        exit 1
    }
    eof {
        exit
    }
}
```

存储命令的可执行脚本：**commond.sh**

```shell
#!/bin/bash
virsh -c 'esx://root@172.16.2.179?no_verify=1' list --all
```

然后就可以执行：

```shell
./pass.sh ./commond.sh ABCDabcd.1234
```

此时就达到了自动化输入的目的。

详细的expect使用参考博客：https://www.cnblogs.com/lixigang/articles/4849527.html