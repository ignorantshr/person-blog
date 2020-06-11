## 使用说明

```
gawk [ POSIX or GNU style options ] -f program-file [ -- ] file ...
gawk [ POSIX or GNU style options ] [ -- ] program-text file ...

program-text 使用单引号包围
OPTIONS
	-F fs  --field-separator fs		指定分隔符
	-v var=value					定义程序中的变量及默认值，引用变量时无需$符号
	-mf	N							指定处理的最大字段数
	-mr	N							指定文件最大行数

program
Built-in Variables
	FS          The input field separator, a space by default.
    OFS         The output field separator, a space by default.
	RS          The input record separator, by default a newline.
	ORS         The output record separator, by default a newline.
	NF          The number of fields in the current input record.
	NR          The total number of input records seen so far.
	
	$0			代表一整行
	$n			代表该行的第n个字段

Patterns
	BEGIN
    END
    BEGINFILE
    ENDFILE
    /regular expression/
    relational expression
    pattern && pattern
    pattern || pattern
    pattern ? pattern : pattern
    (pattern)
    ! pattern
    pattern1, pattern2

Control Statements
"statement" 可以用花括号包围起来
	if (condition) statement [ else statement ]	单行使用else语句时要添加分号。if (...) ... ; else ...
    while (condition) statement
    do statement while (condition)
    for (expr1; expr2; expr3) statement	 C风格的for循环，只不过定义变量时无需指定类型
    for (var in array) statement
    break
    continue
    delete array[index]
    delete array
    exit [ expression ]
    { statements }
    switch (expression) {
    case value|regex : statement
    ...
    [ default: statement ]
    }
```

脚本中的语法一样，但是不需要分号分隔多个命令：

```
[root@centos7 ~]# cat script.gawk
{
text = "Tom"
print text " is a superman."
}
[root@centos7 ~]# gawk -f script.gawk test1.txt
Tom is a superman.
Tom is a superman.
Tom is a superman.
Tom is a superman.
Tom is a superman.
```

样本文件：

```bash
[user1@centos7 ~]$ cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35
```

## 变量

### 内置变量

```
FIELDWIDTHS	定义每个字段的固定宽度，以长度来划分字段
FS			输入字段分隔符，默认为一个空白字符，即空格符或制表符
OFS			输出字段分隔符，默认为一个空白字符
RS			输入记录分隔符，默认为一个空白字符
ORS			输出记录分隔符，默认为一个空白字符
```

设置了 FIELDWIDTHS 后，就会忽略 FS 变量了，并且一旦设定就不能再改变了。

```bash
[user1@centos7 ~]$ gawk 'BEGIN{FIELDWIDTHS="2 5"}{print $1,$2}' data1
da ta11,
da ta21,
da ta31,
```

对于需要将*多行当作一行*读取的可以这样设置 FS、RS：

```bash
[user1@centos7 ~]$ cat data2
aaa
1990
111111111
beijng

bbb
2000
222233
dongjing
[user1@centos7 ~]$ gawk 'BEGIN{FS="\n";RS=""}{print $1,$4}' data2
aaa beijng
bbb dongjing
```

### 数据变量

和数据有关的内置变量：

```
ARGC	当前命令行参数的个数，不计算脚本
ARGV	包含命令行参数的数组，不计算脚本，索引从0开始
ENVIRON	当前shell环境变量组成的数组，索引是shell环境变量名，例如 HOME、PATH
FILENAME	作为数据输入的文件名，在BEGIN中无法使用
FNR		当前数据文件中的处理数据行数
NR		处理的行数
NF		当前行的字段数
```

参数相关变量：

```bash
[user1@centos7 ~]$ gawk 'BEGIN{print ARGC;for (i in ARGV) print ARGV[i]}' data1
2
gawk
data1
```

shell环境变量：

```bash
[user1@centos7 ~]$ gawk 'BEGIN{FS=",";print "data","FNR","NR"}{print $1,FNR,NR}' data1 data1
data FNR NR
data11 1 1
data21 2 2
data31 3 3
data11 1 4
data21 2 5
data31 3 6
```

### 自定义变量

变量名可以是 数字、字母及下划线 的组合，但不能以数字开头，并且区分大小写。

#### 在脚本中给变量赋值

```bash
[user1@centos7 ~]$ gawk 'BEGIN{num=3; print num}' data1
3
```

赋值语句还可以是算式：

```bash
[user1@centos7 ~]$ gawk 'BEGIN{num=3; num=num*num^2; print num}' data1
27
```

#### 在命令行上给变量赋值

