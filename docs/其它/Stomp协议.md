[stomp-specification-1.2](https://stomp.github.io/stomp-specification-1.2.html)

## 概念

STOMP是简单（或流式）文本定向消息传递协议，旨在通过中介服务器在客户端之间传递异步消息。

它为在这些客户端和服务器之间传递的消息定义了基于文本的`wire-format`。

## 协议概述

STOMP是一种基于帧`frame`的协议，其帧在HTTP上建模。

一个帧包括：

- 一个命令`command`
- 一组可选头部`header`
- 一个可选主体`body`

**STOMP服务器**被建模为可以向其发送消息的一组目标`destination`。STOMP协议将目标视为不透明字符串，其语法是特定于服务器实现的。着就导致了目标的传递或“消息交换”语义可能因服务器而异，甚至从目标到目标也不同。

**STOMP客户端**是一个用户代理，可以在两种（可能是同时的）模式下运行：

- 作为生产者，通过一个`SEND`帧发送信息到服务器上的目标。
- 作为消费者，向一个给定的目标发送`SUBSCRIBE`帧并从服务器接收消息作为`MESSAGE`帧。

## STOMP 帧

STOMP是一种基于帧的协议，它假定下面有一个可靠的**双向流**网络协议（如TCP）。客户端和服务器将使用通过流发送的STOMP帧进行通信。帧结构：

```
COMMAND
header1:value1
header2:value2

Body^@
```

- 第一部分：以一个以行尾（EOL）结尾的命令字符串开始，该行包含一个OPTIONAL回车符（八位字节13），后跟一个REQUIRED换行符（八位字节10）。
- 第二部分：0个或多个<key>:<value>格式的头部条目。每个条目后跟EOL。
- 空行：表示头部的结束和主体的开始。
- 第三部分：正文。后跟`NULL`八位字节。本文档中的示例将使用ASCII中的`^@`，control-@来表示NULL八位字节。NULL八位字节可以选择性地跟随多个EOL。

本文档中引用的所有命令和标头名称都区分大小写。

除CONNECT和CONNECTED帧之外的所有帧也将转义在生成的UTF-8编码头中找到的任何回车符，换行符或冒号。C风格的字符串文字转义用于编码在UTF-8编码头中找到的任何回车符，换行符或冒号。

### Body

只有`SEND`, `MESSAGE`, and `ERROR`可能有主体。其它的帧类型不允许。

### 标准headers

可以使用某些标题，并且对于大多数帧具有特殊含义。

#### Header content-length

所有的帧都可能含有`content-length`头部。是一个以八位子节计算的消息主体的长度。

如果存在帧体，则`SEND，MESSAGE和ERROR`帧应该包括内容长度头以便于帧解析。如果帧体包含NULL八位字节，则帧必须包含内容长度头部。

#### Header content-type

如果存在帧体，`SEND，MESSAGE和ERROR`帧应该包括一个`content-type`的头，以帮助帧的接收者解释它的主体。如果设置了`content-type`标头，则其值必须是描述正文格式的`MIME`类型。否则，接收器应该认为主体是二进制blob。

以`text/`开头的MIME类型的隐含文本编码是`UTF-8`。如果使用具有不同编码的基于文本的MIME类型，那么您应该追加`;charset=<encoding>`到MIME类型。

所有STOMP客户端和服务器必须支持UTF-8编码和解码。

#### Header receipt（回执）

除CONNECT之外的任何客户端帧都可以指定具有任意值的回执标头。这将使服务器用RECEIPT帧确认客户帧的处理。

```
SEND
destination:/queue/a
receipt:message-12345

hello queue a^@
```

### 重复的Header条目

STOMP服务器可以通过在邮件中添加标头或在邮件中就地修改标头来“更新”标头值。

如果客户端或服务器收到重复的帧头条目，则只应将第一个头条目用作头条目的值。后续值仅用于维护标头的状态更改历史记录，可以忽略。

### 尺寸限制

为防止恶意客户端利用服务器中的内存分配，服务器可能会设置最大限制：

- the number of frame headers allowed in a single frame
- the maximum length of header lines
- the maximum size of a frame body

如果超出这些限制，服务器应该给客户端发送一个`ERROR`帧，然后关闭连接。

### Connection Lingering（连接回环）

STOMP服务器必须能够支持快速连接和断开连接的客户端。

这意味着服务器可能只允许在重置连接之前短时间内关闭连接。

因此，在重置套接字之前，客户端可能不会接收服务器发送的最后一帧（例如，ERROR帧或RECEIPT帧以回复DISCONNECT帧）。

## 连接

STOMP客户端通过发送`CONNECT`帧来启动到服务器的流或TCP连接：

```
CONNECT
accept-version:1.2
host:stomp.github.org

^@
```

如果服务器接受连接尝试，它将使用CONNECTED帧进行响应：

```
CONNECTED
version:1.2

^@
```

服务器可以拒绝任何连接尝试。服务器应该回复一个ERROR帧，解释连接被拒绝的原因，然后关闭连接。

### CONNECT or STOMP 帧

STOMP服务器必须以与`CONNECT`帧相同的方式处理`STOMP`帧。`CONNECT`帧是为了向后兼容保留的帧类型。`STOMP`帧是Stomp1.2新的帧类型，优点是协议嗅探器/鉴别器将能够区分STOMP连接和HTTP连接。

STOMP 1.2 客户端必须设置以下头部：

- `accept-version`：客户端支持的STOMP协议版本。
- `host`：客户端希望连接到的虚拟主机的名称。建议客户端将此设置为建立套接字的主机名，或者设置为他们选择的任何名称。如果此标头与已知虚拟主机不匹配，则支持虚拟主机的服务器可以选择默认虚拟主机或拒绝连接。

STOMP 1.2 客户端可能会设置以下头部：

- `login`：用于对安全的STOMP服务器进行身份验证的用户标识符。
- `passcode`：用于对安全的STOMP服务器进行身份验证的密码。
- `heart-beat`：[Heart-beating](https://stomp.github.io/stomp-specification-1.2.html#Heart-beating)设置或参考下文。

### CONNECTED 帧

STOMP 1.2 服务端必须设置以下头部：

- `version`：会话`session`将使用的STOMP协议的版本。[Protocol Negotiation](https://stomp.github.io/stomp-specification-1.2.html#Protocol_Negotiation)或参考下文。

STOMP 1.2 访问端可能会设置以下头部：

- `heart-beat`：[Heart-beating](https://stomp.github.io/stomp-specification-1.2.html#Heart-beating)设置或参考下文。

- `session`：唯一标识会话的会话标识符。

- `server`：包含STOMP服务信息的字段。该字段必须包含一个服务器名称字段，后跟可选的注释字段，使用一个空格字符分隔。

    格式：`server = name ["/" version] *(comment)`

    例子：`server:Apache/1.3.9`

### Protocol Negotiation（协议选择）

从STOMP 1.1开始，客户端`CONNECT`帧必须包含`accept-version`头部，以逗号分隔表明它支持的版本。若没有此头部则表明支持STOMP 1.0。

两端都支持的最高版本将用于后续的会话通信。

举例：

客户端：

```
CONNECT
accept-version:1.0,1.1,2.0
host:stomp.github.org

^@
```

服务端会响应返回一个最高版本的协议：

```
CONNECTED
version:1.1

^@
```

如果没有两者都支持的版本，则返回`ERROR`帧并关闭连接：

```
ERROR
version:1.2,2.1
content-type:text/plain

Supported protocol versions are 1.2 2.1^@
```

### Heart-beating

心跳用于确保远程端处于活动状态并且可以正常运行。

为了实现心跳机制，每一方都必须声明它能做什么以及它希望对方做什么。在`CONNECT` and `CONNECTED`帧添加`heart-beat`头部。

使用时，心跳头部必须包含两个用逗号分隔的正整数：

- 第一个表示帧的发送端可以做什么
    - 0表示无法发送心跳
    - 否则它是心跳之间可以保证的最小毫秒数
- 第二个表示帧的发送者希望接收到什么
    - 0表示它不想接收心跳
    - 否则它是心跳之间所需的毫秒数

缺少心跳标题必须与`heart-beat:0,0`标题一样对待，即：不能发送并且不想接收心跳。

```
CONNECT
heart-beat:<cx>,<cy>

CONNECTED:
heart-beat:<sx>,<sy>
```

对于从客户端到服务器的心跳：

- 如果<cx>为0（客户端无法发送心跳）或<sy>为0（服务器不想接收心跳）那么将没有心跳
- 否则，每隔`MAX(<cx>，<sy>)`毫秒就会有一次心跳

对于其它方向的心跳，<sx>和<cy>以相同的方式使用。？？？

关于心跳本身，通过网络连接接收的任何新数据都表明远程端是活着的。在给定的方向上，如果每隔<n>毫秒需要心跳：

- 发送方必须至少每<n>毫秒通过网络连接发送新数据
- 如果发送方没有要发送的真实STOMP帧，它必须发送一个行尾（EOL）
- 如果，在一个至少<n>毫秒的时间窗口内，接收器没有收到任何新数据，它可能认为连接已死
- 由于时序不准确，接收器应该容忍并考虑误差范围

## Client Frames

客户端也许会发送一个不在下文中的帧，但是服务器可能会响应一个`ERROR`帧并关闭连接。

### SEND

`SEND`帧将消息发送到消息传递系统中的目标。

要求有一个`destination`的头部，表明了发送到哪里。主体是要发送的信息。

例：

```
SEND
destination:/queue/a
content-type:text/plain

hello queue a
^@
```

!!! note
	STOMP将此目标视为不透明字符串，并且目标名称不承担传递语义的责任。您应该查阅STOMP服务器的文档，以了解如何构造目标名称，该名称为您提供应用程序所需的传递语义。

SEND支持允许事务地发送的`transaction`标头。

应该包含[`content-length`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-length)和[`content-type`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-type)头。

应用程序可以将任意用户定义的标头添加到SEND帧。用户定义的标头通常用于允许使用者使用`SUBSCRIBE`帧上的选择器基于应用程序定义的标头过滤消息。用户定义的头必须在`MESSAGE`帧中传递。

如果服务器不能处理`SEND`帧，必须发送给客户端一个`ERROR`帧并关闭连接。

### SUBSCRIBE

`SUBSCRIBE`用于注册以监听给定的目标。也要包含`destination`表明客户端想要监听哪个目标。**在订阅目标上接收的任何消息此后将作为`MESSAGE`帧从服务器传送到客户端**。`ack`标头控制消息确认模式。

例：

```
SUBSCRIBE
id:0
destination:/queue/foo
ack:client

^@
```

如果服务器无法成功创建订阅，则服务器必须向客户端发送`ERROR`帧，然后关闭连接。

#### SUBSCRIBE id Header

由于单个连接可以与服务器进行多次订阅，故必须在帧中包含一个`id`头以唯一地标识订阅。

#### SUBSCRIBE ack Header

`ack` 头有三种合法值：

- `auto`（默认值）

    客户端无需为它收到的信息向服务端发送`ACK`帧。此确认模式可能导致传输到客户端的消息被丢弃。

- `client`

    客户端必须为它收到的信息向服务端发送`ACK`帧。如果在客户端发送消息的ACK帧之前连接失败，则服务器将假定消息尚未处理，并且可以将消息重新发送给另一个客户端。客户端发送的ACK帧将被视为**累积确认**。这意味着确认操作对ACK帧中指定的消息以及在ACK消息之前发送到订阅的所有消息都有效。

    如果客户端没有处理某些消息，它应该发送`NACK`帧告诉服务器它没有消耗这些消息。

- `client-individual`。

    确认操作就像客户端确认模式一样，除了客户端发送的ACK或NACK帧不是累积的。这意味着后续消息的`ACK`或`NACK`帧不得导致先前的消息得到确认。

### UNSUBSCRIBE

`UNSUBSCRIBE`用于移除存在的订阅。`id`标头必须与现有订阅的订阅标识符匹配。

例：

```
UNSUBSCRIBE
id:0

^@
```

### ACK

确认客户端消费了信息（参考[SUBSCRIBE ack Header](#522-subscribe-ack-header)）。在通过ACK确认消息之前，不会认为从此订阅接收的任何消息已被消耗。

ACK帧必须包括与正被确认的`MESSAGE`的ack头匹配的`id`头。可选地，可以指定事务头`transaction`，表明消息确认应该是命名事务的一部分。

```
ACK
id:12345
transaction:tx1

^@
```

### NACK

`NACK`与`ACK`相反。它用于告诉服务器客户端没有使用该消息。

然后，服务器可以将消息发送到其他客户端，丢弃它，或将其放入死信队列。确切的行为是特定于服务器的。

NACK采用与ACK相同的标头：`id`（必需）和`transaction`（可选）。

NACK也同样适用`client-individual`和`client`确认模式。

### BEGIN

`BEGIN`用于启动事务。在这种情况下的事务适用于发送和确认——在事务期间发送或确认的任何消息将基于以原子方式处理。

```
BEGIN
transaction:tx1

^@
```

事务标头`transaction`是必需的，事务标识符将用于`SEND，COMMIT，ABORT，ACK和NACK`帧以将它们绑定到指定的事务。在同一连接中，不同的事务必须使用不同的事务标识符。

如果客户端发送`DISCONNECT`帧或`TCP连接`因任何原因失败，则将隐式中止任何尚未提交的已启动事务。

### COMMIT

`COMMIT`用于提交正在进行的事务。

```
COMMIT
transaction:tx1

^@
```

事务头是必需的，并且必须指定要提交的事务的标识符。

### ABORT

ABORT用于回滚正在进行的事务。

```
ABORT
transaction:tx1

^@
```

事务头是必需的，并且必须指定要提交的事务的标识符。

### DISCONNECT

客户端可以通过关闭套接字随时断开与服务器的连接，但不能保证服务器已收到先前发送的帧。

要进行正常关闭（客户端确保服务器已收到所有先前的帧），客户端应该：

1. 发送一个带有`receipt`头部的`DISCONNECT`帧。例：

```
DISCONNECT
receipt:77
^@
```

2. 等待响应`DISCONNECT`的`RECEIPT`帧。例：

```
RECEIPT
receipt-id:77
^@
```

3. 关闭socket。

!!! warning
	如果服务器过快地关闭服务器端的套接字，则客户端可能永远不会收到预期的RECEIPT帧。[Connection Lingering](https://stomp.github.io/stomp-specification-1.2.html#Connection_Lingering)或参考上文。

发送DISCONNECT帧后，客户端不得再发送任何帧。

## Server Frames

有时，服务器会向客户端发送帧（除了初始的CONNECTED帧）。

### MESSAGE

`MESSAGE`帧用于将消息从订阅处传送到客户端。

必须包含：`destination`表明信息将要发送的目标的头部； `message-id`唯一标识头部；`subscription`头部，表示与正在接收消息的订阅的标识符匹配的订阅标头。。

若从订阅处接收的信息要求却认，那么MESSAGE帧必须包含带有任意值的`ack`头部。

主体部分包含信息的内容。

```
# 客户端订阅
SUBSCRIBE
id:0
destination:/queue/a

# 服务端接收信息并发送给订阅者
MESSAGE
subscription:0
message-id:007
destination:/queue/a
content-type:text/plain

hello queue a^@
```

MESSAGE帧应该包括内容[`content-length`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-length)和[`content-type`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-type)标题（如果存在正文）。

MESSAGE帧还将包括在将消息发送到目的地时存在的所有用户定义的标头。请参阅服务器的文档，找出它添加到消息中的服务器特定标头。

### RECEIPT

一旦服务器成功处理了请求回执的客户端帧，就会从服务器向客户端发送`RECEIPT`帧。

必须包含`receipt-id`头部，其值是请求回执的帧的`receipt`头部值。

```
RECEIPT
receipt-id:message-12345

^@
```

由于STOMP是基于流的，因此回执也是**累积确认**（认为先前所有的帧都已经被服务器接收到了）。但是，这些先前的帧可能尚未完全处理。如果客户端断开连接，以前接收的帧应该继续由服务器处理。

### ERROR

如果出现问题，服务器可以发送`ERROR`帧。在这种情况下，它必须在发送ERROR帧后立即关闭连接。

ERROR帧应该包含一个带有错误简短描述的`message`头部，并且正文可以包含更多详细信息（或者可以为空）。

```
ERROR
receipt-id:message-12345
content-type:text/plain
content-length:171
message: malformed frame received

The message:
-----
MESSAGE
destined:/queue/a
receipt:message-12345

Hello queue a!
-----
Did not contain a destination header, which is REQUIRED
for message propagation.
^@
```

如果错误与客户端发送的特定帧有关，则服务器应该添加额外的标头以帮助识别导致错误的原始帧。

ERROR帧应该包括内容[`content-length`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-length)和[`content-type`](https://stomp.github.io/stomp-specification-1.2.html#Header_content-type)标题（如果存在正文）。