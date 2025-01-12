#if 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <pthread.h>
#include "am_socket.h"
#include "am_types.h"
static int Check_cmd_and_set_sunpath(char *cmd)
{
	if(!strcmp(cmd,"cgi_setting_server"))
	{
		strcpy(unc.sun_path,"/tmp/cgi_setting_client_write_data");
		return 1;
	}
	else if(!strcmp(cmd,"music_setting_server"))
	{
		strcpy(unc.sun_path,"/tmp/set_EQ_client_write_data");
		return 2;
	}	
	else if(strstr(cmd, "ap_name")!=0)
	{
		strcpy(unc.sun_path,"/tmp/cgi_setting_client_ap_name");
	}
	else 
	{
		char sun_path_temp[512]={"/tmp/"};
		if(strstr(cmd,"/"))
		{
               	char sun_path[512]={0};
			int i=0;
			if(strlen(cmd)<sizeof(sun_path))
			   strncpy(sun_path,cmd,strlen(cmd));
			else
			  strncpy(sun_path,cmd,sizeof(sun_path));
			while(1)
			{
                         if(sun_path[i]=='/')
					sun_path[i]=' ';
			    i++;
			    if(i>strlen(sun_path)-1)
					break;
			}
			strcat(sun_path_temp,sun_path);
		}
		else
			strcat(sun_path_temp,cmd);
		strncpy(unc.sun_path,sun_path_temp,strlen(sun_path_temp));
	}
	return 1;
}

EXPORT_SYMBOL
void close_websetting_socket_cli_fd(void)
{
	if(socket_cli_fd > 0)
	{
		close(socket_cli_fd);
		unlink(unc.sun_path);
		socket_cli_fd=-1;
	}		
}

EXPORT_SYMBOL
void websetting_client_write(char *cmd_string)
{
	write(socket_cli_fd,cmd_string,strlen(cmd_string));
}

EXPORT_SYMBOL
int websetting_client_read(char *receiveline )
{
	int receivesize;
	if((receivesize = read(socket_cli_fd,receiveline,1000)<=0))
	{
		close_websetting_socket_cli_fd();
		return -1;
	}
	else
	{
		//if(strlen(receiveline))
			//printf("%s\n",receiveline);//fprintf(cgiOut,"%s",receiveline);
		close_websetting_socket_cli_fd();
		return 1;
	}	
}

EXPORT_SYMBOL
int create_websetting_client(char *cmd_string)
{
	struct sockaddr_un uns;
	int err,cnt,ret;
	ret=Check_cmd_and_set_sunpath(cmd_string);
	/**
	* setup a communication channel.
	*/
	unc.sun_family = AF_UNIX;
	if ((socket_cli_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}
	/** before bind, must make sure the path file not exist */
	unlink(unc.sun_path);
	if (bind(socket_cli_fd, (struct sockaddr *)&unc, sizeof(unc)) < 0)
	{
	
		close_websetting_socket_cli_fd();
		return -1;
	}
	cnt = 0;
	while(cnt < 10)
	{
		memset(&uns, 0, sizeof(uns));
	    	uns.sun_family = AF_UNIX;
		if(1==ret)
	    		strcpy(uns.sun_path, "/tmp/cgi_setting_server");
		else if(2==ret)
			strcpy(uns.sun_path, "/tmp/music_setting_server");
		err = connect(socket_cli_fd, (struct sockaddr *)&uns, sizeof(uns));
	    	if ( err < 0) 
		{
			sleep(1);
			cnt++;
	    	}
		else
		{
			break;
		}
	}
	if(cnt >= 10)
	{
		close_websetting_socket_cli_fd();
		return -1;
	}
	return 1;
}
#endif

