There is a few options and it depends if you want to include the changes of patchset-1.

* You only want the changes from patchset-2?
Checkout master and cherry-pick patchset-2, commit -amend and push again
* You want to include changes from patchset-1 and 2?
In this case you may want to combine the two patchsets, easiest is to do a git reset HEAD~2 and commit again, don't forget to copy the Change-ID of your last commit to be able to continue that review

change A被废弃，change B报错

#### 只想要B：
* 切换到其他分支，修改代码，提交到目标分支
* cherry-pick

#### A B都要：
* git reset HEAD^^
* 再次提交，复制最后一次提交的Change-ID以便于可以继续给gerrit 进行代码审查。

