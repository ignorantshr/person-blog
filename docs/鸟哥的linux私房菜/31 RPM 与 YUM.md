RPM 的优点

- RPM 内含已经编译过的程序与配置文件等数据，可以让用户免除重新编译的困扰；
- RPM 在被安装之前，会先检查系统的硬盘容量、操作系统版本等，可避免文件被错误安装；
- RPM 文件本身提供软件版本信息、依赖软件名称、软件用途说明、软件所含文件等信息，便于了解软件；
- RPM 管理的方式使用数据库记录 RPM 文件的相关参数，便于升级、移除、查询与验证。

## rpm指令

```
rpm [OPTION...]

安装 选项
	-i, --install                    install package(s)
	--nodeps                         do not verify package dependencies
	--replacefiles                   文件冲突时覆盖掉就文件
	--replacepkgs                    重新安装已经安装过的软件
	--force                          --replacepkgs --replacefiles
	--test                           don't install, but tell if it would work or not
	--justdb                         由于 RPM 数据库损坏或者是某些缘故产生错误时,可使用这个选项来更新软件在数据库内的相关信息
	--prefix=<dir>                   将软件安装到其他非正规目录

升级 选项
	-U, --upgrade=<packagefile>+     upgrade package(s)
	-F, --freshen=<packagefile>+     只升级已安装的软件
	
查询/验证 共用的选项
	-a, --all                        query/verify 所有包（已安装）
  	-f, --file                       query 文件属于哪个包；verify 文件是否被改动过（已安装）
  	-p, --package                    query/verify 包文件
  	
查询 选项（与 -p 配合使用）
	-q								 查询是否安装
	-c, --configfiles                list all configuration files
	-l, --list                       list files in package
	-s, --state                      display the states of the listed files
	-i								 列出该软件的详细信息
	-R								 列出软件依赖的文件
	
验证 选项（与 -V 配合使用）
	使用 /var/lib/rpm 底下的数据库内容来比对目前 Linux 系统的环境下的所有软件文件，可用于查询文件是否被改动过
	--nodeps                         don't verify package dependencies
	
卸载 选项
	-e, --erase=<package>+           erase (uninstall) package
```

由于 RPM 文件常常会安装/移除/升级等，某些动作或许可能会导致 RPM 数据库 `/var/lib/rpm/` 内的文件损坏。可以通过下面的指令重建数据库：

```
rpmdb [OPTION...]
原来的 rpm --rebuilddb 移动到该命令下来了
Database options:
  --initdb                      initialize database
  --rebuilddb                   rebuild database inverted lists from installed package headers
```

查询

```bash
[root@dev ~]# rpm -q tree
tree-1.6.0-10.el7.x86_64
[root@dev ~]# rpm -q treee
package treee is not installed
```

验证

```bash
[root@dev ~]# rpm -V rootfiles
S.5....T.  c /root/.bashrc
[root@dev ~]# rpm -Vf /root/.bashrc
S.5....T.  c /root/.bashrc
[root@dev ~]# rpm -Vf test.py 
file /root/test.py is not owned by any package
```

前面9个字段代表的意义：

- S ：(file Size differs) 文件的容量大小是否被改变
- M ：(Mode differs) 文件的类型或文件的属性 (rwx) 是否被改变?如是否可执行等参数已被改变
- 5 ：(MD5 sum differs) MD5 这一种指纹码的内容已经不同
- D ：(Device major/minor number mis-match) 设备的主/次代码已经改变
- L ：(readLink(2) path mis-match) Link 路径已被改变
- U ：(User ownership differs) 文件的所属人已被改变
- G ：(Group ownership differs) 文件的所属群组已被改变
- T ：(mTime differs) 文件的建立时间已被改变
- P ：(caPabilities differ) 功能已经被改变

后面的一个字符表示属于什么文件类型：

- c ：配置文件 (config file)
- d ：文件数据文件 (documentation)
- g ：鬼文件~通常是该文件不被某个软件所包含，较少发生 (ghost file)
- l ：许可证文件 (license file)
- r ：自述文件 (read me)

### 数字签名(digital signature)

当你要安装一个 RPM 文件时：

1. 首先你必须要先安装原厂发布的公钥文件;
2. 实际安装原厂的 RPM 软件时，rpm 指令会去读取 RPM 文件的签名信息，与本机系统内的签名信息比对；
3. 若签名相同则予以安装，若找不到相关的签名信息时，则给予警告并且停止安装。

数字公钥一般位于 `/etc/pki/rpm-gpg/` 目录：

```bash
[root@dev ~]# ls -1 /etc/pki/rpm-gpg/
RPM-GPG-KEY-CentOS-7
RPM-GPG-KEY-CentOS-Debug-7
RPM-GPG-KEY-CentOS-Testing-7
RPM-GPG-KEY-EPEL-7
```

安装公钥：

