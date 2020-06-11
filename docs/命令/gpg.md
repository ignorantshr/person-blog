gpg(GnuPG)是GNU发布的免费加密软件。

算法支持：

- 签名: RSA, ?, ?, ELG, DSA
- 加密: IDEA, 3DES, CAST5, BLOWFISH, AES, AES192, AES256, TWOFISH, CAMELLIA128, CAMELLIA192, CAMELLIA256
- 哈希: MD5, SHA1, RIPEMD160, SHA256, SHA384, SHA512, SHA224
- 压缩: Uncompressed, ZIP, ZLIB, BZIP2

```
gpg2 [--homedir dir] [--options file] [options] command [args]
Sign, check, encrypt or decrypt, Default operation depends on the input data

Commands:
	-s, --sign                 签名 make a signature
		--clearsign			   make a clear text signature
 	-b, --detach-sign          make a detached signature
 	-e, --encrypt              加密
 	-c, --symmetric            只使用对称加密算法进行加密
 		--cipher-algo 		   指定加密算法
 	-d, --decrypt              解密 (default)
    	--verify               验证签名
    	
    	--fingerprint [names]  show fingerprints
    -k, --list-keys, --list-public-keys           列出 keys
     	--list-sigs            列出 keys and signatures
     	--check-sigs           列出 并 检查 key signatures
     	--fingerprint          列出 keys and fingerprints
     	
    -K, --list-secret-keys     列出 secret keys
     	--gen-key              生成一个新的密钥对
     	--gen-revoke           生成一个吊销证书
     	--delete-keys          从公钥库中删除密钥对
     	--delete-secret-keys   从私钥库中删除密钥对
     	--sign-key             对一个 key 签名
     	--lsign-key            对一个 key 进行本地签名
     	--edit-key             签名或编辑一个 key
     	--passwd               修改密码
     	
    	--export               export keys
    	--export-secret-key    导出私钥 export secret keys
    	--import               import/merge keys
    	
Options:
	-a, --armor                create ascii armored output
	-r, --recipient USER-ID    收件人，接收方
	-u, --local-user USER-ID   use USER-ID to sign or decrypt
	-z N                       set compress level to N (0 disables)
		--textmode             use canonical text mode
	-o, --output FILE          write output to FILE
	-n, --dry-run              do not make any changes
		--openpgp              use strict OpenPGP behavior
```

使用私钥时需要输入密码，但是两次输入之间不超过一定的时间则无需再次输入，类似与sudo命令。

## 生成key

```bash
# 生成一个只用于签名的 key
[user1@dev ~]$ gpg --gen-key
Please select what kind of key you want:
	# 生成签名和加密密钥对
   (1) RSA and RSA (default)
   (2) DSA and Elgamal
   # 生成签名密钥对
   (3) DSA (sign only)
   (4) RSA (sign only)
Your selection? 4

# 填写个人信息
Real name: sign-user
Email address: sign-user@qq.com
Comment: test sign
You selected this USER-ID:
    "sign-user (test sign) <sign-user@qq.com>"

# 这里可以继续修改
Change (N)ame, (C)omment, (E)mail or (O)kay/(Q)uit? O
You need a Passphrase to protect your secret key.
# 键入密码
……
gpg: key E0F6E6AC marked as ultimately trusted
public and secret key created and signed.

gpg: checking the trustdb
gpg: 3 marginal(s) needed, 1 complete(s) needed, PGP trust model
gpg: depth: 0  valid:   1  signed:   0  trust: 0-, 0q, 0n, 0m, 0f, 1u
pub   4096R/E0F6E6AC 2019-10-15
      Key fingerprint = 7C66 A689 7C6F A1BA 187F  7204 5F46 7860 E0F6 E6AC
uid                  sign-user (test sign) <sign-user@qq.com>

# 生成一个用于签名和加密的 key
[user1@dev ~]$ gpg --gen-key
Please select what kind of key you want:
   (1) RSA and RSA (default)
   (2) DSA and Elgamal
   (3) DSA (sign only)
   (4) RSA (sign only)
Your selection? 1
……
Real name: secret-user
Email address: secret-user@qq.com
Comment: test secret and sign
You selected this USER-ID:
    "secret-user (test secret and sign) <secret-user@qq.com>"
……
gpg: key F5CDD34C marked as ultimately trusted
public and secret key created and signed.

gpg: checking the trustdb
gpg: 3 marginal(s) needed, 1 complete(s) needed, PGP trust model
gpg: depth: 0  valid:   2  signed:   0  trust: 0-, 0q, 0n, 0m, 0f, 2u
# 用于签名的key，也是Primary key
pub   4096R/F5CDD34C 2019-10-15
      Key fingerprint = 9E42 E5DC E58A AF21 FFD0  A7A9 F392 6847 F5CD D34C
uid                  secret-user (test secret and sign) <secret-user@qq.com>
# 用于加密的key，也是 Subkey
sub   4096R/8B3EBB7E 2019-10-15
```

