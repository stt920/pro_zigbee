#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/types.h>  

#include <fcntl.h>
#include <termios.h> //set baud rate
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>


#define buffLen 1024
#define rcvTimeOut 2
  
#define SERVER_IP "192.168.1.137"  
#define SERVER_PORT 1000  
  
#define LOCALPORT 1234  
pid_t pid;
int setOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

int main(int argc, char *argv[])  
{  
   /*----------------------------client init---------------------*/
    int sock = -1;  
    int ret  = -1;  
    int addr_len = 0;  
    int j;
    struct sockaddr_in server_addr;    
    sock = socket(AF_INET, SOCK_DGRAM, 0);  
    memset(&server_addr, 0, sizeof(server_addr));   
    server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(SERVER_PORT);  
    inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr.s_addr));  
    addr_len = sizeof(server_addr);  
 /*-----------------------------sever init--------------------------*/
    int mysocket,len;  
    int i=0;  
    struct sockaddr_in addr;  
    int addr_len2;  
    char msg[200];  
    char buf[300];  
   // int j;
    mysocket=socket(AF_INET,SOCK_DGRAM,0);
    addr_len2=sizeof(struct sockaddr_in);  
    bzero(&addr,sizeof(addr));  
    addr.sin_family=AF_INET;  
    addr.sin_port=htons(LOCALPORT);  
    bind(mysocket,(struct sockaddr *)&addr,sizeof(addr)); 
 /*----------------------------serial init---------------------*/

    int iSetOpt = 0;//SetOpt 的增量i
    int fdSerial = 0;
    //openPort
    fdSerial = open("/dev/s3c2410_serial2", O_RDWR | O_NOCTTY | O_NDELAY );

	if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}
    tcflush(fdSerial, TCIOFLUSH);//清掉串口缓存
    fcntl(fdSerial, F_SETFL, 0);
    char buffRcvData[buffLen] = { 0};
    unsigned int readDataNum = 0;
    unsigned int sendDataNum = 0;
 /*---------------test  udp-------------------------------*/
	buffRcvData[0] = 's';
	buffRcvData[1] = 't';
	buffRcvData[2] = 'a';
	buffRcvData[3] = 'r';
	buffRcvData[4] = 't';
     sendto(sock, buffRcvData, 5, 0, (struct sockaddr *)&server_addr, addr_len); 
 /*----------------------------------------------------------------*/
    if(pid=fork()>0)  //client
    {
    while(1)
    {
    sleep(1);
   // printf("client begin\n");
    readDataNum=read(fdSerial,buffRcvData,100);
    if(readDataNum>0)
    {
    printf("client Send data:%d:",readDataNum);  
    for(j=0;j<readDataNum;j++)
		printf("0x%x,",*(buffRcvData+j));
    printf("\n"); 
    }  
    else
    {
	//printf("readDataNum=0\n");
    }
    addr_len = sizeof(server_addr);  
    if(readDataNum>0)

     ret = sendto(sock, buffRcvData, readDataNum, 0, (struct sockaddr *)&server_addr, addr_len);   
     readDataNum=0;
     
     }
    tcflush(fdSerial, TCIOFLUSH);//清掉串口缓存
    }
    else if(pid==0)   // sever
    while(1)  
    {      
	
 	j=0;
        bzero(msg,sizeof(msg));  
        len = recvfrom(mysocket,msg,sizeof(msg),0,(struct sockaddr *)&addr,(socklen_t*)&addr_len2);  
        printf("%d:",i);  
        i++;  
        inet_ntop(AF_INET,&addr.sin_addr,buf,sizeof(buf));  
        printf("message from ip %s",buf); 
        printf("Received message : ");
        for(j=0;j<len;j++)
        printf(" 0x%x,",*(msg+j));  
	printf("\n");
/*-----------------serial send-----------------------*/
 	 tcflush(fdSerial, TCIOFLUSH);      //清掉串口缓存 
	sendDataNum=write(fdSerial, msg, len);
	printf("sendDataNum:%d\n",sendDataNum);
/*-----------------serial send-----------------------*/
    
    }  
    return 0;  
}

int setOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0)
	{
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (nBits)
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch (nEvent)
	{
	case 'O':                     //奇校验
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':                     //偶校验
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':                    //无校验
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch (nSpeed)
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if (nStop == 1)
	{
		newtio.c_cflag &= ~CSTOPB;
	}
	else if (nStop == 2)
	{
		newtio.c_cflag |= CSTOPB;
	}
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}