```bash
[root@dev ~]# rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
# 安装完成之后,这个密钥的内容会以软件的形式呈现
[root@dev ~]# rpm -qa | grep pubkey
gpg-pubkey-f4a80eb5-53a7ff4b
gpg-pubkey-7bd9bf62-5762b5f8
gpg-pubkey-fe590cb7-5337fb76
gpg-pubkey-352c64e5-52ae6884
[root@dev ~]# rpm -qi gpg-pubkey-f4a80eb5-53a7ff4b | grep Summary
Summary     : gpg(CentOS-7 Key (CentOS 7 Official Signing Key) <security@centos.org>)
```

## yum

`yum provides`除了搜寻指令的提供包之外，也可以搜寻文件的提供包：

```bash
[root@dev ~]# yum provides /root/.bashrc 
Loaded plugins: fastestmirror, langpacks
Loading mirror speeds from cached hostfile
 * base: mirrors.neusoft.edu.cn
 * epel: www.ftp.ne.jp
rootfiles-8.1-11.el7.noarch : The basic required files for the root user's directory
Repo        : base
Matched from:
Filename    : /root/.bashrc

rootfiles-8.1-11.el7.noarch : The basic required files for the root user's directory
Repo        : @anaconda
Matched from:
Filename    : /root/.bashrc
```

### yum 仓库的配置文件

```bash
[root@dev ~]# cat /etc/yum.repos.d/CentOS-Base.repo
[base]
name=CentOS-$releasever - Base
mirrorlist=http://mirrorlist.centos.org/?release=$releasever&arch=$basearch&repo=os&infra=$infra
#baseurl=http://mirror.centos.org/centos/$releasever/os/$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
enabled = 1
```

常见字段（`man yum.conf`）：

- base：代表仓库的名字。中括号一定要存在，里面的名称则可以随意取但必须唯一；
- name：只是说明一下这个软件库的意义而已，重要性不高；
- mirrorlist：列出这个软件库可以使用的映射网站，如果不想使用，可以批注到这行；
- baseurl：这个最重要，因为后面接的就是软件库的实际网址。mirrorlist 是由 yum 程序自行搜寻映像网站， baseurl 则是指定固定的一个软件库网址。支持 `http://, ftp://, file://` 三种协议；
- enable=1：是否启用该仓库；
- gpgcheck=1：是否需要查阅 RPM 文件内的数字签名；
- gpgkey：就是数字签名的公钥文件所在位置，使用默认值即可。

### 数据同步

yum 会先下载软件库的清单到本机的 `/var/cache/yum` 。那我们修改了网址却没有修改软件库名称 (中括号内的文字)，可能就会造成本机的列表与 yum 服务器的列表不同步，此时就会出现无法更新的问题。那么就清除掉本机上面的旧数据吧：

```
yum clean [options] 

options
	packages	将已下载的包文件删除
	headers 	将下载的包文件头删除
	all			将所有包数据都删除
```

### yum 的软件群组功能

```
yum groups [actions] 

actions
	install		
	list		
	remove		
	info		
```

可是当你下达安装命令时，却没有安装：

```bash
[root@dev ~]# yum groups install "Scientific Support"
……
No packages in any requested group available to install or update
```

这是因为 *Scientific Support* 里面的软件都是可选择的 (optional)，而不是主要的 (mandatory)，默认情况下不会安装 optional 的软件，所以需要修改配置文件（使用 `man yum.conf` 查看），添加一行喽：

```bash
[root@dev ~]# vi /etc/yum.conf
[main]
cachedir=/var/cache/yum/$basearch/$releasever
keepcache=0
……
bugtracker_url=http://bugs.centos.org/set_project.php?project_id=23&ref=http://bugs.centos.org/bug_report_page.php?category=yum
distroverpkg=centos-release
# 这里就是新增的配置！
group_package_types=default, mandatory, optional
```

然后就可以安装啦！

## RPM 还是 Tarball

如果我要升级的话，或者是全新安装一个新的软件，那么该选择 RPM 还是 Tarball 来安装呢？事实上考虑的因素很多，不过鸟哥通常是这样建议的：

1. 优先选择原厂的 RPM 功能

2. 选择软件官网释出的 RPM 或者是提供的软件库网址

3. 利用 Tarball 安装特殊软件

   某些特殊用途的软件并不会特别帮你制作 RPM 文件的。此时建议你也不要妄想自行制作 SRPM 来转成 RPM 啦，因为你只有区区一部主机而已。若是你要管理相同的 100 部主机，那么将原始码转制作成 RPM 就有价值。

4. 用 Tarball 测试新版软件

   可以用 tarball 安装新软件到 /usr/local 底下， 那么该软件就能够同时安装两个版本在系统上面了。而且大多数软件安装数种版本时还不会互相干扰。

如果软件的架构差异性太大，或者是无法解决依赖问题，那么与其花大把的时间与精力在解决依赖问题上，还不如直接以 tarball 来安装。

## rpmbuild

对于 SRPM 文件：

```
--rebuild		编译、打包
--recompile		编译、打包、安装
```

更多说明参考：[打包软件](../../rpm/02 打包软件)

