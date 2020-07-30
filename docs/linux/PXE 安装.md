- https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/installation_guide/sect-kickstart-syntax
- https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/installation_guide/chap-installation-server-setup
- https://blog.51cto.com/1130739/1740925
- http://blog.chinaunix.net/uid-22621471-id-4980582.html

PXE（preboot execute environment，预启动执行环境）简单来说就是通过网络安装操作系统。

下面以搭配 kickstart 在局域网内通过 PXE 安装 CentOS7 为例进行说明。

## 服务依赖

pxe 依赖三种服务：

- **DHCP 服务**：负责为客户机分配 IP 地址及指定 TFTP 服务器地址。
- **TFTP 服务**：负责提供操作系统的安装时的启动文件。
- **文件下载服务**：负责提供操作系统镜像目录树及 kickstart 文件。多种服务可选：http、https、nfs、ftp、file。

下文中我将三种服务安装到同一台 centos7 的虚拟机上面，IP 地址为 *192.168.75.133*。为了方便关闭了防火墙及 SELinux 。

服务安装：

```bash
yum install -y dhcp tftp-server httpd syslinux
```

`syslinux `是用来提供 centos7 的启动文件的。

还有一种是通过 `dnsmasq` 来实现 DHCP 服务和 TFTP 服务的，暂时没试过：https://www.linuxidc.com/Linux/2017-02/140512.htm

kickstart 安装（可选）：

```
yum install -y system-config-kickstart pykickstart
```

`system-config-kickstart` 用于图形化配置安装步骤；`pykickstart` 用于校验 kickstart 文件。

### DHCP 服务配置

将模版文件复制到配置目录下：

```bash
cp /usr/share/doc/dhcp-4.2.5/dhcpd.conf.example /etc/dhcp/dhcpd.conf
```

里面大都是一些例子，修改配置文件`/etc/dhcp/dhcpd.conf`：

1. 删掉开头的域名示例：

```
#option domain-name "example.org";
#option domain-name-servers ns1.example.org, ns2.example.org;
```

2. 添加下面的配置：

```
option space PXE;
option PXE.mtftp-ip    code 1 = ip-address;
option PXE.mtftp-cport code 2 = unsigned integer 16;
option PXE.mtftp-sport code 3 = unsigned integer 16;
option PXE.mtftp-tmout code 4 = unsigned integer 8;
option PXE.mtftp-delay code 5 = unsigned integer 8;
option arch code 93 = unsigned integer 16;
subnet 192.168.75.0 netmask 255.255.255.0 {     # 服务器网段
        range 192.168.75.10 192.168.75.20;      # 地址池范围
        option routers 192.168.75.2;			# 网关地址
        option domain-name-servers 172.16.6.200,114.114.114.114,8.8.8.8;
        class "pxeclients" {
                match if substring (option vendor-class-identifier, 0, 9) = "PXEClient";
                next-server 192.168.75.133;		# TFTP 服务器地址

                if option arch = 00:06 {
                        filename "bootia32.efi";	# 暂时没有实验过该文件
                } else if option arch = 00:07 {
                        filename "grubx64.efi";		# UEFI 64位启动
                } else {
                        filename "pxelinux.0";		# BIOS 启动
                }
        }
}
```

然后启动服务：

```bash
systemctl start dhcpd
```

### TFTP 服务配置

你可以修改`/etc/xinetd.d/tftp`文件：

```
service tftp
{
        socket_type             = dgram
        protocol                = udp
        wait                    = yes
        user                    = root
        server                  = /usr/sbin/in.tftpd
        server_args             = -s /var/lib/tftpboot	# 这里填写提供文件的目录
        disable                 = no					# 这里让它开启服务
        per_source              = 11
        cps                     = 100 2
        flags                   = IPv4
}
```

启动服务：

```
systemctl start tftp
```

### 文件下载服务配置

这里我选用的是 http 服务，不用修改配置即可使用。

<br/>

## 文件提取

### http 目录

http 提供的默认文件夹位于 `/var/www/html/` 。

挂载 centos7 的镜像到 `/var/www/html/centos7` 下面，这就是 http 服务提供的镜像目录树的地址。

#### 已有 kickstart 文件

若你已经有了 kickstart 文件，那么直接将它复制到 http 目录下就好，然后修改其中的安装源为：

```
url --url http://192.168.75.133/centos7
```

**只能有一个安装源**，比如若原先的配置文件中存在 `cdrom` ，那么需要将其删除。

保存之后执行语法校验：

```
ksvalidator centos7.cfg
```

#### 通过图形界面生成 kickstart 配置文件

执行：

```bash
system-config-kickstart
```

选择过程中的安装源选择 http，选择完毕保存之后执行语法校验：

```
ksvalidator centos7.cfg
```

将它复制到 http 目录下。



一份简单的 kickstart 文件示例：

