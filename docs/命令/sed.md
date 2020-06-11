```
sed [OPTION]... {script-only-if-no-other-script} [input-file]...

OPTION
	-n, --quiet, --silent					
	-f script-file, --file=script-file		从含有脚本的文件中执行动作
	-i[SUFFIX], --in-place[=SUFFIX]			直接修改文件，如果提供 SUFFIX，则进行备份
	-r, --regexp-extended					使用扩展的正则表达式
	
{script-only-if-no-other-script}：'[n1[,n2]]命令'
常用命令
	a Text	后接字符串，在匹配行的下一行新增字符串
    i Text	后接字符串，在匹配行的上一行新增字符串
    c Text	后接字符串，替换掉 n1-n2 之间的行
    y/inchars/outchars/		按照一一对应的关系进行字符转换
    s/pattern/replacement/[flags]		替换字符串
    d		删除行
    p		打印行
    =		打印行号
    l		打印出所有字符
    w filename	将匹配的写入文件
    r filename	从文件中读取数据
```
还有一个特殊的排除命令`!`，当它加到常用命令的前面时表示对于匹配的行不执行该命令。

## sed 基础

### 使用地址

若要指定某些行，则要使用`行寻址`，sed 中有两种行寻址：

- 以数字形式表示行区间
- 以文本模式匹配行

语法：

```
[address]command

address{
	command1
	command2
}
```

数字行寻址，左闭右闭原则：

```bash
sed -e '2,$d' rpms.list
```

文本行寻址：

```bash
sed -e '/class/d' setup.py
```

文本行寻址使用正则表达式来匹配。



两种寻址方式可以混用：

```
[root@centos7 ~]# sed '/2/,$d' test1.txt
this is line number 1
```

### 替换文件中的内容

#### 替换标记

替换的语法：

```
s/pattern/replacement/[flags]
```

其中flags有以下几种：

- 数字：替换第几处的匹配文本
- g：替换所有的匹配文本
- p：打印匹配的行，经常与`-n`选项联用来只打印匹配的行
- w file：把替换结果写入文件

没有标记则替换第一处匹配文本：

```shell
sed -i 's/refa/hyxz/' rpms.list
```

p 标记：

```
[root@centos7 ~]# cat test1.txt
this is line number 1
this is line number 2
this is line number 3
this is line number 4

[root@centos7 ~]# sed 's/1/that/p' test1.txt
this is line number that
this is line number that
this is line number 2
this is line number 3
this is line number 4
```

w 标记：

```
[root@centos7 ~]# sed 's/1/that/w test1-copy.txt' test1.txt
this is line number that
this is line number 2
this is line number 3
this is line number 4

[root@centos7 ~]# cat test1-copy.txt
this is line number that
```

#### 替换字符

如果内容包含`/`，需要加上前缀进行转义：

```shell
sed -e 's/virt-v2v/\/home\/tmp\/usr\/local\/bin\/virt-v2v/g' convert.sh
```

或者使用`!`来方便书写，多个命令使用分号隔开：

```bash
sed 's!this!that!; s/is/it/' test1.txt
```

注意：当使用`!`时必须使用单引号包围命令。

### 转换字符

自动找到所有的字符进行替换，inchars与outchars的长度必须相同：

```
[root@centos7 ~]# rm -f test1-copy.txt
[root@centos7 ~]# sed 'y/hijk/lmns/' test1.txt
tlms ms lmne number 1
tlms ms lmne number 2
tlms ms lmne number 3
tlms ms lmne number 4
```

### 插入多行

```bash
[root@dev vitest]# nl /etc/passwd | sed '2i first line\
> second line'
     1  root:x:0:0:root:/root:/bin/bash
first line
second line
     2  bin:x:1:1:bin:/bin:/sbin/nologin
```

### 取代行

```shell
[root@dev vitest]# nl /etc/passwd | sed '1,4c No lines.'
No lines.
     5  lp:x:4:7:lp:/var/spool/lpd:/sbin/nologin
```

### 打印行

```shell
[root@dev vitest]# nl /etc/passwd | sed -n '1,3p'
     1  root:x:0:0:root:/root:/bin/bash
     2  bin:x:1:1:bin:/bin:/sbin/nologin
     3  daemon:x:2:2:daemon:/sbin:/sbin/nologin
```

