1.	mkfs.xfs -f -i size=512 device 格式化磁盘
2.	如果brick不能被格式化，或者不能该文件夹不能被移除，执行下面的步骤
	删除brick下面所有的数据，包括.glusterfs
	1.	Run # setfattr -x trusted.glusterfs.volume-id brick and # setfattr -x trusted.gfid brick to remove the attributes from the root of the brick.
	2.	Run # getfattr -d -m . brick to examine the attributes set on the volume. Take note of the attributes.
	3.	Run # setfattr -x attribute brick to remove the attributes relating to the glusterFS file system.
