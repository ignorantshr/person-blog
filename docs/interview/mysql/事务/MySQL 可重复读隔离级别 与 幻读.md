
[TOC]
# MySQL 可重复读隔离级别 与 幻读

[MySQL 可重复读隔离级别，完全解决幻读了吗](https://xiaolincoding.com/mysql/transaction/phantom.html)

前文说道，MySQL InnoDB 引擎的默认隔离级别虽然是「可重复读」，但是它**很大程度上避免幻读现象，并不是完全解决了**，解决的方案有两种：

- 针对**快照读**（普通select 语句），是通过 **MVCC 方式**解决的。因为在此隔离级别下，事务执行过程中看到的数据，一直跟这个事务启动时看到的数据一致，即使中途有其它事务插入了一条数据，也查询不出来，所以就能避免幻读。
- 针对**当前读**（select … for update 等语句），是通过 **next-key lock（记录锁+间隙锁）**解决的。因为在执行这种语句时会加上 `next-key lock`，如果有其它事务在这个锁的范围内插入了一条记录，那么这个插入语句就会被阻塞，所以就避免了幻读。

本文说下其它不能解决的情况。
## 什么是幻读

> The so-called phantom problem occurs within a transaction when the same query produces different sets of rows at different times. For example, if a SELECT is executed twice, but returns a row the second time that was not returned the first time, the row is a “phantom” row.

当同一个查询在不同的时间产生不同的结果集时，事务中就会出现所谓的幻象问题。例如，如果 SELECT 执行了两次，但第二次返回了第一次没有返回的行，则该行是“幻像”行。
## 快照读是如何避免幻读的

可重复读隔离级是由 MVCC（多版本并发控制）实现的，**实现的方式是启动事务后，在执行第一个查询语句后，会创建一个 Read View**，后续的查询语句利用这个 Read View，通过这个 Read View 就可以在 undo log 版本链找到事务开始时的数据，所以**事务过程中每次查询的数据都是一样的，即使中途有其他事务*插入*了新纪录，是查询不出来这条数据的**，所以就很好了避免幻读问题。注意⚠️是**插入**。

详见《MySQL 事务》-> 事务的隔离性 -> 可重复读如何工作 小节。
## 当前读是如何避免幻读的

MySQL 里除了普通查询是快照读，其他都是**当前读，比如 update、insert、delete，这些语句执行前都会查询最新版本的数据**，然后再做进一步的操作。

Innodb 引擎为了解决「可重复读」隔离级别使用「当前读」而造成的幻读问题，就引出了**间隙锁**。

![间隙锁事务举例](img/间隙锁事务举例.webp)

事务 A 执行了这面这条锁定读语句后，就在对表中的记录加上 id 范围为 (2, +∞] 的 next-key lock（next-key lock 是间隙锁 + 记录锁的组合）。

然后，事务 B 在执行插入语句的时候，判断到插入的位置被事务 A 加了 next-key lock，于是事务 B 会生成一个插入意向锁，同时进入等待状态，直到事务 A 提交了事务。这就避免了由于事务 B 插入新记录而导致事务 A 发生幻读的现象。
## 发生幻读的场景

### 1. 更新看不到的记录

![幻读发生](img/幻读发生.drawio.webp)

在可重复读隔离级别下，事务 A 第一次执行普通的 select 语句时生成了一个 ReadView，之后**事务 B 向表中新插入了一条 id = 5 的记录并提交。接着，事务 A 对 id = 5 这条记录进行了更新操作**，在这个时刻，这条新记录的 trx_id 隐藏列的值就变成了事务 A 的事务 id，之后事务 A 再使用普通 select 语句去查询这条记录时就可以看到这条记录了，于是就发生了幻读。

这是因为更新语句是当前读模式。

### 2. 执行语句不一致

- T1 时刻：事务 A 先执行**「快照读语句」**：`select * from t_test where id > 100` 得到了 3 条记录。
- T2 时刻：事务 B 往t_test表中插入一个 id= 200 的记录并提交；
- T3 时刻：事务 A 再执行**「当前读语句」**`select * from t_test where id > 100 for update` 就会得到 4 条记录，此时也发生了幻读现象。

**要避免这类特殊场景下发生幻读的现象的话，就是尽量在开启事务之后，马上执行 `select ... for update` 这类当前读的语句**，因为它会对记录加 next-key lock，从而避免其他事务插入一条新记录。

