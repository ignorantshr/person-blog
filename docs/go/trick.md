### 克隆

[zap](https://github.com/uber-go/zap)

```go
type Logger struct {
}

func (log *Logger) clone() *Logger {
    copy := *log
    return &copy
}
```

### 释放引用

[zapcore](https://github.com/uber-go/zap/zapcore)

```go
func (ce *CheckedEntry) reset() {
    ce.Entry = Entry{}
    ce.ErrorOutput = nil
    ce.dirty = false
    ce.should = WriteThenNoop
    for i := range ce.cores {
        // don't keep references to cores
        ce.cores[i] = nil
    }
    ce.cores = ce.cores[:0]
}
```

### 自身 nil 时获取新实例

[zapcore](https://github.com/uber-go/zap/zapcore)

```go
func (ce *CheckedEntry) Should(ent Entry, should CheckWriteAction) *CheckedEntry {
   if ce == nil {
      ce = getCheckedEntry()
      ce.Entry = ent
   }
   ce.should = should
   return ce
}
```

### 方法转换为符合定义的接口类型

[zap](https://github.com/uber-go/zap)

定义接口类型，定义转换函数：

```go
// An Option configures a Logger.
type Option interface {
    apply(*Logger)
}

// optionFunc wraps a func so it satisfies the Option interface.
type optionFunc func(*Logger)

func (f optionFunc) apply(log *Logger) {
    f(log)
}
```

使用：

```go
// WrapCore wraps or replaces the Logger's underlying zapcore.Core.
func WrapCore(f func(zapcore.Core) zapcore.Core) Option {
    return optionFunc(func(log *Logger) {
        log.core = f(log.core)
    })
}
```

### 类型别名

```go
// Field is an alias for Field. Aliasing this type dramatically
// improves the navigability of this package's API documentation.
type Field = zapcore.Field

// Bool constructs a field that carries a bool.
func Bool(key string, val bool) Field {
    var ival int64
    if val {
        ival = 1
    }
    return Field{Key: key, Type: zapcore.BoolType, Integer: ival}
}
```

完全等价，方便使用