## Tarball 安装的基本步骤

1. 取得原始档：将 tarball 文件在 /usr/local/src 目录下解压缩；
2. 取得步骤流程：进入新建立的目录底下，去查阅 INSTALL 与 README 等相关文件内容 (很重要的步骤！ )；
3.  依赖软件安装：根据 INSTALL/README 的内容察看并安装好一些依赖的软件 (非必要)；
4. 建立 makefile：以自动检测程序 (configure 或 config) 检测作业环境，并建立 Makefile 文件；
5. 编译：以 make 这个程序并使用该目录下的 Makefile 做为他的参数配置文件，来进行 make (编译或其他)的动作；
6. 安装：以 make 这个程序，并以 Makefile 这个参数配置文件，依据 install 这个目标 (target) 的指定来安装到正确的路径！

## 一般 Tarball 软件安装的建议（移除，升级）

为了方便 Tarball 的管理，通常鸟哥会这样建议使用者：
1. 最好将 tarball 的原始数据解压缩到 `/usr/local/src` 当中；

2. 安装时，最好安装到 `/usr/local` 这个默认路径下；

3. 考虑未来的卸载步骤，最好可以将每个软件单独的安装在 `/usr/local/<software>` 底下。例如`/usr/local/wine/`，通过定义安装路径来实现：`./configure --prefix=/usr/local/ntp`；

4. 为安装到单独目录的软件之 man page 加入 man path 搜寻：
    如果你安装的软件放置到 `/usr/local/<software>/` ，那么 man page 搜寻的设定中，可能就得要在 `/etc/man_db.conf` 内的 40~50 行左右处，写入如下的一行：
    `MANPATH_MAP /usr/local/software/bin /usr/local/software/man`

  这样才可以使用 man 来查询该软件的在线文件！

### 移除

如果单独安装在`/usr/local/<software>`，**一般情况下**只要将该软件安装目录移除即可视为该软件已经被移除。

`make uninstall`。

### 升级

`WWW`服务器为了考虑互动性，所以通常会将 PHP+MySQL+Apache 一起安装起来。因为他们三者之间具有相关性，所以安装时必需要三者同时考虑到他们的库文件与相关的编译参数。

如果今天 PHP 需要重新编译的模块比较多，那么可能会连带的，连 Apache 这个程序也需要重新编译过才行。这是没办法的事啦。

## 利用 patch 更新原码

关于`diff`与`patch`命令参考[文件对比](../命令/文件对比.md)。

```bash
[root@dev tar-test]# cat version-1/main.c ; echo "--------" ; cat version-2/main.c 
# include <stdio.h>
--------
# include <string.h>

# 制作 patch file
[root@dev tar-test]# diff -aur version-1/ version-2/ > 1_to_2.patch
[root@dev tar-test]# cat 1_to_2.patch 
diff -aur version-1/main.c version-2/main.c
--- version-1/main.c	2019-12-24 14:03:56.058127380 +0800
+++ version-2/main.c	2019-12-23 17:03:22.759351459 +0800
@@ -1 +1 @@
-# include <stdio.h>
+# include <string.h>

# 应用 patch file
[root@dev tar-test]# patch -p0 < 1_to_2.patch 
# 或者 cd version-1/ ; patch -p1 < ../1_to_2.patch，注意 -p 是根据所在目录决定的
patching file version-1/main.c
[root@dev tar-test]# cat version-1/main.c ; echo "--------" ; cat version-2/main.c 
# include <string.h>
--------
# include <string.h>

# 还原旧版本文件
[root@dev tar-test]# patch -R -p0 < 1_to_2.patch 
patching file version-1/main.c
[root@dev tar-test]# cat version-1/main.c ; echo "--------" ; cat version-2/main.c 
# include <stdio.h>
--------
# include <string.h>
```

