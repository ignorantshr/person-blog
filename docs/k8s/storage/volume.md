- https://kuboard.cn/learning/k8s-intermediate/persistent/volume.html#%E6%95%B0%E6%8D%AE%E5%8D%B7%E6%A6%82%E8%BF%B0

- https://kubernetes.io/zh/docs/concepts/storage/volumes/

## 概述

容器中的文件在磁盘上是临时存放的，这给容器中运行的特殊应用程序带来一些问题：

- 首先，当容器崩溃时，kubelet 将重新启动容器，容器中的文件将会丢失——因为容器会以干净的状态重建。
- 其次，当在一个 `Pod` 中同时运行多个容器时，常常需要在这些容器之间共享文件。

Kubernetes 卷具有明确的生命周期——与包裹它的 Pod 相同。 因此，卷比 Pod 中运行的任何容器的存活期都长，在容器重新启动时数据也会得到保留。 当然，当一个 Pod 不再存在时，卷也将不再存在。根据 Pod 所使用的 Volume（数据卷）类型不同，数据可能随数据卷的退出而删除，也可能被真正持久化，并在下次 Pod 重启时仍然可以使用。

使用卷时, Pod 声明中需要提供卷的类型 (`.spec.volumes` 字段)和卷挂载的位置 (`.spec.containers.volumeMounts` 字段).

Docker  镜像将被首先加载到该容器的文件系统，任何数据卷都被在此之后挂载到指定的路径上。Volume（数据卷）不能被挂载到其他数据卷上，或者通过引用其他数据卷。同一个容器组中的不同容器各自独立地挂载数据卷，即同一个容器组中的两个容器可以将同一个数据卷挂载到各自不同的路径上。

我们现在通过下图来理解 容器组、容器、挂载点、数据卷、存储介质（nfs、PVC、ConfigMap）等几个概念之间的关系：

- 一个容器组可以包含多个数据卷、多个容器
- 一个容器通过挂载点决定某一个数据卷被挂载到容器中的什么路径
- 不同类型的数据卷对应不同的存储介质（图中列出了 nfs、PVC、ConfigMap 三种存储介质，接下来将介绍更多）

