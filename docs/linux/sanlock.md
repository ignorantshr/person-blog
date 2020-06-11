参考资源：

- https://www.ibm.com/developerworks/cn/linux/1404_zhouzs_sanlock/
- https://www.ibm.com/developerworks/cn/linux/l-cn-watchdog/
- https://www.ovirt.org/develop/developer-guide/vdsm/sanlock.html



安装sanlock

```bash
yum install sanlock sanlock-python	# 安装sanlock 与 python 库
systemctl start sanlock.service		# 会自动加载 softdog 模块
```

准备工作

```bash
[root@linux tmp]# mount.nfs 192.168.216.10:/sanlock_test/ sanlock-test/
[root@linux tmp]# dd if=/dev/zero of=sanlock-test/ids bs=1048576 count=1
[root@linux tmp]# dd if=/dev/zero of=sanlock-test/leases bs=1048576 count=1
[root@linux tmp]# chown sanlock:sanlock sanlock-test/{ids,leases}
```

初始化资源

```bash
[root@linux tmp]# sanlock client init -s LS:0:/tmp/sanlock-test/ids:0
init
init done 0
[root@linux tmp]# sanlock client init -r LS:leader:/tmp/sanlock-test/leases:0
init
init done 0
```

运行sanlock

```bash
[root@linux tmp]# ./sanlock-test/ha.py 1 /tmp/sanlock-test/ids /tmp/sanlock-test/leases &
Try to acquire host id LS:1:/tmp/sanlock-test/ids:0
Try to acquire leader lease LS:leader:/tmp/sanlock-test/leases:0
2019 12 CST 09:08:06 Service is running
……
```

此时断开NFS存储网络，系统将在60s后警告20s，然后尝试kill掉进程，如果60s（前40s执行`kill -15`，后20s执行`kill -9`）内kill失败，那么系统将重启。

`/var/log/sanlock.log`只记载 `sanlock` 日志，`/var/log/messages` 会记载 `sanlock`与`wdmd`日志。不过两者的日志都记录不全最好在终端查看：`tailf /var/log/sanlock.log`。

```bash
# 重启后查看日志
[root@linux ~]# vi /var/log/messages
# 断开网络后开始报错
Dec 23 09:25:31 linux sanlock[1816]: 2019-12-23 09:25:31 2395 [2234]: s2 delta_renew read timeout 10 sec offset 0 /tmp/sanlock-test/ids
Dec 23 09:25:31 linux sanlock[1816]: 2019-12-23 09:25:31 2395 [2234]: s2 renewal error -202 delta_length 10 last_success 2365
Dec 23 09:25:51 linux sanlock[1816]: 2019-12-23 09:25:51 2416 [2234]: s2 delta_renew read timeout 10 sec offset 0 /tmp/sanlock-test/ids
Dec 23 09:25:51 linux sanlock[1816]: 2019-12-23 09:25:51 2416 [2234]: s2 renewal error -202 delta_length 20 last_success 2365
# 开始警告
Dec 23 09:26:01 linux sanlock[1816]: 2019-12-23 09:26:01 2425 [1816]: s2 check_our_lease warning 60 last_success 2365
Dec 23 09:26:02 linux sanlock[1816]: 2019-12-23 09:26:02 2426 [1816]: s2 check_our_lease warning 61 last_success 2365
……
Dec 23 09:26:15 linux wdmd[1825]: test warning now 2440 ping 2430 close 2037 renewal 2365 expire 2445 client 1816 sanlock_LS:1
# wdmd 的 watchdog 日志
Dec 23 09:26:15 linux wdmd[1825]: /dev/watchdog0 closed unclean
Dec 23 09:26:15 linux kernel: watchdog watchdog0: watchdog did not stop!
……
Dec 23 09:26:19 linux wdmd[1825]: test warning now 2444 ping 2430 close 2440 renewal 2365 expire 2445 client 1816 sanlock_LS:1
Dec 23 09:26:20 linux sanlock[1816]: 2019-12-23 09:26:20 2444 [1816]: s2 check_our_lease warning 79 last_success 2365
Dec 23 09:26:20 linux wdmd[1825]: test failed rem 55 now 2445 ping 2430 close 2440 renewal 2365 expire 2445 client 1816 sanlock_LS:1
# 80s 结束
Dec 23 09:26:21 linux sanlock[1816]: 2019-12-23 09:26:21 2445 [1816]: s2 check_our_lease failed 80
# kill 掉进程
Dec 23 09:26:21 linux sanlock[1816]: 2019-12-23 09:26:21 2445 [1816]: s2 kill 2236 sig 15 count 1
Dec 23 09:26:21 linux sanlock[1816]: 2019-12-23 09:26:21 2446 [1816]: s2 kill 2236 sig 15 count 2
……
# 不能 kill 掉进程的原因
Dec 23 09:26:30 linux setroubleshoot: SELinux is preventing /usr/sbin/sanlock from using the signal access on a process. For complete SELinux messages run: sealert -l 7a8e5ae9-5274-4d09-81b2-a93b37cd716d
Dec 23 09:26:30 linux python: SELinux is preventing /usr/sbin/sanlock from using the signal access on a process.#012#012*****  Plugin catchall (100. confidence) suggests   **************************#012#012If you believe that sanlock should be allowed signal access on processes labeled unconfined_t by default.#012Then you should report this as a bug.……
# 下面是系统启动日志
Dec 23 09:27:23 linux journal: Runtime journal is using 8.0M (max allowed 91.0M, trying to leave 136.6M free of 902.7M available → current limit 91.0M).
Dec 23 09:27:23 linux kernel: Initializing cgroup subsys cpuset
Dec 23 09:27:23 linux kernel: Initializing cgroup subsys cpu
```

