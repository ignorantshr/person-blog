https://github.com/kubernetes/community/blob/master/contributors/devel/sig-node/container-runtime-interface.md

CRI（*Container Runtime Interface*）是 Kubernetes 的一个组件，由规范/要求（待添加），protobuf API 和 用于容器运行时的库组成，以便与节点上的 kubelet 集成。

换句话说，Kubelet 是 CRI 客户端（运行 gRPC-client），并且期望 CRI实现 （运行 gRPC-server）能够处理服务器端的接口。

k8s 与 CRI、OCI 的关系：

![](img/k8s-CRI-OCI.png)

参考：[Kubernetes + CRI + Kata + Firecracker](https://yq.aliyun.com/articles/692117)