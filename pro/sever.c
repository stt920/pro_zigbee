#include <unistd.h>  
#include <stdio.h>  
#include <arpa/inet.h>  
#include <sys/socket.h>  
#include <string.h> 
/*--------------------------------------*/
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h> //set baud rate

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

//#define rec_buf_wait_2s 2
#define buffLen 1024

/*--------------------------------------*/
#define LOCALPORT 1234  

int setOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

int main(int argc,char *argv[])  
{  
    int mysocket,len;  
    int i=0;  
    struct sockaddr_in addr;  
    int addr_len;  
    char msg[200];  
    char buf[300];  
    int j;
    mysocket=socket(AF_INET,SOCK_DGRAM,0);
    addr_len=sizeof(struct sockaddr_in);  
    bzero(&addr,sizeof(addr));  
    addr.sin_family=AF_INET;  
    addr.sin_port=htons(LOCALPORT);  
    bind(mysocket,(struct sockaddr *)&addr,sizeof(addr)); 
/*-----------------serial setting---------------------*/
    int fdSerial = 0;
      int iSetOpt = 0;//SetOpt 的增量i
    fdSerial = open("/dev/s3c2410_serial2", O_RDWR | O_NOCTTY | O_NDELAY );

	if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}
    printf("Serial fdSerial=%d\n", fdSerial);
    tcflush(fdSerial, TCIOFLUSH);//清掉串口缓存
    fcntl(fdSerial, F_SETFL, 0);
    char buffRcvData[buffLen] = { 0};
    unsigned int readDataNum = 0;
    unsigned int sendDataNum = 0;
/*-----------------serial setting---------------------*/

    while(1)  
    {      
	
 	j=0;
        bzero(msg,sizeof(msg));  
        len = recvfrom(mysocket,msg,sizeof(msg),0,(struct sockaddr *)&addr,(socklen_t*)&addr_len);  
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
}  

/*************Linux and Serial Port *********************/

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


