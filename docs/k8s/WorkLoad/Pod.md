https://kuboard.cn/learning/k8s-intermediate/workload/pod.html

## 概述

### 控制器

用户应该始终使用控制器来创建 Pod，控制器(（例如 Deployment、StatefulSet、DaemonSet 等)可以提供如下特性：

- 水平扩展（运行 Pod 的多个副本）
- rollout（版本更新）
- self-healing（故障恢复）

### Pod Template

Pod Template 是关于 Pod 的定义，但是被包含在其他的 Kubernetes 对象中（例如 Deployment、StatefulSet、DaemonSet 等控制器）。

## 容器的检查

Probe 是指 kubelet 周期性地检查容器的状况。有三种类型的 Probe：

- **ExecAction：** 在容器内执行一个指定的命令。如果该命令的退出状态码为 0，则成功
- **TCPSocketAction：** 探测容器的指定 TCP 端口，如果该端口处于 open 状态，则成功
- **HTTPGetAction：** 探测容器指定端口/路径上的 HTTP Get 请求，如果 HTTP 响应状态码在 200 到 400（不包含400）之间，则成功

Probe 有三种可能的结果：

- **Success：** 容器通过检测
- **Failure：** 容器未通过检测
- **Unknown：** 检测执行失败，此时 kubelet 不做任何处理

Kubelet 可以在两种情况下对运行中的容器执行 Probe：

- **就绪检查 readinessProbe：** 确定容器是否已经就绪并接收服务请求。如果就绪检查失败，kubernetes 将该 Pod 的 IP 地址从所有匹配的 Service 的资源池中移除掉。
- **健康检查 livenessProbe：** 确定容器是否正在运行。如果健康检查失败，kubelete 将结束该容器，并根据 restart policy（重启策略）确定是否重启该容器。

更详细的说明参考：

- https://kuboard.cn/learning/k8s-intermediate/workload/pod-lifecycle.html#%E5%AE%B9%E5%99%A8%E7%9A%84%E6%A3%80%E6%9F%A5
- https://kuboard.cn/learning/k8s-intermediate/workload/pod-health.html#%E4%B8%80%E3%80%81%E9%9C%80%E6%B1%82%E6%9D%A5%E6%BA%90

## 重启策略

定义 Pod 或工作负载时，可以指定 restartPolicy，可选的值有：

- Always （默认值）
- OnFailure
- Never

restartPolicy 将作用于 Pod 中的所有容器。

!!! note
	控制器 Deployment/StatefulSet/DaemonSet 中，只支持 Always 这一个选项，不支持 OnFailure 和 Never 选项。

## 初始化容器

Pod 可以包含多个工作容器和多个初始化容器，初始化容器在工作容器启动之前执行。

工作容器和初始化容器的区别：

