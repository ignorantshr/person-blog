## 介绍

LVM：Logical Volume Manager。

PV：Physical Volume。由分区或磁盘来创建。

VG：Volume Group。

LV：Logical Volume。

PE：Physical Extent。是整个 LVM 最小的储存区块，类似于 FS 中的 block。默认大小是 4M。LVM 的 LV 在32位系统上最多仅能含有 65534 个PE(lvm1的格式)，因此默认的 LVM 的 LV 会有 4M*65534/(1024M/G)=256G。在CentOS 6.x 以后，由于直接使用 lvm2 的各项格式功能,以及系统转为 64 位，因此这个限制已经不存在了。

两种写入机制：

1. 线性模式(linear)。当一个partition或disk的容量用完之后，再写入下一个partition或disk。推荐模式。
2. 交错模式(triped)。将数据写入多个磁盘中，读写性能更好。

lvm 主要强调的是弹性调整，而不是性能，所以默认使用线性模式。如果强调性能与备份，直接使用RAID就完事了。

## 指令

### PV 指令

- pvcreate
- pvscan：扫描所有被用作PV的块设备
- pvs：生成有关 PV 的格式化输出
- pvdisplay：列出PV的属性
- pvremove：移除设备的PV属性

### VG 指令

- vgcreate：-s选项可以指定PE大小
- vgscan
- vgs
- vgdisplay
- vgremove
- vgextend
- vgreduce

### LV 指令

- lvresize

## 扩容与缩减

分为4步：

1. 新增或扩容PV
2. 扩容VG
3. 扩容LV
4. 文件系统的扩容与缩小。XFS 只支持使用`xfs_growfs`增加容量，不支持缩减。EXT 家族两种都支持，使用`resize2fs`。

可以从上述前三步的任意步骤开始。

## 使用 LVM thin Volume 精简分配

首先你要有一个存储池：

```
lvcreate -T -L|--size Size[m|UNIT] VG/NEW_LV
	
	-T		指示这是一个 thin pool
```

```bash
[root@dev ~]# lvcreate -T -L 100M test-vg/thin-pool1
  Rounding up size to full physical extent 104.00 MiB
  Thin pool volume with chunk size 64.00 KiB can address at most 15.81 TiB of data.
  Logical volume "thin-pool1" created.
[root@dev ~]# lvdisplay /dev/test-vg/thin-pool1 
  --- Logical volume ---
						……
  LV Pool metadata       thin-pool1_tmeta
  LV Pool data           thin-pool1_tdata
  LV Status              available
  # open                 0
  LV Size                104.00 MiB
  Allocated pool data    0.00%
  Allocated metadata     10.40%
  Current LE             13
  Segments               1
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     8192
  Block device           253:5
```

然后创建lv：

```
lvcreate -V|--virtualsize Size[m|UNIT] -T VG/LV_thinpool -n NEW_LV
```

```bash
[root@dev ~]# lvcreate -V 1G -T test-vg/thin-pool1 -n lv2
  WARNING: Sum of all thin volume sizes (1.00 GiB) exceeds(超过) the size of thin pool test-vg/thin-pool1 and the size of whole volume group (288.00 MiB).
  WARNING: You have not turned on protection against thin pools running out of space.
  WARNING: Set activation/thin_pool_autoextend_threshold below 100 to trigger automatic extension of thin pools before they get full.
  Logical volume "lv2" created.
[root@dev ~]# lvs test-vg
  LV         VG      Attr       LSize   Pool       Origin Data%  Meta%  Move Log Cpy%Sync Convert
  lv2        test-vg Vwi-a-tz--   1.00g thin-pool1        0.00                                   
  test-lv1   test-vg -wi-a----- 112.00m                                                          
  thin-pool1 test-vg twi-aotz-- 104.00m                   0.00   10.45
```

!!! warning
	注意，如果突破了实际的容量，thin pool 可是会爆炸而让数据损毁的。

```bash
[root@dev ~]# mkfs.xfs /dev/test-vg/lv2
[root@dev ~]# mount /dev/test-vg/lv2 lv2/
[root@dev ~]# cd lv2/
# 显示的有1G，实际上 thin pool 总共只有 104M
[root@dev lv2]# df -hT lv2
Filesystem               Type      Size  Used Avail Use% Mounted on
/dev/mapper/test--vg-lv2 xfs      1014M   33M  982M   4% /root/lv2
[root@dev lv2]# dd if=/dev/zero of=boom bs=1M count=1000
# 竟然真的使用了1G的容量！！！
[root@dev lv2]# ll -h
total 981M
-rw-r--r--. 1 root root 981M Oct 25 15:38 boom
[root@dev lv2]# df -h lv2
Filesystem                Size  Used Avail Use% Mounted on
/dev/mapper/test--vg-lv2 1014M 1014M  608K 100% /root/lv2
```

## LV 磁盘快照

不变的PE数据由快照区与系统区共享，数据变动时，将原始的PE数据移动到快照区，改动的PE数据存放在系统区。由于共享的原因，快照区与被快照的 LV 必须要在同一个 VG 上。由于 thin pool 的限制很多，所以不建议使用 thin pool 快照。

快照区实际上也是LV，创建：

```
lvcreate -s|--snapshot <-L|--size Size[m|UNIT]>|<-l|--extents Number[PERCENT]> -n NEW_LV VG/LV
```

