/***************************************************************
 * copyright (c) 2009, sicui
 * all rights reserved.
 *
 * file name   : socket.h
 * version     : 1.0
 * description :
 * author      : sicui
 * date        : 2016-5-11
 ***************************************************************/

#ifndef __SOCKET_H__
#define __SOCKET_H__

typedef void *SOCKET;
#define INADDR_ANY (0)

enum SOCKET_ADRESS_FAMILY
{
    AF_INET,
    AF_INTT6,
};

enum SOCKET_TYPE
{
    SOCKET_STREAM,
    SOCKET_DGRAM,
};

enum SOCKET_PROTOCOL
{
    IPPROTO_TCP,
    IPPROTO_UDP,
};

//IPV4下socket地址结构
typedef struct sockaddr_in
{
    SOCKET_ADRESS_FAMILY sin_family;
    os_u16 sin_port;
    os_u32 sin_ip;
    char   sin_zeros[8];

};

//socket 通用结构体，支持IPV4和IPV6
typedef struct sockaddr
{
    SOCKET_ADRESS_FAMILY sin_family;
    os_u8   sa_data[14];
};


//socket() 函数用来创建套接字，确定套接字的各种属性
SOCKET socket(os_u32 af, os_u32 type, os_u32 protocol);

//服务器端要用 bind() 函数将套接字与特定的IP地址和端口绑定起来，只有这样，流经该IP地址和端口的数据才能交给套接字处理
//IP地址参数为INADDR_ANY,则有系统内核来自动指定
//port为0，则由系统自动指派1024~5000之间任意端口号
os_ret bind(SOCKET sock, sockaddr *addr, int addrlen);


//客户端要用 connect() 函数与服务器建立连接
os_ret connect(SOCKET sock, sockaddr *serv_addr, int addrlen);

os_void close(SOCKET sock);

//shutdown只关闭读写收发通道，不释放sock资源
enum SOCKET_SHNTDOWN_TYPE
{
    SD_RECEIVE = 0,   //关闭接收
    SD_SEND,
    SD_BOTH,
};
os_ret shutdown(SOCKET sock, os_u32 how);

//int getpeername(int sockfd,struct sockaddr* addr,int* addrlen);
//int gethostname(char*hostname,size_t size);



//对于服务器端程序，使用 bind() 绑定套接字后，还需要使用 listen() 函数让套接字进入被动监听状态，再调用 accept() 函数，就可以随时响应客户端的请求了。

os_ret listen(SOCKET sock, os_u32 backlog);   //backlog 为请求队列的最大长度。

//当套接字处于监听状态时，可系统调用accept()比较起来有点复杂。在远程的主机可能试图使用connect()连接你使用
/*listen()正在监听的端口。但此连接将会在队列中等待，直到使用accept()处理它。调用accept()
 之后，将会返回一个全新的套接口文件描述符来处理这个单个的连接。这样，对于同一个连接
 来说，你就有了两个文件描述符。原先的一个文件描述符正在监听你指定的端口，新的文件描
 述符可以用来调用send()和recv()。
 以通过 accept() 函数来接收客户端请求*/
//accept() 返回一个新的套接字来和客户端通信，addr 保存了客户端的IP地址和端口号，而 sock 是服务器端的套接字，大家注意区分。后面和客户端通信时，要使用这个新生成的套接字，而不是原来服务器端的套接字。
//accept() 会阻塞程序执行（后面代码不能被执行），直到有新的请求到来
SOCKET accept(SOCKET sock,  sockaddr *addr, os_u32 *addrlen);

//从服务器端发送数据使用 send() 函数
//sock 为要发送数据的套接字，buf 为要发送的数据的缓冲区地址，len 为要发送的数据的字节数，flags 为发送数据时的选项
//flags 参数一般设置为 0 或 NULL
os_ret send(SOCKET sock,  os_u8 *buf, os_u32 len, os_u32 flags);


//在客户端接收数据使用 recv() 函数，它的原型为：
os_ret recv(SOCKET sock, os_u8 *buf, os_u32 len, os_u32 flags);

/*
sock：用于传输UDP数据的套接字；
buf：保存待传输数据的缓冲区地址；
nbytes：带传输数据的长度（以字节计）；
flags：可选项参数，若没有可传递0；
to：存有目标地址信息的 sockaddr 结构体变量的地址；
addrlen：传递给参数 to 的地址值结构体变量的长度。
*/
os_ret sendto(SOCKET sock, os_u8 *buf, os_u32 nbytes, os_u32 flags, sockaddr *to, os_u32 addrlen);

//本函数用于从（已连接）套接口上接收数据，并捕获数据发送源的地址
//对于SOCK_STREAM类型套接口，忽略from和fromlen参数
os_ret recvfrom(SOCKET sock, os_u8 *buf, os_u32 nbytes, os_u32 flags, sockaddr *from, os_u32 *addrlen);

#endif