```bash
[user1@centos7 ~]$ gawk -v num=3 'BEGIN{num=num*num^2; print num}' data1
27
```

若使用`-v`选项来定义变量，那么该参数必须放在脚本之前；若不使用`-v`选项，那么在`BEGIN`块中不可用：

```bash
[user1@centos7 ~]$ gawk 'BEGIN{num=2; num=num*num^2; print num}{print num}' num=3 data1
8
3
3
3
```

### 数组变量

#### 定义与遍历数组

gawk 使用关联数组提供数组功能。数组的索引及值都可以既是*字符串*又可以是*数字*。

```
var[index] = value
```

```bash
[user1@centos7 ~]$ cat array_test.gawk
BEGIN{
a["a"] = 1
a["c"] = "helk"
a["b"] = 0
a[2] = "gfdhj"
}
{
for (e in a)
{
    print "a["e"] =",a[e]
}
}
[user1@centos7 ~]$ gawk -f array_test.gawk -

a[a] = 1
a[b] = 0
a[c] = helk
a[2] = gfdhj
```

注意：索引值不会按照任何特定顺序返回。

#### 删除数组

删除某个元素：

```
delete array[index]
```

删除整个数组：

```
delete array
```

## 匹配模式

gawk 支持多种匹配模式来过数据记录。

### 正则表达式

gawk 支持基础正则表达式（BRE）和扩展正则表达式（ERE）来进行模式匹配。在使用时，正则表达式必须位于花括号前：

```bash
[user1@centos7 ~]$ gawk -F ',' '/,d/{print $1}' data1
data11
data21
data31
```

gawk 会对**记录**进行匹配。

### 匹配操作符

匹配操作符（matching operator）允许将正则表达式*限定在指定的字段*。

```
$N [!]~ /re/
```

若使用了`!`则表示排除匹配。

```bash
[user1@centos7 ~]$ gawk -F ',' '$1 ~ /data21/{print $1}' data1
data21
```

### 数学表达式

在匹配模式中还可以使用数学表达式，用于比较数字。

操作符有：

- ==
- <=
- <
- \>=
- \>

找出所有用户组ID为 0 的用户：

```bash
[user1@centos7 ~]$ gawk -F ':' '$4 == 0{print $1}' /etc/passwd
root
sync
shutdown
halt
operator
```

若对文本数据使用数学表达式则必须完全匹配：

```
[user1@centos7 ~]$ gawk -F ':' '$1 == "root"{print $1}' /etc/passwd
root
```

## 流程控制语句

参考使用说明中的控制语句语法。

使用条件语句：

```shell
[root@dev vitest]# cat -n /etc/passwd | awk -F: 'NR==1 {printf "%12s%12s\n", "name", "bash"}; $3 < 5 {print $1"\t"$7}'
        name        bash
     1  root    /bin/bash
     2  bin     /sbin/nologin
     3  daemon  /sbin/nologin
     4  adm     /sbin/nologin
     5  lp      /sbin/nologin
# 分隔符也可以在脚本里面定义，效果同上
[root@dev vitest]# cat -n /etc/passwd | awk 'BEGIN {FS=":"}; NR==1 {printf "%12s%12s\n", "name", "bash"}; $3 < 5 {print $1"\t"$7}'
# 脚本中定义的变量可以直接使用，无需$符号
[root@dev vitest]# cat -n /etc/passwd | awk 'BEGIN {FS=":"; head1="name"}; NR==1 {head2="bash"; printf "%12s%12s\n", head1, head2}; $3 < 5 {print $1"\t"$7}'
```

awk还支持运算：

```shell
cat pay. txt | \
> awk ' {if(NR==1) printf "%10s %10s %10s %10s %10s\n", $1, $2, $3, $4, "Total"}
> NR>=2{total = $2 + $3 + $4
> printf "%10s %10d %10d %10d %10.2f\n", $1, $2, $3, $4, total}'
```

## 格式化打印

```
printf "format string", var1, var2, ...
```

和C语言一样。

```
e	使用科学计数法表示
o	按照八进制显示
x	按照十六进制显示
X	按照十六进制显示，但使用大写字母
```

另外三种修饰符：

```
width	指定输出字段最小宽度。若短于此值，则右对齐，否则按照实际长度输出
.prec	指定浮点数的小数位数，或字符串的最大字符数
- 		使用左对齐
```

## 内置函数

### 数学函数

```
sin(x)	x是弧度
cos(x)	x是弧度
int(x)	取整，直接取整数部分，不会四舍五入
log(x)	x的自然对数
exp(x)	自然数e的x次方
sqrt(x)	x的平方根
rand()	比0大比1小的随机浮点数
```