解决selinux的问题之后，再次尝试，发现进程被成功地kill掉了，系统不会重启：

```bash
2019 12 CST 10:08:09 Service is running

[1]+  Done                    ./sanlock-test/ha.py 1 /tmp/sanlock-test/ids /tmp/sanlock-test/leases
[root@linux tmp]# vi /var/log/messages
Dec 23 10:08:07 linux kernel: watchdog watchdog0: watchdog did not stop!
Dec 23 10:08:07 linux wdmd[1750]: test warning now 2444 ping 2434 close 0 renewal 2366 expire 2446 client 1737 sanlock_LS:1
Dec 23 10:08:07 linux wdmd[1750]: /dev/watchdog0 closed unclean
Dec 23 10:08:07 linux sanlock[1737]: 2019-12-23 10:08:07 2444 [1737]: s1 check_our_lease warning 78 last_success 2366
Dec 23 10:08:08 linux wdmd[1750]: test warning now 2445 ping 2434 close 2444 renewal 2366 expire 2446 client 1737 sanlock_LS:1
Dec 23 10:08:08 linux sanlock[1737]: 2019-12-23 10:08:08 2445 [1737]: s1 check_our_lease warning 79 last_success 2366
Dec 23 10:08:09 linux wdmd[1750]: test failed rem 58 now 2446 ping 2434 close 2444 renewal 2366 expire 2446 client 1737 sanlock_LS:1
Dec 23 10:08:09 linux sanlock[1737]: 2019-12-23 10:08:09 2446 [1737]: s1 check_our_lease failed 80
Dec 23 10:08:09 linux sanlock[1737]: 2019-12-23 10:08:09 2446 [1737]: s1 kill 1831 sig 15 count 1
Dec 23 10:08:09 linux sanlock[1737]: 2019-12-23 10:08:09 2446 [1737]: dead 1831 ci 2 count 1
Dec 23 10:08:10 linux wdmd[1750]: /dev/watchdog0 reopen
# 这里指示进程已经被 kill 了
Dec 23 10:08:10 linux sanlock[1737]: 2019-12-23 10:08:10 2447 [1737]: s1 all pids clear
```



python文件`ha.py`

```python
#!/usr/bin/python
# coding=utf-8

import sys
import time
from multiprocessing import Process
import sanlock


# simpleHA 主程序，加入 sanlock 集群并尝试获取 Paxos Lease
def serviceMain(hostId, lockspacePath, leasePath):
    sfd = sanlock.register()  # 把本进程注册到 sanlock 服务上
    while True:
        try:
            # 若尚未加入 sanlock 集群，则尝试获取对 hostId 的 Delta Lease
            # 并尝试加入 sanlock 集群
            if not sanlock.inq_lockspace("LS", hostId, lockspacePath):
                print "Try to acquire host id LS:%s:%s:0" % (hostId,
                                                             lockspacePath)
                sanlock.add_lockspace("LS", hostId, lockspacePath)

            # 尝试获取 Delta Lease
            print "Try to acquire leader lease LS:leader:%s:0" % leasePath
            sanlock.acquire("LS", "leader", [(leasePath, 0)], sfd)
        except sanlock.SanlockException as e:
            print e
            # 无法加入集群，或无法获取 Delta Lease
            # 10 秒后重试
            print "Failed to acquire leases, try again in 10s."
            time.sleep(10)
        else:
            break  # 成功获取 Paxos Lease，不再重试

    # 成功加入了 sanlock 集群并获取了 Paxos Lease
    # 执行实际的应用服务
    serve()


# 让它一直运行
def serve():
    while True:
        print time.strftime("%Y %m %Z %H:%M:%S"), "Service is running"
        time.sleep(10)

    print time.strftime("%Y %m %Z %H:%M:%S"), "Service crashed"


if __name__ == "__main__":
    try:
        hostId = int(sys.argv[1])
        lockspacePath = sys.argv[2]
        leasePath = sys.argv[3]
    except Exception:
        sys.stderr.write(
        	"Usage: %s host_id lockspace_path lease_path\n" % sys.argv[0])
        exit(1)

    p = Process(target=serviceMain,
                args=(hostId, lockspacePath, leasePath))
    p.start()
    p.join()

```

