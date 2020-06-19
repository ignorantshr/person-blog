- https://kubernetes.io/zh/docs/concepts/configuration/configmap/

ConfigMap 是一种 API 对象，用来将**非机密性**的数据保存到健值对中。使用时可以用作环境变量、命令行参数或者存储卷中的配置文件。

ConfigMap 将您的环境配置信息和 [容器镜像](https://kubernetes.io/docs/concepts/overview/what-is-kubernetes/#why-containers) 解耦，便于应用配置的修改。当您需要储存机密信息时可以使用 [Secret](https://kubernetes.io/docs/concepts/configuration/secret/) 对象。

## ConfigMap 对象

ConfigMap 是一个 API [对象](https://kubernetes.io/docs/concepts/overview/working-with-objects/kubernetes-objects/)，让你可以存储其他对象所需要使用的配置。和其他 Kubernetes 对象都有一个 `spec` 不同的是，ConfigMap 使用 `data` 块来存储元素（键名）和它们的值。

ConfigMap 的名字必须是一个合法的 [DNS 子域名](../../命名规则#1-dns-dns-subdomain-names)。

## ConfigMaps 和 Pods

您可以写一个引用 ConfigMap 的 Pod 的 `spec`，并根据 ConfigMap 中的数据在该 Pod 中配置容器。这个 Pod 和 ConfigMap 必须要在同一个 [命名空间](https://kubernetes.io/docs/concepts/overview/working-with-objects/namespaces) 中。

这是一个 ConfigMap 的示例，它的一些键只有一个值，其他键的值看起来像是配置的片段格式。

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  Name: game-demo
data:
  # 类属性键；每一个键都映射到一个简单的值
  player_initial_lives: 3
  ui_properties_file_name: "user-interface.properties"
  #
  # 类文件键
  game.properties: |
    enemy.types=aliens,monsters
    player.maximum-lives=5
  user-interface.properties: |
    color.good=purple
    color.bad=yellow
    allow.textmode=true
```

可以使用四种方式来使用 ConfigMap 配置 Pod 中的容器：

1. 容器 entrypoint 的命令行参数
2. 容器的环境变量
3. 在只读卷里面添加一个文件，让应用来读取
4. 编写代码在 Pod 中运行，使用 Kubernetes API 来读取 ConfigMap

这些不同的方法适用于不同的数据使用方式。对前三个方法，[kubelet](https://kubernetes.io/docs/reference/generated/kubelet) 使用 ConfigMap 中的数据在 Pod 中启动容器。

这是一个 Pod 的示例，它通过使用 `game-demo` 中的值来配置一个 Pod：

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: configmap-demo-pod
spec:
  containers:
    - name: demo
      image: game.example/demo-game
      env:
        # 定义环境变量
        - name: PLAYER_INITIAL_LIVES # 请注意这里和 ConfigMap 中的键名是不一样的
          valueFrom:
            configMapKeyRef:
              name: game-demo           # 这个值来自 ConfigMap
              key: player_initial_lives # 需要取值的键
        - name: UI_PROPERTIES_FILE_NAME
          valueFrom:
            configMapKeyRef:
              name: game-demo
              key: ui_properties_file_name
      volumeMounts:
      - name: config
        mountPath: "/config"
        readOnly: true
  volumes:
    # 您可以在 Pod 级别设置卷，然后将其挂载到 Pod 内的容器中
    - name: config
      configMap:
        # 提供你想要挂载的 ConfigMap 的名字
        name: game-demo
```

ConfigMap 不会区分单行属性值和多行类似文件的值，重要的是 Pods 和其他对象如何使用这些值。比如，定义一个卷，并将它作为 `/config` 文件夹安装到 `demo` 容器内，并创建四个文件：

- `/config/player_initial_lives`
- `/config/ui_properties_file_name`
- `/config/game.properties`
- `/config/user-interface.properties`

