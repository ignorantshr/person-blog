## 锁定不同对象
- synchronized关键字加到static静态方法和synchronized(class)代码块上都是是给Class类上锁
- 而synchronized关键字加到非static静态方法上是给对象上锁。

**warning**：当给`String`上锁时，锁定的是一个常量

## 等待/通知机制
![线程的几种状态](img/线程的状态.png "状态")
当方法wait()被执行后，锁自动被释放，但执行完notify()方法后，锁不会自动释放。必须执行完notify()方法所在的synchronized代码块后才释放。

当线程呈wait状态时，对线程对象调用interrupt方法会出现InterrupedException异常。

## 锁

“可重入锁”概念是：自己可以再次获取自己的内部锁。

简单用法：

```java
lock.lock();
try {
    for(int i=0; i < 3; ++i){
        System.out.println(Thread.currentThread().getName() + " -> " + i);
    }
}finally {
    lock.unlock();
}
```

**warning**：最好不要把获取锁的过程写在try语句块中，因为如果在获取锁时发生了异常，异常抛出的同时也会导致锁无法被释放。

Lock接口提供的synchronized关键字不具备的主要特性：

|        特性        |                             描述                             |
| :----------------: | :----------------------------------------------------------: |
| 尝试非阻塞地获取锁 | 当前线程尝试获取锁，如果这一时刻锁没有被其它线程获取到，则成功获取并持有锁 |
|  能被中断地获取锁  | 获取到锁的线程能相应中断，当获取到锁的线程被中断时，中断异常会被抛出，同时释放锁 |
|     超时获取锁     |   在指定的截止时间之前获取锁，超过截止时间后仍无法获取返回   |

**warning**：必须在condition.await()方法调用之前调用lock.lock()代码获得同步监视器，不然会报错。

### ReenTrantLock 比 synchronized 增加了一些高级功能

相比synchronized，ReenTrantLock增加了一些高级功能。主要来说主要有三点：①等待可中断；②可实现公平锁；③可实现选择性通知（锁可以绑定多个条件）

1. ReenTrantLock提供了一种能够中断等待锁的线程的机制，通过lock.lockInterruptibly()来实现这个机制。也就是说正在等待的线程可以选择放弃等待，改为处理其他事情。
2. ReenTrantLock可以指定是公平锁还是非公平锁。而synchronized只能是非公平锁。所谓的公平锁就是先等待的线程先获得锁。 ReenTrantLock默认情况是非公平的，可以通过 ReenTrantLock类的ReentrantLock(boolean fair)构造方法来制定是否是公平的。
3. synchronized关键字与wait()和notify/notifyAll()方法相结合可以实现等待/通知机制，ReentrantLock类当然也可以实现，但是需要借助于Condition接口与newCondition() 方法。Condition是JDK1.5之后才有的，它具有很好的灵活性，比如可以实现多路通知功能也就是在一个Lock对象中可以创建多个Condition实例（即对象监视器），线程对象可以注册在指定的Condition中，从而可以有选择性的进行线程通知，在调度线程上更加灵活。 在使用notify/notifyAll()方法进行通知时，被通知的线程是由 JVM 选择的，用ReentrantLock类结合Condition实例可以实现“选择性通知” ，这个功能非常重要，而且是Condition接口默认提供的。而synchronized关键字就相当于整个Lock对象中只有一个Condition实例，所有的线程都注册在它一个身上。如果执行notifyAll()方法的话就会通知所有处于等待状态的线程这样会造成很大的效率问题，而Condition实例的signalAll()方法 只会唤醒注册在该Condition实例中的所有等待线程。

## 线程池

阿里巴巴不推荐的创建方式：

**1. 线程资源必须通过线程池提供，不允许在应用中自行显示创建线程**

​	*使用线程池的好处是减少在创建和销毁线程上所消耗的时间以及系统资源开销，解决资源不足的问题。如果不使用线程池，有可能会造成系统创建大量同类线程而导致消耗完内存或者“过度切换”的问题。*

**2. 不允许使用 Executors 去创建，而是通过 ThreadPoolExecutor 的方式**

​	*Executors 返回线程池对象的弊端如下*：

- FixedThreadPool 和 SingleThreadExecutor ： 允许请求的队列长度为 Integer.MAX_VALUE,可能堆积大量的请求，从而导致OOM。
- CachedThreadPool 和 ScheduledThreadPool ： 允许创建的线程数量为 Integer.MAX_VALUE ，可能会创建大量线程，从而导致OOM。

`Executor`框架