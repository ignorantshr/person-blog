#!/bin/bash
# ====================================================================
# 用户参数输入位置:
# basedir=你用来储存此脚本所预计备份的数据之目录(请独立文件系统)
basedir=/backup/weekly

# ====================================================================
# 底下请不要修改了!用默认值即可!
PATH=/bin:/usr/bin:/sbin:/usr/sbin; export PATH
export LANG=C

# 设定要备份的服务的配置文件,以及备份的目录
named=$basedir/named
postfixd=$basedir/postfix
vsftpd=$basedir/vsftp
sshd=$basedir/ssh
sambad=$basedir/samba
wwwd=$basedir/www
others=$basedir/others
userinfod=$basedir/userinfo
# 判断目录是否存在,若不存在则予以建立。
for dirs in $named $postfixd $vsftpd $sshd $sambad $wwwd $others $userinfod
do
	[ ! -d "$dirs" ] && mkdir -p $dirs
done

# 1. 将系统主要的服务之配置文件分别备份下来,同时也备份 /etc 全部。
cp -a /var/named/chroot/{etc,var}	$named
cp -a /etc/postfix /etc/dovecot.conf	$postfixd
cp -a /etc/vsftpd/*			$vsftpd
cp -a /etc/ssh/*			$sshd
cp -a /etc/samba/*			$sambad
cp -a /etc/{my.cnf,php.ini,httpd}	$wwwd
cd /var/lib
  tar -jpc -f $wwwd/mysql.tar.bz2 	mysql
cd /var/www
  tar -jpc -f $wwwd/html.tar.bz2 	html cgi-bin
cd /
  tar -jpc -f $others/etc.tar.bz2	etc
cd /usr/
  tar -jpc -f $others/local.tar.bz2	local

# 2. 关于使用者参数方面
cp -a /etc/{passwd,shadow,group}	$userinfod
cd /var/spool
  tar -jpc -f $userinfod/mail.tar.bz2	mail
cd /
  tar -jpc -f $userinfod/home.tar.bz2	home
cd /var/spool
  tar -jpc -f $userinfod/cron.tar.bz2	cron at
