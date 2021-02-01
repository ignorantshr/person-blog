## 安装 jar 包

```bash
mvn install:install-file -Dfile=model-4.3.31.jar -DgroupId=org.ovirt.engine.api -DartifactId=model -Dversion=4.3.31 -Dpackaging=jar
```

## 安装源码包

若是源码包将类型更改为`jar-source`即可：

```bash
mvn install:install-file -Dfile=model-4.3.31-sources.jar -DgroupId=org.ovirt.engine.api -DartifactId=model -Dversion=4.3.31 -Dpackaging=jar-source
```

## 安装文档包

若是java文档包将类型更改为`javadoc`即可：

```bash
mvn install:install-file -Dfile=model-4.3.31-javadoc.jar -DgroupId=org.ovirt.engine.api -DartifactId=model -Dversion=4.3.31 -Dpackaging=javadoc
```

## 脚本

为简化安装命令，可以使用`install-mvn-jar.sh`脚本安装：

```bash
#!/bin/sh
# 使用 maven 安装 jar 包到 ~/.m2

set -e

[[ $# -ne 2 ]] && echo -e "Usage:\n\tsh $0 <filename> <groupId>" && exit 1

fn=$1
gid=$2

arr=(${fn//-/ })
aid=${arr[0]}
ver=${arr[1]}
if [[ ${#arr[*]} -eq 2 ]]; then
        ver=${ver%.*}
        type=jar
else
        ctype=(${arr[2]//./ /})
        case ${ctype[0]} in
                sources)        type=jar-source;;
                javadoc)        type=javadoc;;
                *)              echo "Cannot recognize the type: ${ctype[0]}" && exit 1
        esac
fi

mvn install:install-file -Dfile=${fn} -DgroupId=${gid} -DartifactId=${aid} -Dversion=${ver} -Dpackaging=${type}
```