```bash
# 查看还有多少空间
[root@dev ~]# vgdisplay test-vg
  Total PE              36
  Alloc PE / Size       14 / 112.00 MiB
  Free  PE / Size       22 / 176.00 MiB
# 创建快照区
[root@dev ~]# lvcreate -s -l 14 -n test-lv1-snapshot1 test-vg/test-lv1
  Logical volume "test-lv1-snapshot1" created.
# 查看快照
[root@dev ~]# lvdisplay test-vg/test-lv1-snapshot1
  --- Logical volume ---
						……
  LV snapshot status     active destination for test-lv1
  LV Status              available
  # open                 0
  LV Size                112.00 MiB		# 原始LV的容量
  Current LE             14
  COW-table size         112.00 MiB		# 能够记录的最大容量
  COW-table LE           14
  Allocated to snapshot  0.02%			# 被使用的容量
  Snapshot chunk size    4.00 KiB
  Segments               2
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     8192
  Block device           253:5
[root@dev ~]# lvs test-vg
  LV                 VG      Attr       LSize   Pool Origin   Data%  Meta%  Move Log Cpy%Sync Convert
  test-lv1           test-vg owi-aos--- 112.00m                                                      
  test-lv1-snapshot1 test-vg swi-a-s--- 112.00m      test-lv1 0.02
# 挂载查看
[root@dev ~]# blkid /dev/mapper/test--vg-test--lv1*
/dev/mapper/test--vg-test--lv1: UUID="3073018e-d42a-4310-9f93-2cae719abe8f" TYPE="xfs" 
/dev/mapper/test--vg-test--lv1--snapshot1: UUID="3073018e-d42a-4310-9f93-2cae719abe8f" TYPE="xfs"
# 因为两者的 UUID 相同，所以需要 nouuid（不要使用uuid检测重复挂载的文件系统） 选项
[root@dev ~]# mount -o nouuid /dev/test-vg/test-lv1-snapshot1 test-lv1-snapshot1/
# 两者是相同的！
[root@dev test-lv1]# df -hT /dev/test-vg/*
Filesystem                                Type  Size  Used Avail Use% Mounted on
/dev/mapper/test--vg-test--lv1            xfs   109M  5.9M  103M   6% /root/test-lv1
/dev/mapper/test--vg-test--lv1--snapshot1 xfs   109M  5.9M  103M   6% /root/test-lv1-snapshot1
```

### 复原

若变更的数据量比快照区大，那么就会失效。

```bash
# 备份快照区的文件系统，挂载点末尾一定不能带有”/“ ！！！
[root@dev ~]# xfsdump -l 0 -L test-lv1 -M test-lv1 -f test-lv1-0.dump /root/test-lv1-snapshot1
# 移除快照LV
[root@dev ~]# umount test-lv1-snapshot1/
[root@dev ~]# lvremove test-vg/test-lv1-snapshot1
Do you really want to remove active logical volume test-vg/test-lv1-snapshot1? [y/n]: y
  Logical volume "test-lv1-snapshot1" successfully removed
# 重新格式化原始LV
[root@dev ~]# umount test-lv1
[root@dev ~]# mkfs.xfs -f /dev/test-vg/test-lv1
[root@dev ~]# mount /dev/test-vg/test-lv1 test-lv1
# 使用快照的备份恢复原始LV
[root@dev ~]# xfsrestore -f test-lv1-0.dump test-lv1
```

### 快照区与原始区身份反转

若把两者的身份反转（即把快照区当作测试用的地方，把原始区当作快照区的备份）会是什么情况呢？

在快照区的动作（比如测试、练习）也会写在快照区，而原始区会保持不变，这样一来，只需要删除快照区，就可以将所有的变动数据都删除，再重新创建一个快照区就还能重新使用原始环境！

## LVM 的移除

如果你还没有将 LVM 关闭就直接将那些 partition 删除或转为其他用途的话,系统是会发生很大的问题的！

1. 先卸载 LVM 类型的FS
2. 使用 lvremove 移除 LV
3. 使用 `vgchange -a n VG`取消激活VG
4. 使用 vgremove 移除 VG
5. 使用 pvremove 移除 PV
6. 修改分区或disk类型

```bash
[root@dev ~]# umount /root/test-lv1
[root@dev ~]# lvs
  LV       VG      Attr       LSize   Pool Origin Data%  Meta%  Move Log Cpy%Sync Convert
  root     cl      -wi-ao---- <44.00g                                                    
  swap     cl      -wi-ao----   5.00g                                                    
  test-lv1 test-vg -wi-a----- 112.00m                                                    
[root@dev ~]# lvremove test-vg/test-lv1
Do you really want to remove active logical volume test-vg/test-lv1? [y/n]: y
  Logical volume "test-lv1" successfully removed
[root@dev ~]# vgchange -a n test-vg
  0 logical volume(s) in volume group "test-vg" now active
[root@dev ~]# vgremove test-vg 
  Volume group "test-vg" successfully removed
[root@dev ~]# pvs
  PV         VG Fmt  Attr PSize   PFree  
  /dev/sda2  cl lvm2 a--  <49.00g      0 
  /dev/sdb1     lvm2 ---  100.00m 100.00m
  /dev/sdb2     lvm2 ---  100.00m 100.00m
  /dev/sdb3     lvm2 ---  100.00m 100.00m
[root@dev ~]# pvremove /dev/sdb{1..3}
  Labels on physical volume "/dev/sdb1" successfully wiped.
  Labels on physical volume "/dev/sdb2" successfully wiped.
  Labels on physical volume "/dev/sdb3" successfully wiped.
[root@dev ~]# gdisk /dev/sdb
Number  Start (sector)    End (sector)  Size       Code  Name
   1            2048          206847   100.0 MiB   8300  Linux filesystem
```

