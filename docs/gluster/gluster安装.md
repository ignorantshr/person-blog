### 安装
先安装yum源，这是最新版本；centos的库不是最新版，如果版本不匹配会造成挂载失败

```shell
yum install centos-release-gluster
```

centos快速安装：

<https://wiki.centos.org/SpecialInterestGroup/Storage/gluster-Quickstart>

快速使用：

<https://gluster.readthedocs.io/en/latest/Quick-Start-Guide/Quickstart/>

服务端：
```shell
yum install glusterfs-server
```

客户端：
```shell
# 挂载fuse模块
modprobe fuse
dmesg | grep -i fuse

yum install -y centos-release-gluster
sudo yum -y install openssh-server wget fuse fuse-libs openib libibverbs
yum install -y glusterfs glusterfs-fuse glusterfs-rdma
vi /etc/hosts # 添加所有的服务器，包括自己
mount。。。。
```

### 数据一致性

##### 服务端的Quorum
*	cluster.server-quorum-ratio：0-100 依据结点数量 >=
*	cluster.server-quorum-type：none | server 
*	一旦失去了一致性，不可写也不可读
*	一个volume不满足quorum时，任何更新volume配置和集群的增删操作都不能执行
*	glusterd服务停止或者网络断开会被同等对待

##### 客户端的Quorum
*	在 replicated volume brick=2的情况下，需要将quorum-count = 2 才能预防脑裂
*	cluster.quorum-type：none|auto|fixed auto：在线brick数量>=总的brick数量/2;fixed与quorum-count配合使用
*	cluster.quorum-count： 与cluster.quorum-type =fixed配合使用，满足此数量的brick在线时才允许进行写操作；如果 cluster.quorum-type =auto，那么这个限定不起作用
*	cluster.quorum-reads：yes|no yes时满足数量的brick在线才可读取
*	如果只有两个 replica brick 并且想要高可用性，就不用启用client-quorum

### 脑裂的预防及恢复：

<https://access.redhat.com/documentation/en-us/red_hat_gluster_storage/3.3/html/administration_guide/sect-Managing_Split-brain#chap-Managing_Red_Hat_Storage_Volumes-splibrain_mount_point>

*taps*:最优的disperse volume
	性能最佳： （bricks - redundancy） 为2的幂