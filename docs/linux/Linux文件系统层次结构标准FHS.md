[Filesystem Hierarchy Standard（FHS）](https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard)文件系统层次结构标准定义了Linux发行版中的**目录结构和目录内容**。

## 目录结构

在`FHS`中，所有文件和目录都显示在根目录`/`下，即使它们存储在不同的物理或虚拟设备上。如果安装了某些子系统（如X Window System），则某些目录仅存在于特定系统上。

| 目录              | 描述                                                         |
| :---------------- | :----------------------------------------------------------- |
| `/`               | 主层次结构根目录和整个文件系统层次结构的根目录。             |
| `/bin`            | 需要在单用户模式下可以的必要命令的可执行文件；是面向所有用户的，例如 [cat](https://en.wikipedia.org/wiki/Cat_(Unix)), [ls](https://en.wikipedia.org/wiki/Ls), [cp](https://en.wikipedia.org/wiki/Cp_(Unix)). |
| `/boot`           | 引导启动文件。一般是单独的分区。[Boot loader](https://en.wikipedia.org/wiki/Boot_loader) files, *e.g.*, [kernels](https://en.wikipedia.org/wiki/Kernel_(computer_science)), [initrd](https://en.wikipedia.org/wiki/Initrd). |
| `/dev`            | 设备文件。 *e.g.*, `/dev/null`, `/dev/disk0`, `/dev/sda1`, `/dev/tty`, `/dev/random`. |
| `/etc`            | 特定主机，系统范围内的配置文件。FHS限制*/etc*存放静态配置文件，不能包含二进制文件 |
| `/etc/opt`        | 存储在*/opt*中的附加包的配置文件。                           |
| `/etc/sgml`       | 处理[SGML](https://en.wikipedia.org/wiki/SGML)软件的配置文件，比如目录。 |
| `/etc/X11`        | [X Window System](https://en.wikipedia.org/wiki/X_Window_System) （版本 11）的配置文件 |
| `/etc/xml`        | 处理[XML](https://en.wikipedia.org/wiki/XML)软件的配置文件，比如目录。 |
| `/home`           | 用户的*home*目录，包含保存的文件、个人设置等。一般是单独的分区。 |
| `/lib`            | `/bin` 和`/sbin`中可执行文件的必要库文件。                   |
| `/lib<qual>`      | 可选格式必要库。此类目录是可选的,但如果它们存在,则它们具有一些要求。例如*/lib64*。 |
| `/media`          | 可移除媒体文件（比如[CD-ROMs](https://en.wikipedia.org/wiki/CD-ROM)）挂载点。 |
| `/mnt`            | 临时挂载的文件系统。                                         |
| `/opt`            | 可选应用软件包。 [application software](https://en.wikipedia.org/wiki/Application_software) [packages](https://en.wikipedia.org/wiki/Software_package_(installation))。 |
| `/proc`           | 虚拟文件系统，以文件形式提供进程和内核信息。在linux中，对应一个[procfs](https://en.wikipedia.org/wiki/Procfs)（在许多类 Unix 计算机系统中， **procfs** 是 进程 文件系统 的缩写，包含一个伪文件系统（启动时动态生成的文件系统），用于通过内核访问进程信息。这个文件系统通常被挂载到 `/proc` 目录。由于 /proc 不是一个真正的文件系统，它也就不占用存储空间，只是占用有限的内存。https://zh.wikipedia.org/wiki/Procfs）挂载。 通常自动生成并由系统动态填充。 |
| `/root`           | root用户的*home*目录                                         |
| `/run`            | 运行时变化的数据：自从最后一次启动的正在运行的系统的信息。比如，当前登录的用户和运行的守护进程。在这个目录下的文件必须在启动进程开始时删除或截断（清空）；但是在提供[temporary filesystem](https://en.wikipedia.org/wiki/Temporary_filesystem) ([tmpfs](https://en.wikipedia.org/wiki/Tmpfs))目录的操作系统上是不必要的。 |
| `/sbin`           | 必要的系统级的可执行文件。*e.g.*, fsck, init, route.         |
| `/srv`            | 系统提供的具体站点数据。例如为web服务提供的数据和脚本，为FTP服务提供的数据，为版本控制系统提供的仓库等。 |
| `/sys`            | 包含关于设备、驱动和一些内核特性的信息。                     |
| `/tmp`            | 临时文件 (see also `/var/tmp`)。系统重启时通常不会保留这些文件，并可能严重限制大小。 |
| `/usr`            | 对于只读用户数据的二级层次结构；包含大部分的（多）用户工具和应用程序。 |
| `/usr/bin`        | 非必要命令可执行文件（单用户模式下非必需）；面向所有用户。   |
| `/usr/include`    | 标准头文件。                                                 |
| `/usr/lib`        | `/usr/bin` 和`/usr/sbin`中可执行文件的库。                   |
| `/usr/lib<qual>`  | 可选格式库。*e.g.* `/usr/lib32` for 32-bit libraries on a 64-bit machine (optional). |
| `/usr/local`      | 本地数据的三级层次结构，特定于此主机。通常有更多子目录， *e.g.*, `bin`, `lib`, `share`。 |
| `/usr/sbin`       | 非必要系统级的可执行文件。*e.g.*, [daemons](https://en.wikipedia.org/wiki/Daemon_(computer_software)) for various [network-services](https://en.wikipedia.org/wiki/Network-services). |
| `/usr/share`      | 非架构依赖的共享数据。                                       |
| `/usr/src`        | 源码。*e.g.*, the kernel source code with its header files.  |
| `/usr/X11R6`      | [X Window System](https://en.wikipedia.org/wiki/X_Window_System), Version 11, Release 6 (up to FHS-2.3, optional). |
| `/var`            | 变化的文件——在系统的正常操作过程中内容一直变化的文件。例如 logs, spool files, and temporary e-mail files。有时是单独的一个分区。 |
| `/var/cache`      | 应用程序缓存数据。这些数据是在本地生成的一个耗时的I/O结果或计算结果。应用程序必须能够再生或恢复数据。缓存的文件可以被删除而不导致数据丢失。 |
| `/var/lib`        | 状态信息。 程序在运行时修改的持久性数据, *e.g.*, databases, packaging system metadata, etc. |
| `/var/lock`       | 锁文件。一类跟踪当前正在使用的资源的文件。                   |
| `/var/log`        | 日志文件，包含大量日志文件。                                 |
| `/var/mail`       | 电子邮件。在一些发行版中，这些文件可能位于废弃的 `/var/spool/mail`. |
| `/var/opt`        | 存储可选包的`/opt`中的变化数据。                             |
| `/var/run`        | 运行时变化数据。该目录包含了自从启动以来的系统信息数据。In FHS 3.0, `/var/run` is replaced by `/run`; a system should either continue to provide a `/var/run` directory, or provide a symbolic link from `/var/run` to `/run`, for backwards compatibility. |
| `/var/spool`      | 等待处理的任务的[Spool](https://en.wikipedia.org/wiki/Spooling) ，*e.g.*, print queues and outgoing mail queue. |
| `/var/spool/mail` | 废弃的用户邮件目录。                                         |
| `/var/tmp`        | 在重启之后应该保存的文件。                                   |

FHS只是一个规范，不一定要严格遵循它。