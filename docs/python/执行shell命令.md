- 阻塞式命令调用方法：
```python
def _executeCmd(cmd, isSudo=False):
    if isSudo:
        cmd.insert(0, 'sudo')

    status, output = commands.getstatusoutput(' '.join(cmd))
    if status:
        raise Exception(output)
    return output
```
- 利用子进程执行命令调用方法（详细使用方法参见：subprocess模块）：
```python
def aaa():
    sub = subprocess.Popen("ssh root@172.16.7.210 df /home", shell=True, stdout=subprocess.PIPE)
    try:
        sub.wait()
        return sub.stdout.read()
    except:
        print "aaa kill"
        sub.terminate()
```

