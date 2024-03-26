/* https://www.cnblogs.com/skyfsm/p/7102367.html */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>


#define MAX_EVENT_NUMBER 1024  //event的最大数量
#define BUFFER_SIZE 10      //缓冲区大小
#define ENABLE_ET  1       //是否启用ET模式

/* 将文件描述符设置为非拥塞的  */
int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

/* 将文件描述符fd上的EPOLLIN注册到epoll_fd指示的epoll内核事件表中，参数enable_et指定是否对fd启用et模式 */
void AddFd(int epoll_fd, int fd, bool enable_et)
{
    struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN; //注册该fd是可读的
	if(enable_et)
	{
	    event.events |= EPOLLET;
	}

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //向epoll内核事件表注册该fd
	SetNonblocking(fd);
}

/*  LT工作模式特点：稳健但效率低 */
void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
	int i;
	for(i = 0; i < number; i++) //number: 就绪的事件数目
	{
	    int sockfd = events[i].data.fd;
		if(sockfd == listen_fd)  //如果是listen的文件描述符，表明有新的客户连接到来
		{
		    struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
			AddFd(epoll_fd, connfd, false);  //将新的客户连接fd注册到epoll事件表,使用lt模式
		}
		else if(events[i].events & EPOLLIN)	//有客户端数据可读
		{
			// 只要缓冲区的数据还没读完，这段代码就会被触发。这就是LT模式的特点：反复通知，直至处理完成
		    printf("lt mode: event trigger once!\n");
			memset(buf, 0, BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if(ret <= 0)  //读完数据了，记得关闭fd
			{
			    close(sockfd);
				continue;
			}
            printf("get %d bytes of content: %s\n", ret, buf);

		}
		else
		{
		    printf("something unexpected happened!\n");
		}
	}
}

/* ET工作模式特点：高效但潜在危险 */
void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
	int i;
	for(i = 0; i < number; i++)
	{
	    int sockfd = events[i].data.fd;
	    if(sockfd == listen_fd)
		{
		    struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
			AddFd(epoll_fd, connfd, true);  //使用et模式
		}
		else if(events[i].events & EPOLLIN)
		{
			/* 这段代码不会被重复触发，所以我么循环读取数据，以确保把socket读缓存的所有数据读出。这就是我们消除ET模式潜在危险的手段 */
			
		    printf("et mode: event trigger once!\n");
			while(1)
			{
			    memset(buf, 0, BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
				if(ret < 0)
				{
					/* 对于非拥塞的IO，下面的条件成立表示数据已经全部读取完毕，此后epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作 */
					
				    if(errno == EAGAIN || errno == EWOULDBLOCK)
					{
					    printf("read later!\n");
						break;
					}

					close(sockfd);
					break;
				}
				else if(ret == 0)
				{
				    close(sockfd);
				}
				else //没读完，继续循环读取
				{
				    printf("get %d bytes of content: %s\n", ret, buf);
				}
			}
		}
		else
		{
		    printf("something unexpected happened!\n");
		}
	}
}


int main(int argc, char* argv[])
{
    if(argc <= 2)
	{
		printf("usage:  ip_address + port_number\n");
		return -1;
	}
	
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	
	int ret = -1;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	
	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0)
	{
		printf("fail to create socket!\n");
		return -1;
	}
    
	ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
	if(ret == -1)
	{
		printf("fail to bind socket!\n");
		return -1;
	}
	
	ret = listen(listen_fd, 5);
	if(ret == -1)
	{
		printf("fail to listen socket!\n");
		return -1;
	}
	
	struct epoll_event events[MAX_EVENT_NUMBER];
	int epoll_fd = epoll_create(5);  //事件表大小为5
	if(epoll_fd == -1)
	{
		printf("fail to create epoll!\n");
		return -1;
	}
	
	AddFd(epoll_fd, listen_fd, true); //使用ET模式epoll,将listen文件描述符加入事件表
	
	while(1)
	{
		int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if(ret < 0)
		{
			printf("epoll failure!\n");
			break;
		}
		
		if(ENABLE_ET)
		{
			et_process(events, ret, epoll_fd, listen_fd);
		}
		else
		{
			lt_process(events, ret, epoll_fd, listen_fd);  
		}
		
	}
	
	close(listen_fd);
	return 0;

}

