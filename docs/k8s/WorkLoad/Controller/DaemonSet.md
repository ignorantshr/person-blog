https://kuboard.cn/learning/k8s-intermediate/workload/wl-daemonset/

DaemonSet 控制器确保所有（或一部分）的节点都运行了一个指定的 Pod 副本。

- 每当向集群中添加一个节点时，指定的 Pod 副本也将添加到该节点上
- 当节点从集群中移除时，Pod 也就被垃圾回收了
- 删除一个 DaemonSet 可以清理所有由其创建的 Pod

DaemonSet 的典型使用场景有：

- 在每个节点上运行集群的存储守护进程，例如 glusterd、ceph
- 在每个节点上运行日志收集守护进程，例如 fluentd、logstash
- 在每个节点上运行监控守护进程

## 创建

```yml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: fluentd-elasticsearch
  namespace: kube-system
  labels:
    k8s-app: fluentd-logging
spec:
  selector:
    matchLabels:
      name: fluentd-elasticsearch
  template:
    metadata:
      labels:
        name: fluentd-elasticsearch
    spec:
      tolerations:
      # this toleration is to have the daemonset runnable on master nodes
      # remove it if your masters can't run pods
      - key: node-role.kubernetes.io/master
        effect: NoSchedule
      containers:
      - name: fluentd-elasticsearch
        image: fluent/fluentd-kubernetes-daemonset:v1.7.1-debian-syslog-1.0
        resources:
          limits:
            memory: 200Mi
          requests:
            cpu: 100m
            memory: 200Mi
        volumeMounts:
        - name: varlog
          mountPath: /var/log
        - name: varlibdockercontainers
          mountPath: /var/lib/docker/containers
          readOnly: true
      terminationGracePeriodSeconds: 30
      volumes:
      - name: varlog
        hostPath:
          path: /var/log
      - name: varlibdockercontainers
        hostPath:
          path: /var/lib/docker/containers
```

### Pod Template

在 DaemonSet 中，您必须指定 `.spec.template.metadata.labels` 字段和 `.spec.tempalte.spec` 字段。

DaemonSet 的 `.spec.template.spec.restartPolicy` 字段必须为 Always，或者不填（默认值为 Always）

### Pod Selector

自 Kubernets v1.8 以后，`.spec.selector` 是必填字段，且您指定该字段时，必须与 `.spec.template.metata.labels` 字段匹配（不匹配的情况下创建 DaemonSet 将失败）。DaemonSet 创建以后，`.spec.selector` 字段就**不可再修改**。如果修改，可能导致不可预见的结果。

`.spec.selector` 由两个字段组成：

- matchLabels；
- matchExpressions： 通过指定 key、value列表以及运算符，可以构造更复杂的选择器

如果两个字段同时存在，则必须同时满足两个条件的 Pod 才被选中。

!!! warning
	任何情况下，您不能以任何方式创建符合 DaemonSet 的 `.spec.selector` 选择器的 Pod。否则 DaemonSet Controller 会认为这些 Pod 是由它创建的。这将导致不可预期的行为出现。实际上，**不推荐任何单独创建的 pod 的方式**。

### 只在部分节点上运行