不知道为什么随机数函数总是一个值：

```bash
# 第一次执行
[user1@centos7 ~]$ gawk '{print rand()}'

0.237788

0.291066

0.845814
# 第二次执行
[user1@centos7 ~]$ gawk '{print rand()}'

0.237788

0.291066

0.845814
```

### 位操作函数

```
and(v1, v2)	
or(v1, v2)
xor(v1, v2)			异或
lshift(val, count)	左移
rshift(val, count)
compl(val)			val补运算，即 求补码
```

```bash
[user1@centos7 ~]$ gawk 'BEGIN{print compl(1)}'
9007199254740990
[user1@centos7 ~]$ gawk 'BEGIN{print compl(9007199254740990)+1}'
2
```

### 字符串函数

```
asort(s[, d])	对数组s按元素排序，索引会被替换成数字，若提供了d则存储在d数组中
asorti(s[, d])	对数组s按索引排序，索引值会成为元素值，索引会被替换成数字，若提供了d则存储在d数组中
match(s, r[, a])	返回字符串s中正则表达式r出现位置的索引。若指定了数组a，则会存储s中匹配到r的部分
sub(r, s[, t])	查找变量$0或目标字符串t（若提供了t）来匹配正则表达式r，替换第一处。
gsub(r, s[, t])	查找变量$0或目标字符串t（若提供了t）来匹配正则表达式r，全部替换
gensub(r, s, h[, t])	查找变量$0或目标字符串t（若提供了t）来匹配正则表达式r。若h是一个以“g”或“G”
						开头的字符串，则替换全部；若h是一个数字，表示要替换掉第h处。
index(s, t)		返回字符串t在字符串s中的索引值，没有则返回0
length([s])		返回字符串s的长度；若没有指定的话，返回$0的长度
split(s, a[, r])	将s用FS或正则表达式r分割放到数组a中。返回字段总数
sprintf(format, variables)	提供格式化之后的字符串
substr(s, i[, n])	返回s中从索引值i开始的n个字符组成的子串。若未提供n则直到sde结尾。
tolower(s)		
toupper(s)		
```

```bash
[user1@centos7 ~]$ cat array_test.gawk
BEGIN{
a["a"] = 1
a["c"] = "helk"
a["b"] = 0
a[2] = "gfdhj"
}

{
for (e in a)
{
    print "a["e"] =",a[e]
}

print "-----------"

asort(a, b)
for (e in b)
{
    print "b["e"] =",b[e]
}

}
```

元素值排序函数：

```
[user1@centos7 ~]$ gawk -f array_test.gawk

a[a] = 1
a[b] = 0
a[c] = helk
a[2] = gfdhj
-----------
b[4] = helk
b[1] = 0
b[2] = 1
b[3] = gfdhj
```

索引值排序函数：

```
[user1@centos7 ~]$ vi array_test.gawk
……
asorti(a, b)
……
[user1@centos7 ~]$ gawk -f array_test.gawk

a[a] = 1
a[b] = 0
a[c] = helk
a[2] = gfdhj
-----------
b[4] = c
b[1] = 2
b[2] = a
b[3] = b
```

```bash
[user1@centos7 ~]$ gawk 'BEGIN{print gensub("abc", "123", "gsdf", "abcdabc")}'
123d123
[user1@centos7 ~]$ gawk 'BEGIN{print gensub("abc", "123", "Gsdf", "abcdabc")}'
123d123
[user1@centos7 ~]$ gawk 'BEGIN{print gensub("abc", "123", 2, "abcdabc")}'
abcd123
```

### 时间函数

```
mktime(datestr)		将一个按照"YYYY MM DD HH MM SS [DST]"格式指定的日期转换成时间戳
strftime(format[, timestamp])	将当前时间的时间戳或timestamp格式化，采用shell 函数date()的格式
systime()			返回当前时间的时间戳
```

```bash
[user1@centos7 ~]$ gawk 'BEGIN{print strftime("%A, %B %d, %Y")}'
Monday, April 13, 2020
```

## 自定义函数

### 定义函数

```
function name([parameters])
{
	statements
}
```

函数必须写在所有的代码块（包括BEGIN）之前。

```bash
[user1@centos7 ~]$ cat func.gawk
function display(msg){
    print msg
}
BEGIN {
    display("hello!")
}
[user1@centos7 ~]$ gawk -f func.gawk
hello!
```

### 创建函数库

把所有的函数放到一个文件中，然后在命令行中使用`-f`选项使用（可以有多个该选项）。