# 网络硬件

![](images/13-network%20programming.png)

# 计算机网络

计算机网络是一个复杂的系统，它可以被看作是由众多盒子与线组成的地理上的链接
* 系统网（system area network)
* 局域网  (local area network)
* 广域网 (wide area network)
# 以太网

以太网是一种低级别的网络，是由集线器(hub)组成的网络结构， 数据会被广播到全局

![](images/13-network%20programming-1.png)

# 网桥

网桥是连接两个局域网的一种存储与转发设备，在以太网构造的局域网上，最终的寻址是以数据链路层的MAC地址作为标识的(就是用MAC地址可以在局域网上找到一台唯一的机器)，网桥能从发来的数据包中提取MAC信息，并且根据MAC信息对数据包进行有目的的转发，而不采用广播的方式，这样就能减少广播风暴的出现，提升整个网络的效率。

![](images/13-network%20programming-2.png)

# 路由器

连接不同局域网和广域网的设备，路由器有着特定的路由法则，利用路由器可以将数据传输到全世界各个网域

![](images/13-network%20programming-3.png)

![](images/13-network%20programming-4.png)

# 协议

用来规范网络传输的一种共同的约定

# 网络的总体大纲

![](images/13-network%20programming-5.png)


# IP/UDP/TCP
* IP 提供主机到主机之间的通信
* UDP提供进程到进程之间的不可靠通信
* TCP提供进程到进程的可靠通信


# Socket 网络编程

## 1.IP 地址
```c
struct in_addr{
	uint32_t s_addr;
};
```

```c
// l is long (32 bit), s is short (16 bit)
htol() // from host to network -> 计算机序到大端序 32 位 ip地址转换
hots() // from host to network -> 计算机序到大端序 16 位 端口号
ntol() // from network to host -> 大端序到计算机序 32 位 ip地址转换
ntos() // from network to host -> 大端序到计算机序 16 位 端口号
```

```c
inet_pton() // 点分十进制字符串转 ip地址二进制数字
inet_ntop() // ip二进制数字转点分十进制字符串
```

## DNS 域名解析系统(domain names system)

![](images/13-network%20programming-6.png)

## TCP
* tcp连接是点对点的连接,连接了两个进程
* tcp连接是全双工的,可以同时双方通信
* tcp连接是可信赖的,是建立在连接的基础上的

**套接字(socket)是连接的端点**, 其格式为 (ip地址:port)

端口号是一个十六位的整数标志着一个进程, 端口号分为两种
* 临时端口:1024-65535
* 公认端口:0-1023

常见的公认端口:
* 7:echo server
* 22:ssh
* 25:smtp
* 80:http
* 443:https

## 连接

![](images/13-network%20programming-7.png)

# socket接口

* 对于系统内核来说:socket就是网络连接的两个端点
* 对于应用程序来说:socke就是一个文件描述符,允许读/写

![](images/13-network%20programming-8.png)

## socke地址结构体

```c
struct sockaddr{
	uint16_t sa_family; // 协议族
	char sa_data[14]; // 地址信息
};

struct sockaddr_in{
	uint16_t sim_family; // 协议族
	uint16_t sin_port; // 端口号
	struct in_addr sin_addr; // ip地址结构体
	unsigned char sin_zero[8]; // 填充0
};
```

![](images/13-network%20programming-9.png)

![](images/13-network%20programming-10.png)

## tcp连接的过程

![](images/13-network%20programming-11.png)

## 建立套接字

`int socket(int domain, int type, int protocol);`
* domain: 32位的ipv4地址
* type: 使用哪种方式通信:TCP(字节流), UDP:数据报
* protocol: 协议,一般设定为0
* 返回套接字的文件描述符

## 绑定

`typedef struct socketaddr SA;`
`int bind(int sockfd, SA *addr, socklen_t addrlen);`
* 将套接字绑定到ip地址上

## 监听

`int listen(int sockfd, int backlog)`
* backlog 是最大连接数
## 等待

`int accept(int listenfd, SA *addr, int *addrlen)`
* 返回通信对象的文件描述符,服务器利用这个套接字描述符与客户端通信

---
## 连接

`int connect(int clientfd, SA *addr, socket_t addrlen)`
* 客户端连接服务器调用的函数 
* SA是目标对象(服务器的地址)
* 返回一个整数, 表示连接是否成功

![](images/13-network%20programming-12.png)

## 断开连接

使用close函数, 发生一个EOF数据

## getaddrinfo函数

* 代替了 `gethostbyname` 和`getservbyname` 函数
* 适用于ipv4和ipv6
* 多线程环境下更好
* 更加复杂
* 很多情况下不太方便使用

```c
int getaddrinfo(
	const char *host, // 真实的地址字符串,域名,点分十进制,冒号分十六进制
	const char *service, // 服务器端口或名字
	const struct addrinfo *hints, //输入参数
	struct addrinfo **result // 输出链表
)

void freeaddrinfo(struct addrinfo *result); // 释放链表
const char *gai_strerror(int errcode); //返回错误信息
```

![](images/13-network%20programming-13.png)

* addrinfo结构体

![](images/13-network%20programming-14.png)

* getnameinfo结构体

![](images/13-network%20programming-15.png)

# client

![](images/13-network%20programming-17.png)

![](images/13-network%20programming-18.png)

# 服务器

![](images/13-network%20programming-16.png)

![](images/13-network%20programming-19.png)

![](images/13-network%20programming-20.png)

---

![](images/13-network%20programming-21.png)

![](images/13-network%20programming-22.png)

![](images/13-network%20programming-23.png)

# testing server

```bash
telnet <host> <port>
```

# http

http可以是静态的,也可以是动态的

## url:统一资源定位符
* 由两部分组成,第一部分是域名,第二部分是定位符

## HTTP请求
* `<method> <uri> <version>`
* `<method>` 是 `GET, POST, OPTIONS, HEAD, PUT, DELETE, TRACE` 中的一种
* `<uri>` 是一种定位符代理, `url` 是 `uri`  的一种
* `<version>` 是http的版本,有`http/1.0, http/1.1`两种

### 请求头
`<header name>: <header data>`

![](images/13-network%20programming-24.png)

## HTTP响应
HTTP响应是在响应头后面,空一行书写的

![](images/13-network%20programming-25.png)

 ## GET参数

* 参数列表以`"?"` 开始
* 参数分割以 `"&"`作为标志
* 空格以`+`或`%20`表示