![Kubernetes教程：数据卷](https://kuboard.cn/assets/img/image-20190904201849792.70b324a5.png)

## Volume 的类型

Kubernetes 支持下列类型的卷：

- [awsElasticBlockStore](https://kubernetes.io/zh/docs/concepts/storage/volumes/#awselasticblockstore)
- [azureDisk](https://kubernetes.io/zh/docs/concepts/storage/volumes/#azuredisk)
- [azureFile](https://kubernetes.io/zh/docs/concepts/storage/volumes/#azurefile)
- [cephfs](https://kubernetes.io/zh/docs/concepts/storage/volumes/#cephfs)
- [cinder](https://kubernetes.io/zh/docs/concepts/storage/volumes/#cinder)
- [configMap](https://kubernetes.io/zh/docs/concepts/storage/volumes/#configmap)
- [csi](https://kubernetes.io/zh/docs/concepts/storage/volumes/#csi)
- [downwardAPI](https://kubernetes.io/zh/docs/concepts/storage/volumes/#downwardapi)
- [emptyDir](https://kubernetes.io/zh/docs/concepts/storage/volumes/#emptydir)
- [fc (fibre channel)](https://kubernetes.io/zh/docs/concepts/storage/volumes/#fc)
- [flexVolume](https://kubernetes.io/zh/docs/concepts/storage/volumes/#flexVolume)
- [flocker](https://kubernetes.io/zh/docs/concepts/storage/volumes/#flocker)
- [gcePersistentDisk](https://kubernetes.io/zh/docs/concepts/storage/volumes/#gcepersistentdisk)
- [gitRepo (deprecated)](https://kubernetes.io/zh/docs/concepts/storage/volumes/#gitrepo)
- [glusterfs](https://kubernetes.io/zh/docs/concepts/storage/volumes/#glusterfs)
- [hostPath](https://kubernetes.io/zh/docs/concepts/storage/volumes/#hostpath)
- [iscsi](https://kubernetes.io/zh/docs/concepts/storage/volumes/#iscsi)
- [local](https://kubernetes.io/zh/docs/concepts/storage/volumes/#local)
- [nfs](https://kubernetes.io/zh/docs/concepts/storage/volumes/#nfs)
- [persistentVolumeClaim](https://kubernetes.io/zh/docs/concepts/storage/volumes/#persistentvolumeclaim)
- [projected](https://kubernetes.io/zh/docs/concepts/storage/volumes/#projected)
- [portworxVolume](https://kubernetes.io/zh/docs/concepts/storage/volumes/#portworxvolume)
- [quobyte](https://kubernetes.io/zh/docs/concepts/storage/volumes/#quobyte)
- [rbd](https://kubernetes.io/zh/docs/concepts/storage/volumes/#rbd)
- [scaleIO](https://kubernetes.io/zh/docs/concepts/storage/volumes/#scaleio)
- [secret](https://kubernetes.io/zh/docs/concepts/storage/volumes/#secret)
- [storageos](https://kubernetes.io/zh/docs/concepts/storage/volumes/#storageos)
- [vsphereVolume](https://kubernetes.io/zh/docs/concepts/storage/volumes/#vspherevolume)

### emptyDir

当 Pod 指定到某个节点上时，首先创建的是一个 `emptyDir` 卷，并且只要 Pod 在该节点上运行，卷就一直存在。 就像它的名称表示的那样，卷最初是空的。 尽管 Pod 中的容器挂载 `emptyDir` 卷的路径可能相同也可能不同，但是这些容器都可以读写 `emptyDir` 卷中相同的文件。 当 Pod 因为某些原因被从节点上删除时，`emptyDir` 卷中的数据也会永久删除。

`emptyDir` 的一些用途：

- 缓存空间，例如基于磁盘的归并排序。
- 为耗时较长的计算任务提供检查点，以便任务能方便地从崩溃前状态恢复执行。
- 在 Web 服务器容器服务数据时，保存内容管理器容器获取的文件。

默认情况下， `emptyDir` 卷存储在支持该节点所使用的介质上；这里的介质可以是磁盘或 SSD 或网络存储，这取决于您的环境。 但是，您可以将 `emptyDir.medium` 字段设置为 `"Memory"`，以告诉 Kubernetes 为您安装 tmpfs（基于 RAM 的文件系统）。 虽然 tmpfs 速度非常快，但是要注意它与磁盘不同。 tmpfs 在节点重启时会被清除，并且您所写入的所有文件都会计入容器的内存消耗，受容器内存限制约束。

#### Pod 示例

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: test-pd
spec:
  containers:
  - image: k8s.gcr.io/test-webserver
    name: test-container
    volumeMounts:
    - mountPath: /cache
      name: cache-volume
  volumes:
  - name: cache-volume
    emptyDir: {}
```

### nfs

`nfs` 卷能将 NFS (网络文件系统) 挂载到您的 Pod 中。 不像  那样会在删除 Pod 的同时也会被删除， 卷的内容在删除 Pod 时会被保存，卷只是被卸载掉了。 这意味着  卷可以被预先填充数据，并且这些数据可以在 Pod 之间"传递"。参考 [NFS 示例](https://github.com/kubernetes/examples/tree/master/staging/volumes/nfs)。

### hostPath

`hostPath` 卷能将主机节点文件系统上的文件或目录挂载到您的 Pod 中。 虽然这不是大多数 Pod 需要的，但是它为一些应用程序提供了强大的逃生舱。

例如，`hostPath` 的一些用法有：

- 运行一个需要访问 Docker 引擎内部机制的容器；请使用 `hostPath` 挂载 `/var/lib/docker` 路径。
- 在容器中运行 cAdvisor 时，以 `hostPath` 方式挂载 `/sys`。
- 允许 Pod 指定给定的 `hostPath` 在运行 Pod 之前是否应该存在，是否应该创建以及应该以什么方式存在。

除了必需的 `path` 属性之外，用户可以选择性地为 `hostPath` 卷指定 `type`。

支持的 `type` 值如下：

| 取值                | 行为                                                         |
| ------------------- | ------------------------------------------------------------ |
|                     | 空字符串（默认）用于向后兼容，这意味着在安装 hostPath 卷之前不会执行任何检查。 |
| `DirectoryOrCreate` | 如果在给定路径上什么都不存在，那么将根据需要创建空目录，权限设置为 0755，具有与 Kubelet 相同的组和所有权。 |
| `Directory`         | 在给定路径上必须存在的目录。                                 |
| `FileOrCreate`      | 如果在给定路径上什么都不存在，那么将在那里根据需要创建空文件，权限设置为 0644，具有与 Kubelet 相同的组和所有权。 |
| `File`              | 在给定路径上必须存在的文件。                                 |
| `Socket`            | 在给定路径上必须存在的 UNIX 套接字。                         |
| `CharDevice`        | 在给定路径上必须存在的字符设备。                             |
| `BlockDevice`       | 在给定路径上必须存在的块设备。                               |

!!! warning
	- 具有相同配置（例如从 podTemplate 创建）的多个 Pod 会由于节点上文件的不同而在不同节点上有不同的行为。
	- 当 Kubernetes  计划增加基于资源的调度，但这个特性将不会考虑对 hostPath 的支持。
	- 基础主机上创建的文件或目录只能由 root 用户写入。您需要在 [特权容器](https://kubernetes.io/docs/user-guide/security-context) 中以 root 身份运行进程，或者修改主机上的文件权限以便容器能够写入 `hostPath` 卷。



!!! warning
	应当注意,`FileOrCreate` 类型不会负责创建文件的父目录。如果挂载挂载文件的父目录不存在，pod 启动会失败。为了确保这种 `type` 能够工作，可以尝试把文件和它对应的目录分开挂载，比如下面的示例：

#### FileOrCreate pod 示例

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: test-webserver
spec:
  containers:
  - name: test-webserver
    image: k8s.gcr.io/test-webserver:latest
    volumeMounts:
    - mountPath: /var/local/aaa
      name: mydir
    - mountPath: /var/local/aaa/1.txt
      name: myfile
  volumes:
  - name: mydir
    hostPath:
      # 确保文件所在目录成功创建。
      path: /var/local/aaa
      type: DirectoryOrCreate
  - name: myfile
    hostPath:
      path: /var/local/aaa/1.txt
      type: FileOrCreate
```

### persistentVolumeClaim

persistentVolumeClaim 数据卷用来挂载 PersistentVolume 存储卷。PersistentVolume 存储卷为用户提供了一种在无需关心具体所在云环境的情况下”声明“ 所需持久化存储的方式。

请参考 [存储卷](https://kuboard.cn/learning/k8s-intermediate/persistent/pv.html)

### configMap

[`configMap`](https://kubernetes.io/docs/tasks/configure-pod-container/configure-pod-configmap/) 资源提供了向 Pod 注入配置数据的方法。 对象中存储的数据可以被  类型的卷引用，然后被应用到 Pod 中运行的容器化应用。

在数据卷中引用 ConfigMap 时：

- 您可以直接引用整个 ConfigMap 到数据卷，此时 ConfigMap 中的每一个 key 对应一个文件名，value 对应该文件的内容
- 您也可以只引用 ConfigMap 中的某一个名值对，此时可以将 key 映射成一个新的文件名

具体使用方法请参考 [使用 ConfigMap 配置您的应用程序](https://kuboard.cn/learning/k8s-intermediate/config/config-map.html#configmap-数据卷)。

!!! note
	 容器以 [subPath](#subpath) 卷挂载方式使用 ConfigMap 时，将无法接收 ConfigMap 的更新。

### secret

secret 数据卷可以用来注入敏感信息（例如密码）到容器组。您可以将敏感信息存入 kubernetes secret 对象，并通过  Volume（数据卷）以文件的形式挂载到容器组（或容器）。secret 数据卷使用 tmpfs（基于 RAM 的文件系统）挂载。

**适用场景**

- 将 HTTPS 证书存入 kubernets secret，并挂载到 /etc/nginx/conf.d/myhost.crt、/etc/nginx/conf.d/myhost.pem 路径，用来配置 nginx 的 HTTPS 证书

!!! note
	 容器以 [subPath](#subpath) 卷挂载方式使用 Secret 时，将无法接收 Secret 的更新。

## 使用 subPath

- https://kuboard.cn/learning/k8s-intermediate/persistent/volume-mount-point.html#%E6%95%B0%E6%8D%AE%E5%8D%B7%E5%86%85%E5%AD%90%E8%B7%AF%E5%BE%84
- https://kubernetes.io/zh/docs/concepts/storage/volumes/#%E4%BD%BF%E7%94%A8-subpath

`volumeMounts.subPath` 属性可用于指定所引用的卷内的子路径，而不是其根路径。

### 示例

下面是一个使用同一共享卷的、内含 LAMP 栈（Linux Apache Mysql PHP）的 Pod 的示例。 HTML 内容被映射到卷的 `html` 文件夹，数据库将被存储在卷的 `mysql` 文件夹中：

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: my-lamp-site
spec:
    containers:
    - name: mysql
      image: mysql
      env:
      - name: MYSQL_ROOT_PASSWORD
        value: "rootpasswd"
      volumeMounts:
      - mountPath: /var/lib/mysql
        name: site-data
        subPath: mysql
    - name: php
      image: php:7.0-apache
      volumeMounts:
      - mountPath: /var/www/html
        name: site-data
        subPath: html
    volumes:
    - name: site-data
      persistentVolumeClaim:
        claimName: my-lamp-site-data
```

### 使用带有扩展环境变量的 subPath

使用 `volumeMounts.subPathExpr` 字段，可以通过容器的环境变量指定容器内路径。使用此特性时，必须启用 `VolumeSubpathEnvExpansion`（自 Kubernetes v1.15 开始，是默认启用的。）

如下面的例子，该 Pod 使用 `subPathExpr` 在 hostPath 数据卷 `/var/log/pods` 中创建了一个目录 `pod1`（该参数来自于Pod的名字）。此时，宿主机目录 `/var/log/pods/pod1` 挂载到了容器的 `/logs` 路径：

```yml
apiVersion: v1
kind: Pod
metadata:
  name: pod1
spec:
  containers:
  - name: container1
    env:
    - name: POD_NAME
      valueFrom:
        fieldRef:
          apiVersion: v1
          fieldPath: metadata.name
    image: busybox
    command: [ "sh", "-c", "while [ true ]; do echo 'Hello'; sleep 10; done | tee -a /logs/hello.txt" ]
    volumeMounts:
    - name: workdir1
      mountPath: /logs
      subPathExpr: $(POD_NAME)
      readOnly: false
  restartPolicy: Never
  volumes:
  - name: workdir1
    hostPath:
      path: /var/log/pods
```

## 容器内路径

`mountPath` 数据卷被挂载到容器的路径，不能包含 `:`。

## 权限

容器对挂载的数据卷是否具备读写权限，如果 `readOnly` 为 `true`，则只读，否则可以读写（为 `false` 或者不指定）。默认为 `false`

## 资源

`emptyDir` 卷的存储介质（磁盘、SSD 等）是由保存 kubelet 根目录（通常是 ）的文件系统的介质确定。 卷或者  卷可以消耗的空间没有限制，容器之间或 Pod 之间也没有隔离。

## 挂载卷的传播

挂载卷的传播能力允许将容器安装的卷共享到同一 Pod 中的其他容器，甚至共享到同一节点上的其他 Pod。

卷的挂载传播特性由 Pod 中的 `spec.containers[*].volumeMounts.mountPropagation` 字段控制。 它的值包括：

- `None`：此卷挂载将不会感知到主机后续在此卷或其任何子目录上执行的挂载变化。 类似的，容器所创建的卷挂载在主机上是不可见的。这是默认模式。 该模式等同于 [Linux 内核文档](https://www.kernel.org/doc/Documentation/filesystems/sharedsubtree.txt)中描述的 `private` 挂载传播选项。

- `HostToContainer`：此卷挂载将会感知到主机后续针对此卷或其任何子目录的挂载操作。
- `Bidirectional`：在数据卷被挂载到容器之后，宿主机向该数据卷对应目录添加挂载时，对容器是可见的；同时，从容器中向该数据卷创建挂载，同样也对宿主机可见。



!!! warning
	`Bidirectional` 形式的挂载传播可能比较危险。 它可以破坏主机操作系统，因此它只被允许在特权容器中使用。 强烈建议您熟悉 Linux 内核行为。 此外，由 Pod 中的容器创建的任何卷挂载必须在终止时由容器销毁（卸载）。

### 配置

在某些部署环境中，挂载传播正常工作前，必须在 Docker 中正确配置挂载共享（mount share），如下所示。

编辑您的 Docker `systemd` 服务文件，按下面的方法设置 `MountFlags`：

```shell
MountFlags=shared
```

或者，如果存在 `MountFlags=slave` 就删除掉。然后重启 Docker 守护进程：

```shell
sudo systemctl daemon-reload
sudo systemctl restart docker
```