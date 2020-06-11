https://docs.python.org/2.7/library/subprocess.html#frequently-used-arguments

该模块让你产生新的进程，连接到它们的`input/output/error`管道，获取它们的返回码。简单来说就是调用命令，并获取输出和退出的状态码。

官方强烈推荐POSIX用户使用第三方模块：[subprocess32](https://pypi.org/project/subprocess32/)（用于Python 2的Python 3子进程模块的后端接口。）。

## 使用subprocess模块

以下是简单使用方法，本质上调用了`Popen`接口，高级用法查阅下文的`Popen`接口。

#### `subprocess.call`(*args*, \*, *stdin=None*, *stdout=None*, *stderr=None*, *shell=False*)

运行*args*命令。等待命令完成，然后返回*returncode*属性。

```python
>>> subprocess.call(["ls", "-l"])
0

>>> subprocess.call("exit 1", shell=True)
1
```

!!! warning
	不要在这个函数中使用`stdout=PIPE`或`stderr=PIPE`。

#### `subprocess.check_call`(*args*, \*, *stdin=None*, *stdout=None*, *stderr=None*, *shell=False*)

同上，返回值为0时则返回，否则抛出[`CalledProcessError`](https://docs.python.org/2.7/library/subprocess.html#subprocess.CalledProcessError)异常，异常中有*returncode*属性。

```python
>>> subprocess.check_call(["ls", "-l"])
0

>>> subprocess.check_call("exit 1", shell=True)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "/usr/lib64/python2.7/subprocess.py", line 542, in check_call
    raise CalledProcessError(retcode, cmd)
subprocess.CalledProcessError: Command 'exit 1' returned non-zero exit status 1
```

警告同上。

#### `subprocess.check_output`(*args*, \*, *stdin=None*, *stderr=None*, *shell=False*, *universal_newlines=False*)

将输出作为字节字符串返回。返回值非0时抛出[`CalledProcessError`](https://docs.python.org/2.7/library/subprocess.html#subprocess.CalledProcessError)异常，异常对象中有*returncode*和*output*属性。

```python
>>> subprocess.check_output("echo 123456", shell=True)
'123456\n'

>>> subprocess.check_output("ll", shell=True)
/bin/sh: ll: command not found
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "/usr/lib64/python2.7/subprocess.py", line 575, in check_output
    raise CalledProcessError(retcode, cmd, output=output)
subprocess.CalledProcessError: Command 'll' returned non-zero exit status 127
```

也可以将标准错误重定向到标准输出，注意`exit 0`：

```python
>>> subprocess.check_output("ll; exit 0", stderr=subprocess.STDOUT, shell=True)
'/bin/sh: ll: command not found\n'
```

!!! warning
	不要在这个函数中使用`stderr=PIPE`。

#### `subprocess.PIPE`

可用作Popen的*stdin*，*stdout*， *stderr*参数的特殊值，表示应打开标准流的管道。

#### `subprocess.STDOUT`

可以用作Popen的*stderr*参数的特殊值，表示标准错误应该与标准输出进入相同的句柄。

#### `returncode`

子进程的退出状态

## 经常使用的参数

[`Popen`](https://docs.python.org/2.7/library/subprocess.html#subprocess.Popen)拥有众多的参数，下面介绍使用频率最高的几个。

- *args*：要执行的命令，必要参数。可以是字符串或者序列（列表，推荐值，因为它允许模块处理任何所需的转义和引用参数。例如，允许文件名中的空格）。若是字符串，必须使用**`shell=True`选项**或者**命令没有附带任何参数**。
- *stdin*， *stdout*，*stderr*：指定命令的标准输入、标准输出和标准错误的文件句柄。可选的值有四个：
    - [`PIPE`](https://docs.python.org/2.7/library/subprocess.html#subprocess.PIPE)：指示子进程应该创建一个新的管道。
    - 存在的文件描述符（一个正数）
    - 存在的文件对象
    - `None`：不会进行重定向的操作；子进程的文件句柄从父进程继承。
    - 另外，*stderr*的值可以是`subprocess.STDOUT`。

- 当stdout或stderr的值是PIPE且universal_newlines为True时，所有行的结尾将转换为`\n`。
- *shell*：控制是否通过shell执行命令。值为`True`时，将支持shell特性（比如shell管道、文件名通配符、环境变量扩展、`~`扩展等）。但是python也提供了许多类shell特性的实现。

!!! warning
	从不信任的输入源不接受输入后未经处理直接作为命令的输入会导致程序漏洞[shell injection](http://en.wikipedia.org/wiki/Shell_injection#Shell_injection)。因此，在从外部输入构造命令字符串的情况下，强烈建议不要使用`shell=True`。`shell=False`会禁用所有的基本shell功能，但不会引起这个漏洞。

```python
>>> from subprocess import call
>>> filename = input("What file would you like to display?\n")
What file would you like to display?
# 高危动作！！！
non_existent; rm -rf / #
>>> call("cat " + filename, shell=True) # Uh-oh. This will end badly...
```

## Popen 构造器

```python
class subprocess.Popen(args, bufsize=0, executable=None, stdin=None, stdout=None, stderr=None, preexec_fn=None, close_fds=False, shell=False, cwd=None, env=None, universal_newlines=False, startupinfo=None, creationflags=0)
```

*args*应该是列表或字符串。默认情况下，如果是列表则执行列表中的第一个条目；如果是字符串则视平台决定。

*args*是字符串的情况：

- Unix：该字符串被解释为要执行的程序的名称或路径。但是，这只适用于没有向这个程序传参的情况。

*args*是列表的情况：

- Windows：被转换为一个字符串。

**`shell=True`时，建议将*args*作为一个字符串而不是列表传入**：

- Unix：shell默认是`/bin/sh`，可以通过指定*executable*参数来替换。若*args*是字符串，则必须安照实际在shell操作时的格式来写字符串（比如文件名中的空格要使用转义符号或者用引号包围）；若*args*是列表，则第一项指定命令，其它项被视为shell本身的参数，**而不是命令的参数**。相当于：

```python
Popen(['/bin/sh', '-c', args[0], args[1], ...])
```

- Windows：环境变量`COMSPEC`指定默认shell。唯一需要`shell=True`的情况就是执行内置Windows命令（dir、copy等）的时候。运行批量脚本或基于控制台的可执行文件时不需要设置此参数。