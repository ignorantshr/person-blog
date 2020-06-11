[TOC]

## 概念

[`asyncore`](https://docs.python.org/2.7/library/asyncore.html?highlight=dispatcher#)是Python的异步通信模块。

此模块提供了编写异步套接字服务客户端和服务端的基本结构。

是类比多线程的一种实现方式。在I/O受限的情况下很实用。多线程适用受限于处理器的程序。

该模块提供了一些类，它们的基本思想是创建一个或多个网络通道`channel`。

一旦初始channel被创建，就调用`loop()`方法，将一直持续到最后一个channel（包括在异步服务期间添加到map中的channel）关闭。

下面是一些方法和类。

## *asyncore.loop([timeout[, use_poll[, map[, count]]]])*

进入一个polling循环，直到到达了循环次数*count*或所有的channel关闭。*timeout*用于设置相应的 [`select()`](https://docs.python.org/2.7/library/select.html#select.select) or [`poll()`](https://docs.python.org/2.7/library/select.html#select.poll) 调用，以秒为单位，默认30秒。*use_poll* 为*True*时，优先使用`poll()`，默认为*False*。

*map*是将socket.id与监视的channel映射起来的字典。channels被关闭时将从map中删除。channels（[`asyncore.dispatcher`](https://docs.python.org/2.7/library/asyncore.html?highlight=dispatcher#asyncore.dispatcher), [`asynchat.async_chat`](https://docs.python.org/2.7/library/asynchat.html#asynchat.async_chat) 及它们子类的实例）可以被自由混合。

## *class asyncore.dispatcher*

它是对低级的[`socket`](https://docs.python.org/2.7/library/socket.html#module-socket)对象的简单封装。它有几个简单的`event-handling`方法用于异步循环的调用。它可以被看作是正常的非阻塞式的socket对象。

在某些时间或某些连接状态触发的**低级别事件**（如可读、可写状态）会告诉异步循环某些**更高级别的事件**（如连接状态）已经发生（比如，socket可写时该socket连接已经被建立了）。隐含的更高级别事件是：

|       Event        |                     Description                      |
| :----------------: | :--------------------------------------------------: |
| `handle_connect()` |   暗示在 the first read or write event 时已经发生    |
|  `handle_close()`  | 暗示在 a read event with no data available时已经发生 |
| `handle_accept()`  | 暗示在 a read event on a listening socket时已经发生  |

在异步处理期间，每个被映射的channel的[`readable()`](https://docs.python.org/2.7/library/asyncore.html?highlight=dispatcher#asyncore.dispatcher.readable) and [`writable()`](https://docs.python.org/2.7/library/asyncore.html?highlight=dispatcher#asyncore.dispatcher.writable)方法用于指示是否channel的socket应该被加入到读写事件列表。

于是，channel的事件范围要比基本的socket事件大。下方列表的方法可在子类中重写：

- `handle_read`()

    当异步循环检测到通道套接字上的read()调用会成功时调用。

- `handle_write`()

    当异步循环检测到一个可写的socket能被写入时调用。通常，此方法为了性能考虑将实现必要的缓冲。例如：

```python
def handle_write(self):
	sent = self.send(self.buffer)
	self.buffer = self.buffer[sent:]
```

-  `handle_expt`()

    当socket连接带有out of band (OOB)数据的时候调用。OOB支持很少，几乎不用。

- `handle_connect`()

- `handle_close`()

- `handle_error`()

    在引发异常并且未以其他方式处理时调用。默认打印精简回溯信息。

- `handle_accept`()

- `readable`()

    默认是*True*。

- `writable`()

    默认是*True*。

此外，每个通道都委托或扩展了许多**套接字方法**。其中大多数与其套接字几乎完全相同。如下：

- `create_socket`(*family*, *type*)

- `connect`(*address*)

    作为一个正常的socket对象。*address* 是元组，第一个元素是主机，第二个元素时端口号。

- `send`(*data*)

- `recv`(*buffer_size*)

    从socket的终端读取最大*buffer_size*个字节。空字符串意味着channel已经从另一端被关闭了。注意，recv()使用[`EAGAIN`](https://docs.python.org/2.7/library/errno.html#errno.EAGAIN) or [`EWOULDBLOCK`](https://docs.python.org/2.7/library/errno.html#errno.EWOULDBLOCK)可能会引发[`socket.error`](https://docs.python.org/2.7/library/socket.html#socket.error)，即使select.select()或select.poll()已报告套接字已准备好进行读取。

- `listen`(*backlog*)

    侦听对套接字的连接。*backlog* 指定了最大排队连接数量，至少为1；最大的连接数量是系统决定的（通常是5）。

- `bind`(*address*)

- `accept`()

- `close`()

## *class asyncore.dispatcher_with_send*

dispatcher的子类，添加了简单的缓冲输出能力。更复杂的用法使用[`asynchat.async_chat`](https://docs.python.org/2.7/library/asynchat.html#asynchat.async_chat)。

## *class asyncore.file_dispatcher*

file_dispatcher接受文件描述符或文件对象以及可选的map参数，并将其包装以与poll()或loop()函数一起使用。

如果提供了一个文件对象或任何带有`fileno()`方法的对象，那么该方法将被调用并传递给`file_wrapper`构造器。只在UNIX可用。

## *class asyncore.file_wrapper*

file_wrapper采用整数文件描述符并调用[`os.dup()`](https://docs.python.org/2.7/library/os.html#os.dup)来复制句柄，以便原始句柄可以独立于file_wrapper而关闭。

此类实现了足够的方法来模拟socket以供`file_dispatcher`类使用。只在UNIX可用。

## 举例

客户端向socket中写数据：

```python
from asyncore import dispatcher
import asyncore
import socket
_map = {}


# 接收的信息经常不完整，why
class EchoClient(dispatcher):
    def __init__(self, addr, data):
        dispatcher.__init__(self)
        self.data = data
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect(addr)

    def handle_write(self):
        while self.writable():
            sent = self.send(self.data)
            print "send %d data." % sent
            self.data = self.data[sent:]

    def writable(self):
        return len(self.data) > 0

    def handle_read(self):
        re = self.recv(8192)
        print "%s recv %d : %s" % (repr(self.socket), len(re), re)
        # self.handle_close()

    def handle_close(self):
        print "close client %s" % repr(self.addr)
        self.close()

echoClient1 = EchoClient(("127.0.0.1", 9001), "abcdefg")
echoClient2 = EchoClient(("127.0.0.1", 9001), "hijklmn")

_map[echoClient1._fileno] = echoClient1
_map[echoClient2._fileno] = echoClient2
# asyncore.loop(map=_map, count=2)
asyncore.loop(use_poll=True, map=_map, timeout=.5, count=2)
for dis in _map.values():
    dis.handle_close()
_map.clear()
```

服务端监听并读取数据：

```python
import asyncore
from asyncore import dispatcher as dispatcher
import socket


# 这里必须另写一个处理的服务器，否则服务端不会关闭socket
class EchoHandler(dispatcher):

    _data = ''

    def handle_read(self):
        data = self.recv(1024)
        self._data += data
        # self.send("send %d from server: %s" % (len(data), data))
        # self.send("send %d from server: %s" % (len(self._data), self._data))
        # self._data = ''

    def handle_write(self):
        while self.writable():
            sent = self.send(self._data)
            msg = "\nsend %d from server." % (len(self._data))
            self._data = self._data[sent:]
            self.send(msg)

    def writable(self):
        return len(self._data) > 0

    def handle_close(self):
        print ("close %s" % repr(self.addr))
        self.close()


class EchoServer(dispatcher):

    def __init__(self, host, port):
        dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(2)

    def handle_accept(self):
        cli = self.accept()
        if cli is not None:
            conn, addr = cli
            print("get connect from %s.\n" % repr(addr))
            EchoHandler(conn)

server = EchoServer("127.0.0.1", 9001)
asyncore.loop()
```

