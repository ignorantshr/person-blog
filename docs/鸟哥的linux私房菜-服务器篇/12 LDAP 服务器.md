

[openldap](https://www.openldap.org/doc/admin24/guide.html)

## 快速开始

### Server

```
yum install openldap-servers
```

1. 添加数据库的配置文件`/usr/local/etc/ldap/mdb.ldif`：

```
dn: olcDatabase=mdb,cn=config
objectClass: olcDatabaseConfig
objectClass: olcMdbConfig
olcDatabase: mdb
OlcDbMaxSize: 1073741824
olcSuffix: dc=my-domin,dc=com
olcRootDN: cn=Manager,dc=my-domain,dc=com
olcRootPW: secret
olcDbDirectory: /var/lib/openldap-data
olcDbIndex: objectClass eq
```

2. 使用`slapadd`添加条目：

```
slapadd -l /usr/local/etc/ldap/mdb.ldif
```

3. 修改权限：

```bash
chown -R ldap:ldap /var/lib/ldap/
```

4. 启动：

```
systemctl start slapd
```
<!--
修改`/etc/openldap/slapd.d/cn\=config/olcDatabase\=\{2\}hdb.ldif`，添加密码：

```
olcRootPW: secret
```

重新启动：

```
systemctl restart slapd
```
-->

5. 检验：

```
[root@controller ldap]# ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
# extended LDIF
#
# LDAPv3
# base <> with scope baseObject
# filter: (objectclass=*)
# requesting: namingContexts
#

#
dn:
namingContexts: dc=my-domain,dc=com

# search result
search: 2
result: 0 Success

# numResponses: 2
# numEntries: 1
```

### Client

```
yum install openldap-clients
```

下面是添加一条条目的步骤

1. 创建配置文件 `my-organization.ldif`：

```
dn: dc=my-domain,dc=com
objectClass: dcObject
objectClass: organization
o: my-organization
dc: my-domain

dn: cn=Manager,dc=my-domain,dc=com
objectClass: organizationalRole
cn: Manager
```

2. 使用`ldapadd`添加到服务器：

```
[root@controller ldap]# ldapadd -x -D "cn=Manager,dc=my-domain,dc=com" -w secret -f my-organization.ldif
adding new entry "dc=my-domain,dc=com"

adding new entry "cn=Manager,dc=my-domain,dc=com"

```

3. 检查新添加的条目：

```
[root@controller ldap]# ldapsearch -x -b 'dc=my-domain,dc=com' '(objectclass=*)'
# extended LDIF
#
# LDAPv3
# base <dc=my-domain,dc=com> with scope subtree
# filter: (objectclass=*)
# requesting: ALL
#

# my-domain.com
dn: dc=my-domain,dc=com
objectClass: dcObject
objectClass: organization
o: my-organization
dc: my-domain

# Manager, my-domain.com
dn: cn=Manager,dc=my-domain,dc=com
objectClass: organizationalRole
cn: Manager

# search result
search: 2
result: 0 Success

# numResponses: 3
# numEntries: 2
```

