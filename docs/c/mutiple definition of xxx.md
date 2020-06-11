参考：https://www.cnblogs.com/fnlingnzb-learner/p/5890065.html

!!! warning
	不要将全局变量和全局方法的定义放在头文件中。

1. 给每个头文件都加上条件编译

```c
#ifndef TEST_H
#define TEST_H
......
#endif // TEST_H
```

2. 使用extern

    在头文件中使用`extern`关键字声明全局变量，在`.c`文件中定义并引入头文件。