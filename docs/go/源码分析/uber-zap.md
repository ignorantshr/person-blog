[zap](https://github.com/uber-go/zap)

[从Go高性能日志库zap看如何实现高性能Go组件](https://mp.weixin.qq.com/s/i0bMh_gLLrdnhAEWlF-xDw)

## zapcore

大量使用了 `sync.Pool` 来获取实例

### EntryCaller

日志方法调用者

```go
// EntryCaller represents the caller of a logging function.
type EntryCaller struct {
   Defined bool
   PC      uintptr
   File    string
   Line    int
}
```

### LevelEnabler

决定了给定的 level 是否开启 。

```go
type LevelEnabler interface {
   Enabled(Level) bool
}
```

### WriteSyncer

带刷出缓存数据的 witer

```go
// *os.File (and thus, os.Stderr and os.Stdout) implement WriteSyncer
type WriteSyncer interface {
   io.Writer
   Sync() error
}
```

### Encoder

编码器，core.Write 使用 encoder 构造输出内容

```go
type Encoder interface {
   ObjectEncoder

   // Clone copies the encoder, ensuring that adding fields to the copy doesn't
   // affect the original.
   Clone() Encoder

   // EncodeEntry encodes an entry and fields, along with any accumulated
   // context, into a byte buffer and returns it.
   EncodeEntry(Entry, []Field) (*buffer.Buffer, error)
}
```

#### ArrayEncoder

数组编码器接口，将任意类型添加到zap支持的混合类型数组中。

```go
type ArrayEncoder interface {
    // Built-in types. 
   // go 内置类型接口， ArrayEncoder 的子集
    PrimitiveArrayEncoder

    // Time-related types.
    AppendDuration(time.Duration)
    AppendTime(time.Time)

    // Logging-specific marshalers.
    AppendArray(ArrayMarshaler) error
    AppendObject(ObjectMarshaler) error

    // AppendReflected uses reflection to serialize arbitrary objects, so it's
    // slow and allocation-heavy.
    AppendReflected(value interface{}) error
}
```

##### ArrayMarshaler

```go
type ArrayMarshaler interface {
    MarshalLogArray(ArrayEncoder) error
}
```

zap 对 go数组类型的数据结构封装了一层，实现了 ArrayMarshaler 接口，比如 `type bools []bool` 。这样 ArrayEncoder 就可以在添加 `bools`数组时调用 ArrayMarshaler 接口的 `MarshalLogArray(ArrayEncoder)` 方法，使用 ArrayEncoder 的 `AppendBool` 方法遍历自身将元素编码到zap数组类型的 (k,v)的 v 中。

#### ObjectEncoder

对象编码器，声明了各种类型的存储方法，具体实现需要通过这些方法存储要输出的内容

```go
type ObjectEncoder interface {
    // Logging-specific marshalers.
    AddArray(key string, marshaler ArrayMarshaler) error
    AddObject(key string, marshaler ObjectMarshaler) error

    // Built-in types.
    AddBinary(key string, value []byte)     // for arbitrary bytes
    AddByteString(key string, value []byte) // for UTF-8 encoded bytes
    AddBool(key string, value bool)
    。。。
}
```

##### ObjectMarshaler

```go
type ObjectMarshaler interface {
    MarshalLogObject(ObjectEncoder) error
}
```

与 ArrayMarshaler 类似，用户可以自定义数据结构的相关实现。

### Core

```go
type Core interface {
    // 需要此接口用于 Check 判断
       LevelEnabler

    // With adds structured context to the Core.
    // 存储要输出的结构化字段
    With([]Field) Core
    // 检查自己这个 core 是否可以记录 entry 日志，可以的话把自己加到 CheckedEntry 的 cores 中
    Check(Entry, *CheckedEntry) *CheckedEntry

    // 使用 encoder 序列化 entry 和任何支持的 filed 到 writer
    // 在调用 Write 之前必须调用 Check
    Write(Entry, []Field) error

    // Sync flushes buffered logs (if any).
    Sync() error
}
```

```go
func NewCore(enc Encoder, ws WriteSyncer, enab LevelEnabler) Core {
   return &ioCore{
      LevelEnabler: enab,
      enc:          enc,
      out:          ws,
   }
}

type ioCore struct {
   LevelEnabler
   enc Encoder
   out WriteSyncer
}

func (c *ioCore) Write(ent Entry, fields []Field) error {
    buf, err := c.enc.EncodeEntry(ent, fields)
    if err != nil {
        return err
    }
    _, err = c.out.Write(buf.Bytes())
    buf.Free()
    if err != nil {
        return err
    }
    if ent.Level > ErrorLevel {
        // Since we may be crashing the program, sync the output. Ignore Sync
        // errors, pending a clean solution to issue #370.
        c.Sync()
    }
    return nil
}
```

### Entry

代表了一条完整的日志信息

```go
type Entry struct {
   Level      Level
   Time       time.Time
   LoggerName string
   Message    string
   Caller     EntryCaller
   Stack      string
}
```

### CheckedEntry

维护了一个同意记录 entry 的 cores 列表。

```go
type CheckedEntry struct {
   Entry
   ErrorOutput WriteSyncer
   dirty       bool // best-effort detection of pool misuse
   should      CheckWriteAction
   cores       []Core
}

// 调用每个 cores 输出日志，然后把自己放回 pool 中
func (ce *CheckedEntry) Write(fields ...Field) {
    var err error
    for i := range ce.cores {
        err = multierr.Append(err, ce.cores[i].Write(ce.Entry, fields))
    }

    should, msg := ce.should, ce.Message
    putCheckedEntry(ce)

    switch should {
    case WriteThenPanic:
        panic(msg)
    case WriteThenFatal:
        exit.Exit()
    }
}
```

### 说明

Core 作为核心部件，提供了以下几个功能：

1. 判断 Entry 是否允许记录，以及添加到 CheckdEntry 队列中去

2. 使用 Encoder 对日志进行格式化（编码），使用 WriteSyncer 对日志进行输出

3. 允许自定义增加结构化日志信息

CheckdEntry 作为实际的调用者，包含了原本的日志信息，维护了 Core 列表，从而逐个调用 Core 的具体日志输出方法，达到同一条日志多种输出效果。

Encoder 避免使用反射，通过明确的类型调用，直接拼接字符串，最小化性能开销。
