参考文档：https://access.redhat.com/documentation/en-us/red_hat_gluster_storage/3.5/html/administration_guide/formatting_and_mounting_bricks



在重新使用已被删除的 brick 时，会提示它已被其它 volume 使用，此时需要执行一些步骤来重用它。

- 若该 brick 可以格式化，那么直接执行`mkfs.xfs -f -i size=512 device` 格式化磁盘即可。

- 如果 brick 不能被格式化，又分为两种情况：

    - 如果可以删除 brick 文件夹，那么删除后重新创建即可；

    - 如果该 brick 文件夹不能被删除，执行下面的步骤：

        1. 删除brick下面所有的数据，包括 `.glusterfs`

        2. 查看 brick 目录的所有属性

            getfattr -d -m . <brick>

        3. 移除 brick 目录的所有与 glusterFS 相关的属性

            setfattr -x xxx <brick>

        4. 检查 brick 目录的所有属性是否清理完毕

            getfattr -d -m . <brick>



例子：

```bash
# 删除所有数据
[root@node2 glusterfs]# rm -rf /data/glusterfs/v3/b1/*
[root@node2 glusterfs]# rm -rf /data/glusterfs/v3/b1/.*

# 查看 brick 目录的所有属性
# 注意，下面列出的属性并没有包含所有可能存在的属性
[root@node2 glusterfs]# getfattr -d -m . /data/glusterfs/v2/b1/brick/
getfattr: Removing leading '/' from absolute path names
# file: data/glusterfs/v2/b1/brick/
security.selinux="unconfined_u:object_r:default_t:s0"
trusted.gfid=0sAAAAAAAAAAAAAAAAAAAAAQ==
trusted.glusterfs.dht=0sAAAAAQAAAAAUmESi/////w==
trusted.glusterfs.mdata=0sAQAAAAAAAAAAAAAAAF/Z0zkAAAAAH7wGLwAAAABf2dMNAAAAAAMvHBsAAAAAX9nTEgAAAAAaMP8x
trusted.glusterfs.volume-id=0saYP8TrVjSq+Gzfi5BW+Mmg==

# 移除 brick 目录的所有与 glusterFS 相关的属性
[root@node2 glusterfs]# setfattr -x trusted.gfid /data/glusterfs/v2/b1/brick/
[root@node2 glusterfs]# setfattr -x trusted.glusterfs.volume-id /data/glusterfs/v2/b1/brick/
[root@node2 glusterfs]# setfattr -x trusted.glusterfs.dht /data/glusterfs/v2/b1/brick/
[root@node2 glusterfs]# setfattr -x trusted.glusterfs.mdata /data/glusterfs/v2/b1/brick/

# 检查 brick 目录的所有属性是否清理完毕
[root@node2 glusterfs]# getfattr -d -m . /data/glusterfs/v2/b1/brick/
getfattr: Removing leading '/' from absolute path names
# file: data/glusterfs/v2/b1/brick/
security.selinux="unconfined_u:object_r:default_t:s0"
```

