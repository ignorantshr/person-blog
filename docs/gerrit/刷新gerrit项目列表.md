* 把服务器的id_rsa.pub加入到admin用户中
* ssh -p 29418 admin@localhost gerrit flush-caches --cache project_list
* 或者重启gerrit服务
