# 用 threading.Event 实现线程间通信

使用threading.Event可以使一个线程等待其他线程的通知， Event默认内置了一个标志，初始值为False。
一旦该线程通过**wait()**方法进入等待状态，直到另一个线程调用该Event的**set()**方法将内置标志设置为True时，该Event会通知所有等待状态的线程恢复运行。

**clear()**方法则相反。
