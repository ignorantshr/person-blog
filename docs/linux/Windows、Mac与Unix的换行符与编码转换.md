参考《鸟哥的linux私房菜》。

## 换行符

Windows系统的文件的换行符是`CR`（^M）与`LF`（$），在linux中表现为：

```bash
[sink@dev vitest]$ cat -A .gitignore
autorun.inf^M$
uchkvk.pif^M$
```

可以使用`dos2unix`来与linux格式相互转换。

### 安装

#### 从镜像获取

挂载centos镜像，安装：

```bash
$ rpm -ivh dos2unix-6.0.3-4.el7.x86_64.rpm
```

#### 从yum获取

```bash
$ yum search dos2unix
dos2unix.x86_64 : Text file format converters
```

### 使用

```
dos2unix/unix2dos/mac2unix/unix2mac [options] [file ...] [-n infile outfile ...]

-k, --keepdate        keep output file date
-n, --newfile         write to new file
  infile              original file in new file mode
  outfile             output file in new file mode
-o, --oldfile         write to old file
  file ...            files to convert in old file mode
```

```bash
[sink@dev vitest]$ cat -A .gitignore
autorun.inf^M$
uchkvk.pif^M$
# linux
[sink@dev vitest]$ dos2unix -n .gitignore .gitignore-linux
dos2unix: converting file .gitignore to file .gitignore-linux in Unix format ...
[sink@dev vitest]$  cat -A .gitignore-linux
autorun.inf$
uchkvk.pif$
# Windows
[sink@dev vitest]$ unix2dos -n .gitignore-linux .gitignore-Windows
unix2dos: converting file .gitignore-linux to file .gitignore-Windows in DOS format ...
[sink@dev vitest]$ cat -A .gitignore-Windows
autorun.inf^M$
uchkvk.pif^M$
# macOS
[sink@dev vitest]$ unix2mac -n .gitignore-linux .gitignore-mac
unix2mac: converting file .gitignore-linux to file .gitignore-mac in Mac format ...
[sink@dev vitest]$ cat -A .gitignore-mac
autorun.inf^Muchkvk.pif^M

[sink@dev vitest]$ file .gitignore-*
.gitignore-linux:   ASCII text
.gitignore-mac:     ASCII text, with CR line terminators
.gitignore-Windows: ASCII text, with CRLF line terminators
```

### 其它字符转换方法

```
tr [OPTION]... SET1 [SET2]
从标准输入 转换、去重、删除字符，写入标准输出

-c, -C, --complement    使用完整的 SET1 
-d, --delete            删除 SET1 中的字符
-s, --squeeze-repeats   对相连的单个字符去重

SETs：
	\\              backslash
	\b              backspace
	\n              new line
	\r              return
	\t              horizontal tab
	CHAR1-CHAR2     all characters from CHAR1 to CHAR2 in ascending order
	[CHAR*]         in SET2, copies of CHAR until length of SET1
	[CHAR*REPEAT]   REPEAT copies of CHAR, REPEAT octal（八进制） if starting with 0
	[:alnum:]       all letters and digits
	[:alpha:]       all letters
	[:blank:]       all horizontal whitespace
	[:digit:]       all digits
	[:lower:]       all lower case letters
	[:space:]       all horizontal or vertical whitespace
	[:upper:]       all upper case letters
	[:xdigit:]      all hexadecimal digits
	[=CHAR=]        all characters which are equivalent to CHAR
```

```bash
[root@dev vitest]# echo abcdABCD | tr [:lower:] [:upper:]
ABCDABCD
[root@dev vitest]# echo abcdABCD | tr [a-z] [A-Z]
ABCDABCD
[root@dev vitest]# echo abcdABCD | tr [a-z] [1-9]
1234ABCD

[root@dev vitest]# cat -A .gitignore-Windows | tr -d '^M'
autorun.inf$
uchkvk.pif$
[root@dev vitest]# cat .gitignore-Windows | tr -d '\r' | cat -A
autorun.inf$
uchkvk.pif$

[root@dev vitest]# echo aaabbbb | tr -s a
abbbb
[root@dev vitest]# echo aaabbbb | tr -s a b
b
```



## 编码

在Windows系统中转换编码可以使用`记事本的另存为`、`Nodepade++`等工具。



```
iconv [OPTION...] [FILE...]
Convert encoding of given files from one encoding to another.

-f, --from-code=NAME       encoding of original text
-t, --to-code=NAME         encoding for output
-l, --list                 list all known coded character sets
-o, --output=FILE          输出到新的文件。默认更改原文件
```

```bash
# 先在Windows中编写文件，放到linux中
# 打开新终端设置GBK编码，此时可正常查看文件
[sink@dev vitest]$ cat charactor.txt
abcd
大运河
[sink@dev vitest]$ iconv -f gbk -t utf-8 -o charactor-utf8.txt charactor.txt
# 再打开新的终端设置为UTF-8编码，查看文件
[sink@dev vitest]$ cat charactor-utf8.txt
abcd
大运河

# 注意：file命令并不能准确的识别到Windows的文件编码
[sink@dev vitest]$ file -i charactor*
charactor.txt:      text/plain; charset=iso-8859-1
charactor-utf8.txt: text/plain; charset=utf-8
```

