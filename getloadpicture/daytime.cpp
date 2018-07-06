//访问daytime
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <netdb.h>
 
int main(int argc, char **argv)
{
	if (argc != 2) {
	     fprintf(stderr, "Usage: %s hostname\n", basename(argv[0]));
	     return 1;
	}
	
	char *host = argv[1];
	
        //获取目标主机地址信息
	struct hostent *hostinfo = gethostbyname(host);
	assert(hostinfo);
 
        //获取daytime服务信息	
	struct servent *servinfo = getservbyname("daytime","tcp");
	assert(servinfo);
	printf( "daytime port is %d\n", ntohs( servinfo->s_port ) );
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = servinfo->s_port;
 
        //因为h_addr_list本身就是网络字节序的地址列表，故直接赋值
    address.sin_addr = *(struct in_addr*)*hostinfo->h_addr_list;
	
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	int result = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
	
	assert(result != -1);
	
	char buffer[128];
	result = read(sockfd, buffer, sizeof(buffer));
	assert(result > 0);
	
	buffer[result] = '\0';
	fprintf(stderr, "the day time is:%s\n", buffer);
	close(sockfd);
	
	
	return 0;
}