[TOC]

## select模块

[select](https://docs.python.org/2.7/library/select.html?#module-select)模块提供系统方法`select()`和`poll()`的调用，在linux 2.5+中`epoll()`可用。

在Windows，只支持socket；在其他操作系统，还支持其他的文件类型。

该模块定义了以下方法：

- `select.error`异常

    错误发生时捕获异常。附带的值是一对包含来自`errno`的数字错误代码和相应的字符串。

- `select.epoll`([*sizehint=-1*])

    返回边缘轮询对象（可用作I/O事件的Edge或Level Triggered接口）。

- `select.poll`()

    返回轮询对象（支持注册和取消注册文件描述符，然后从I/O事件轮询它们）。

- `select.select`(*rlist*, *wlist*, *xlist*[, *timeout*])

    这是直接指向Unix`select()`系统调用的接口。前三个参数是*可等待对象*序列：既可以是代表文件描述符的整数值，也可以是含有返回整数值的无参`fileno()`方法的对象：

    - *rlist*：等待直到可读
    - *wlist*：等待直到可写
    - *xlist*：等待一个异常*exceptional condition*（查看操作系统手册）

    允许空序列，但是三个都为空要看系统是否支持（Unix支持，Windows不支持）。

    - *timeout*：以秒为单位的浮点数。此参数被忽略时，此方法被阻塞直到至少一个文件描述符准备完毕。为`0`时指定一个轮询并且永远不会阻塞。

        返回值是准备好的对象列表：子集对应着三个上述列表。达到超时时间却没有准备好的文件描述符将返回三个空列表。

    在序列中可接受的文件类型有：python文件对象，socket.socket()返回的对象，自定义的类（带有属性`fileno()`方法，并且返回的是真正的文件描述符）。

## epoll 对象

epoll - I/O event notification facility。

The  epoll API performs a similar task to poll(2): monitoring multiple file descriptors to see if I/O is possible on any of them.  The epoll API can be used either as an edge-triggered or a level-triggered interface and scales well to large numbers of watched file descriptors.

事件掩码 *eventmask*：

|    Constant    |                           Meaning                            |
| :------------: | :----------------------------------------------------------: |
|   `EPOLLIN`    |                      Available for read                      |
|   `EPOLLOUT`   |                     Available for write                      |
|   `EPOLLPRI`   |                     Urgent data for read                     |
|   `EPOLLERR`   |          Error condition happened on the assoc. fd           |
|   `EPOLLHUP`   |              Hang up happened on the assoc. fd               |
|   `EPOLLET`    | Set Edge Trigger behavior, the default is Level Trigger behavior |
| `EPOLLONESHOT` | Set one-shot behavior. After one event is pulled out, the fd is internally disabled |
| `EPOLLRDNORM`  |                   Equivalent to `EPOLLIN`                    |
| `EPOLLRDBAND`  |               Priority data band can be read.                |
| `EPOLLWRNORM`  |                   Equivalent to `EPOLLOUT`                   |
| `EPOLLWRBAND`  |                Priority data may be written.                 |
|   `EPOLLMSG`   |                           Ignored.                           |

- `epoll.close`()

    关闭epoll 对象的控制文件描述符

- `epoll.fileno`()

    返回控制fd的文件描述符号码

- `epoll.fromfd`(*fd*)

    从给定的文件描述符创建epoll 对象

- `epoll.register`(*fd*[, *eventmask*])

    用epoll 对象注册一个fd文件描述符。注册一个已经存在的文件描述符会引起一个*IOError*。

- `epoll.modify`(*fd*, *eventmask*)

    修改一个注册文件描述符

- `epoll.unregister`(*fd*)

    从epoll 对象移除注册过的文件描述符

- `epoll.poll`([*timeout=-1*[, *maxevents=-1*]])

    等待事件。是以秒为单位的浮点数。

## polling 对象

`poll()`系统调用，在大部分的Unix系统上支持，为同时为多个客户端提供服务的网络服务器提供更好的可扩展性。 `poll()`比`select()`更好地扩展。`select()` is O(highest file descriptor), while `poll()` is O(number of file descriptors).

事件掩码 *eventmask*s 是可选的二进制掩码，描述了想要检查的事件类型，并且可以是以下事件的组合（通过 *位或* 操作），默认检查前三种类型：

|  Constant  |                 Meaning                  |
| :--------: | :--------------------------------------: |
|  `POLLIN`  |          There is data to read           |
| `POLLPRI`  |       There is urgent data to read       |
| `POLLOUT`  | Ready for output: writing will not block |
| `POLLERR`  |       Error condition of some sort       |
| `POLLHUP`  |                 Hung up                  |
| `POLLNVAL` |   Invalid request: descriptor not open   |

- `poll.register`(*fd*[, *eventmask*])

    使用polling 对象注册一个文件描述符。注册已注册的文件描述符不是错误，并且与仅仅注册描述符一样具有相同的效果。

- `poll.modify`(*fd*, *eventmask*)

    修改已经注册过的fd。和`register`方法有一样的效果。若修改未注册过的则会引起一个[`IOError`](https://docs.python.org/2.7/library/exceptions.html#exceptions.IOError)异常，errno是`ENOENT`。

- `poll.unregister`(*fd*)

    移除一个被polling 对象追踪的文件描述符。如果移除不存在的fd则会引起[`KeyError`](https://docs.python.org/2.7/library/exceptions.html#exceptions.KeyError)异常。

- `poll.poll`([*timeout*])

    轮询注册的文件描述符，返回可能为空的包含两个元素`(fd, eventmask)`的元组列表，用于具有要报告的事件或错误的描述符。空列表表明调用超时并且没有文件描述符有事件要报告。若给了*timeout*，则指定系统在返回之前等待事件的时间长度（以毫秒为单位）；若*timeout*被忽略、负数或None，调用会阻塞直到有事件发生。

## 举例

客户端通过socket发送数据：

```python
import socket

cli = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cli.connect(('127.0.0.1', 9002))

while True:
    msg = raw_input('> ')
    cli.send(msg)
    # close socket after sent exit message
    if msg == 'exit':
        cli.close()
        break
    else:
        print cli.recv(8192)
```

服务端通过poll监听socket并接收数据：

```python
import select
import socket

READ_ONLY_MASK = (select.POLLIN | select.POLLPRI | select.POLLHUP |
                      select.POLLERR)

server_soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_soc.bind(('127.0.0.1', 9002))
server_soc.listen(2)
# 在非阻塞模式下，若调用recv()方法不能找到任何数据或send()方法不能立即处理数据，会发生异常，等价于 s.settimeout(0.0)；在阻塞模式下，调用会阻塞，直到它们可以被处理等价于 s.settimeout(None)。
server_soc.setblocking(False)

poller = select.poll()
poller.register(server_soc, select.POLLIN)
_map = {}

while True:
    events = poller.poll(20000)
    if len(events) > 0:
        for fd, event in events:
            if fd == server_soc.fileno():
                print "server event: %d " % event
                cli, addr = server_soc.accept()
                print "get connect from %s." % repr(addr)
                cli.setblocking(False)
                poller.register(cli, READ_ONLY_MASK)
                _map[cli.fileno()] = cli
            else:
                print "cli event: %d " % event
                cli = _map[fd]
                if event == select.POLLIN:
                    data = cli.recv(1024)
                    print "get data from %s: %s" % (repr(cli.getpeername()), data)
                    if data == 'exit':
                        poller.unregister(cli)
                        del _map[fd]
                        cli.close()
                        continue
                    cli.send(data)
    else:
        print "there is no events, exit."
        break

server_soc.close()
```

