https://kuboard.cn/learning/k8s-intermediate/workload/wl-statefulset/#statefulset-%E6%A6%82%E8%BF%B0

StatefulSet 用于管理 Stateful（有状态）的应用程序。

StatefulSet 管理 Pod 时，确保其 Pod 有一个按顺序增长的 ID。

与 Deployment 最大的不同在于 StatefulSet 始终将一系列不变的名字分配给其 Pod。这些 Pod 从同一个模板创建，但是并不能相互替换：每个 Pod 都对应一个特有的持久化存储标识。

## 使用场景

对于有如下要求的应用程序，StatefulSet 非常适用：

- 稳定、唯一的网络标识（dnsname）
- 每个Pod始终对应各自的存储路径（PersistantVolumeClaimTemplate）
- 按顺序地增加副本、减少副本，并在减少副本时执行清理
- 按顺序自动地执行滚动更新

## 限制

- Pod 的存储要么由 storage class 对应的 [PersistentVolume Provisioner](https://github.com/kubernetes/examples/blob/master/staging/persistent-volume-provisioning/README.md) 提供，要么由集群管理员事先创建

- 删除或 scale down 一个 StatefulSet 将不会删除其对应的数据卷。这样做的考虑是数据安全

- 删除 StatefulSet 时，将无法保证 Pod 的终止是正常的。如果要按顺序 gracefully 终止 StatefulSet 中的 Pod，可以在删除 StatefulSet 前将其 scale down 到 0

- 当使用默认的 [Pod Management Policy](https://kuboard.cn/learning/k8s-intermediate/workload/wl-statefulset/update.html) (OrderedReady) 进行滚动更新时，可能进入一个错误状态，并需要[人工介入](https://kuboard.cn/learning/k8s-intermediate/workload/wl-statefulset/update.html)才能修复

## 创建 StatefulSet

```yml
---
# Headless Service
# https://kubernetes.io/docs/concepts/services-networking/service/#headless-services
apiVersion: v1
kind: Service
metadata:
  name: nginx
  labels:
    app: nginx
spec:
  ports:
  - port: 80
    name: web
  clusterIP: None
  selector:
    app: nginx
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: web
spec:
  selector:
    matchLabels:
      app: nginx # has to match .spec.template.metadata.labels
  serviceName: "nginx"
  replicas: 3 # by default is 1
  template:
    metadata:
      labels:
        app: nginx # has to match .spec.selector.matchLabels
    spec:
      terminationGracePeriodSeconds: 10
      containers:
      - name: nginx
        image: nginx:1.7.9
        ports:
        - containerPort: 80
          name: web
        volumeMounts:
        - name: www
          mountPath: /usr/share/nginx/html
  volumeClaimTemplates:
  - metadata:
      name: www
    spec:
      accessModes: [ "ReadWriteOnce" ]
      # 需要自行创建
      storageClassName: "my-storage-class"
      resources:
        requests:
          storage: 1Gi
```

## Pod 的标识

StatefulSet 中的 Pod 具备一个唯一标识，该标识由以下几部分组成：

- 序号
- 稳定的网络标识
- 稳定的存储

该标识始终与 Pod 绑定，无论该 Pod 被调度（重新调度）到哪一个节点上。

###  稳定的网络 ID

- StatefulSet 中 Pod 的 hostname 格式为 $(StatefulSet name)-$(Pod 序号)。上面的例子将要创建三个 Pod，其名称分别为： web-0，web-1，web-2。
- StatefulSet 可以使用 Headless Service 来控制其 Pod 所在的域。该域（domain）的格式为 $(service  name).$(namespace).svc.cluster.local，其中 “cluster.local” 是集群的域（cluster 是集群的名字，也是可以替换的）。
- StatefulSet 中每一个 Pod 将被分配一个 dnsName，格式为： $(podName).$(所在域名)

### 稳定的存储

Kubernetes 为每一个 VolumeClaimTemplate 创建一份 PersistentVolume（存储卷）。在上面的例子中，每一个 Pod 都将由 StorageClass（存储类）`my-storage-class` 为其创建一个 1Gib 大小的 PersistentVolume（存储卷）。当 Pod 被调度（或重新调度）到一个节点上，其挂载点将挂载该存储卷声明（关联到该 PersistentVolume）。

!!! note
	- 当 Pod 或 StatefulSet 被删除时，其关联的 PersistentVolumeClaim（存储卷声明）以及其背后的 PersistentVolume（存储卷）仍然存在。
	- 如果相同的 Pod 或 StatefulSet 被再次创建，则，新建的名为 web-0 的 Pod 仍将挂载到原来名为 web-0 的 Pod 所挂载的存储卷声明及存储卷。
    - 这确保了 web-0、web-1、web-2 等，不管被删除重建多少次，都将 “稳定” 的使用各自所对应的存储内容

## 部署和伸缩

### 执行顺序

- 在创建一个副本数为 N 的 StatefulSet 时，其 Pod 将被按 {0 ... N-1} 的顺序逐个创建
- 在删除一个副本数为 N 的 StatefulSet （或其中所有的 Pod）时，其 Pod 将按照相反的顺序（即 {N-1 ... 0}）终止和删除
- 在对 StatefulSet 执行扩容（scale up）操作时，新增 Pod 所有的前序 Pod 必须处于 Running（运行）和 Ready（就绪）的状态
- 终止和删除 StatefulSet 中的某一个 Pod 时，该 Pod 所有的后序 Pod 必须全部已终止

StatefulSet 中 `pod.spec.terminationGracePeriodSeconds` 不能为 0。

### Pod 管理策略

在 Kubernetes 1.7 及其后续版本中，可以为 StatefulSet 设定 `.spec.podManagementPolicy` 字段，以便您可以继续使用 StatefulSet 唯一 ID 的特性，但禁用其有序创建和销毁 Pod 的特性。该字段的取值如下：

- OrderedReady：默认值
- Parallel：StatefulSet Controller 将同时并行地创建或终止其所有的 Pod。



!!! important
	此选项只影响到伸缩（scale up/scale down）操作。更新操作不受影响。

## 更新策略

在 Kubernetes 1.7 及之后的版本中，可以为 StatefulSet 设定 `.spec.updateStrategy` 字段，以便您可以在改变 StatefulSet 中 Pod 的某些字段时（container/labels/resource request/resource limit/annotation等）禁用滚动更新。

### On Delete

`.spec.updateStrategy.type=OnDelte`。当修改 `.spec.template` 的内容时，StatefulSet Controller 将不会自动更新其 Pod。必须手工删除 Pod，此时 StatefulSet Controller 在重新创建 Pod 时，使用修改过的 `.spec.template` 的内容创建新 Pod。

### Rolling Updates

`.spec.updateStrategy.type=RollingUpdate`。默认值。

#### Partitions

通过指定 `.spec.updateStrategy.rollingUpdate.partition` 字段，可以分片（partitioned）执行RollingUpdate 更新策略。当更新 StatefulSet 的 `.spec.template` 时：

- 序号大于或等于 `.spec.updateStrategy.rollingUpdate.partition` 的 Pod 将被删除重建

- 序号小于 `.spec.updateStrategy.rollingUpdate.partition` 的 Pod 将不会更新，及时手工删除该 Pod，kubernetes 也会使用前一个版本的 `.spec.template` 重建该 Pod

#### Forced Rollback

当使用默认的 Pod 管理策略时（OrderedReady），很有可能会进入到一种卡住的状态，需要人工干预才能修复。

如果您更新 Pod template 后，该 Pod 始终不能进入 Running 和 Ready 的状态（例如，镜像错误或应用程序配置错误），StatefulSet 将停止滚动更新并一直等待。

此时，如果您仅仅将 Pod template 回退到一个正确的配置仍然是不够的。在修复 Pod template 以后，您还必须删除掉所有已经尝试使用有问题的 Pod template 的 Pod。StatefulSet此时才会开始使用修复了的 Pod template 重建 Pod。