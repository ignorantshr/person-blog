- https://kubernetes.io/zh/docs/concepts/configuration/taint-and-toleration/
- https://kubernetes.io/docs/concepts/scheduling-eviction/taint-and-toleration/

节点亲和性（详见[这里](https://kubernetes.io/docs/concepts/configuration/assign-pod-node/#node-affinity-beta-feature)），是 *pod* 的一种属性（偏好或硬性要求），它使 *pod* 被吸引到一类特定的节点。Taint 则相反，它使 节点 能够 ***排斥*** 一类特定的 pod。

Taint 和 toleration 相互配合，可以用来避免 pod 被分配到不合适的节点上。每个节点上都可以应用一个或多个 taint ，这表示**对于那些不能容忍这些 taint 的 pod，是不会被该节点接受的**。如果将 toleration 应用于 pod 上，则表示这些 pod 可以（但不要求）被调度到具有匹配 taint 的节点上。

## 概念

向节点添加污点：

```
kubectl taint nodes node1 key=value:NoSchedule
```

给节点 `node1` 增加一个 taint，它的 key 是 `key`，value 是 `value`，effect 是 `NoSchedule`。

删除污点：

```
kubectl taint nodes node1 key:NoSchedule-
```



然后可以在 PodSpec 中定义 pod 的 toleration。有以下两种方式：

```yaml
tolerations:
- key: "key"
  operator: "Equal"
  value: "value"
  effect: "NoSchedule"
```

```yaml
tolerations:
- key: "key"
  operator: "Exists"
  effect: "NoSchedule"
```

一个 toleration 和一个 taint **相”匹配”是指它们有一样的 key 和 effect** ，并且：

- 如果 `operator` 是 `Exists` （此时 toleration 不能指定 `value`）
    - 若 key 为空，那么这个 toleration 能容忍任意 taint。
- 如果 `operator` 是 `Equal` ，则它们的 `value` 应该相等，默认值。
- 若 effect 为空，那么匹配该 key 所有的 effect。

Kubernetes 处理多个 taint 和 toleration 的过程就像一个过滤器：从一个节点的所有 taint 开始遍历，过滤掉那些 pod 中存在与之相匹配的 toleration 的 taint。**余下未被过滤的 taint 的 effect 值**决定了 pod 是否会被分配到该节点，特别是以下情况：

- 如果未被过滤的 taint 中存在一个以上 effect 值为 `NoSchedule` 的 taint，则 Kubernetes **不会**将 pod 分配到该节点。但是如果在给节点添加 taint 之前，该 pod 已经在上述节点运行，那么它还可以继续运行在该节点上。
- 如果未被过滤的 taint 中不存在 effect 值为 `NoSchedule` 的 taint，但是存在 effect 值为 `PreferNoSchedule` 的 taint，则 Kubernetes 会 ***尝试*** 将 pod 分配到该节点。
- 如果未被过滤的 taint 中存在一个以上 effect 值为 `NoExecute` 的 taint，则 Kubernetes **不会**将 pod 分配到该节点（如果 pod 还未在节点上运行），或者将 pod 从该节点**驱逐**（如果 pod 已经在节点上运行）。
    - 如果 pod 存在一个 effect 值为 `NoExecute` 的 toleration 指定了可选属性 `tolerationSeconds` 的值，则表示在给节点添加了上述 taint 之后，pod 还能继续在节点上运行的时间。

## 基于 taint 的驱逐

**FEATURE STATE:** `Kubernetes v1.18 [stable]`

taint 的 effect 值 `NoExecute` ，它会影响已经在节点上运行的 pod

- 如果 pod 不能忍受 effect 值为 `NoExecute` 的 taint，那么 pod 将**马上被驱逐**
- 如果 pod 能够忍受 effect 值为 `NoExecute` 的 taint，但是在 toleration 定义中没有指定 `tolerationSeconds`，则 pod **还会**一直在这个节点上运行。
- 如果 pod 能够忍受 effect 值为 `NoExecute` 的 taint，而且指定了 `tolerationSeconds`，则 pod 还能在这个节点上继续运行这个指定的**时间长度**。

当节点的某些状态条件为真时，node controller 会自动给节点添加一个 taint。[详见](https://kubernetes.io/zh/docs/concepts/configuration/taint-and-toleration/#%E5%9F%BA%E4%BA%8E-taint-%E7%9A%84%E9%A9%B1%E9%80%90)。

在节点被驱逐时，节点控制器或者 kubelet 会添加带有 `NoExecute` 效应的相关污点。如果异常状态恢复正常，kubelet 或节点控制器能够移除相关的污点。

系统实际上会以 rate-limited 的方式添加 taint。在像 master 和 node 通讯中断等场景下，这避免了 pod 被大量驱逐。

[DaemonSet](https://kubernetes.io/docs/concepts/workloads/controllers/daemonset/) 中的 pod 被创建时，针对以下 taint 自动添加的 `NoExecute` 的 toleration 将不会指定 `tolerationSeconds`：

- `node.kubernetes.io/unreachable`
- `node.kubernetes.io/not-ready`

这保证了出现上述问题时 DaemonSet 中的 pod 永远不会被驱逐。

自 Kubernetes 1.8 起， DaemonSet 控制器自动为所有守护进程添加如下 `NoSchedule` toleration 以防 DaemonSet 崩溃：

- `node.kubernetes.io/memory-pressure`
- `node.kubernetes.io/disk-pressure`
- `node.kubernetes.io/out-of-disk` (*只适合 critical pod*)
- `node.kubernetes.io/unschedulable` (1.10 或更高版本)
- `node.kubernetes.io/network-unavailable` (*只适合 host network*)