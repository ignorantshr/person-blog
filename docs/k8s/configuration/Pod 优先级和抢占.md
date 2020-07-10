- https://kubernetes.io/docs/concepts/configuration/pod-priority-preemption/

**FEATURE STATE:** `Kubernetes v1.14 [stable]`

Pod 可以拥有优先级（priority）。若一个 pod 不能被调度，调度器会尝试抢占/驱逐（preempt/evict）低优先级的 pods。

## 如何使用

1. 添加 [PriorityClass](#priorityclass)；
2. 创建带有 [`priorityClassName`](https://kubernetes.io/docs/concepts/configuration/pod-priority-preemption/#pod-priority) 的 pod。

## 如何关闭抢占特性

不建议关闭此特性。

Kubernetes >= 1.15，若启用了 `NonPreemptingPriority` 特性，PriorityClass 有选项设置：`preemptionPolicy: Never` 。这将防止 PriorityClass 的 Pod 抢占其他Pod。

驱逐性被 kube-scheduler 的  `disablePreemption` 控制，默认是`false`。它只能在组件配置中使用，下面是一个例子：

```yaml
apiVersion: kubescheduler.config.k8s.io/v1alpha1
kind: KubeSchedulerConfiguration
algorithmSource:
  provider: DefaultProvider

...

disablePreemption: true
```

## PriorityClass

它是非命名空间对象，值在`value`字段中指定，值越高则优先级越高。名字必须时合法的[DNS子域名](../../命名规则/#1-dns-dns-subdomain-names)，并且不能以`syste-`作为前缀。

value 可以是任何小于等于10亿的32位整数值。更大的数字被系统保留以便某些系统性的 pod 不会被抢占或驱逐。

PriorityClass 还有两个可选选项：

- `globalDefault`：用于那些没有定义优先级的 pod ，并且只能有一个启用了此选项的 PriorityClass 存在于系统之中。若在系统中没有启用了该选项的 PriorityClass，那么 value 就是 0。
- `description`。

关于已存在的集群有几点要注意的地方：

- 若你使用了此特性更新了已存在的集群，已存在的 pods 实际上的优先级为 0；
- 另外，设置了`globalDefault`为`true`的 PriorityClass 不会改变已存在的 pods 的优先级。这个值只会影响在此值创建之后再创建的 pods；
- 若你删除了 PriorityClass，使用该 PriorityClass 的已存在的 pods 的优先级不会被改变，但是不能再创建新的使用此 PriorityClass 的 pods了。

例子：

```yaml
apiVersion: scheduling.k8s.io/v1
kind: PriorityClass
metadata:
  name: high-priority
value: 1000000
globalDefault: false
description: "This priority class should be used for XYZ service pods only."
```

### 对调度顺序的影响

当启用了 pod 优先级后，优先级越高的 pod 在调度队列中越靠前。若无法调度某个 pod，那么会继续尝试调度低优先级的 pod。

## Preemption

pod 被创建之后，会进入到一个队列中等待被调度。调度器选择一个 pod P 然后尝试分配给某个 node。若没有 node 满足调度条件，就会触发该 pod P 的抢占逻辑。抢占逻辑会尝试发现一个 node ，该 node 可以移除一个或多个低级 pod 来满足 pod P 的调度条件。当这些低级 pod 消失后，pod P 就可以被调度到该 node 上面了。

P **绝不会抢占同等优先级或更高优先级的 pod。**

### 向用户公开的信息

当 pod P 在 node N 上面抢占其它的 pod 时，Pod P 状态的`naminatedNodeName`字段会被设置为 node N 的名字。

但是 P 不一定会被调度到 `naminatedNodeName` 上面。在驱逐的 pod 被抢占后，它们会被优雅地结束。若正在等待低级 pod 结束的时候有其它 node 可以调度，那么 P 会调度到其它 node 上面。还有另一种情况，若有一个更高优先级的 pod 到了，那么调度器可以将 N 给高级 pod 使用，在这种情况下，调度器会清除 P 的 `naminatedNodeName` 字段。通过这样做，调度程序使 Pod P 有资格抢占另一个节点上的 Pod 。

### 抢占策略的限制

#### PodDisruptionBudget 

[Pod Disruption Budget (PDB)](../../workload/pod)（pod 毁坏方案） 是支持的，但不能保证会起作用。只会尽最大努力来保证 PDB。调度器会先查找不会违反 PDB 的 pod，若找不到才会违反 PDB 规则并移除 pod。

一种情况就是若低级的 pod 有 PDB 规则，那么会寻找更高级的 pod。

#### 低优先级 pods 之间的 inter-pod 亲和性

一个 node 能否用于抢占只需要回答一个问题：*“如果所有的比待定 pod 优先级低的 pods 都从该 node 上面移除了，这个待定的 pod 可以被调度到此 node 上面吗？”* 。当然了，调度器会尽可能地移除少一点的节点。

若待定的 pod 与低级 pod 之间有 inter-pod 亲和性，并且在没有这些 pod 的情况下不能满足亲和性规则。在这种情况下，调度器不会抢占此 node 上面的 pod，它会寻找其它合适的 node 。**此时不能保证待定的 pod 被调度。**

官方对这种情况的建议是只在相等或更高优先级的 pod 之间建立亲和性关系。

#### 跨节点抢占

**调度器不支持跨节点抢占**。假设一个 node N 被考虑为了调度 pod P 而驱逐其它 pods，但是此时只有另一个节点上面的某个 pod 也被抢占了之后 P 才能被调度到 N。这时就出现了需要跨节点抢占的情况。这是一个例子：

- 正在考虑将 Pod P 用于 node N。
- pod Q 正运行在和 N 同一个域的其它 node 上。
- P 和 Q 在域的层面上具有反亲和性。
- P 和同一个域里面其它的 pods 没有反亲和性关系。
- 为了在 N 上面调度 P，Q 可以被抢占，但是调度器不会执行跨节点抢占，所以 P 会被认为不能在 N 上面调度。