### 出现的问题

在要输入密码的过程中，可能不会出现弹窗，生成密码过程直接被取消了：

```
gpg: cancelled by user
gpg: Key generation canceled.
```

此时可以尝试这样做：

```bash
# 查看 tty 的权限是否和当前用户一致，这里看到明显不对（这是我从root用户切换过来的）
[gpg-user@convert-rpm ~]$ ls -la $(tty)
crw--w----. 1 root tty 136, 0 Oct 16 06:06 /dev/pts/0
# 那么就退出重新使用普通用户登录，再次查看
[gpg-user@convert-rpm ~]$ ll -a $(tty)
crw--w----. 1 gpg-user tty 136, 0 Oct 16 06:07 /dev/pts/0
# 此时权限足够，可以正常的出现文字界面的弹窗啦！
```



还有一个问题是在输完密码之后就挂起了，没有反应了：

```
We need to generate a lot of random bytes.……this gives the random number
generator a better chance to gain enough entropy.
```

这是随机数不足导致的，此时打开另一个窗口，运行下面的指令，生成足够的随机数（就是操作磁盘），过一会二就会完成密码的生成了：

```bash
dd if=/dev/sda of=/dev/zero
```



## 查看key

这些选项在脚本或程序中使用时应搭配`--with-colons`选项，变为易于机器解析的输出。

```bash
[user1@dev ~]$ gpg -k
/home/user1/.gnupg/pubring.gpg
------------------------------
# 签名用的公钥
类型	 算法/密钥ID(唯一标识符，fingerprint的后8位)
pub   4096R/E0F6E6AC 2019-10-15
uid                  sign-user (test sign) <sign-user@qq.com>

# 签名用的公钥
pub   4096R/F5CDD34C 2019-10-15
# 密钥的信息
uid                  secret-user (test secret and sign) <secret-user@qq.com>
# 加密用的公钥
sub   4096R/8B3EBB7E 2019-10-15

[user1@dev ~]$ gpg -K
/home/user1/.gnupg/secring.gpg
------------------------------
# 签名用的私钥
sec   4096R/E0F6E6AC 2019-10-15
uid                  sign-user (test sign) <sign-user@qq.com>

# 签名用的私钥
sec   4096R/F5CDD34C 2019-10-15
uid                  secret-user (test secret and sign) <secret-user@qq.com>
# 加密用的私钥
ssb   4096R/8B3EBB7E 2019-10-15
```

```bash
# 使用机器解析格式输出并格式化输出
[user1@dev ~]$ gpg --list-sigs --with-colon | awk -F: 'BEGIN {ORS="\n"; fileds=21; for(i=1;i<=fileds;++i) printf "%-5d ", i; printf "\n"}; {print $0; for(i=1;i<=fileds;++i) printf "%-5.5s ", $i; printf "\n"}'
1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18    19    20    21
tru::1:1571190311:0:3:1:5
tru         1     15711 0     3     1     5
pub:u:4096:1:5F467860E0F6E6AC:1571129294:::u:::scSC:
pub   u     4096  1     5F467 15711             u                 scSC
uid:u::::1571129294::65ED1361B91C5E7E527F0CD76F7296C54E1E0EF9::sign-user (test sign) <sign-user@qq.com>:
uid   u                       15711       65ED1       sign-
sig:::1:5F467860E0F6E6AC:1571129294::::sign-user (test sign) <sign-user@qq.com>:13x:::::2:
sig               1     5F467 15711                   sign- 13x                           2
pub:u:4096:1:F3926847F5CDD34C:1571129468:::u:::scESC:
pub   u     4096  1     F3926 15711             u                 scESC
uid:u::::1571129468::E31596C92F9195FD6F05F65AFD02F2A712FD9CF2::secret-user (test secret and sign) <secret-user@qq.com>:
uid   u                       15711       E3159       secre
sig:::1:F3926847F5CDD34C:1571129468::::secret-user (test secret and sign) <secret-user@qq.com>:13x:::::2:
sig               1     F3926 15711                   secre 13x                           2
sub:u:4096:1:A26E02608B3EBB7E:1571129468::::::e:
sub   u     4096  1     A26E0 15711                               e
sig:::1:F3926847F5CDD34C:1571129468::::secret-user (test secret and sign) <secret-user@qq.com>:18x:::::2:
sig               1     F3926 15711                   secre 18x                           2
pub:u:4096:1:E16574E67A028145:1571190309:::u:::scSC:
pub   u     4096  1     E1657 15711             u                 scSC
uid:u::::1571190309::7B6291D3B9F2863E967E7429E864B6A71296E660::local-sign-user (sign key in local) <local-sign-user@local.com>:
uid   u                       15711       7B629       local
sig:::1:E16574E67A028145:1571190309::::local-sign-user (sign key in local) <local-sign-user@local.com>:13x:::::2:
sig               1     E1657 15711                   local 13x                           2
```

