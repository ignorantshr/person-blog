# 有文件修改
### 正常流程

	git rebase -i 68598f1d65da0dd3de283cf5ac04bbf17b58d1a1
	修改文件
	git add .
	git commit --amend
	git rebase --continue

### 文件冲突
	git rebase -i 48a55705063291c68208880bbcc7581e3c2e5074
	修改文件
	git add .
	git rebase --continue
	解决冲突
	git add .
	git rebase –-continue

*tips*:忘记了流程。。。

# 合并历史提交
先到想要合并的提交中最早的一次的**前一次**
```
git rebase -i 48a55705063291c68208880bbcc7581e3c2e5074
```
或
```
git rebase -i HEAD~2
```

第一列是rebase具体执行的操作，其中操作可以选择，其中含义如下：

- 选择pick操作，git会应用这个补丁，以同样的提交信息（commit message）保存提交
- 选择reword操作，git会应用这个补丁，但需要重新编辑提交信息
- 选择edit操作，git会应用这个补丁，但会因为amending而终止
- 选择squash操作，git会应用这个补丁，但会与之前的提交合并
- 选择fixup操作，git会应用这个补丁，但会丢掉提交日志
- 选择exec操作，git会在shell中运行这个命令

根据上述的提示将想要合并的提交改变其操作，保存之后会提示修改后的信息，如果需要可以再次更改提交信息。下面的补丁会应用到上面的pick提交。

