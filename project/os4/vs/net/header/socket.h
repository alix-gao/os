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

//IPV4��socket��ַ�ṹ
typedef struct sockaddr_in
{
    SOCKET_ADRESS_FAMILY sin_family;
    os_u16 sin_port;
    os_u32 sin_ip;
    char   sin_zeros[8];

};

//socket ͨ�ýṹ�壬֧��IPV4��IPV6
typedef struct sockaddr
{
    SOCKET_ADRESS_FAMILY sin_family;
    os_u8   sa_data[14];
};


//socket() �������������׽��֣�ȷ���׽��ֵĸ�������
SOCKET socket(os_u32 af, os_u32 type, os_u32 protocol);

//��������Ҫ�� bind() �������׽������ض���IP��ַ�Ͷ˿ڰ�������ֻ��������������IP��ַ�Ͷ˿ڵ����ݲ��ܽ����׽��ִ���
//IP��ַ����ΪINADDR_ANY,����ϵͳ�ں����Զ�ָ��
//portΪ0������ϵͳ�Զ�ָ��1024~5000֮������˿ں�
os_ret bind(SOCKET sock, sockaddr *addr, int addrlen);


//�ͻ���Ҫ�� connect() �������������������
os_ret connect(SOCKET sock, sockaddr *serv_addr, int addrlen);

os_void close(SOCKET sock);

//shutdownֻ�رն�д�շ�ͨ�������ͷ�sock��Դ
enum SOCKET_SHNTDOWN_TYPE
{
    SD_RECEIVE = 0,   //�رս���
    SD_SEND,
    SD_BOTH,
};
os_ret shutdown(SOCKET sock, os_u32 how);

//int getpeername(int sockfd,struct sockaddr* addr,int* addrlen);
//int gethostname(char*hostname,size_t size);



//���ڷ������˳���ʹ�� bind() ���׽��ֺ󣬻���Ҫʹ�� listen() �������׽��ֽ��뱻������״̬���ٵ��� accept() �������Ϳ�����ʱ��Ӧ�ͻ��˵������ˡ�

os_ret listen(SOCKET sock, os_u32 backlog);   //backlog Ϊ������е���󳤶ȡ�

//���׽��ִ��ڼ���״̬ʱ����ϵͳ����accept()�Ƚ������е㸴�ӡ���Զ�̵�����������ͼʹ��connect()������ʹ��
/*listen()���ڼ����Ķ˿ڡ��������ӽ����ڶ����еȴ���ֱ��ʹ��accept()������������accept()
 ֮�󣬽��᷵��һ��ȫ�µ��׽ӿ��ļ�������������������������ӡ�����������ͬһ������
 ��˵��������������ļ���������ԭ�ȵ�һ���ļ����������ڼ�����ָ���Ķ˿ڣ��µ��ļ���
 ����������������send()��recv()��
 ��ͨ�� accept() ���������տͻ�������*/
//accept() ����һ���µ��׽������Ϳͻ���ͨ�ţ�addr �����˿ͻ��˵�IP��ַ�Ͷ˿ںţ��� sock �Ƿ������˵��׽��֣����ע�����֡�����Ϳͻ���ͨ��ʱ��Ҫʹ����������ɵ��׽��֣�������ԭ���������˵��׽��֡�
//accept() ����������ִ�У�������벻�ܱ�ִ�У���ֱ�����µ�������
SOCKET accept(SOCKET sock,  sockaddr *addr, os_u32 *addrlen);

//�ӷ������˷�������ʹ�� send() ����
//sock ΪҪ�������ݵ��׽��֣�buf ΪҪ���͵����ݵĻ�������ַ��len ΪҪ���͵����ݵ��ֽ�����flags Ϊ��������ʱ��ѡ��
//flags ����һ������Ϊ 0 �� NULL
os_ret send(SOCKET sock,  os_u8 *buf, os_u32 len, os_u32 flags);


//�ڿͻ��˽�������ʹ�� recv() ����������ԭ��Ϊ��
os_ret recv(SOCKET sock, os_u8 *buf, os_u32 len, os_u32 flags);

/*
sock�����ڴ���UDP���ݵ��׽��֣�
buf��������������ݵĻ�������ַ��
nbytes�����������ݵĳ��ȣ����ֽڼƣ���
flags����ѡ���������û�пɴ���0��
to������Ŀ���ַ��Ϣ�� sockaddr �ṹ������ĵ�ַ��
addrlen�����ݸ����� to �ĵ�ֵַ�ṹ������ĳ��ȡ�
*/
os_ret sendto(SOCKET sock, os_u8 *buf, os_u32 nbytes, os_u32 flags, sockaddr *to, os_u32 addrlen);

//���������ڴӣ������ӣ��׽ӿ��Ͻ������ݣ����������ݷ���Դ�ĵ�ַ
//����SOCK_STREAM�����׽ӿڣ�����from��fromlen����
os_ret recvfrom(SOCKET sock, os_u8 *buf, os_u32 nbytes, os_u32 flags, sockaddr *from, os_u32 *addrlen);

#endif
