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

