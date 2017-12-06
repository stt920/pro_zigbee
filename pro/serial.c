#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <termios.h> //set baud rate

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

//#define rec_buf_wait_2s 2
#define buffLen 1024
#define rcvTimeOut 2

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

int readDataTty(int fd, char *rcv_buf, int TimeOut, int Len)
{
	int retval;
	fd_set rfds;
	struct timeval tv;
	int ret, pos;
	tv.tv_sec = TimeOut / 1000;  //set the rcv wait time  
	tv.tv_usec = TimeOut % 1000 * 1000;  //100000us = 0.1s  

	pos = 0;
	while (1)
	{
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			perror("select()");
			break;
		}
		else if (retval)
		{
			ret = read(fd, rcv_buf + pos, 1);
			if (-1 == ret)
			{
				break;
			}

			pos++;
			if (Len <= pos)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return pos;
}

int sendDataTty(int fd, char *send_buf, int Len)
{
	ssize_t ret;

	ret = write(fd, send_buf, Len);
	if (ret == -1)
	{
		printf("write device error\n");
		return -1;
	}

	return ret;
}



int main(int argc, char** argv)
{
	int iSetOpt = 0;//SetOpt 的增量i

	

	int fdSerial = 0;

	//openPort
	fdSerial = open("/dev/s3c2410_serial2", O_RDWR | O_NOCTTY | O_NDELAY );
	if (-1 == fdSerial)
	{
		perror("Can't Open Serial Port ----- ttys1");
		return(-1);
	}
	else
	{
		printf("open ttyS1 .....\n");
	}

	if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}
	printf("Serial fdSerial=%d\n", fdSerial);

	tcflush(fdSerial, TCIOFLUSH);//清掉串口缓存
	fcntl(fdSerial, F_SETFL, 0);


	char buffRcvData[buffLen] = { 0};
        char buffRcvData2[buffLen] = { 0};


	unsigned int readDataNum = 0;
        unsigned int sendDataNum = 0;
	buffRcvData[0] = 's';
	buffRcvData[1] = 't';
	buffRcvData[2] = 'a';
	buffRcvData[3] = 'r';
	buffRcvData[4] = 't';
//////////////////////////////////////////////////////////////////////////////////////////////////////
	sendDataNum=sendDataTty(fdSerial, buffRcvData, 5);
	printf("sendNum:=%d\n",sendDataNum);
	 int i;
	//for(i=0;i<5;i++)
			//printf("%c",buffRcvData[i]);
	//printf("\n");
	
        sleep(1);
	while (1){
		
		//readDataNum = readDataTty(fdSerial, buffRcvData2, rcvTimeOut, buffLen);
		readDataNum=read(fdSerial,buffRcvData2,5);
		printf("readNum:=%d\n",readDataNum);
		for(i=0;i<5;i++)
		{
			printf("%c",buffRcvData2[i]);
			
		}
		printf("\n");

		//tcflush(fdSerial, TCIOFLUSH);
		
		sendDataNum=sendDataTty(fdSerial, buffRcvData2, readDataNum);
		printf("sendNum:=%d\n",sendDataNum);
           	//buffRcvData2[buffLen] = { 0};
		sleep(1);
         
		/*
		printf("rcv:%d",readDataNum);	
		if(readDataNum>0)
		{	
		for(i=0;i<10;i++)
			printf("%c ",buffRcvData[i]);
		printf("\n");


		sendDataNum=sendDataTty(fdSerial, buffRcvData, readDataNum);
		
		printf("sed:%d\n",sendDataNum);
		for(i=0;i<10;i++)
			printf("%c ",buffRcvData[i]);
		
		printf("\n");
		}
		readDataNum=0;
         */
		
	}
	
	
	return 1;
}
