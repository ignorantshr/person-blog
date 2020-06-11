https://opencontainers.org/about/overview/

`OCI` （Open Container Initiative）：开放容器计划是一个开放治理结构，其明确目的是围绕容器格式和运行时创建开放行业标准。

`OCI` 当前包含两个规范： the Runtime Specification（`runtime-spec`）和 the Image Specification（`image-spec`）。

运行时规范概述了如何运行在磁盘上解压缩的`filesystem bundle`。

在较高级别上，OCI 实现（例如 runc、kata-containers）将下载 OCI 映像，然后将该映像解压缩为 OCI 运行时`filesystem bundle`，然后被 OCI Runtime 运行。