打印出不可打印的字符

```
[root@centos7 ~]# sed 'l' test1.txt
this is line number 1$
this is line number 1
this is line number 2$
this is line number 2
```

### 读写文件

读取文件并插入：

```
[root@centos7 ~]# cat test2.txt
this is an added line.
[root@centos7 ~]# sed '/1/r test2.txt' test1.txt
this is line number 1
this is an added line.
this is line number 2
```

## sed 进阶

样本文件：

```
[root@centos7 ~]# cat test1.txt
this is line number 1
this is line number 2
this is line number 3
this is line number 4
```

### 多行命令

sed 默认一次之处理一行数据，若想处理多行，则需要使用 多行命令，有以下三种：

- `N`：将数据流中的下一行加进来创建一个`多行组`（multiline group）进行处理
- `D`：删除多行的第一行
- `P`：打印多行的第一行

#### next 命令

##### 单行的 next 命令

`n`命令会让 sed 将下一行加载到模式空间（工作空间）进行处理。

```
[root@centos7 ~]# sed '/1/{n ;d}' test1.txt
this is line number 1
this is line number 3
this is line number 4
```

##### 多行的 next 命令

`N`命令会让 sed 将下一行添加到模式空间已有的文本之后再进行处理。

```
# 只会替换第一行的数据
[root@centos7 ~]# sed 'N; s/number/num/' test1.txt
this is line num 1
this is line number 2
this is line num 3
this is line number 4
# 也可以跨行替换
[root@centos7 ~]# sed '/1/{N ; s/\n/  $$$$ /}' test1.txt
this is line number 1  $$$$ this is line number 2
this is line number 3
this is line number 4
```

注意，如果恰好读到最后一行，那么就没有数据给 sed 去添加了，sed 会停止工作：

```
[root@centos7 ~]# sed '/4/{N ; s/4/5/}' test1.txt
this is line number 1
this is line number 2
this is line number 3
this is line number 4
```

解决这个问题的方法是将单行命令放在 N 前面（前提条件是该文件是奇数行）：

```
[root@centos7 ~]# sed -i '/^$/d' test1.txt
[root@centos7 ~]# sed 's/3/5/; N; s/number/num/' test1.txt
this is line num 1
this is line number 2
this is line number 5
```

#### 多行删除命令

`D`只删除多行中的第一行：

```
[root@centos7 ~]# sed 'N ; /1/d' test1.txt
this is line number 3
[root@centos7 ~]# sed 'N ; /1/D' test1.txt
this is line number 2
this is line number 3
```

删除header所在行的上一行空白行：

```
[root@centos7 ~]# sed '/^$/{N ; /header/D}' blank.txt
```

`D`的独特之处在于强制 sed 返回到脚本的起始处：

```
[root@centos7 ~]# sed -i '$a this is line number 4' test1.txt
[root@centos7 ~]# cat test1.txt
this is line number 1
this is line number 2
this is line number 3
this is line number 4
[root@centos7 ~]# sed 'N ; D ; P' test1.txt
this is line number 4
[root@centos7 ~]# sed 'N ; P ; D' test1.txt
this is line number 1
this is line number 2
this is line number 3
this is line number 4
```

#### 多行打印命令

`P`只打印多行中的第一行。

### 模式空间与保持空间

sed 有一块`模式空间`（pattern space）用于存储待编辑的文本；还有一块`保持空间`（hold space）用于保存一些临时的文本。

下面是一些操作命令：

```
h	将模式空间复制到保持空间
H	将模式空间附加到保持空间
g	将保持空间复制到模式空间
G	将保持空间附加到模式空间
x	交换两个空间的内容
```

反转两行：

```
[root@centos7 ~]# sed -n '/2/{ h; n; p; g; p }' test1.txt
this is line number 3
this is line number 2
```

反转整个文件内容：

```
[root@centos7 ~]# sed -n '{1!G; h; $p}' test1.txt
this is line number 4
this is line number 3
this is line number 2
this is line number 1
```

