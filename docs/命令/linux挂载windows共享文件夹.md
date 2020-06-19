
```shell
mount.cifs //172.16.2.27/mkdocs blog/ -o user=xxx,pass=xxx
```

失败尝试添加选项再试：

```shell
mount.cifs //172.16.2.27/mkdocs blog/ -o user=xxx,pass=xxx,vers=1.0,sec=ntlm
```

