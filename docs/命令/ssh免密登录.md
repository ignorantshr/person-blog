命令添加公钥
```shell
ssh-copy-id -i ~/.ssh/id_rsa.pub root@xxx
```
添加完成即可免密登录`root@xxx`

如果要求使用`ssh-agent`来托管验证的私钥，则使用如下方法：

```shell
# 生成认证socket，通常位于 /tmp/ssh-xxx/agent.xxx
eval `ssh-agent -s`
# 添加私钥
ssh-add
```

ssh-agent是一个密钥管理器，运行ssh-agent以后，使用ssh-add将私钥交给ssh-agent保管，其他程序需要身份验证的时候可以将验证申请交给ssh-agent来完成整个认证过程。

