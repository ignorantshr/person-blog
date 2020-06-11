## 网卡

### 查看内核是否捕获到网卡

```
[root@dev ~]# dmesg | grep eth
[    4.042720] e1000 0000:02:01.0 eth0: (PCI:66MHz:32-bit) 00:0c:29:e3:9c:a4
[    4.042725] e1000 0000:02:01.0 eth0: Intel(R) PRO/1000 Network Connection
[    4.404580] e1000 0000:02:05.0 eth1: (PCI:66MHz:32-bit) 00:0c:29:e3:9c:ae
[    4.404583] e1000 0000:02:05.0 eth1: Intel(R) PRO/1000 Network Connection
[    4.754667] e1000 0000:02:06.0 eth2: (PCI:66MHz:32-bit) 00:0c:29:e3:9c:b8
[    4.754671] e1000 0000:02:06.0 eth2: Intel(R) PRO/1000 Network Connection
```

或查看设备：

```
[root@dev ~]# lspci | grep -i ethernet
02:01.0 Ethernet controller: Intel Corporation 82545EM Gigabit Ethernet Controller (Copper) (rev 01)
02:05.0 Ethernet controller: Intel Corporation 82545EM Gigabit Ethernet Controller (Copper) (rev 01)
02:06.0 Ethernet controller: Intel Corporation 82545EM Gigabit Ethernet Controller (Copper) (rev 01)
[root@dev ~]# lspci -s 02:06.0 -v
02:06.0 Ethernet controller: Intel Corporation 82545EM Gigabit Ethernet Controller (Copper) (rev 01)
	Subsystem: VMware PRO/1000 MT Single Port Adapter
	Physical Slot: 38
	Flags: bus master, 66MHz, medium devsel, latency 0, IRQ 16
	Memory at fd560000 (64-bit, non-prefetchable) [size=128K]
	Memory at fdfd0000 (64-bit, non-prefetchable) [size=64K]
	I/O ports at 20c0 [size=64]
	[virtual] Expansion ROM at fd520000 [disabled] [size=64K]
	Capabilities: [dc] Power Management version 2
	Capabilities: [e4] PCI-X non-bridge device
	Kernel driver in use: e1000
	Kernel modules: e1000
```

### 查看网卡使用的模块

根据上一步了解到所用模块是`e1000`。

如果没有网卡驱动的话，就需要自行编译喽。不过**建议使用本来就支持linux的网卡**！

指定网卡所使用的模块：

```bash
[root@www ~]# vim /etc/modprobe.d/ether.conf
alias	eth0	e1000
alias	eth1	e1000		# 因为鸟哥有两张网卡嘛!
[root@www ~]# sync; reboot
```



## 网络配置文件

- `/etc/sysconfig/network-scripts/ifcfg-xxx`
    - `GATEWAY`代表的是整个主机的默认网关，所以**整个主机只能有一个`GATEWAY`配置**。若使用了DHCP或ADSL拨号上网，也不需要该参数，因为它们会自动分配网关。
    - `NM_CONTROLLED`：是否受到NetworkManager的管控。
    - `HWADDR`：MAC地址。如果一部主机上面插了两张相同芯片的网卡，代表两者使用的模块为同一个，可能会造成MAC的误判，此时就可以指定该字段来区分了。
- `/etc/sysconfig/network`
    - NETWORKING=要不要有网络
    - NETWORKING_IPV6=支持IPv6?
    - HOSTNAME=你的主机名
- `/etc/resolv.conf`：DNS服务器
- `/etc/hosts`
- `/etc/services`：服务所使用的协议及端口
- `/etc/protocols`：协议



## ADSL 拨号上网

先安装所需软件：

```bash
yum install rp-pppoe ppp
```

`rp-pppoe` 使用的是 *Point to Point (ppp) over Ethernet* 的点对点协议所产生的网络接口，因此当你顺利的拨号成功之后，会多产生一个实体网络接口`ppp0`。由于 ppp0 是建立在以太网络卡上的，你必须要有以太网卡，同时，即使拨号成功后，你也不能将没有用到的 eth0 关闭。

```bash
# 设置，只需要设置一次，除非后续需要改动
pppoe-setup
# 启动
adsl-start,	pppoe-start	或	network	restart
# 取消拨号功能
ifcfg-ppp0 文件： ONBOOT=no；chkconfig pppoe-server off
```

## 无线网络

无线网络需要两个设备配合使用：

- 无线基地台 (Wireless Access Point，简称 AP)
- 无线网卡

配置epel仓库，并安装工具：

```bash
yum install wireless-tools
```

然后使用`iwlist scan`搜寻无线网卡，使用`iwconfig`配置。

再次强调，设置无线网络时一定要注意**网络安全**问题！