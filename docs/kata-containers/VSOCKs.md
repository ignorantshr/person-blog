## VM 与 主机间的通信方案

虚拟机中的进程与主机中的进程通信有两种不同的方式。第一种是通过使用串行端口，虚拟机中的进程可以从串行端口设备 读/写 数据，主机中的进程可以从 Unix 套接字 读/写 数据。

大多数 GNU/Linux 发行版都支持串行端口，使其成为最可移植的解决方案。但是，串行链接一次只能对一个进程进行读/写访问。要解决此限制，必须对资源(串口和Unix套接字)进行多路复用。在 Kata container 中，这些资源通过使用 [`kata-proxy`](https://github.com/kata-containers/proxy)  和  [Yamux](https://github.com/hashicorp/yamux) 进行多路复用，下面的关系图显示了如何实现它。

```
.----------------------.
| .------------------. |
| | .-----.  .-----. | |
| | |cont1|  |cont2| | |
| | `-----'  `-----' | |
| |       \   /      | |
| |    .---------.   | |
| |    |  agent  |   | |
| |    `---------'   | |
| |         |        | |
| |    .-----------. | |
| |POD |serial port| | |
| `----|-----------|-' |
|      |  socket   |   |
|      `-----------'   |
|           |          |
|       .-------.      |
|       | proxy |      |
|       `-------'      |
|           |          |
|  .------./ \.------. |
|  | shim |   | shim | |
|  `------'   `------' |
| Host                 |
`----------------------'
```

可以使用 `ss | grep kata`命令查看。

<br/>

一个更新更简单的方法是 [VSOCKs](https://wiki.qemu.org/Features/VirtioVsock)，它可以接受来自多个客户端的连接，并且不需要多路复用器（kata-proxy和Yamux）。VSOCK 通信图：

```
.----------------------.
| .------------------. |
| | .-----.  .-----. | |
| | |cont1|  |cont2| | |
| | `-----'  `-----' | |
| |       |   |      | |
| |    .---------.   | |
| |    |  agent  |   | |
| |    `---------'   | |
| |       |   |      | |
| | POD .-------.    | |
| `-----| vsock |----' |
|       `-------'      |
|         |   |        |
|  .------.   .------. |
|  | shim |   | shim | |
|  `------'   `------' |
| Host                 |
`----------------------'
```



## 系统要求

kernel 版本 >=  v4.8，并且必须载入  `vhost_vsock` 模块或者内置  `vhost_vsock` 模块(`CONFIG_VHOST_VSOCK=y`)

```bash
sudo modprobe -i vhost_vsock
```

Kata Containers >= v1.2.0，并且在  `/usr/share/defaults/kata-containers/configuration.toml`  中须配置 `use_vsock=true`。

### 使用 VMWare guest

要在 VMWare guest 环境中使用带有 VSOCKs 的 Kata Containers，首先要停止 VMWare-tools 服务并卸载 VMWare Linux 内核模块。

```bash
sudo systemctl stop vmware-tools
sudo modprobe -r vmw_vsock_vmci_transport
sudo modprobe -i vhost_vsock
```

## 使用 VSOCKs 的优势

### 高密度

使用 `kata-proxy` 对虚拟机和主机之间的连接进行多路复用时，每个 POD 使用 4.5MB 内存。当我们谈论密度时，每千字节很重要，这可能是运行另一个POD与否之间的决定性因素。当 pod 多起来之后，这将占用很多资源。

### 可靠性

`kata-proxy` 负责解决虚拟机与主机进程之间的多路复用，如果它死掉了，那么所有的连接都会断开。由于通过 VSOCK 进行的通信是直接的，因此失去与容器通信的唯一方法是 VM 本身或 [shim](https://github.com/kata-containers/shim) 死亡，如果发生这种情况，则容器将自动删除。

