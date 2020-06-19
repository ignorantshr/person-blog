- https://kuboard.cn/learning/k8s-intermediate/workload/wl-job/

- https://kubernetes.io/docs/concepts/workloads/controllers/job/

Kubernetes中的 Job 对象将创建一个或多个 Pod，并确保指定数量的 Pod 可以成功执行到进程正常结束：

- 当 Job 创建的 Pod 执行成功并正常结束时，Job 将记录成功结束的 Pod 数量
- 当成功结束的 Pod 达到指定的数量时，Job 将完成执行
- 删除 Job 对象时，将清理掉由 Job 创建的 Pod

例子

```yml
apiVersion: batch/v1
kind: Job
metadata:
  name: pi
spec:
  template:
    spec:
      containers:
      - name: pi
        image: perl
        command: ["perl",  "-Mbignum=bpi", "-wle", "print bpi(2000)"]
      restartPolicy: Never
  backoffLimit: 4
```

执行结果可以通过 `kubectl logs <pod-name>` 查看。

## 创建 Job

### Pod Template

指定合适的[重启策略 restartPolicy](https://kuboard.cn/learning/k8s-intermediate/workload/pod-lifecycle.html#重启策略) `.spec.template.spec.restartPolicy`，此处只允许使用 `Never` 和 `OnFailure` 两个取值。

### Parallel Jobs

有三种主要的任务类型适合使用 Job 运行：

- Non-parallel Jobs 
    - 通常，只启动一个 Pod，除非该 Pod 执行失败
    - Pod 执行成功并结束以后，Job 也立刻进入完成 completed 状态
- Parallel Jobs with a fixed completion count 
    - `.spec.completions` 为一个非零正整数
    - Job 将创建至少 `.spec.completions` 个 Pod，编号为 1 - `.spec.completions` （**尚未实现**）
    - Job 记录了任务的整体执行情况，当 1 - `.spec.completions` 中每一个编号都有一个对应的 Pod 执行成功时，Job 进入完成状态
- Parallel Jobs with a work queue 
    - 不指定 `.spec.completions`，使用 `.spec.parallelism`
    - Pod 之间必须相互之间自行协调并发，或者使用一个外部服务决定每个 Pod 各自执行哪些任务。例如，一个 Pod 可能会从工作队列中获取多达 n 个项目的批处理。
    - 每个 Pod 都可以独立判断其他同僚（peers）是否完成，并确定整个Job是否完成
    - 当 Job 中任何一个 Pod 成功结束，将不再为其创建新的 Pod
    - 当所有的 Pod 都结束了，且至少有一个 Pod 执行成功后才结束，则 Job 判定为成功结束
    - 一旦任何一个 Pod 执行成功并退出，Job 中的任何其他 Pod 都应停止工作和输出信息，并开始终止该 Pod 的进程

completions 和 parallelism

- 对于 non-parallel Job，`.spec.completions` 和 `.spec.parallelism` 可以不填写，默认值都为 1
- 对于 fixed completion count Job，需要设置 `.spec.completions` 为您期望的个数；同时不设置 `.spec.parallelism` 字段（默认值为 1）
- 对于 work queue Job，不能设置 `.spec.completions` 字段，且必须设置 `.spec.parallelism` 为0或任何正整数

### Controlling Parallelism 并发控制

并发数 `.spec.parallelism` 可以被设置为0或者任何正整数，如果不设置，默认为1，如果设置为 0，则 Job 被暂停，直到该数字被调整为一个正整数。

实际的并发数（同一时刻正在运行的 Pod 数量）可能比设定的并发数 `.spec.parallelism` 要大一些或小一些，不一定严格相等，主要的原因有：

- 对于 fixed completion count Job，实际并发运行的 Pod 数量不会超过剩余未完成的数量。如果 `.spec.parallelism` 比这个数字更大，将被忽略
- 对于 work queue Job，任何一个 Pod 成功执行后，将不再创建新的 Pod （剩余的 Pod 将继续执行）
- Job 控制器可能没有足够的时间处理并发控制
- 如果 Job 控制器创建 Pod 失败（例如，[ResourceQuota](https://kuboard.cn/learning/k8s-advanced/policy/rq.html) 不够用，没有足够的权限等）
- 同一个Job中，在已创建的 Pod 出现大量失败的情况下，Job 控制器可能限制 Pod 的创建
- 当 Pod 被优雅地关闭时（gracefully shut down），需要等候一段时间才能结束

## 处理Pod和容器的失败

Pod 中的**容器**可能会因为多种原因执行失败，例如：

- 容器中的进程退出了，且退出码（exit code）不为 0
- 容器因为超出内存限制而被 Kill
- 其他原因

如果 Pod 中的容器执行失败，且 `.spec.template.spec.restartPolicy = "OnFailure"`，则 Pod 将停留在该节点上，但是容器将被重新执行。此时，您的应用程序需要处理在原节点（失败之前的节点）上重启的情况。或者，您也可以设置为 `.spec.template.spec.restartPolicy = "Never"`。

整个 **Pod** 也可能因为多种原因执行失败，例如：

- Pod 从节点上被驱逐（节点升级、重启、被删除等）
- Pod 的容器执行失败，且 `.spec.template.spec.restartPolicy = "Never"`

当 Pod 执行失败时，Job 控制器将创建一个新的 Pod。此时，您的应用程序需要**处理在一个新 Pod 中重新启动的情况**。具体来说，需要处理临时文件、锁、未完成的输出信息以及前一次执行可能遗留下来的其他东西。

!!! warning
	- 即使您指定 `.spec.parallelism = 1`、 `.spec.completions = 1` 以及 `.spec.template.spec.restartPolicy = "Never"`，同一个应用程序仍然可能被启动多次
	- 如果指定 `.spec.parallelism` 和 `.spec.completions` 的值都大于 1，则，将可能有多个 Pod 同时执行。此时，您的 Pod 还必须能够处理并发的情况

### Pod 失败重试策略

某些情况下（例如，配置错误），您可能期望在 Job 多次重试仍然失败的情况下停止该 Job。此时，可通过 `.spec.backoffLimit` 来设定 Job 最大的重试次数。该字段的默认值为 6。

Job 中的 Pod 执行失败之后，Job 控制器将按照一个指数增大的时间延迟（10s,20s,40s ... 最大为 6 分钟）来多次重新创建 Pod。如果没有新的 Pod 执行失败，则重试次数的计数将被重置。

建议在 debug 时，设置 `restartPolicy = "Never"`，或者使用日志系统确保失败的 Job 的日志不会丢失。

## Job的终止和清理

当 Job 完成后：

- 将不会创建新的 Pod
- 已经创建的 Pod 也不会被清理掉。此时，您仍然可以继续查看已结束 Pod 的日志，以检查 errors/warnings 或者其他诊断用的日志输出
- Job 对象也仍然保留着，以便您可以查看该 Job 的状态
- 由用户决定是否删除已完成的 Job 及其 Pod 
    - 可通过 `kubectl` 命令删除 Job，例如： `kubectl delete jobs/pi` 或者 `kubectl delete -f https://kuboard.cn/statics/learning/job/job.yaml`
    - 删除 Job 对象时，由该 Job 创建的 Pod 也将一并被删除

Job 通常会顺利的执行下去，但是在如下情况可能会非正常终止：

- 某一个 Pod 执行失败（且 `restartPolicy=Never`）
- 或者某个容器执行出错（且  `restartPolicy=OnFailure`） 
    - 此时，Job 按照上节《Pod 失败重试策略》 `.spec.bakcoffLimit` 描述的方式进行处理
    - 一旦重试次数达到了 `.spec.backoffLimit` 中的值，Job 将被标记为失败，且尤其创建的所有 Pod 将被终止
- Job 中设置了 `.spec.activeDeadlineSeconds`。该字段限定了 Job 对象在集群中的存活时长，一旦达到 `.spec.activeDeadlineSeconds` （比 `.spec.backoffLimit `的优先级高）指定的时长，该 Job 创建的所有的 Pod 都将被终止，Job 的 Status 将变为 `type:Failed` 、 `reason: DeadlineExceeded`。

## Job的自动清理

系统中已经完成的 Job 通常是不在需要里的，长期在系统中保留这些对象，将给 apiserver 带来很大的压力。如果通过更高级别的控制器（例如 [CronJobs](https://kuboard.cn/learning/k8s-intermediate/workload/wl-cronjob/)）来管理 Job，则 CronJob 可以根据其中定义的基于容量的清理策略（capacity-based cleanup policy）自动清理Job。

### TTL 机制

**FEATURE STATE:** `Kubernetes v1.12 [alpha]`

除了 CronJob 之外，TTL 机制是另外一种自动清理已结束Job（`Completed` 或 `Finished`）的方式：

- TTL 机制由 [TTL 控制器](https://kuboard.cn/learning/k8s-intermediate/workload/wl-ttl/) 提供
- 在 Job 对象中指定 `.spec.ttlSecondsAfterFinished` 字段可激活该特性

-  `.spec.ttlSecondsAfterFinished` 值为 100，则，在其结束 `100` 秒之后，将可以被自动删除
- 如果 `.spec.ttlSecondsAfterFinished` 被设置为 `0`，则 TTL 控制器在 Job 执行结束后，立刻就可以清理该 Job 及其 Pod
- 如果 `.spec.ttlSecondsAfterFinished` 值未设置，则 TTL 控制器不会清理该 Job

## Job 模式

Kubernetes Job 对象可以用来支持 Pod 的并发执行，但是：

- Job 对象并非设计为支持需要紧密相互通信的Pod的并发执行，例如科学计算。
- Job 对象支持并发处理一系列相互独立但是又相互关联的工作任务，例如： 
    - 发送邮件
    - 渲染页面
    - 转码文件
    - 扫描 NoSQL 数据库中的主键
    - 其他

在一个复杂的系统中，可能存在多种类型的工作任务，本文只考虑批处理任务（batch job）。

- 每个工作任务一个 Job 对象 v.s. 一个 Job 对象负责所有的工作任务 
    - 当工作任务特别多时，第二种选择（一个 Job 对象负责所有的工作任务）更合适一些
    - 第一种选择（每个工作任务一个 Job 对象）将为管理员和系统带来很大的额外开销，因为要管理很多数量的 Job 对象
- Pod的数量与工作任务的数量相等 v.s. 每个Pod可以处理多个工作任务 
    - 第一种选择（Pod的数量与工作任务的数量相等）通常只需要对现有的代码或容器做少量的修改
    - 第二种选择（每个Pod可以处理多个工作任务）更适合工作任务的数量特别多的情况，相较于第一种选择可以降低系统开销
- 使用工作队列，此时： 
    - 需要运行一个队列服务
    - 需要对已有的程序或者容器做修改，以便其可以配合队列工作
    - 如果是一个已有的程序，改造时可能存在难度

| 模式                                                         | 单个 Job 对象 | Pod的数量少于工作任务 | 无需修改已有代码 | 在 Kube 1.1 可以运行 |
| ------------------------------------------------------------ | ------------- | --------------------- | ---------------- | -------------------- |
| [Job Template Expansion](https://kubernetes.io/docs/tasks/job/parallel-processing-expansion/) |               |                       | ✓                | ✓                    |
| [Queue with Pod Per Work Item](https://kubernetes.io/docs/tasks/job/coarse-parallel-processing-work-queue/) | ✓             |                       | sometimes        | ✓                    |
| [Queue with Variable Pod Count](https://kubernetes.io/docs/tasks/job/fine-parallel-processing-work-queue/) | ✓             | ✓                     |                  | ✓                    |
| Single Job with Static Work Assignment                       | ✓             |                       | ✓                |                      |

当您指定 `.spec.completions` 时，Job 控制器创建的每个 Pod 都有一个相同的 [spec](https://kuboard.cn/learning/k8s-intermediate/workload/wl-job/pattern.html)。这意味着，同一个 Job 创建的所有的 Pod 都使用：

- 相同的执行命令
- 相同的容器镜像
- 相同的数据卷
- 相同的环境变量（例如，不同时间点创建的Pod，[Service的环境变量](https://kuboard.cn/learning/k8s-intermediate/service/service-details.html#环境变量) 可能会不同）

Job 的不同模式本质上讲，是如何为一组工作任务分配 Pod。下表总结了不同的模式下 `.spec.parallelism` 和 `.spec.completions` 字段的设置。（表中 `w` 代表工作任务的数量）

| Pattern                                                      | `.spec.completions` | `.spec.parallelism` |
| ------------------------------------------------------------ | ------------------- | ------------------- |
| [Job Template Expansion](https://kubernetes.io/docs/tasks/job/parallel-processing-expansion/) | 1                   | should be 1         |
| [Queue with Pod Per Work Item](https://kubernetes.io/docs/tasks/job/coarse-parallel-processing-work-queue/) | W                   | any                 |
| [Queue with Variable Pod Count](https://kubernetes.io/docs/tasks/job/fine-parallel-processing-work-queue/) | 1                   | any                 |
| Single Job with Static Work Assignment                       | W                   | any                 |

