- https://kuboard.cn/learning/k8s-intermediate/persistent/storage-class.html#%E5%AD%98%E5%82%A8%E7%B1%BB%E6%A6%82%E8%BF%B0
- https://kubernetes.io/zh/docs/concepts/storage/storage-classes/

## 概述

StorageClass 存储类用于描述集群中可以提供的存储的类型。不同的存储类可能对应着不同的：

- 服务等级（quality-of-service level）
- 备份策略
- 集群管理员自定义的策略

Kubernetes 本身并不清楚各种类代表什么，由集群管理员自行约定。

## StorageClass 资源

每个 StorageClass 都包含 `provisioner`、`parameters` 和 `reclaimPolicy` 字段， 这些字段会在 StorageClass 需要动态分配 `PersistentVolume` 时会使用到。

StorageClass 对象的命名很重要，用户使用这个命名来请求生成一个特定的类。 当创建 StorageClass 对象时，管理员设置 StorageClass 对象的命名和其他参数，**一旦创建了对象就不能再对其更新**。

```yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: standard
provisioner: kubernetes.io/aws-ebs
parameters:
  type: gp2
reclaimPolicy: Retain
allowVolumeExpansion: true
mountOptions:
  - debug
volumeBindingMode: Immediate
```

### 存储分配器

每个 StorageClass 都有一个分配器（`provisioner`），用来决定使用哪个卷插件分配 PV。该字段必须指定。

- [内部分配器](https://kubernetes.io/zh/docs/concepts/storage/storage-classes/#%E5%AD%98%E5%82%A8%E5%88%86%E9%85%8D%E5%99%A8)，其名称前缀为 "kubernetes.io" 并打包在 Kubernetes 中。
- [外部分配器](https://github.com/kubernetes-sigs/sig-storage-lib-external-provisioner)

### 回收策略

**由 StorageClass 动态创建的 PersistentVolume** 会在类的 `reclaimPolicy` 字段中指定回收策略，可以是 `Delete` 或者 `Retain`。如果 StorageClass 对象被创建时没有指定 `reclaimPolicy`，它将默认为 `Delete`。

通过 StorageClass 手动创建并管理的 PersistentVolume 会使用它们被创建时指定的回收政策。

### 允许卷扩展

PersistentVolume 可以配置为可扩展。将此功能设置为 `true` 时，允许用户通过编辑相应的 PVC 对象来调整卷大小。

当基础存储类的 `allowVolumeExpansion` 字段设置为 true 时，以下类型的卷支持卷扩展。

| 卷类型               | Kubernetes 版本要求       |
| -------------------- | ------------------------- |
| gcePersistentDisk    | 1.11                      |
| awsElasticBlockStore | 1.11                      |
| Cinder               | 1.11                      |
| glusterfs            | 1.11                      |
| rbd                  | 1.11                      |
| Azure File           | 1.11                      |
| Azure Disk           | 1.11                      |
| Portworx             | 1.11                      |
| FlexVolume           | 1.13                      |
| CSI                  | 1.14 (alpha), 1.16 (beta) |

!!! important
	此功能仅可用于扩容卷，不能用于缩小卷。

### 卷绑定模式

`volumeBindingMode` 字段控制了  应该发生在什么时候。

- 即刻绑定 `Immediate`

    存储卷声明创建后，立刻动态创建存储卷并将其绑定到存储卷声明。默认值。

- 首次使用时绑定 `WaitForFirstConsumer`

    直到存储卷声明第一次被容器组使用时，才创建存储卷，并将其绑定到存储卷声明。