关于各字段的含义参考：https://git.gnupg.org/cgi-bin/gitweb.cgi?p=gnupg.git;a=blob;f=doc/DETAILS;h=315f56e31157c6e8998f4335bf63d594b10e8923;hb=HEAD

常见的字段意义，序号代表第几个字段：

1. 记录的类型
    - pub :: Public key
    - sub :: Subkey (secondary key)
    - sec :: Secret key
    - ssb :: Secret subkey (secondary key)
    - sig :: Signature
    - uid :: User id
    - crt :: X.509 certificate
2. 有效期
    - o :: Unknown (this key is new to the system)
    - u :: The key is ultimately valid.
3. 密钥长度，单位是bit
4. 公钥算法。与列出的算法顺序相对应
5. 密钥ID
6. 创建日期，对于 uid 和 uat 类型的记录，此值是自签的日期。从1970.01.01以来的秒数
7. 过期日期。
8. 证书的 serial number，UID的哈希，信任签名的信息（第一个值是信任深度 ）
9. Ownertrust，只在主键上存在，信任签名的正则表达式
10. User-ID
11. 签名类型
12. 密钥能力，可以是以下作用的任意组合
     - e :: Encrypt
     - s :: Sign
     - c :: Certify
     - a :: Authentication
     - ? :: Unknown capability
     - D :: 表示禁用的密钥
     - 主密钥上的大写字母表示整个密钥的可用功能
13. 颁发者证书指纹或其他信息
14.  
15.  
16. sig的哈希算法。与列出的算法顺序相对应

## 编辑key

```bash
[user1@dev ~]$ gpg --edit-key shihr
Secret key is available.

pub  4096R/E0F6E6AC  created: 2019-10-15  expires: never       usage: SC
                     trust: ultimate      validity: ultimate
[ultimate] (1). sign-user (test sign) <sign-user@qq.com>
# 使用 ? 列出可用命令
gpg>
```

## 导入导出key

导出

```bash
[user1@dev ~]$ gpg --export secret-user --textmode -o secret-user.pub
[user1@dev ~]$ scp secret-user.pub gpg-user@192.168.216.158:/home/gpg-user/gpg-test

[user1@dev ~]$ gpg --export --armor F5CDD34C
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2.0.22 (GNU/Linux)
mQINBF2liHwBEACzdVPSVCyMUNEiS2HCpPyuV5uRYGXOxQMgtSZZX7vOxwUH3SLm
……
-----END PGP PUBLIC KEY BLOCK-----
```

导入

```bash
[gpg-user@convert-rpm ~]$ gpg --import secret-user.pub
gpg: key F5CDD34C: public key "secret-user (test secret and sign) <secret-user@qq.com>" imported
gpg: Total number processed: 1
gpg:               imported: 1  (RSA: 1)
[gpg-user@convert-rpm ~]$ gpg -K
[gpg-user@convert-rpm ~]$ gpg -k
/home/gpg-user/.gnupg/pubring.gpg
---------------------------------
pub   4096R/F5CDD34C 2019-10-15
uid                  secret-user (test secret and sign) <secret-user@qq.com>
sub   4096R/8B3EBB7E 2019-10-15
```

## 加解密文件

