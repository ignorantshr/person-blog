## 配置反向代理

在`/etc/nginx/nginx.conf`中创建顶级 [`stream {}`](https://nginx.org/en/docs/stream/ngx_stream_core_module.html#stream) 块：

```nginx
stream {
    include /etc/nginx/conf.d/stream/*.conf;
}
```

在 stream 中写入一些包含  [`listen`](https://nginx.org/en/docs/stream/ngx_stream_core_module.html#listen) 指令的 [`server {}`](https://nginx.org/en/docs/stream/ngx_stream_core_module.html#server) 虚拟服务块，并使用代理指令  [`proxy_pass`](https://nginx.org/en/docs/stream/ngx_stream_proxy_module.html#proxy_pass) （不要在该指令中指定协议 http/https）：

```nginx
# cat /etc/nginx/conf.d/stream/load-balance.conf
upstream dns_servers {
	server 192.168.136.130:53;
    server 192.168.136.131:53;
}

server {
    listen 8080;
    # 不要指定协议，因为你已经在 listen 指令中指定过了
    proxy_pass 192.168.17.201:9001;
}

server {
	listen 5000 udp;
    # 不要指定协议，因为你已经在 listen 指令中指定过了
	proxy_pass dns_servers;
}
```

默认是 TCP 流量，所以不加 tcp 参数；若代理的是 UDP 流量，则需要添加 udp 参数。

若代理服务器有多个网络接口，那么你可以指定连接到后端服务器的 *源地址*，使用 [`proxy_bind`](https://nginx.org/en/docs/stream/ngx_stream_proxy_module.html#proxy_bind) 指令实现：

```nginx
stream {
    # ...
    server {
        listen     127.0.0.1:12345;
        proxy_pass backend.example.com:12345;
        proxy_bind 127.0.0.1:12345;
    }
}
```

可选的，可以调整两个内存缓冲器的大小，NGINX 可以在其中防置来自客户端和上游连接的数据。使用[`proxy_buffer_size`](https://nginx.org/en/docs/stream/ngx_stream_proxy_module.html#proxy_buffer_size)  指令控制：

```nginx
stream {
    # ...
    server {
        listen            127.0.0.1:12345;
        proxy_pass        backend.example.com:12345;
        proxy_buffer_size 16k;
    }
}
```

### stream 块中 upstream 块的负载均衡算法

除了默认算法，这些算法指定指令必须位于 server 指令之前。

未详细说明的算法请参考 [《HTTP 负载均衡》](../03 HTTP 负载均衡)

#### Round Robin

#### [Least Connections](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#least_conn) 

#### [Least Time](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#least_time) (NGINX Plus only) 

用于计算最低平均等待时间的方法取决于 `least_time` 指令包含以下哪个参数：

- `connect`  – Time to connect to the upstream server
- `first_byte` – Time to receive the first byte of data
- `last_byte`  – Time to receive the full response from the server

#### [Hash](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#hash)

```nginx
upstream stream_backend {
    hash $remote_addr;
    server backend1.example.com:12345;
    server backend2.example.com:12345;
}
```

*$remote_addr* 是一个内置变量，更多内置变量参考：https://nginx.org/en/docs/http/ngx_http_core_module.html

该算法还可以被用于配置 *session persistence*。指定一个可选的一致性参数 `consistent` 以应用 [ketama](https://www.last.fm/user/RJ/journal/2007/04/10/rz_libketama_-_a_consistent_hashing_algo_for_memcache_clients) 一致性哈希方法：

```nginx
hash $remote_addr consistent;
```

#### [Random](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#random)

- `least_conn` – The least number of active connections
- `least_time=connect` (NGINX Plus) – The time to connect to the upstream server ([`$upstream_connect_time`](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#var_upstream_connect_time))
- `least_time=first_byte` (NGINX Plus) – The least average time to receive the first byte of data from the server ([`$upstream_first_byte_time`](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#var_upstream_first_byte_time))
- `least_time=last_byte` (NGINX Plus) – The least average time to receive the last byte of data from the server ([`$upstream_session_time`](https://nginx.org/en/docs/stream/ngx_stream_upstream_module.html#var_upstream_session_time))

```nginx 
random two least_time=last_byte;
```

## 健康检测

被动的健康检测参数 [`max_fails`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#max_fails), [`fail_timeout`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#fail_timeout), [`slow_start`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#slow_start) 同样也适用于 TCP 和 UDP，说明请参见   [《HTTP 负载均衡》](../03 HTTP 负载均衡) 的 *服务器慢启动* 章节。

只有 NGINX Plus 支持主动的健康检测，参考：

- https://docs.nginx.com/nginx/admin-guide/load-balancer/tcp-health-check/#active-tcp-health-checks
- https://docs.nginx.com/nginx/admin-guide/load-balancer/udp-health-check/#hc_active