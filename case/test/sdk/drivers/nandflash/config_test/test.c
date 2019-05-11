
#include "test.h"



extern int Write_flash(unsigned int sum_param,char *param_buf);
extern int Read_flash(unsigned int sum_param,char *param_buf);
extern int RW_Speed(unsigned int sum_param,char * param_buf);
extern int Destory_xx_block(unsigned int sum_param,char * param_buf);
//extern  INT32U bSet_Value_ECC(INT8U *Buffer);
extern  int m_bootcheck(void);
extern  int Destory_FW(unsigned int sum_param,char * param_buf);
extern int PowerOff_Protect(unsigned int sum_param,char * param_buf);


int main(void)
{
	FILE * fp;
	//char line[100];
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
    INT8U Buf[256];
	
	char module_num_buf[10],module_buf[50];
	char item_num_buf[10],item_buf[50];
	unsigned int module_num=1;
	unsigned int item_num=1;
	unsigned int line_num=0;
	unsigned int param_num=1;
    
    //////////////////////////////////////////////////
    //�ڴ�����ϴ�test��ϵͳ���������
    NAND_Init();
    //M_ReadBOOTData(Buf);
    if(0==m_bootcheck())
    printf("<Test Info> BootData is ok!!\n");


	char *tp=NULL;
	char tmp_path[100]="/mnt/udisk/";
	char *fun_name[6]={"Destory_xx_block","Destory_FW","Write_flash","Read_flash","RW_Speed","PowerOff_Protect"};
	
loop:
    line_num=0;//���뵽�µ�ģ�������ļ���Ҫ���¸�ֵ
	memset(tmp_path,0,sizeof(char)*100);
	strcat(tmp_path,"/mnt/udisk/");//���뵽�µ�ģ�������ļ���Ҫ���¸�ֵ
	fp = fopen("/mnt/udisk/module.cfg", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	if((read = getline(&line, &len, fp)) != -1)
	{
		//printf("Retrieved line of length %zu :\n", read);
		printf("<Test Info> %s", line);
		sscanf(line,"%*[^ ] %[^';'];%*s",module_num_buf);
		module_num=atoi(module_num_buf);
		//printf("%d\n",item_num);
		line_num++;
	}
	else
		return EXIT_FAILURE;
	while (line_num<module_num)
	{
		getline(&line, &len, fp);//ʹfp���뵽��һ��
		line_num++;
	}//ѭ��������line_num��item_num��ȣ�ͬʱfpָ����Ҫ������Ŀ�Ŀ�ʼλ��
    printf("<Test Info> jump to testmodule : %5d\n",line_num);

	if ((read = getline(&line, &len, fp)) != -1)
	{
		printf("<Test Info> %s", line);
		////////////
		sscanf(line,"%*[^ ] %[^';'];%*s",module_buf);//module_buf���ģ�����֣�ͬʱҲ�ǹ��ܺ�������
		strcat(tmp_path,module_buf);
		strcat(tmp_path,".cfg");//�õ�������ģ��������ļ���
		printf("<Test Info> tmp_path==%s\n",tmp_path);
		fclose(fp);
		fp=NULL;
		fp = fopen(tmp_path, "r");
		if (fp == NULL)
		{
		    printf("<Test Info> open item cfg file fail !!\n");
			exit(EXIT_FAILURE);
		}
		line_num=0;
		if((read = getline(&line, &len, fp)) != -1)
		{
			//printf("Retrieved line of length %zu :\n", read);
			printf("<Test Info> %s", line);
			sscanf(line,"%*[^ ] %[^';'];%*s",item_num_buf);
			item_num=atoi(item_num_buf);
			//printf("%d\n",item_num);
			line_num++;
		}
		while (line_num<item_num)
		{
			getline(&line, &len, fp);//ʹfp���뵽��һ��
			line_num++;
		}//ѭ��������line_num��item_num��ȣ�ͬʱfpָ����Ҫ������Ŀ�Ŀ�ʼλ��
		printf("<Test Info> jump to testitem : %5d\n",line_num);
		if ((read = getline(&line, &len, fp)) != -1)
		{
			printf("<Test Info> %s", line);
			sscanf(line,"%*[^ ] %[^';'];%*s",item_buf);
			tp=item_buf;//item_buf�зŵ��Ǹ����������ַ�����ÿ������������'\0'����
			while (*tp!='\0')
			{
				if (*tp==',')
				{
					*tp='\0';
					param_num++;
				}
				tp++;
			}//ѭ��������õ����������Ͳ����ַ���
			/////
			unsigned char ii;
			unsigned char slectfun_num;
			for (ii=0;ii<6;ii++)
			{
				if (strcmp(fun_name[ii],module_buf)==0)
				{
					slectfun_num=ii;
					//printf("slectfun_num== %5d\n",ii);
					break;
				}
			}
			switch (slectfun_num)
			{
			case 0:
				printf("<Test Info> run Destory_xx_block\n");
                if(0==Destory_xx_block(param_num,item_buf))
				    printf("<Test Info> Destory_xx_block ok!!\n");
				else 
				    printf("<Test Info> Destory_xx_block error!!\n");
				break;
			case 1:
				printf("<Test Info> run Destory_FW\n");
                if(0==Destory_FW(param_num,item_buf))
				    printf("<Test Info> Destory_FW ok!!\n");
				else 
				    printf("<Test Info> Destory_FW error!!\n");
				break;
			case 2:
				printf("<Test Info> run Write_flash\n");
				if(0==Write_flash(param_num,item_buf))
				    printf("<Test Info> Write_flash ok!!\n");
				else 
				    printf("<Test Info> Write_flash error!!\n");
				break;
			case 3:
				printf("<Test Info> run Read_flash\n");
                if(0==Read_flash(param_num,item_buf))
				    printf("<Test Info> Read_flash ok!!\n");
				else 
				    printf("<Test Info> Read_flash error!!\n");
				break;
			case 4:
				printf("<Test Info> run RW_Speed\n");
                if(0==RW_Speed(param_num,item_buf))
				    printf("<Test Info> RW_Speed ok!!\n");
				else 
				    printf("<Test Info> RW_Speed error!!\n");
				break;
			case 5:
				printf("<Test Info> run PowerOff_Protect\n");
                PowerOff_Protect(param_num,item_buf);
				break;
			default:
				printf("<Test Info> other condition happened!\n");
				break;
			}

			fclose(fp);
			fp=NULL;

			item_num++;
            //int fp2;
			char newitemnum_buf[10];
			sprintf(newitemnum_buf,"%d",item_num);

            printf("======> tmp_path== %s\n",tmp_path);
            
            fp=fopen(tmp_path, "rb+");
            if(fp==NULL)
            {
                printf("<Test Info> open fp2 fail !!\n");
                exit(EXIT_FAILURE);
            }
            if(item_num<=9&&item_num>0)
			{
				fseek(fp,15,SEEK_SET);         
            }
			else if(item_num>=10&&item_num<=99)
			{
				fseek(fp,14,SEEK_SET);              
            }
            
			else
            {
				fseek(fp,13,SEEK_SET);               
            }

            fputs(newitemnum_buf,fp);


            /*fp2=open(tmp_path,O_RDWR);
            
			
			if (fp2 == -1)
            {
                printf("<Test Info> open fp2 fail !!\n");
                exit(EXIT_FAILURE);
            }         
				
			if(item_num<=9&&item_num>0)
			{
				lseek(fp2,15,SEEK_SET);
                write(fp2,newitemnum_buf,1);
            }
			else if(item_num>=10&&item_num<=99)
			{
				lseek(fp2,14,SEEK_SET);
                write(fp2,newitemnum_buf,2);
            }
            
			else
            {
				lseek(fp2,13,SEEK_SET);
                write(fp2,newitemnum_buf,3);
            }

            if(fsync(fp2)==-1)
            {
                printf("<Test Info> fsync fail !!\n");
                exit(EXIT_FAILURE);
            }
            close(fp2);
            ///////////////////////////////////
            fp=fopen(tmp_path, "rb+");
            */
            

		} 
		else
		{
			printf("<Test Info> test item over!!\n");
			fclose(fp);
			fp=NULL;
			module_num++;
			char newmodulenum_buf[10];
			sprintf(newmodulenum_buf,"%d",module_num);

			fp = fopen("/mnt/udisk/module.cfg", "rb+");
			if (fp == NULL)
				exit(EXIT_FAILURE);
			if(module_num<=9&&module_num>0)
				fseek(fp,18,0);
			else if(module_num>=10&&module_num<=99)
				fseek(fp,17,0);
			else
				fseek(fp,16,0);
			fputs(newmodulenum_buf,fp);

           /*
            if(fsync(fp)==-1)
            {
                printf("<Test Info> fsync fail !!\n");
                exit(EXIT_FAILURE);
            }*/

			fclose(fp);
			fp=NULL;

			//////////Ҫ�Զ�������һ��ģ��
			goto loop;
		}
	} 
	else
	{
		printf("<Test Info> test module over!!\n");
	}

	fclose(fp);
	if (line)
		free(line);
		
	////////////////////////////////////////////////////
	//�ڴ���ӱ��β��Ժ��ϵͳcheck�Ĵ���
	//�����ӡ��Ϣ
	///////////////////////////////////////////////////        
    m_bootcheck();
	///////////////////////////////////////////////////
	//�ڴ���ӹر�ϵͳ�Ĵ���
	//
	////////////////////////////////////////////////////
    //sleep(100);
    //M_SetupWD(1);
	
	return EXIT_SUCCESS;


}
