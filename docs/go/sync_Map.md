
 `sync.Map` 设计非常精巧，但也很复杂。它的核心目标是让 **读取操作尽可能快，甚至无需加锁**。

#### 核心思想
它内部维护两个 map：一个用于快速、无锁读取的 `read` map，和一个需要加锁才能访问的 `dirty` map。

1.  **`read` map**: 这是一个 `atomic.Value`，里面存放一个 `readOnly` 结构体。这个结构体包装了一个 Go 内置的 `map`。因为是原子值，所以可以被多个 goroutine 并发地、无锁地读取。它存储了 map 的一个“快照”。
2.  **`dirty` map**: 这是一个普通的 Go `map`，由一个 `sync.Mutex` 互斥锁保护。它包含了 `read` map 的所有数据，以及任何新增或修改的数据。可以把它看作是“最新最全”的数据集。

#### 数据结构 (简化)
```go
type Map struct {
    mu      sync.Mutex
    read    atomic.Value // 存储 readOnly 结构体
    dirty   map[any]*entry
    misses  int          // 在 read 中未命中的次数
}

type readOnly struct {
    m       map[any]*entry
    amended bool // dirty map 是否包含了 read map 中没有的新 key
}

type entry struct {
    p unsafe.Pointer // *interface{}
}
var expunged = unsafe.Pointer(new(interface{})) // 唯一哨兵
```

- `read`：只读快照（类型类似 `readOnly`），逻辑不可变。
- `dirty`：可变表（`map[any]*entry`），承载所有写入与未命中补充。
- `mu`：全局互斥锁，仅在需要访问或更新 `dirty` 进行提升时持有。
- `misses`：读未命中的计数，用来判断何时触发提升（把 `dirty` 合并进 `read`）。
- `entry`: 原子状态槽，对于 dirty 中存在的每个 entry，read 和 dirty 是共用的
  - `p`：一个 `unsafe.Pointer` 指向实际的 `interface{}` 值。
  - 三态语义：
    - **有效值**：`p` 指向一个非 nil 的 `interface{}`。
    - **nil**：逻辑删除，值不可见，但可“复活”（重新写值）——取决于是否被标记为 expunged。
    - **expunged**：特殊哨兵指针，表示彻底删除，用于阻止从 `read` 直接复活；若要复活必须在 `dirty` 新建 entry。


#### 操作流程

*   **读取 (Load)**
    1.  **快速路径**: 直接从 `read` 这个 `atomic.Value` 中加载 `readOnly` map。如果找到了 key 并且它没有被标记为“已删除”，直接返回。这个过程完全无锁，非常快。
    2.  **慢速路径**: 如果在 `read` 中没找到，就需要加锁 (`mu.Lock()`) 访问 `dirty` map。
    3.  加锁后，**再次检查 `read`**（双重检查），因为可能在你等待锁的时候，其他 goroutine 已经把 `dirty` map 提升为了新的 `read` map。
    4.  如果仍然没有，就从 `dirty` map 中查找。
    5.  同时，`misses` 计数器会递增。当 `misses` 数量达到 `len(dirty)` 时，会触发一次“提升”操作：将 `dirty` map 变成新的 `read` map，并清空 `dirty`。这是一种成本分摊机制。

*   **写入 (Store)**
    1.  首先检查 key 是否存在于 `read` map 中。如果存在，尝试通过原子操作（CAS）直接更新 entry 的值。这也是一个快速路径。
    2.  如果 key 不在 `read` map 中，或者原子更新失败，就进入慢速路径。
    3.  加锁 (`mu.Lock()`)，将新的 key-value 写入 `dirty` map。如果 `dirty` map 为 nil，则需要先将 `read` map 的内容完整复制过来。

*   **删除 (Delete)**
    删除操作是“软删除”。它只是在 `read` map 中通过原子操作将 entry 的值标记为一个特殊的“已删除”状态（`expunged`），而不是真正地从 map 中移除。真正的物理删除发生在 `dirty` map 提升为 `read` map 的过程中。

#### 优点
*   **极快的读取性能**: 在 key 已存在且不被频繁修改的情况下，读取操作几乎等同于一次原子指针读取，性能极高。

#### 缺点
*   **写入性能波动大**: 当 `dirty` map 被提升为 `read` map 时，需要复制整个 map，这是一个耗时操作，会造成性能的突然抖动（Latency Spike）。
*   **高内存占用**: 在某些情况下，`dirty` map 可能是 `read` map 的一个完整副本，导致双倍的内存占用。
*   **代码逻辑复杂**: `read`、`dirty`、`misses`、`amended` 等状态使得内部逻辑非常复杂，难以理解和维护。
*   **`Range` 遍历性能差**: `Range` 操作需要加锁，并且要合并 `read` 和 `dirty` 两部分数据，效率不高。