指定 `.spec.template.spec.nodeSelector` ，DaemonSet Controller 将只在指定的节点上创建 Pod （参考 [节点选择器 nodeSelector](https://kuboard.cn/learning/k8s-intermediate/config/assign-pod-node.html#节点选择器-nodeselector)）。同样的，如果指定 `.spec.template.spec.affinity` ，DaemonSet Controller 将只在与 [node affinity](https://kubernetes.io/docs/concepts/configuration/assign-pod-node/)[ ](https://kubernetes.io/docs/concepts/configuration/assign-pod-node/) 匹配的节点上创建 Pod。

## 污点和容忍

在调度 DaemonSet 的 Pod 时，污点和容忍（[taints and tolerations](https://kuboard.cn/learning/k8s-intermediate/config/taints-toleration/)）会被考量到，同时，以下容忍（toleration）将被**自动添加**到 DaemonSet 的 Pod 中：

| Toleration Key                         | Effect     | Version | 描述                                                         |
| -------------------------------------- | ---------- | ------- | ------------------------------------------------------------ |
| node.kubernetes.io/not-ready           | NoExecute  | 1.13+   | 节点出现问题时（例如网络故障），DaemonSet 容器组将不会从节点上驱逐 |
| node.kubernetes.io/unreachable         | NoExecute  | 1.13+   | 节点出现问题时（例如网络故障），DaemonSet 容器组将不会从节点上驱逐 |
| node.kubernetes.io/disk-pressure       | NoSchedule | 1.8+    |                                                              |
| node.kubernetes.io/memory-pressure     | NoSchedule | 1.8+    |                                                              |
| node.kubernetes.io/unschedulable       | NoSchedule | 1.12+   | 默认调度器针对 DaemonSet 容器组，容忍节点的 `unschedulable`属性 |
| node.kubernetes.io/network-unavailable | NoSchedule | 1.12+   | 默认调度器针对 DaemonSet 容器组，在其使用 host network 时，容忍节点的 `network-unavailable` 属性 |

## 与 DaemonSet 通信

与 DaemonSet 容器组通信的模式有：

- **Push：** DaemonSet 容器组用来向另一个服务推送信息，例如数据库的统计信息。这种情况下 DaemonSet 容器组没有客户端
- **NodeIP + Port：** DaemonSet 容器组可以使用 `hostPort`，此时可通过节点的 IP 地址直接访问该容器组。客户端需要知道节点的 IP 地址，以及 DaemonSet 容器组的 端口号
- **DNS：** 创建一个 [headless service](https://kubernetes.io/docs/concepts/services-networking/service/#headless-services)，且该 Service 与 DaemonSet 有相同的 Pod Selector。此时，客户端可通过该 Service 的 DNS 解析到 DaemonSet 的 IP 地址

- **Service：** 创建一个 Service，且该 Service 与 DaemonSet 有相同的 Pod Selector，客户端通过该 Service，可随机访问到某一个节点上的 DaemonSet 容器组

## 更新

### 更新信息

- 在改变节点的标签时：
    - 如果该节点匹配了 DaemonSet 的 `.spec.template.spec.nodeSelector`，DaemonSet 将会在该节点上创建一个 Pod
    - 如果该节点原来匹配 DaemonSet 的 `.spec.template.spec.nodeSelector`，现在不匹配了，则，DaemonSet 将会删除该节点上对应的 Pod
- 您可以修改 DaemonSet 的 Pod 的部分字段，但是，DaemonSet 控制器在创建新的 Pod 时，仍然会使用原有的 Template 进行 Pod 创建。滚动更新不受此影响。
- 您可以删除 DaemonSet。如果在 `kubectl` 命令中指定 `--cascade=false` 选项，DaemonSet 容器组将不会被删除。紧接着，如果您创建一个新的 DaemonSet，与之前删除的 DaemonSet 有相同的 `.spec.selector`，新建 DaemonSet 将直接把这些未删除的 Pod 纳入管理。DaemonSet 根据其 `updateStrategy` 决定是否更新这些 Pod

### 滚动更新

https://kubernetes.io/docs/tasks/manage-daemon/update-daemon-set/

支持版本：Kubernetes version >=1.6。

#### 更新策略

- **OnDelete**：若你使用 `OnDelete` 更新策略，在你更新 template 之后，只有你手动删除旧的 DaemonSet pods 之后才会创建新的节点。这与 Kubernetes version <=1.5 的版本行为一致。
- **RollingUpdate**：默认值。若你使用 `RollingUpdate` 更新策略，旧的 DaemonSet pods 被杀掉，自动创建新的 DaemonSet pods。在整个更新过程中每个 node 最多只有一个 DaemonSet pod 会运行。

## 故障排除

DaemonSet 在执行滚动更新的时候可能会卡住，下面是一些可能的原因。

### 一些节点资源不足

因为新的 DaemonSet pods 不能在至少一个节点上进行调度，所以部署被卡住了。

使用下面命令的输出与 `kubectl get nodes` 做对比，找到那些还未被调度的节点：

```
kubectl get pods -l name=fluentd-elasticsearch -o wide -n kube-system
```

然后你就需要释放一些资源了。

### 损坏的部署

若最近地 DaemonSet template 是错误的（例如，镜像不存在）， DaemonSet 部署不会执行。

只需要再次更新  DaemonSet template 就好了，新部署不会被旧的卡住。

### 时钟偏差

若在 DaemonSet 中指定了 `.spec.minReadySeconds` ，主节点和节点之间的时钟偏差会使 DaemonSet 无法检测到正确的部署进度。

## Deployment

DaemonSet 和 Deployment 一样，他们都创建长时间运行的 Pod（例如 web server、storage server 等）

- Deployment 适用于无状态服务（例如前端程序），对于这些程序而言，扩容（scale up）/ 缩容（scale down）、滚动更新等特性比精确控制 Pod 所运行的节点更重要。
- DaemonSet 更适合如下情况：
    - Pod 的副本总是在所有（或者部分指定的）节点上运行
    - 需要在其他 Pod 启动之前运行

