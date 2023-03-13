[Go 语言设计与实现](https://draveness.me/golang/)

## 数据结构

### 数组

#### 初始化

总结起来，在不考虑逃逸分析的情况下，如果数组中元素的个数小于或者等于 4 个，那么所有的变量会直接在栈上初始化，如果数组元素大于 4 个，变量就会在静态存储区初始化然后拷贝到栈上，这些转换后的代码才会继续进入[中间代码生成](https://draveness.me/golang/docs/part1-prerequisite/ch02-compile/golang-ir-ssa/)和[机器码生成](https://draveness.me/golang/docs/part1-prerequisite/ch02-compile/golang-machinecode/)两个阶段，最后生成可以执行的二进制文件。

#### 访问和赋值

数组和字符串的一些简单越界错误都会在编译期间发现，例如：直接使用整数或者常量访问数组。只有当编译器无法对数组下标是否越界无法做出判断时才会加入 `PanicBounds` 指令交给**运行时**进行判断，在使用字面量整数访问数组下标时会生成非常简单的中间代码。

赋值的过程中会先确定目标数组的地址，再通过 `PtrIndex` 获取目标元素的地址，最后使用 `Store` 指令将数据存入地址中，从上面的这些 SSA 代码中我们可以看出 上述数组寻址和赋值都是在**编译阶段**完成的，没有运行时的参与。

对数组的访问和赋值需要同时依赖编译器和运行时，它的大多数操作在[编译期间](https://draveness.me/golang/docs/part1-prerequisite/ch02-compile/golang-compile-intro/)都会转换成**直接读写内存**，在中间代码生成期间，编译器还会插入运行时方法 [`runtime.panicIndex`](https://draveness.me/golang/tree/runtime.panicIndex) 调用防止发生越界错误。

### 切片

[`reflect.SliceHeader`](https://draveness.me/golang/tree/reflect.SliceHeader)

```go
type SliceHeader struct {
    Data uintptr
    Len  int
    Cap  int
}
```

#### 初始化

##### 使用下标

使用下标创建切片是最原始也最接近汇编语言的方式，它是所有方法中最为底层的一种，编译器会将 `arr[0:3]` 或者 `slice[0:3]` 等语句转换成 `OpSliceMake` 操作。

使用下标初始化切片不会拷贝原数组或者原切片中的数据，它只会创建一个指向原数组的切片结构体，所以修改新切片的数据也会修改原切片。

##### 字面量

使用字面量 `[]int{1, 2, 3}` 创建新的切片时，[`cmd/compile/internal/gc.slicelit`](https://draveness.me/golang/tree/cmd/compile/internal/gc.slicelit) 函数会在**编译期间**将它展开：

1. 根据切片中的元素数量对底层数组的大小进行推断并创建一个数组；
2. 将这些字面量元素存储到初始化的数组中；
3. 创建一个同样指向 `[3]int` 类型的数组指针；
4. 将静态存储区的数组 `vstat` 赋值给 `vauto` 指针所在的地址；
5. 通过 `[:]` 操作获取一个底层使用 `vauto` 的切片；

##### `make` 关键字

类型检查期间的 [`cmd/compile/internal/gc.typecheck1`](https://draveness.me/golang/tree/cmd/compile/internal/gc.typecheck1)除了校验参数之外，当前函数会将 `OMAKE` 节点转换成 `OMAKESLICE`，中间代码生成的 [`cmd/compile/internal/gc.walkexpr`](https://draveness.me/golang/tree/cmd/compile/internal/gc.walkexpr) 函数会依据下面两个条件转换 `OMAKESLICE` 类型的节点：

1. 切片的大小和容量是否足够小；
2. 切片是否发生了逃逸，最终在堆上初始化

当切片发生逃逸或者非常大时，运行时需要 [`runtime.makeslice`](https://draveness.me/golang/tree/runtime.makeslice)（主要工作是计算切片占用的内存空间并在堆上申请一片连续的内存，大小=切片中元素大小×切片容量） 在堆上初始化切片。

如果当前的切片不会发生逃逸并且切片非常小的时候，`make([]int, 3, 4)` 会初始化数组并通过下标 `[:3]` 得到数组对应的切片，这两部分操作都会在编译阶段完成，编译器会在栈上或者静态存储区创建数组并将 `[:3]` 转换成上一节提到的 `OpSliceMake` 操作。

[`runtime.makeslice`](https://draveness.me/golang/tree/runtime.makeslice) 在最后调用的 [`runtime.mallocgc`](https://draveness.me/golang/tree/runtime.mallocgc) 是用于申请内存的函数，这个函数的实现还是比较复杂，如果遇到了比较小的对象会直接初始化在 Go 语言调度器里面的 P 结构中，而大于 32KB 的对象会在堆上初始化。