```bash
# 公钥加密
[gpg-user@convert-rpm ~]$ gpg -r F5CDD34C -e -o secret-f origin-f.txt
gpg: 8B3EBB7E: There is no assurance this key belongs to the named user
# 这里可以看出加密是使用 Subkey 来加密的，而不是 Primary key
pub  4096R/8B3EBB7E 2019-10-15 secret-user (test secret and sign) <secret-user@qq.com>
 Primary key fingerprint: 9E42 E5DC E58A AF21 FFD0  A7A9 F392 6847 F5CD D34C
      Subkey fingerprint: 3303 FF26 B8EF 54AB 380C  27B6 A26E 0260 8B3E BB7E

It is NOT certain that the key belongs to the person named
in the user ID.  If you *really* know what you are doing,
you may answer the next question with yes.

Use this key anyway? (y/N) y
[gpg-user@convert-rpm ~]$ scp secret-f user1@192.168.216.163:~
# 私钥解密
[user1@dev ~]$ gpg -d -o decryption-f secret-f

You need a passphrase to unlock the secret key for
user: "secret-user (test secret and sign) <secret-user@qq.com>"
4096-bit RSA key, ID 8B3EBB7E, created 2019-10-15 (main key ID F5CDD34C)
# 这里同样也能看出是第二个key来加密的
gpg: encrypted with 4096-bit RSA key, ID 8B3EBB7E, created 2019-10-15
      "secret-user (test secret and sign) <secret-user@qq.com>"
[user1@dev ~]$ cat decryption-f
hello world
```

## 签名

```bash
# 私钥签名
[user1@dev ~]$ gpg -s -r 8B3EBB7E -o signed-f origin-f.txt
gpg: WARNING: recipients (-r) given without using public key encryption

You need a passphrase to unlock the secret key for
user: "sign-user (test sign) <sign-user@qq.com>"
4096-bit RSA key, ID E0F6E6AC, created 2019-10-15
# 创建 ascii 格式输出的签名文件
[user1@dev ~]$ gpg -a -s -u F5CDD34C -o signed-f origin-f.txt
[user1@dev ~]$ cat signed-f
-----BEGIN PGP MESSAGE-----
Version: GnuPG v2.0.22 (GNU/Linux)

owEBTgKx/ZANAwACAfOSaEf1zdNMAaweYgxvcmlnaW4tZi50eHRdpoqpaGVsbG8g
……
-----END PGP MESSAGE-----
# 创建明文签名文件。注意： 验证明文签名时，gpg 仅验证构成明文签名数据的内容，而不是直接在仪表板行之后的明文签名或标题行之外的任何其他数据。此格式还有其它缺陷，建议避免使用明文签名，而采用分离签名。
[user1@dev ~]$ gpg --clearsign -s -u F5CDD34C -o signed-f origin-f.txt
[user1@dev ~]$ cat signed-f
-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

hello world
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v2.0.22 (GNU/Linux)

iQIcBAEBAgAGBQJdpop/AAoJEPOSaEf1zdNMjJsP/jiUpYyUr+/jjSFKnW2YoMq2
……
-----END PGP SIGNATURE-----
# 创建分离式的签名文件，里面不包含文件内容，验证时需要和原文件一起验证
[user1@dev ~]$ gpg -b -s -u F5CDD34C -o signed-f origin-f.txt

# 公钥验证
[gpg-user@convert-rpm ~]$ gpg --verify signed-f
gpg: Signature made Tue 15 Oct 2019 09:00:53 PM EDT using RSA key ID F5CDD34C
gpg: Good signature from "secret-user (test secret and sign) <secret-user@qq.com>"
gpg: WARNING: This key is not certified with a trusted signature!
gpg:          There is no indication that the signature belongs to the owner.
Primary key fingerprint: 9E42 E5DC E58A AF21 FFD0  A7A9 F392 6847 F5CD D34C
# 验证分离式签名文件
[gpg-user@convert-rpm ~]$ gpg --verify signed-f origin-f.txt
gpg: Signature made Tue 15 Oct 2019 11:21:56 PM EDT using RSA key ID F5CDD34C
gpg: Good signature from "secret-user (test secret and sign) <secret-user@qq.com>"
```



## 对外部导入公钥添加信任

在上文可以看到在使用公钥时发出了警告：

```
gpg: WARNING: This key is not certified with a trusted signature!
gpg:          There is no indication that the signature belongs to the owner.
```

方法一：编辑公钥，添加信任等级

