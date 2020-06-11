### 制作镜像

```shell
mkisofs -o xxx.iso -J -R -V bb diretory
```

-V后面的bb为指定光盘的卷册集ID，diretory为需要打包的文件夹名字

*	如果需要保持原始文件名，要添加-J参数，否则打包后，文件名全改变了
*	如果需要排除部分文件夹，可以使用-x excludefolder


### 更新rpm包

##### 1.	将当前系统镜像文件拷贝至指定目录，挂载镜像文件，与本系统的iso文件保持一致。

  挂载镜像：
```shell
  	mkdir /root/cdrom
    mount vms.iso /root/cdrom
```
  复制镜像文件：

```shell
    mkdir /home/myerh
    cp -r /media/erhos/. /home/myerh/
```

##### 2.	直接下载安装包

参见[只下载rpm包](../命令/更新、禁用yum仓库、只下载rpm.md#只下载不安装rpm)

##### 3.	将打包好的rpm包拷贝到Packages目录中，如果该rpm包是修改后的则删除原有的rpm包，如果是新建的则不需要

```shell  
  cp redhat-logo-60.0.14-14.el6.centos.noarch.rpm /home/myerh/Packages
  rm -f /home/myerh/packages/redhat-logo-60.0.14-12.el6.centos.noarch.rpm
```
如果镜像本身没有该rpm包，需要在对应的`isolinux/xxx.cfg`引导文件中添加该包的索引，依赖包不需要添加

	eg.
	%packages
	vdsm
	tree
	%end

##### 4.	更新rpm仓库

    拷贝xml文件到home目录下 cp 2727...comps.xml /home
    清空repodata文件夹内容 rm -f repodata/*
    拷贝xml文件到repodata并重命名 mv /home/...comps.xml repodata/comps.xml
    createrepo -d -g repodata/comps.xml .

##### 5.	替换对应的logo 在home/myerh/isolinux/erh中替换图片

##### 6.	打包iso镜像（该操作在home/myerh目录下进行）
（/home/ovirt3.5.iso为文件iso文件名，ovirt为ISO卷标,需要跟iso文件中的名字保持一致。）

```shell
  mkisofs -R -J -T -r -l -d -o /home/ovirt3.5.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -V ovirt .
```

