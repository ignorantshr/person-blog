- https://kuboard.cn/learning/k8s-intermediate/persistent/pv.html
- https://kubernetes.io/docs/concepts/storage/persistent-volumes/

## 概述

与管理计算资源相比，管理存储资源是一个完全不同的问题。为了更好的管理存储，Kubernetes 引入了 PersistentVolume 和 PersistentVolumeClaim 两个新的 API 资源，将存储管理抽象成两个关注点：

- 如何提供存储
- 如何使用存储

*PersistentVolume*（PV，持久卷）是集群中的一块存储空间，由集群管理员管理、或者由 [Storage Classes](https://kubernetes.io/docs/concepts/storage/storage-classes/)（存储类）自动管理。PV 和 node 一样，是集群中的资源（kubernetes 集群由存储资源和计算资源组成）。PVs 就像 Volumes 一样是 volume 的插件，但是有自己的生命周期，并且独立于任何使用它的 Pod。这个 API 对象描述了如何提供存储的细节信息（NFS、cephfs等存储的具体参数）。

PersistentVolumeClaim（PVC，持久卷声明）代表用户使用存储的请求。它与 Pod 类似，Pods 消耗 node 计算资源，PVCs 消耗 PV 存储资源。声明可以请求特定的大小和访问模式（例如，它们可以被挂载一次读/写或多次只读）。

根据应用程序的特点不同，其所需要的存储资源也存在不同的要求，例如读写性能等。集群管理员必须能够提供关于  PersistentVolume 的更多选择，无需用户关心存储卷背后的实现细节。为了这些需求，Kubernetes 引入了 *StorageClass*（存储类）资源。

## 卷和声明的生命周期

### 提供方式

#### Static

集群管理员创建许多 PV 。它们提供了可供集群中应用程序使用的关于实际存储的具体信息。它们存在于 Kubernetes API 中，可供使用。

![Kubernetes教程：存储卷/存储卷声明_静态提供存储卷](https://kuboard.cn/assets/img/image-20191016151323906.f89d2436.png)

#### Dynamic

当管理员创建的所有静态PV均与用户的 PersistentVolumeClaim 不匹配时，群集可能会尝试动态地为 PVC 专门配置一个卷。PVC 必须请求一个 StorageClass，并且管理员必须已经创建并配置了该类，才能进行动态预配置。

![Kubernetes教程：存储卷/存储卷声明_动态提供存储卷](https://kuboard.cn/assets/img/image-20191016151308410.375b744e.png)

### Binding

假设用户创建了一个 PersistentVolumeClaim 存储卷声明，并指定了需求的存储空间大小以及访问模式。Kubernets master 将立刻为其匹配一个 PersistentVolume 存储卷，并将存储卷声明和存储卷绑定到一起。主控制器中的一个控制循环监视新的 PVCs，若找到一个匹配的 PV (如果可能的话) ，就将它们绑定在一起。如果一个 PV 被动态地提供给一个新的 PVC，那么循环将始终将 PV 绑定到 PVC。否则，用户将总是至少得到他们所要求的容量，但是容量可能超过所要求的内容。一旦绑定，PersistentVolumeClaim 将拒绝其他 PersistentVolume 的绑定关系。PVC 与 PV 之间的绑定关系是一对一的映射。

如果不存在匹配的 PV，PVC 将无限期保持未绑定。

### Using

Pods 使用 PVC 作为 volumes。在 Pod yaml 中的 `volumes` 部分导入 `persistentVolumeClaim` 部分即可。[详见下文](#claims-as-volumes)。

### 使用中的存储对象保护

*Storage Object in Use Protection* 是为了确保正在被 Pod 使用的 PVCs 和 绑定到 PVCs 的 PVs 不会被删除，以避免可能的数据丢失。

若用户删除了正在被 Pod 使用的 PVC，PVC 不会被立即移除。只有在任何 Pod 都不再使用该 PVC 时，它才会被移除。同理，管理员删除 PV 时，PV 也会在不再绑定到 PVC 时再被移除。

### 回收（Reclaiming）

当用户使用完 volume，他们可以从 API 删除 PVC 对象，允许资源的回收。在 PV 被声明它已经被释放时，PV 的回收策略会告知集群下一步的做法。

#### 保留

`Retain` 回收策略允许手动回收资源。PVC 被删除时，PV 仍然存在，并且 volume 被认为 "released"。但是由于原先的数据还在上面，所以对于其他 PVC 来说还是不可用的。管理员可以按照下面的步骤回收 volume：

- 删除 PV。其数据仍然存在于对应的外部存储介质中（nfs、cefpfs、glusterfs 等）
- 手工删除对应存储介质上的数据
- 手工删除对应的存储介质，您也可以创建一个新的 PersistentVolume 并再次使用该存储介质

#### 删除

 `Delete` 将从 kubernete 集群移除 PersistentVolume 以及其关联的外部存储介质。

#### 回收

如果受底层 volume 插件支持， `Recycle` 回收策略将对卷执行基本清理（`rm-rf/thevolume/*`），并使其再次可用于新申请。

!!! warning
	不推荐使用 `Recycle` 策略，建议使用动态配置，即 storageclass。

### 扩展 PVCs

你可以扩展以下类型的 volume：

- gcePersistentDisk
- awsElasticBlockStore
- Cinder
- glusterfs
- rbd
- Azure File
- Azure Disk
- Portworx
- FlexVolumes
- CSI

要使用此功能，需要在 storage class 中设置 `allowVolumeExpansion` 字段。例：

```yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: gluster-vol-default
provisioner: kubernetes.io/glusterfs
parameters:
  resturl: "http://192.168.10.100:8080"
  restuser: ""
  secretNamespace: ""
  secretName: ""
# 在这里设置支持扩展
allowVolumeExpansion: true
```

要为 PVC 请求更大的 volume ，请编辑 PVC 对象并指定更大的体积。永远不会创建新的 PersistentVolume 来满足声明，而是调整现有卷的大小。

#### 调整包含文件系统的 volume 的大小

如果文件系统是 XFS、 Ext3 或 Ext4，则只能调整包含文件系统的卷的大小。

当卷包含文件系统时，只有当新 Pod 在 ReadWrite 模式下使用 PersistentVolumeClaim 时，才会调整文件系统的大小。 文件系统扩展可以在 Pod 启动时进行，也可以在 Pod 运行且底层文件系统支持在线扩展时进行。

如果驱动程序将 RequiresFSResize 功能设置为 true，那么 [FlexVolume](https://kubernetes.io/zh/docs/concepts/storage/volumes/#flexVolume) 允许调整大小。 FlexVolume 可以在 Pod 重新启动时调整大小。

#### 调整正在使用的 PVC 大小

在这种情况下，您不需要删除并重新创建使用现有 PVC 的 Pod 或部署。 文件系统扩展后，所有使用中的PVC都将自动供其 Pod 使用。 此功能对 Pod 或 deployment 中未使用的 PVC 无效。 您必须创建一个使用 PVC 的 Pod，然后才能完成扩展。

类似于其他卷类型——在使用的 Pod 时 FlexVolume 卷也可以扩展。

!!! note
	只有当底层驱动程序支持调整大小时，FlexVolume 才可能调整大小。

## PV 类型

Persistentvolume 类型以插件的形式实现。k8s 当前支持以下插件：

- GCEPersistentDisk
- AWSElasticBlockStore
- AzureFile
- AzureDisk
- CSI
- FC (Fibre Channel)
- FlexVolume
- Flocker
- NFS
- iSCSI
- RBD (Ceph Block Device)
- CephFS
- Cinder (OpenStack block storage)
- Glusterfs
- VsphereVolume
- Quobyte Volumes
- HostPath (Single node testing only -- local storage is not supported in any way and WILL NOT WORK in a multi-node cluster)
- Portworx Volumes
- ScaleIO Volumes
- StorageOS

## PVs

PV 的名字符合 `DNS 子域名` 命令规则。

```yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: pv0003
spec:
  capacity:
    storage: 5Gi
  volumeMode: Filesystem
  accessModes:
    - ReadWriteOnce
  persistentVolumeReclaimPolicy: Recycle
  storageClassName: slow
  mountOptions:
    - hard
    - nfsvers=4.1
  nfs:
    path: /tmp
    server: 172.17.0.2
```

!!! note
	在群集中使用 PersistentVolume 可能需要与卷类型有关的帮助程序。在本例中就需要 `/sbin/mount.nfs`。

| 字段名称                | 可选项/备注                                                  |
| ----------------------- | ------------------------------------------------------------ |
| 容量 Capacity           | 通常，一个 PersistentVolume 具有一个固定的存储容量（capacity） |
| Volume Mode             | Kubernetes 1.9 之前的版本，所有的存储卷都被初始化一个文件系统。当前可选项有：<br/>Block：使用一个 块设备（raw block device）；<br/>Filesystem（默认值）：使用一个文件系统 |
| Access Modes            | 可被单节点读写（ReadWriteOnce ）；<br/>可被多节点只读（ReadOnlyMany）； <br/>可被多节点读写（ReadWriteMany） |
| 存储类 StorageClassName | 带有存储类 StorageClassName 属性的 PersistentVolume 只能绑定到请求该 StorageClass 存储类的 PersistentVolumeClaim。<br/> 没有 StorageClassName 属性的 PersistentVolume 只能绑定到无特定 StorageClass 存储类要求的 PVC。 |
| 回收策略 Reclaim Policy | 保留（Retain） – 手工回收；<br/>再利用（Recycle）– 清除后重新可用 (rm -rf /thevolume/*) ；<br/>删除（Delete） – 删除 PV 及存储介质 |
| Mount Options           | 挂载选项用来在挂载时作为 mount 命令的参数，不会对选项进行验证 |
| 状态 Phase              | Available – 可用的 PV，尚未绑定到 PVC；<br/>Bound – 已经绑定到 PVC；<br/>Released – PVC 已经被删除，但是资源还未被集群回收；<br/>Failed – 自动回收失败 |

!!! important
	一个卷一次只能使用一种访问模式挂载，即使它支持多种访问模式。

并非所有的卷插件都支持所有的访问模式，详情请参考：https://kubernetes.io/docs/concepts/storage/persistent-volumes/#access-modes。

## PVCs

PVC 的名字符合 `DNS 子域名` 命令规则。

```yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: myclaim
spec:
  accessModes:
    - ReadWriteOnce
  volumeMode: Filesystem
  resources:
    requests:
      storage: 8Gi
  storageClassName: slow
  selector:
    matchLabels:
      release: "stable"
    matchExpressions:
      - {key: environment, operator: In, values: [dev]}
```

| 字段名称              | 可选项/备注                                                  |
| --------------------- | ------------------------------------------------------------ |
| 存储类                | 只有该 StorageClass 存储类的 PV 才可以绑定到此 PVC           |
| 读写模式 Access Modes | 可被单节点读写（ReadWriteOnce ）；<br/>可被多节点只读（ReadOnlyMany）； <br/>可被多节点读写（ReadWriteMany） |
| Volume Modes          | blockfilesystem - default                                    |
| 总量                  | 请求存储空间的大小                                           |

### [Selector](https://kubernetes.io/docs/concepts/overview/working-with-objects/labels/#label-selectors)

matchLabel 和 matchExpressions 的所有要求都进行`与`运算——即必须完全满足才能匹配。

### Class

集群对没有 storageClassName 的 PVC 的处理具体取决于是否打开了  [`DefaultStorageClass` admission plugin](https://kubernetes.io/docs/reference/access-authn-authz/admission-controllers/#defaultstorageclass) 插件。

- 如果许可插件已打开，管理员可以指定默认的 StorageClass。 所有没有 storageClassName 的 PVC 只能绑定到该默认的PV。指定默认 StorageClass 的方法是将 StorageClass 对象中的注释`storageclass.kubernetes.io/is-default-class` 设置为 `true`。 如果管理员未指定默认值，群集将响应PVC 创建，就像准入插件已关闭一样。 如果指定了多个默认值，准入插件将禁止创建所有PVC。
- 如果准入插件已关闭，则没有默认 StorageClass 的概念。 所有没有 storageClassName 的 PVC 只能绑定到没有 class 的 PV。  在这种情况下，不具有 storageClassName 的 PVC 与将其 storageClassName 设置为 `""` 的PVC的处理方式相同（即没有 StorageClassName 属性的 PersistentVolume 只能绑定到无特定 StorageClass 存储类要求的 PVC。）。

## Claims As Volumes

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: mypod
spec:
  containers:
    - name: myfrontend
      image: nginx
      volumeMounts:
      - mountPath: "/var/www/html"
        name: mypd
  volumes:
    - name: mypd
      persistentVolumeClaim:
        claimName: myclaim
```

!!! note
	因为 PVC 是 namespace 对象，所以只能在一个名称空间中使用“多”模式（ROX，RWX）挂载声明。

## 编写合适的配置

如果您正在编写在大范围集群上运行的配置模板或示例，并且需要持久存储，建议您使用以下模式:

- 将 PersistentVolumeClaim 对象包含在您的配置包中（与Deployment，ConfigMap等一起）。
- 不要在配置中包含 PersistentVolume 对象，因为实例化配置的用户可能没有创建 PersistentVolume 的权限。
- 在实例化模板时，为用户提供 storage class 名称的选项。
    - 如果用户提供了 storage class 名称，则将该值放入 `persistentVolumeClaim.storageClassName`
    - 否则置空
- 在您的工具中，请注意在一段时间之后没有绑定的 PVC，并提示给用户。因为这可能表明集群没有动态存储支持（在这种情况下，用户应该创建一个匹配的 PV）或者集群没有存储系统（在这种情况下，用户不能部署需要求的 PVC 配置）。