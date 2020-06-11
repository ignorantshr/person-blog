1.	distributed-replicated volume类型：
	删除一个brick，可能会自动新建被删除的brick，造成所有的brick的权限改变，造成所有的存储域失效。此时需要手动修改所有brick的权限：chmod a+xr bricks
2.	主机找不到主存储域或者挂载点有问题：
	重启主机