```
#platform=x86, AMD64, or Intel EM64T
#version=DEVEL
# Install OS instead of upgrade
install
# Keyboard layouts
keyboard 'us'
# Root password
rootpw --iscrypted $1$ViFiDjsQ$V6J0csml5B4D/oMjZm.6s1
# System language
lang en_US
# System authorization information
auth  --useshadow  --passalgo=sha512
# Use text mode install
text
# SELinux configuration
selinux --enforcing
# Do not configure the X Window System
skipx


# Firewall configuration
firewall --enabled --ssh
# Reboot after installation
reboot
# System timezone
timezone Asia/Shanghai --isUtc
# Use network installation
url --url="http://192.168.75.133/centos7"
# System bootloader configuration
bootloader --location=mbr
# Clear the Master Boot Record
zerombr
# Partition clearing information
clearpart --all --initlabel
# Autopart
autopart --type=lvm

%packages
@^minimal
%end
```

其中的包选择可以是 `environment`， `group`，或 `package names`，它们可以在镜像文件的 `repodata/*-c7-x86_64-comps.xml` 或 `repo/*-comps-variant.architecture.xml` 文件中找到，三种格式如下：

- `environment`：`@^minimal`
- `group`：`@base`
- `package names`：`ntp*`

只能选择一个安装环境，但是可以选择多个组或包。详细说明可在文章头部的红帽的链接中找到。

### TFTP 目录

这里使用的也是默认目录`/var/lib/tftpboot`。

```bash
cd /var/lib/tftpboot
```

复制操作系统启动所需的文件：

```
[root@controller tftpboot]# cp /var/www/html/centos7/images/pxeboot/{initrd.img,vmlinuz} .
```

#### BIOS 启动

复制 BIOS 安装时的启动文件：

```
[root@controller tftpboot]# cp /usr/share/syslinux/pxelinux.0 .
```

复制 BIOS 启动的菜单文件：

```
[root@controller tftpboot]# cp /var/www/html/centos7/isolinux/{boot.msg,splash.png} .	# 文字提示与背景图片，可有可无
[root@controller tftpboot]# cp /var/www/html/centos7/isolinux/vesamenu.c32 .			# 菜单样式文件，若没有该文件则需要在 default 中指定 label
[root@controller tftpboot]# mkdir pxelinux.cfg
[root@controller tftpboot]# cp /var/www/html/centos7/isolinux/isolinux.cfg pxelinux.cfg/default
```

修改菜单文件，修改其中的 kickstart 文件选项及删除设备启动：

```
[root@controller tftpboot]# sed -i "s#inst.stage2=hd:LABEL=CentOS\\\x207\\\x20x86_64##; s#quiet#ks=http://192.168.75.133/centos7.cfg quiet#" pxelinux.cfg/default
```

如果你想要跳过安装前的测试，还要修改`pxelinux.cfg/default`默认选择项为：

```
label linux
  menu label ^Install CentOS 7
  # 新的默认选项
  menu default
  kernel vmlinuz
  append initrd=initrd.img  ks=http://192.168.75.133/centos7.cfg quiet

label check
  menu label Test this ^media & install CentOS 7
  # 注释此处
  #menu default
  kernel vmlinuz
  append initrd=initrd.img  rd.live.check ks=http://192.168.75.133/centos7.cfg quiet
```

#### UEFI 启动（x86_64）

复制 UEFI 启动的菜单文件及安装时的启动文件：

```
[root@controller tftpboot]# cp /var/www/html/centos7/EFI/BOOT/{grub.cfg,grubx64.efi} .
```

和 BIOS 一样，也要修改菜单文件，同时还要修改其中 `initrd.img` 与 `vmlinuz` 的路径：

```
[root@controller tftpboot]# sed -i "s#/images/pxeboot/##; s#inst.stage2=hd:LABEL=CentOS\\\x207\\\x20x86_64##; s#quiet#ks=http://192.168.75.133/centos7.cfg quiet#" grub.cfg
```

如果你想要跳过安装前的测试，还要修改`grub.cfg`默认选择项为：`set default="0"`。

## 目录结构

配置完成后你的 http 服务器文件目录结构应该是这样的：

```
[root@controller html]# tree -L 1 /var/www/html
/var/www/html
├── centos7
└── centos7.cfg

1 directory, 1 file
```

 tftp 服务器文件目录结构应该是这样的

```
[root@controller tftpboot]# tree /var/lib/tftpboot/
/var/lib/tftpboot/
├── boot.msg
├── grub.cfg
├── grubx64.efi
├── initrd.img
├── pxelinux.0
├── pxelinux.cfg
│   └── default
├── splash.png
├── vesamenu.c32
└── vmlinuz

1 directory, 9 files
```

## 问题解决

最常出现的错误就是 `dracut-init timeout` 了，导致该错误的原因多种多样，我遇到的一些是：

- kickstart 文件路径不对，客户机找不到 kickstart 文件
- kickstart 文件里面同时指定了多个安装源，比如 `cdrom` 与 `url` 同时存在
- 菜单文件中指定了 `inst.stage2` 选项
- 菜单文件中 `initrd.img` 与 `vmlinuz` 的路径不对
- 虚拟机内存过小，可以调成 2GB 以上试试