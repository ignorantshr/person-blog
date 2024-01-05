
# [MGit](https://github.com/ignorantshr/mgit)
从零实现一个 git，参考来源：[Write yourself a Git!](https://wyag.thb.lt/)。
为了与 `.git` 区分，本工具使用 `.mgit` 作为版本控制系统的目录。

看源码之前建议先了解一下下面的相关概念。

## 概念：

### 对象
- `object`。git 用来管理、组织、存储、的形式。例如 blob、tree、commit、tag 都是 object。

    object 通过 hash 算法生成一个 hash 值作为文件名，文件内容存放各种 object 的具体内容，存放在 `.git/objects` 目录下面。

- `blob`。存储仓库中的单个文件内容。
- `tree`。描述仓库的内容。tree 将 blob 和 子树 关联起来，组织成一棵树，形成树状结构，这样就为恢复历史版本提供了依据。`git checkout` 其实就是找到树，然后根据其描述的组织关系恢复到系统文件。

### 引用
- 引用机制。除了通过 object 存储对象，git 还采取了引用机制来方便组织和管理它们。引用分为 直接引用 和 间接引用 两种形式：
    - `.git/refs` 下存放的是直接引用，也就是说他们的文件内容直接指向某个 object
    - `.git/HEAD` 在一般情况下存放的是间接引用形式，例如`ref: refs/heads/master`，可以通过指向的文件路径寻找其存放的 object hash 值，从而找到具体文件。
- `tag`。tag 可以理解为指向任何对象的引用。分为轻量级 tag 和带信息的 tag，后者就是 tag object。存放在`refs/tags`目录。
- `branch`。branch 其实就是指向某个 commit 的引用。存放在`refs/heads`目录。
- `HEAD`。HEAD 是指向当前 branch 或 某个 commit 的引用，用于跟随定位你在哪个历史版本的位置。

### 索引文件

- `index file` 或 `staging area`。要在 Git 中提交，首先需要使用 `git add` 和 `git rm` “暂存” 一些更改，然后才能提交这些更改。在最后一次提交和下一次提交之间的这个中间阶段称为“暂存区域”。

    git 使用 `index file` 来实现暂存区的管理。index 文件记录了文件的信息，通过与 HEAD 指向的 tree 作对比，可以得出暂存区与 HEAD 的差异；通过与 文件系统（仓库中的实际文件） 作对比，可以得出暂存区与 文件系统 的差异；通过这两次对比就能实现`git status`命令，知道 修改了什么文件、新增了什么文件、删除了什么文件。
    `git add/rm` 命令用来修改索引文件内容；然后需要 `git commit` 来把索引文件保存到磁盘上成为一个历史版本。
- `commit`。commit 命令主要就是把 index 里面平铺的结构按照树形结构写回磁盘，将 commit 和 tree 关联起来，附加一些提交信息，即构成了一次提交对象。将 commit 对象写到磁盘之后，还会更新分支所指向的 commit。