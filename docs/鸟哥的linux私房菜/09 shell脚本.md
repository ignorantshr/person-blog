## 脚本的执行方式

除了root外，可执行的脚本必须具有读权限。

有以下5中执行方式：

1. 绝对路径
2. 相对路径
3. 放在变量PATH中
4. 以bash程序来执行。 "bash test.sh"，按照[指令搜寻顺序](./08 bash.md)来执行。
5. 以source程序来执行。source指令与上述4种方式不同的是，它不会以新的shell来执行，而是在当前shell环境下执行脚本，故可以用来重新加载环境变量："source ~/.bashrc"。

## 脚本的debug

```
sh [GNU long option] [option] ...
sh [GNU long option] [option] script-file ...

option
	-n  Read commands but do not execute them.可以检查语法错误
	-v  Print shell input lines as they are read.
    -x  Print commands and their arguments as they are executed.
```

```shell
[sink@dev vitest]$ cat add-until.sh
#!/bin/sh

read -p 'input a number: ' num
num=${num:-10}
echo -n "1+2+...+${num} = "
sum=0
for ((i=1; i<=${num};i++))
do
        sum=$((${sum} + i))
done
#until [ ${num} -lt 1 ]
#do
#       sum=$((${sum} + $num))
#       num=$((${num} - 1))
#done
echo ${sum}

# 检查语法
[sink@dev vitest]$ sh -n add-until.sh
# 将读到的语句都打印出来
[sink@dev vitest]$ sh -v add-until.sh
#!/bin/sh

read -p 'input a number: ' num
input a number: 2
num=${num:-10}
echo -n "1+2+...+${num} = "
1+2+...+2 = sum=0
for ((i=1; i<=${num};i++))
do
        sum=$((${sum} + i))
done
#until [ ${num} -lt 1 ]
#do
#       sum=$((${sum} + $num))
#       num=$((${num} - 1))
#done
echo ${sum}
3
# 打印执行的语句及参数
[sink@dev vitest]$ sh -x add-until.sh
+ read -p 'input a number: ' num
input a number: 2
+ num=2
+ echo -n '1+2+...+2 = '
1+2+...+2 = + sum=0
+ (( i=1 ))
+ (( i<=2 ))
+ sum=1
+ (( i++ ))
+ (( i<=2 ))
+ sum=3
+ (( i++ ))
+ (( i<=2 ))
+ echo 3
3
```