```bash
[gpg-user@convert-rpm ~]$ gpg --edit-key F5CDD34C
pub  4096R/F5CDD34C  created: 2019-10-15  expires: never       usage: SC
					 # 信任等级（未设置）	有效期
                     trust: unknown       validity: unknown
sub  4096R/8B3EBB7E  created: 2019-10-15  expires: never       usage: E
[ unknown] (1). secret-user (test secret and sign) <secret-user@qq.com>
gpg> trust
# 这里的意思是说你需要去验证这个公钥的合法性，可以通过对比密钥的 fingerprint 来检测
Please decide how far you trust this user to correctly verify other users' keys
(by looking at passports, checking fingerprints from different sources, etc.)

  1 = I don't know or won't say
  2 = I do NOT trust
  # 稍微信任
  3 = I trust marginally
  4 = I trust fully
  5 = I trust ultimately
  m = back to the main menu
# 这里选择5，表示无条件信任
Your decision? 5
Do you really want to set this key to ultimate trust? (y/N) y

pub  4096R/F5CDD34C  created: 2019-10-15  expires: never       usage: SC
                     trust: ultimate      validity: unknown
sub  4096R/8B3EBB7E  created: 2019-10-15  expires: never       usage: E
[ unknown] (1). secret-user (test secret and sign) <secret-user@qq.com>
Please note that the shown key validity is not necessarily correct
unless you restart the program.
```

方法二：编辑公钥，使用本地私钥进行信任签名

```bash
[gpg-user@convert-rpm ~]$ gpg --edit-key F5CDD34C
gpg> tsign

pub  4096R/F5CDD34C  created: 2019-10-15  expires: never       usage: SC
                     trust: undefined     validity: unknown
 Primary key fingerprint: 9E42 E5DC E58A AF21 FFD0  A7A9 F392 6847 F5CD D34C

     secret-user (test secret and sign) <secret-user@qq.com>

Please decide how far you trust this user to correctly verify other users' keys
(by looking at passports, checking fingerprints from different sources, etc.)

  1 = I trust marginally
  2 = I trust fully

Your selection? 2

Please enter the depth of this trust signature.
A depth greater than 1 allows the key you are signing to make
trust signatures on your behalf.

Your selection? 2
# 这里不懂？？？
Please enter a domain to restrict this signature, or enter for none.

Your selection? 2

Are you sure that you want to sign this key with your
key "local-sign-user (use for local sign) <local-sign-user@local.com>" (C86BB985)

Really sign? (y/N) y

You need a passphrase to unlock the secret key for
user: "local-sign-user (use for local sign) <local-sign-user@local.com>"
4096-bit RSA key, ID C86BB985, created 2019-10-16

gpg> save

# 再次编辑，可以看到属性已经发生了变化
[gpg-user@convert-rpm ~]$ gpg --edit-key F5CDD34C
pub  4096R/F5CDD34C  created: 2019-10-15  expires: never       usage: SC
                     trust: full          validity: full
sub  4096R/8B3EBB7E  created: 2019-10-15  expires: never       usage: E
[  full  ] (1). secret-user (test secret and sign) <secret-user@qq.com>
```



```bash
# 后面使用公钥就不会有警告啦！
[gpg-user@convert-rpm ~]$ gpg --verify signed-f
gpg: Signature made Tue 15 Oct 2019 09:00:53 PM EDT using RSA key ID F5CDD34C
gpg: Good signature from "secret-user (test secret and sign) <secret-user@qq.com>"
```

## 使用对称密钥进行加密

```bash
# 加密，需要输入密码
[user1@dev ~]$ gpg -c -o encryp-f origin-f.txt
[user1@dev ~]$ scp encryp-f gpg-user@192.168.216.158:~
# 解密
[gpg-user@convert-rpm ~]$ gpg -d encryp-f
gpg: CAST5 encrypted data
gpg: encrypted with 1 passphrase
hello world
gpg: WARNING: message was not integrity protected

# 指定加密算法
[user1@dev ~]$ gpg -c --cipher-algo AES -o encryp-f origin-f.txt
[user1@dev ~]$ gpg -d encryp-f
gpg: AES encrypted data
gpg: encrypted with 1 passphrase
hello world
```

支持的算法可以使用`--verison`选项列出：

```bash
[user1@dev ~]$ gpg --version
……

Home: ~/.gnupg
Supported algorithms:
Pubkey: RSA, ?, ?, ELG, DSA
Cipher: IDEA, 3DES, CAST5, BLOWFISH, AES, AES192, AES256, TWOFISH,
        CAMELLIA128, CAMELLIA192, CAMELLIA256
Hash: MD5, SHA1, RIPEMD160, SHA256, SHA384, SHA512, SHA224
Compression: Uncompressed, ZIP, ZLIB, BZIP2
```



## 资料

https://www.gnupg.org/howtos/zh/index.html