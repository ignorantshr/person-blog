```
mysql>use mysql;
MariaDB [mysql]> update user set host = '%' where host = 'localhost';
mysql>flush privileges;
```

