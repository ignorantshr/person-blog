## HTTP 负载均衡

### 将 http 流量代理给一组服务

需要在 http context 中使用 [`upstream`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html?&_ga=2.112454404.1045546250.1587992338-1762490011.1587992338#upstream) 指令。

使用 [`server`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html?&_ga=2.112953220.1045546250.1587992338-1762490011.1587992338#server) 指令来配置服务器（注意，不要和虚拟服务 `server block` 搞混了）。例如，以下配置定义了一个名为backend的组，它由三个服务器配置组成（可以在三个以上的实际服务器中解析）：

```
http {
    upstream backend {
        server backend1.example.com weight=5;
        server backend2.example.com;
        server 192.0.0.1 backup;
    }
}
```

为了将请求传递给一个服务器集群（server group），可以使用 [`proxy_pass`](https://nginx.org/en/docs/http/ngx_http_proxy_module.html#proxy_pass) 指令（或[`fastcgi_pass`](https://nginx.org/en/docs/http/ngx_http_fastcgi_module.html#fastcgi_pass), [`memcached_pass`](https://nginx.org/en/docs/http/ngx_http_memcached_module.html#memcached_pass), [`scgi_pass`](https://nginx.org/en/docs/http/ngx_http_scgi_module.html#scgi_pass), or [`uwsgi_pass`](https://nginx.org/en/docs/http/ngx_http_uwsgi_module.html#uwsgi_pass) 这些协议所对应的指令）：

```
http {
    upstream backend {
        server backend1.example.com;
        server backend2.example.com;
        server 192.0.0.1 backup;
    }
    
    server {
        location / {
            proxy_pass http://backend;
        }
    }
}
```

backend 组由三台服务器组成，其中两台运行同一应用程序的实例，而第三台是备用服务器。因为没有在 upstream 中指定负载均衡算法，所以使用**默认的轮循算法**。

注意了，如果配置的服务器无法连接，看看是否是SELinux和防火墙的问题！

### 负载均衡算法

#### Round Robin

默认算法，会将请求均匀地分配给每个服务器，会考虑权重参数 [server weights](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#weights) 的影响。

#### [Least Connections](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#least_conn)

请求会被发送到被激活的连接数最少的服务器，会考虑权重参数 [server weights](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#weights) 的影响。

```
upstream backend {
    least_conn;
    server backend1.example.com;
    server backend2.example.com;
}
```

#### [IP Hash](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#ip_hash)

根据客户端的IP地址来分发请求，除非那台服务器不可用。使用IPv4地址的前三个八位字节或整个IPv6地址来计算哈希值。

```
upstream backend {
    ip_hash;
    server backend1.example.com;
    server backend2.example.com;
}
```

若有一台服务器需要临时维护，可以使用 [`down`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#down) 参数来保留它，分发给它的请求会分发给下一台服务器。

```
upstream backend {
    server backend1.example.com;
    server backend2.example.com;
    server backend3.example.com down;
}
```

#### Generic [Hash](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#hash)

如何分发请求取决于用户自定义的`key`（字符串、变量或两者的组合），

```
upstream backend {
    hash $request_uri consistent;
    server backend1.example.com;
    server backend2.example.com;
}
```

`hash` 指令的可选一致性参数`consistent`会启用 [ketama](https://www.last.fm/user/RJ/journal/2007/04/10/rz_libketama_-_a_consistent_hashing_algo_for_memcache_clients) 一致性哈希负载平衡。这意味着请求将被平均的映射到服务器集群上面。若一台服务器被移除或添加，这样会导致较少的重新映射，有利于缓存的保存。

#### [Least Time](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#least_time) (NGINX Plus only) 

NGINX Plus选择具有最低平均延迟和最少活动连接数的服务器。平均最低平均延迟是根据`least_time` 指令中指定的参数计算得出的：

- `header` – Time to receive the first byte from the server
- `last_byte` – Time to receive the full response from the server
- `last_byte inflight` – Time to receive the full response from the server, taking into account incomplete requests

```
upstream backend {
    least_time header;
    server backend1.example.com;
    server backend2.example.com;
}
```

#### [Random](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#random)

每个请求都会被随机分配给一台服务器。如果指定了`two`参数，首先，NGINX会考虑服务器权重随机选择两个服务器，然后使用指定的方法选择其中一台服务器：

- `least_conn` – The least number of active connections
- `least_time=header` (NGINX Plus) – The least average time to receive the response header from the server ([`$upstream_header_time`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#var_upstream_header_time))
- `least_time=last_byte` (NGINX Plus) – The least average time to receive the full response from the server ([`$upstream_response_time`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#var_upstream_response_time))

```
upstream backend {
    random two least_conn;
    server backend1.example.com;
    server backend2.example.com;
    server backend3.example.com;
    server backend4.example.com;
}
```

随机负载均衡方法适用于应用于多个负载均衡器将请求传递到同一组后端的分布式环境。For environments where the load balancer has a full view of all requests, use other load balancing methods.

!!! note
	除了默认算法，其他的负载均衡指令都需要在 upstream 块中位于 server 指令的前面。

### 权重

默认情况下，NGINX使用 Round Robin 方法根据请求的权重在组中的服务器之间分配请求。使用 server 指令中的 [`weight`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#weight) 参数来分配服务器的权重，默认值是`1`。

### 服务器慢启动

服务器慢启动（slow‑start）功能可防止连接数过多而使最近恢复的服务器不堪重负，连接可能会超时并导致服务器再次被标记为故障。

在NGINX Plus中，慢启动允许上游服务器在恢复或可用后将其权重从0逐渐恢复到设定值。使用 [`slow_start`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#slow_start) 参数来指定：

```
upstream backend {
    server backend1.example.com slow_start=30s;
    server backend2.example.com;
    server 192.0.0.1 backup;
}
```

**注意**：如果服务器集群中只有一台服务器，那么这些参数是不会生效的：[`max_fails`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#max_fails), [`fail_timeout`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#fail_timeout), [`slow_start`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#slow_start)。

`fail_timeout` 在指定的连接尝试次数内必须失败的时间，才能将服务器视为不可用（默认值是 10 秒）；以及被标记为不可用状态的时间；

`max_fails `设置在`fail_timeout` 期间必须发生的失败尝试次数，以将服务器标记为不可用（默认值是 1）。

他们两个组合起来使用的意思就是：如果 NGINX 无法在 `fail_timeout` 秒内 `max_fails ` 次向服务器发送请求或没有收到来自服务器的响应，它就将服务器在 `fail_timeout` 秒内标记为不可用。当服务被标记为不可用时， NGINX 将暂时停止向其发送请求，直到再次将其标记为活动状态为止。

### 启用会话持久性

会话持久性（session persistence）意味着NGINX Plus可以识别用户会话并将给定会话中的所有请求路由到 upstream 中的同一台服务器。

对于 NGINX 的会话持久性，请使用上文所述的 hash 或 ip_hash 指令。至于 NGINX Plus 支持三种会话持久性方法。这些方法是使用 [`sticky`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#sticky) 指令设置的：

- [**Sticky cookie**](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#sticky_cookie)：NGINX Plus将会话 cookie 添加到 upstream group 的第一个响应中，并标识发送响应的服务器。后续客户端发送带有 cookie 请求时就能路由到那台服务器。

```
upstream backend {
    server backend1.example.com;
    server backend2.example.com;
    sticky cookie srv_id expires=1h domain=.example.com path=/;
}
```

srv_id 是 cookie 的名字；expires 设置浏览器的保存时间

- [**Sticky route**](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#sticky_route)：NGINX Plus在收到第一个请求时为客户端分配一个“路由”。将所有后续请求与server指令的route参数进行比较，以标识将请求代理到的服务器。

```
upstream backend {
    server backend1.example.com route=a;
    server backend2.example.com route=b;
    sticky route $route_cookie $route_uri;
}
```

- [**Sticky learn**](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#sticky_learn)：NGINX Plus首先通过检查请求和响应来找到会话标识符。然后NGINX Plus“学习”哪个 upstream 服务器对应于哪个会话标识符。通常，这些标识符在HTTP cookie中传递。如果一个请求包含一个已经“学习”的会话标识符，NGINX Plus将请求转发到相应的服务器：

```
upstream backend {
   server backend1.example.com;
   server backend2.example.com;
   sticky learn
       create=$upstream_cookie_examplecookie
       lookup=$cookie_examplecookie
       zone=client_sessions:1m
       timeout=1h;
}
```

### 限制连接数量

使用NGINX Plus，可以通过使用 [`max_conns`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#max_conns) 参数指定最大数量来限制到上游（upstream）服务器的连接数量。

如果达到了 max_conns 限制，则将请求放置在队列中以进行进一步处理，前提是还包括了 [`queue`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#queue) 指令来设置队列中可以同时存在的最大请求数：

```
upstream backend {
    server backend1.example.com max_conns=3;
    server backend2.example.com;
    queue 100 timeout=70;
}
```

若队列已满或者在超时期间服务器不能被选择，客户端会收到一个错误。

**注意**：如果在其他[worker processes](https://nginx.org/en/docs/ngx_core_module.html#worker_processes)中打开了空闲的 [keepalive](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#keepalive) 连接，则会忽略 max_conns 限制。结果会导致在与多个工作进程共享内存的配置（ [shared with multiple worker processes](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#zone)）中，与服务器的连接总数可能会超过 max_conns 值。

### 与多个工作进程共享数据

如果 upstream 块不包含 [`zone`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#zone) 指令，那么每个 work process 都会保留自己的服务器组配置副本，并维护自己的一组相关计数器。计数器包括与组中每个服务器的当前连接数以及将请求传递给服务器的失败尝试数。这样会导致服务器组配置无法动态修改，并且会出现一些不符合预期而情况（比如[`max_fails`](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#health_passive)的设置表现会不符合预期）。

在  upstream 中使用 zone 指令，可以让这些工作线程共享内存中存储的配置。该指令在使用 [active health checks](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#health_active) 和 [dynamic reconfiguration](https://docs.nginx.com/nginx/admin-guide/load-balancer/dynamic-configuration-api/) 时是强制性的。

#### 设置内存大小

所需的内存量取决于启用了哪些功能（例如[session persistence](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#sticky), [health checks](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#health_active), 或 [DNS re‑resolving](https://docs.nginx.com/nginx/admin-guide/load-balancer/http-load-balancer/#resolve)）以及如何标识上游服务器。

例如，通过启用 sticky_route 会话持久性方法和单个运行状况检查，一个256 KB的区域可以容纳有关指示数量的上游服务器的信息：

- 128 servers (each defined as an IP‑address:port pair)
- 88 servers (each defined as hostname:port pair where the hostname resolves to a single IP address)
- 12 servers (each defined as hostname:port pair where the hostname resolves to multiple IP addresses)

### 使用DNS配置HTTP负载平衡

对于在 server 指令中用域名标识的 upstream 组中的服务器，NGINX Plus可以监视相应DNS记录中IP地址列表的更改，并自动将更改应用于上游组的负载平衡，而无需重新启动。这可以通过在 http 块中包含 [`resolver`](https://nginx.org/en/docs/http/ngx_http_core_module.html#resolver) 指令以及 server 指令的  [`resolve`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#resolve) 参数来完成：

```
http {
    resolver 10.0.0.1 valid=300s ipv6=off;
    resolver_timeout 10s;
    server {
        location / {
            proxy_pass http://backend;
        }
    }
    upstream backend {
        zone backend 32k;
        least_conn;
        # ...
        server backend1.example.com resolve;
        server backend2.example.com resolve;
    }
}
```

## 健康检测

参数 [`max_fails`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#max_fails), [`fail_timeout`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#fail_timeout), [`slow_start`](https://nginx.org/en/docs/http/ngx_http_upstream_module.html#slow_start) 是用于慢健康检测的，详细说明请参考上文。

主动的健康检测只有 NGINX Plus 才支持，具体参考：

https://docs.nginx.com/nginx/admin-guide/load-balancer/http-health-check/#hc_active