解析：G 将保持空间附加到模式空间（此时只有读取到的一行）的后面，然后再将附加完成的模式空间复制到保持空间，这样就完成了类似于栈的数据结构，当作用到最后一行的时候，此时模式空间就已经完成了文本反转，打印出来即可。

### 改变执行流程

#### 分支

触发条件之后进行命令跳转：

```
[address]b [label]
```

*label*定义了要跳转的位置，如果不指定的话则跳转到脚本的结尾处，即*不执行后续的任何命令*。

```
[root@centos7 ~]# sed '1,2b ; s/this/that/ ; s/number/num/' test1.txt
this is line number 1
this is line number 2
that is line num 3
that is line num 4
```

定义标签的情况下，*匹配的行会执行标签后续的所有命令，不匹配的行则执行除 b 命令之外的全部命令*。

```
[root@centos7 ~]# sed '1,2b simple; s/this/that/ ; :simple ; s/number/num/' test1.txt
this is line num 1
this is line num 2
that is line num 3
that is line num 4
```

标签的最大长度是7。

标签也可以位于 b 命令的前方，达到循环效果：

```
[root@centos7 ~]# sed -n ':start ; s/s/N/p ; /s/b start' test1.txt
thiN is line number 1
thiN iN line number 1
thiN is line number 2
thiN iN line number 2
thiN is line number 3
thiN iN line number 3
thiN is line number 4
thiN iN line number 4
```

注意不要搞成无现循环了哦：

```
[root@centos7 ~]# sed -n ':start ; s/[0-9]/N/p ; /s/b start' test1.txt
this is line number N
^C
```

#### 测试

测试命令会根据替换命令的成功与否来进行跳转。

```
[subsitution-command]t [label]
```

用法与分支命令类似，但测试命令不会造成无限循环：

```
[root@centos7 ~]# sed -n ':start ; s/is/N/p ; t start' test1.txt
thN is line number 1
thN N line number 1
thN is line number 2
thN N line number 2
thN is line number 3
thN N line number 3
thN is line number 4
thN N line number 4
```

### 模式替代

#### &符号

`&`符号可以用于表示匹配到的字符串。

```
[root@centos7 ~]# sed -n 's!this!& is!p' test1.txt
this is is line number 1
this is is line number 2
this is is line number 3
this is is line number 4
```

#### 子模式

使用小括号来定义替换模式中的子模式，然后在替代字段中使用`\数字`的方式引用子模式。

```
[root@centos7 ~]# sed -n 's!th\(.s\)!\1!p' test1.txt
is is line number 1
is is line number 2
is is line number 3
is is line number 4
```

## 创建 sed 实用工具

### 加倍行间距

```
[root@centos7 ~]# sed '$!G' test1.txt
this is line number 1

this is line number 2

this is line number 3

this is line number 4
```

若本来就有空白行，它会把空白行也加倍。解决办法是先把空白行删除，再插入新的空白行:

```bash
sed '/^$/d ; $!G' test1.txt
```

### 给文件中的行编号

```
[root@centos7 ~]# sed '=' test1.txt | sed 'N ; s/\n/ /'
1 this is line number 1
2 this is line number 2
3 this is line number 3
4 this is line number 4
```

### 打印末尾几行

打印最后2行：

```
[root@centos7 ~]# sed ':start ; {$q ; N ; 3,$D ; b start}' test1.txt
this is line number 3
this is line number 4
```

解析：用 N 将前面的行一直连接起来，等读到第3行时，开始从第一行删除，删除的行的数量是 $ - N 行，故要从 N + 1 行开始算起（假设开始计算的行数为x，那么需要满足 $ - x + 1 = $ - N），剩下的就是未被删除的行，等读到末行时再终止循环，打印出结果。

### 删除空白行

#### 删除连续的空白行

关键在于创建*包含一个非空白行和一个空白行*的地址区间。

```bash
sed '/./,/^$/!d' file.txt
```

#### 删除开头的空白行

思路就是反向匹配

```bash
sed '/./,$!d' file.txt
```

#### 删除结尾的空白行

将末尾的空白行连接起来，然后删除。

```
sed ':start ; /^\n*$/{$d ; N ; b start}' file.txt
```

### 删除HTML标签

```
sed 's/<[^>]*>//g' file.txt
```

