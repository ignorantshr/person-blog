- https://kubernetes.io/zh/docs/concepts/configuration/secret/

## 概述

Secret 是一种包含少量敏感信息例如密码、token 或 key 的对象。将此类敏感信息存储到 `Secret` 中，可以更好地：

- 控制其使用
- 降低信息泄露的风险

//FIXME secret 只是对数据进行 `base64` 编码，感觉没什么用啊。

Pod 使用 secret 的方式：

- 作为 Pod 的数据卷挂载
- 作为 Pod 的环境变量
- kubelet 在抓取容器镜像时，作为 docker 镜像仓库的用户名密码

### 内置 secret

Service Account 使用 API 凭证自动创建和附加 secret

Kubernetes 自动创建包含访问 API 凭据的 secret，并自动修改您的 pod 以使用此类型的 secret。

如果需要，可以禁用或覆盖自动创建和使用 API 凭据。但是，如果您需要的只是安全地访问 apiserver，我们推荐这样的工作流程。

参阅 [ServiceAccount](https://kubernetes.io/docs/tasks/configure-pod-container/configure-service-account/) 文档。

### 自行创建 secret

#### 使用`kubectl`创建

Secrets 可以包含 Pods 访问数据库所需的用户凭据。

创建文件：

```bash
# Create files needed for the rest of the example.
echo -n 'admin' > ./username.txt
echo -n '1f2d1e2e67df' > ./password.txt
```

```shell
kubectl create secret generic db-user-pass --from-file=./username.txt --from-file=./password.txt
```

默认的 key 的名字是文件名，你可以在选项`--from-file`中指定 key：

```shell
kubectl create secret generic db-user-pass --from-file=username=./username.txt --from-file=password=./password.txt
```

!!! note
	特殊字符（例如 `$`, `\`,`*`,`=` 和 `!` ）需要转义。 如果您使用的密码具有特殊字符，则需要使用 `\` 字符对其进行转义。 例如，如果您的实际密码是 `S!B\*d$zDsb` ，则应通过以下方式执行命令： `kubectl create secret generic dev-db-secret --from-literal=username=devuser --from-literal=password=S\!B\\*d\$zDsb` ，但在大部分 shell 中最简单的方法是使用单引号将字符串包围起来：`'S!B\*d$zDsb'`。您无需从文件中转义密码中的特殊字符（ `--from-file` ）。

##### 查看 secret

```bash
# kubectl describe secrets/db-user-pass
Name:         db-user-pass
Namespace:    default
Labels:       <none>
Annotations:  <none>

Type:  Opaque

Data
====
password.txt:  12 bytes
username.txt:  5 bytes
```

#### 手动创建 Secret

 secret 包含两种类型，数据（data）和字符串数据（stringData）。 数据字段用于存储使用 base64 编码的任意数据。 提供 stringData 字段是为了方便起见，它允许您将机密数据作为未编码的字符串提供。

##### 例

```shell
echo -n 'admin' | base64
YWRtaW4=
echo -n '123456' | base64
MTIzNDU2
```

`my-secret.yaml`

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: mysecret
type: Opaque
data:
  username: YWRtaW4=
stringData:
  password: 123456
```

```shell
kubectl get secret/mysecret -o yaml
apiVersion: v1
data:
  password: MTIzNDU2
  username: YWRtaW4=
```

!!! note
	如果在 data 和 stringData 中都指定了相同的字段，则使用 stringData 中的值。data 和 stringData 的键必须由字母数字字符 '-', '_' 或者 '.' 组成。

!!! note
	数据的序列化 JSON 和 YAML 值被编码为 base64 字符串。换行符在这些字符串中无效，因此必须省略。在 Darwin/macOS 上使用 `base64` 实用程序时，用户应避免使用 `-b` 选项来分隔长行。相反，Linux用户 *应该* 在 `base64` 命令中添加选项 `-w 0` ，或者，如果 `-w` 选项不可用的情况下，执行 `base64 | tr -d '\n'`。

#### 从生成器创建 Secret

[略](https://kubernetes.io/zh/docs/concepts/configuration/secret/#%E4%BB%8E%E7%94%9F%E6%88%90%E5%99%A8%E5%88%9B%E5%BB%BA-secret)。

## 编辑Secret

执行命令 `kubectl edit secrets mysecret` 可以编辑已经创建的 Secret 的 data 字段。

## 使用 Secret

### 在 Pod 中使用 Secret 文件

在 Pod 中的 volume 里使用 Secret：

1. 创建一个 secret 或者使用已有的 secret。多个 pod 可以引用同一个 secret。
2. 修改您的 pod 的定义在 `spec.volumes[]` 下增加一个 volume。可以给这个 volume 随意命名，它的 `spec.volumes[].secret.secretName` 必须等于 secret 对象的名字。
3. 将 `spec.containers[].volumeMounts[]` 加到需要用到该 secret 的容器中。指定 `spec.containers[].volumeMounts[].readOnly = true` 和 `spec.containers[].volumeMounts[].mountPath` 为您想要该 secret 出现的尚未使用的目录。
4. 修改您的镜像或者命令行让程序从该目录下寻找文件。Secret 的 `data` 映射中的每一个键都成为了 `mountPath` 下的一个文件名。

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: mypod
spec:
  containers:
  - name: mypod
    image: redis
    volumeMounts:
    - name: foo
      mountPath: "/etc/foo"
      readOnly: true
  volumes:
  - name: foo
    secret:
      secretName: mysecret
```

#### 向特定路径映射 secret 密钥

```yaml
  volumes:
  - name: foo
    secret:
      secretName: mysecret
      items:
      - key: username
        path: my-group/my-username
```

将会发生什么呢：

- `username` secret 存储在 `/etc/foo/my-group/my-username` 文件中而不是 `/etc/foo/username` 中。
- **`password` secret 没有被映射**

#### Secret 文件权限

您可以为单个 Secret 密钥设置文件访问权限位，`0644` 是默认值。

```yaml
  volumes:
  - name: foo
    secret:
      secretName: mysecret
      defaultMode: 0400
  # 或
  volumes:
  - name: foo
    secret:
      secretName: mysecret
      items:
      - key: username
        path: my-group/my-username
        mode: 511
```

!!! note
	JSON 规范不支持八进制符号，因此使用 `256` 值作为 `0400` 权限。如果您使用 yaml 而不是 json 作为 pod，则可以使用八进制符号以更自然的方式指定权限。

#### 从 Volume 中消费 secret 值

在挂载的 secret volume 的容器内，secret key 将作为文件，并且 secret 的值使用 base-64 解码并存储在这些文件中。

#### Secret 作为环境变量

将 secret 作为 pod 中的[环境变量 ](https://kubernetes.io/docs/concepts/containers/container-environment-variables.md)使用：

1. 创建一个 secret 或者使用一个已存在的 secret。多个 pod 可以引用同一个 secret。
2. 修改 Pod 定义，为每个要使用 secret 的容器添加对应 secret key 的环境变量。消费 secret key 的环境变量应填充 secret 的名称，并键入 `env[x].valueFrom.secretKeyRef`。
3. 修改镜像或者命令行，以便程序在指定的环境变量中查找值。

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: secret-env-pod
spec:
  containers:
  - name: mycontainer
    image: redis
    env:
      - name: SECRET_USERNAME
        valueFrom:
          secretKeyRef:
            name: mysecret
            key: username
      - name: SECRET_PASSWORD
        valueFrom:
          secretKeyRef:
            name: mysecret
            key: password
  restartPolicy: Never
```

#### 使用 imagePullSecret

imagePullSecret 是将包含 Docker（或其他）镜像注册表密码的 secret 传递给 Kubelet 的一种方式，因此可以代表您的 pod 拉取私有镜像。

**手动指定 imagePullSecret**

imagePullSecret 的使用在 [镜像文档](https://kubernetes.io/docs/concepts/containers/images/#specifying-imagepullsecrets-on-a-pod) 中说明。

### 安排 imagePullSecrets 自动附加

您可以手动创建 imagePullSecret，并从 serviceAccount 引用它。使用该 serviceAccount 创建的任何 pod  和默认使用该 serviceAccount 的 pod 将会将其的 imagePullSecret 字段设置为服务帐户的  imagePullSecret 字段。有关该过程的详细说明，请参阅 [将 ImagePullSecrets 添加到服务帐户](https://kubernetes.io/docs/tasks/configure-pod-container/configure-service-account/#adding-imagepullsecrets-to-a-service-account)。

#### 自动挂载手动创建的 Secret

手动创建的 secret（例如包含用于访问 github 帐户的令牌）可以根据其服务帐户自动附加到 pod。请参阅 [使用 PodPreset 向 Pod 中注入信息](https://kubernetes.io/docs/tasks/run-application/podpreset/) 以获取该进程的详细说明。