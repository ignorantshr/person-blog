- https://kuboard.cn/learning/k8s-intermediate/workload/wl-cronjob/
- https://kubernetes.io/zh/docs/concepts/workloads/controllers/cron-jobs/

一个 CronJob 对象就像 *crontab* (cron table) 文件中的一行。它用 [Cron](https://en.wikipedia.org/wiki/Cron) 格式进行编写，并周期性地在给定的调度时间执行 Job。

!!! warning
	所有 CronJob 的 `schedule:` 时间都是基于初始 Job 的主控节点的时区。
	如果你的控制平面在 Pod 或是裸容器中运行了主控程序 (kube-controller-manager)， 那么为该容器设置的时区将会决定定时任务的控制器所使用的时区。

## CronJob 的限制

CronJob 创建 Job 对象，每个 Job 的执行次数**大约**为一次。 我们之所以说 "大约"，是因为在某些情况下，可能会创建两个 Job，或者不会创建任何 Job。 我们试图使这些情况尽量少发生，但不能完全杜绝。因此，Job 应该是 ***幂等的***。

当以下两个条件都满足时，Job 将至少运行一次：

- `startingDeadlineSeconds` 被设置为一个较大的值，或者不设置该值（默认值将被采纳）
- `concurrencyPolicy` 被设置为 `Allow`

对于每一个 CronJob，CronJob 控制器将检查自上一次执行的时间点到现在为止有多少次执行被错过了。如果错过的执行次数超过了 100，则 CronJob 控制器将不再创建 Job 对象，并记录错误。

需要注意的是，如果 `startingDeadlineSeconds` 字段非空，则控制器会统计从 `startingDeadlineSeconds` 设置的值到现在而不是从上一个计划时间到现在错过了多少次 Job。例如，如果 `startingDeadlineSeconds` 是 `200`，则控制器会统计在过去 200 秒中错过了多少次 Job。

CronJob 仅负责创建与其调度时间相匹配的 Job，而 Job 又负责管理其代表的 Pod。

## 使用CronJob执行自动任务

### 创建CronJob

```yml
apiVersion: batch/v1beta1
kind: CronJob
metadata:
  name: hello
spec:
  schedule: "*/1 * * * *"
  jobTemplate:
    spec:
      template:
        spec:
          containers:
          - name: hello
            image: busybox
            args:
            - /bin/sh
            - -c
            - date; echo Hello from the Kubernetes cluster
          restartPolicy: OnFailure
```

!!! warning
	所有对 CronJob 对象作出的修改，尤其是 `.spec` 的修改，都只对修改之后新建的 Job 有效，已经创建的 Job 不会受到影响。

### 编写CronJob YAML

#### jobTemplate

`.spec.jobTemplate`是任务的模版，它是必须的。它和 [Job](https://kubernetes.io/docs/concepts/workloads/controllers/jobs-run-to-completion/)的语法完全一样，除了它是嵌套的没有 `apiVersion` 和 `kind`。 编写任务的 `.spec` ，请参考 [编写任务的Spec](https://kubernetes.io/docs/concepts/workloads/controllers/jobs-run-to-completion/#writing-a-job-spec)。

### 开始的最后期限

`.spec.startingDeadlineSeconds `域是可选的，参考上文。

### 并发性规则

`.spec.concurrencyPolicy` 也是可选的。它声明了 CronJob 创建的任务执行时发生重叠如何处理。spec 仅能声明下列规则中的一种：

- `Allow` (默认)：CronJob 允许并发任务执行。
- `Forbid`： CronJob 不允许并发任务执行；如果新任务的执行时间到了而老任务没有执行完，CronJob 会忽略新任务的执行。
- `Replace`：如果新任务的执行时间到了而老任务没有执行完，CronJob 会用新任务替换当前正在运行的任务。

请注意，并发性规则仅适用于相同 CronJob 创建的任务。如果有多个 CronJob，它们相应的任务总是允许并发执行的。

### 挂起

`.spec.suspend`域也是可选的。如果设置为 `true` ，后续发生的执行都会挂起。这个设置对已经开始的执行不起作用。默认是关闭的。

!!! warning
	在调度时间内挂起的执行都会被统计为错过的任务。当 `.spec.suspend` 从 `true` 改为 `false` 时，且没有 [开始的最后期限](https://kubernetes.io/zh/docs/tasks/job/automated-tasks-with-cron-jobs/#starting-deadline)，错过的任务会被立即调度。

### 任务历史限制

`.spec.successfulJobsHistoryLimit` 和 `.spec.failedJobsHistoryLimit`是可选的。 这两个域声明了有多少执行完成和失败的任务会被保留。 默认设置为3和1。限制设置为0代表相应类型的任务完成后不会保留。

