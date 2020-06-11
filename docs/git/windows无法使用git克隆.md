```shell
vi ~/.ssh/config

Host *.*.*.*
KexAlgorithms +diffie-hellman-group1-sha1
```

[git中文指南](https://git-scm.com/book/zh/v2)

分支合并策略：快速向前合并（搭配 rebase 使用）来合并微小的功能或者修复 bug，使用三路合并来整合长期运行的功能。后者导致的合并提交作为两个分支的连接标志。