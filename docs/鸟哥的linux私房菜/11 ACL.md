ACL：Access Control List，用于定义文件和目录的更细粒度的任意访问权限。

## 检测ACL

```bash
[sink@dev 16:30:25 vitest]$ dmesg | grep ACL
[    2.493446] systemd[1]: systemd 219 running in system mode. (+PAM +AUDIT +SELINUX +IMA -APPARMOR +SMACK +SYSVINIT +UTMP +LIBCRYPTSETUP +GCRYPT +GNUTLS +ACL +XZ +LZ4 -SECCOMP +BLKID +ELFUTILS +KMOD +IDN)
[    5.829942] SGI XFS with ACLs, security attributes, no debug enabled
```

可见XFS支持ACL。

## 设置ACL

```
setfacl [-bkndRLPvh] [{-m|-x} acl_spec] [{-M|-X} acl_file] file ...
setfacl --restore=file

OPTIONS
	-b, --remove-all
	-k, --remove-default
	-d, --default						设置 Default AC，只对目录有效，在该目录新建时使用.
	-n, --no-mask						不要重新计算有效权限掩码。
	-R, --recursive						递归地操作。不能与--restore合用
	-m  --modify						modify the ACL of a file or directory.
	-M  --modify-file					
	-x  --remove						remove ACL entries
	-X  --remove-file
	--  End of command line options. All remaining parameters are interpreted as file names, even if they start with a dash.
	-   If the file name parameter is a single dash, setfacl reads a list of files from standard input.

ACL ENTRIES
	# perms是 r、w、x 的组合，若不设置则为“-”
	[d[efault]:] [u[ser]:]uid [:perms]
        Permissions of a named user. Permissions of the file owner if uid is empty.

	[d[efault]:] g[roup]:gid [:perms]
		Permissions of a named group. Permissions of the owning group if gid is empty.
		
	[d[efault]:] o[ther][:] [:perms]
        Permissions of others.

    [d[efault]:] m[ask][:] [:perms]
        Effective rights mask
	
```

```bash
[sink@dev 16:51:17 vitest]$ ll
total 0
drwxrwxr-x 2 sink sink 6 Oct 14 16:50 test-dir
-rw-rw-r-- 1 sink sink 0 Oct 14 16:51 test-f

# 设置 user1 对文件没有任何权限
[sink@dev 16:54:04 vitest]$ setfacl -m u:user1:- test-f; ll test-f
-rw-rw-r--+ 1 sink sink 0 Oct 14 16:51 test-f
[sink@dev 16:54:51 vitest]$ getfacl test-f
# file: test-f
# owner: sink
# group: sink
user::rw-
user:user1:---
group::rw-
mask::rw-
other::r--
[sink@dev 16:56:28 vitest]$ su user1
Password:
[user1@dev vitest]$ cat test-f
cat: test-f: Permission denied

# 设置其它用户只有 user1 对文件夹有访问权限
[sink@dev 16:58:48 vitest]$ setfacl -m user1:rx test-dir/; ll -d test-dir/
drwxrwx---+ 2 sink sink 6 Oct 14 16:50 test-dir/
[sink@dev 16:59:51 vitest]$ su user1
Password:
[user1@dev vitest]$ cd test-dir/
[user1@dev test-dir]$
[user1@dev test-dir]$ touch user1-f
touch: cannot touch ‘user1-f’: Permission denied

# 用户列表为空，就设置拥有者的权限
[sink@dev test-dir]$ setfacl -m u::r sink-f1; ll sink-f1
-r--rw-r-- 1 sink sink 0 Oct 14 17:13 sink-f1

# 可以同时指定多个 -m 选项
[sink@dev test-dir]$ setfacl -m g:user1:r -m user2:r sink-f2

# mask表示权限范围，当它起作用时，即使有某些权限，但不在mask的范围中，那么权限也不会生效
# 收回除拥有者之外的所有人的写权限和执行权限
[sink@dev vitest]$ setfacl -m m:r test-dir/; ll -d test-dir/
drwxr-----+ 2 sink sink 21 Oct 14 17:13 test-dir
[sink@dev vitest]$ getfacl test-dir/
# file: test-dir/
# owner: sink
# group: sink
user::rwx
# 虽然这里的权限显示没改变，但还是会受到 mask 的限制，全部变为只读
user:user1:r-x                  #effective:r--
group::rwx                      #effective:r--
mask::r--
other::---
[sink@dev vitest]$ su user1
Password:
# 缺少x权限
[user1@dev vitest]$ cd test-dir/
bash: cd: test-dir/: Permission denied
# 只有r权限
[user1@dev vitest]$ ll test-dir/
ls: cannot access test-dir/sink-f1: Permission denied
total 0
-????????? ? ? ? ?            ? sink-f1

# 可继承ACL
[sink@dev vitest]$ setfacl -m d:g:user1:rx test-dir/
# 这样一来，在test-dir下面新建文件或目录都会继承test-dir的ACL权限
```

## 查看ACL

```
getfacl [-aceEsRLPtpndvh] file ...
getfacl [-aceEsRLPtpndvh] -
```