- 初始化容器总是运行并自动结束
- kubelet 按顺序执行 Pod 中的初始化容器，前一个初始化容器成功结束后，下一个初始化容器才开始运行。所有的初始化容器成功执行后，才开始启动工作容器
- 如果 Pod 的任意一个初始化容器执行失败，kubernetes 将反复重启该 Pod，直到初始化容器全部成功（除非 Pod 的 restartPolicy 被设定为 Never），因此初始化容器中的代码必须是 [**幂等**](https://kuboard.cn/glossary/idempotent.html) 的。
- 初始化容器的 Resource request / limits 处理不同，请参考 [Resources](#resources)
- 初始化容器不支持 [就绪检查 readiness probe](https://kuboard.cn/learning/k8s-intermediate/workload/pod-lifecycle.html#container-probes)，因为初始化容器必须在 Pod ready 之前运行并结束
- 初始化容器的端口是不能够通过 kubernetes Service 访问的
- Pod 在初始化过程中处于 Pending 状态
- 改变工作容器的镜像，将只重启该工作容器，而不重启 Pod

### Resources

在确定初始化容器的执行顺序以后，以下 resource 使用规则将适用：

- 所有初始化容器中最高的 resource request/limit 是最终生效的 request/limit
- 对于 Pod 来说，最终生效的 resource request/limit 是如下几个当中较高的一个： 
    - 所有工作容器某一个 resource request/limit 的和
    - 最终生效的初始化容器的 request/limit 的和
- Kubelet 依据最终生效的 request/limit 执行调度，这意味着，在执行初始化容器时，就已经为 Pod 申请了其资源需求

## 容器的毁坏

### 自愿的和非自愿的毁坏

除非有人（人或者控制器）销毁Pod，或者出现不可避免的硬件/软件故障，Pod不会凭空消失。此类不可避免的情况，我们称之为非自愿的毁坏（involuntary disruption）。例如：

- 节点所在物理机的硬件故障
- 集群管理员误删了虚拟机
- 云供应商或管理程序故障导致虚拟机被删
- Linux 内核故障
- 集群所在的网络发生分片，导致节点不可用
- 节点资源耗尽，导致 Pod 被驱逐

还有一类毁坏，我们称之为自愿的毁坏（voluntary disruptions）。主要包括由应用管理员或集群管理员主动执行的操作。**应用管理员**可能执行的操作有：

- 删除 Deployment 或其他用于管理 Pod 的控制器
- 修改 Deployment 中 Pod 模板的定义，导致 Pod 重启
- 直接删除一个 Pod

**集群管理员**可能执行的操作有：

- [排空节点](https://kubernetes.io/docs/tasks/administer-cluster/safely-drain-node/) 以便维修或升级

- 排空节点，以将集群缩容

- 从节点上删除某个 Pod，以使得其他的 Pod 能够调度到该节点上

### Disruption Budget如何工作

应用程序管理员可以为每一个应用程序创建 `PodDisruptionBudget` 对象（PDB）。PDB限制了多副本应用程序在自愿的毁坏情况发生时，最多有多少个副本可以同时停止。

集群管理员以及托管供应商在进行系统维护时，应该使用兼容 PodDisruptionBudget 的工具（例如 `kubectl drain`，此类工具调用 [Eviction API](https://kubernetes.io/docs/tasks/administer-cluster/safely-drain-node/#the-eviction-api)[ ](https://kubernetes.io/docs/tasks/administer-cluster/safely-drain-node/#the-eviction-api)）而不是直接删除 Pod 或者 Deployment。

- PDB 通过 Pod 的 `.metadata.ownerReferences` 查找到其对应的控制器（Deployment、StatefulSet）
- PDB 通过 控制器（Deployment、StatefulSet）的 `.spec.replicas` 字段来确定期望的副本数
- PDB 通过控制器（Deployment、StatefulSet）的 label selector 来确定哪些 Pod 属于同一个应用程序
- PDB 不能阻止 [非自愿的毁坏](https://kuboard.cn/learning/k8s-intermediate/workload/disruption.html#自愿的和非自愿的毁坏) 发生，但是当这类毁坏发生时，将被计入到当前毁坏数里
- 通过 `kubectl drain` 驱逐 Pod 时，Pod 将被优雅地终止（gracefully terminated，参考 [terminationGracePeriodSeconds](https://kuboard.cn/learning/k8s-intermediate/workload/pod.html#termination-of-pods)）
- 在滚动更新过程中被删除的 Pod 也将计入到 PDB 的当前毁坏数，但是控制器（例如  Deployment、StatefulSet）在执行滚动更新时，并不受 PDB 的约束。**滚动更新过程中，同时可以删除的 Pod  数量在控制器对象（Deployment、StatefulSet等）的定义中规定**。

### 配置PodDisruptionBudget

通常如下几种 Kubernetes 控制器创建的应用程序可以使用 PDB：

- Deployment
- ReplicationController
- ReplicaSet
- StatefulSet

值与其他的控制器或直接创建的 Pod 参考下面的章节：《任意控制器和选择器》

### 思考应用程序如何应对毁坏

- 无状态的前端： 
    - 关注点：不能让服务能力（serving capacity）降低超过 10%
    - 解决方案：在 PDB 中配置 minAvailable 90%
- 单实例有状态应用： 
    - 关注点：未经同意不能关闭此应用程序
    - 解决方案1： 不使用 PDB，并且容忍偶尔的停机
    - 解决方案2： 在 PDB 中设置  maxUnavailable=0。与集群管理员达成一致（不是通过Kubernetes，而是邮件、电话或面对面），请集群管理员在终止应用之前与你沟通。当集群管理员联系你时，准备好停机时间，删除 PDB 以表示已准备好应对毁坏。并做后续处理
- 多实例有状态应用，例如 consul、zookeeper、etcd： 
    - 关注点：不能将实例数降低到某个数值，否则写入会失败
    - 解决方案1： 在 PDB 中设置 maxUnavailable 为 1 （如果副本数会发生变化，可以使用此设置）
    - 解决方案2： 在 PDB 中设置 minAvailable 为最低数量（例如，当总副本数为 5 时，设置为3）（可以同时容忍更多的毁坏数）
- 可以重新开始的批处理任务： 
    - 关注点：当发生自愿毁坏时，Job仍然需要完成其执行任务
    - 解决方案： 不创建 PDB。Job 控制器将会创建一个 Pod 用于替换被毁坏的 Pod

### 定义PodDisruptionBudget

`PodDisruptionBudget` 包含三个字段：

- 标签选择器 `.spec.selector` 用于指定 PDB 适用的 Pod。此字段为必填
- `.spec.minAvailable`：当完成驱逐时，最少仍然要保留多少个 Pod 可用。该字段可以是一个整数，也可以是一个百分比
- `.spec.maxUnavailable`： 当完成驱逐时，最多可以有多少个 Pod 被终止。该字段可以是一个整数，也可以是一个百分比

`minAvailable` 或 `maxUnavailable` 可以指定为整数或者百分比。使用百分比进行计算时 Kubernetes 将向上舍入。

在一个 `PodDisruptionBudget` 中，只能指定 `maxUnavailable` 和 `minAvailable` 中的一个。 `maxUnavailable` 只能应用到那些有控制器的 Pod 上。

推荐使用 `maxUnavailable` 这种形式的定义，因为当控制器的 `spec.replicas` 发生变化时，应用受到的影响更小一些。例如，将其副本数伸缩到 10，如果使用 `minAvailable=2` 这种形式，则可能会有 8 个 Pod 被毁坏。而如果使用 `maxUnavailable=1` 这种形式，应用程序将可以保存 9 个可用实例。

```yml
apiVersion: policy/v1beta1
kind: PodDisruptionBudget
metadata:
  name: nginx-pdb
spec:
  maxUnavailable: 1
  selector:
    matchLabels:
      test: nginx
```

获取更多关于 pdb  的信息：

```bash
kubectl get pdb nginx-pdb -o yaml
```

### 任意控制器和选择器

PDB 可以用于保护其他类型控制器（例如，“operator”）创建的 Pod，或者直接创建的 Pod（bare pod），但是有如下限定：

- 只能使用 `.spec.minAvailable`，不能使用 `.spec.maxUnavailable`
- `.spec.minAvailable` 字段中只能使用整型数字，不能使用百分比



!!! warning
	PDB 的标签选择器之间不能重叠。

