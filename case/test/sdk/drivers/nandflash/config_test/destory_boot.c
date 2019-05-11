#include "destory_boot.h"

//param1:Ҫ�����Ŀ��� 
//param2:������ʽ��־��0Ϊָ����Ų�����1Ϊָ����Χ����
//param3:��ſ�ŵ�����
//return:  1:ok    0:fail   

void static hex_printf2(  const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
//	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  /*%4d*/\n",iLoop);
		}
	}
	printf("\n");

}

int static m_eraseblks(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	unsigned int i;
	if (0==flag)
	{
		//ָ����Ž���erase
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBlk_Erase(m_param[i], 1);
		}
	} 
	else
	{
		//ָ����Χ����erase
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBlk_Erase(*m_param+i, 1);
		}
	}
	return 0x01;
}

//param1:Ҫ����д�Ŀ��� 
//param2:����д��ʽ��־��0Ϊָ���������д��1Ϊָ����Χ����д,ֻд������ǰ4��sector
//ע��д�ĵ�λΪpage������Ҫ��4��sector
//param3:��ſ�ŵ�����
//return:  1:ok    0:fail  
int static m_phywriteblks(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	unsigned int i;
	unsigned char *TmpBuf=(void *)malloc(4*512);
	memset(TmpBuf,0xaa,4*512);
    //hex_printf2(TmpBuf,512);
    
	if (0==flag)
	{
		//ָ����Ž���phywrite
		for (i=0;i<sum_blks;i++)
		{
		    //printf("m_param[%d]==%5d\n",i,m_param[i]);
            //NAND_PhyBoot_Read(m_param[i],0,1,testbuf);
            //hex_printf2(testbuf,512);
            //hex_printf2(TmpBuf,512);
            //NAND_PhyBlk_Erase(m_param[i],1);
			NAND_PhyBoot_Write(m_param[i],0,8,TmpBuf);
            //NAND_PhyBoot_Read(m_param[i],0,1,testbuf);
            //hex_printf2(testbuf,512);
		}
	} 
	else
	{
		//ָ����Χ����phywrite
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBoot_Write(*m_param+i, 0,8, TmpBuf);
		}
	}
	free(TmpBuf);
	return 0x01;
}

//param1:Ҫ�ƻ��Ŀ��� 
//param2:�ƻ���ʽ��־��0Ϊ������1Ϊ��������д,2Ϊ��д
//param3:��ſ�ŵ�����
//return:  1:ok    0:fail  
int static assign_destory(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	switch (flag)
	{
	case 0:
		m_eraseblks(sum_blks,0,m_param);
		break;
	case 1:
		m_eraseblks(sum_blks,0,m_param);
		m_phywriteblks(sum_blks,0,m_param);
		break;
	case 2:
		m_phywriteblks(sum_blks,0,m_param);
		break;
	default:
		break;
	}
	return 0x01;
}

//param1:Ҫ�ƻ��Ŀ��� 
//param2:�ƻ���ʽ��־��0Ϊ������1Ϊ��������д,2Ϊ��д
//param3:��ʼ�ƻ�ָ��
//return:  1:ok    0:fail  
int static range_destory(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	switch (flag)
	{
	case 0:
		m_eraseblks(sum_blks,1,m_param);
		break;
	case 1:
		m_eraseblks(sum_blks,1,m_param);
		m_phywriteblks(sum_blks,1,m_param);
		break;
	case 2:
		m_phywriteblks(sum_blks,1,m_param);
		break;
	default:
		break;
	}
	return 0x01;
}

int static random_destory(unsigned int flag,int *m_p)
{
    unsigned char i=0;
    unsigned char j;
    unsigned char k1,k2;
    unsigned int a;
    unsigned int *m_param=m_p;
    srand((unsigned int)time(0));
    k1=rand()%4;//�õ�mbrcҪ�ƻ��Ŀ���
    while (i<k1)
	{
		srand((unsigned int) time(0));
		a=rand()%4;
		for (j=0;j<i;j++)
		{
			if (a==m_param[j])
			{
				break;
			}
		}
		if (i==j)
		{
			m_param[i]=a;
			i++;
		}
	}//ѭ��������õ�����Ŀ��
    srand((unsigned int)time(0));
    k2=rand()%4;//�õ�brecҪ�ƻ��Ŀ���
    i=0;
    m_param=m_param+k1;
    while (i<k2)
	{
		srand((unsigned int) time(0));
		a=rand()%4+4;
		for (j=0;j<i;j++)
		{
			if (a==m_param[j])
			{
				break;
			}
		}
		if (i==j)
		{
			m_param[i]=a;
			i++;
		}
	}
    //����whileѭ�������������еõ�Ҫ�ƻ��Ŀ飬�һ���
    k2=k1+k2;//�õ��ܹ�Ҫ�ƻ��Ŀ���
    printf("<Test Info> random destory %d blks: ",k2);
    for(i=0;i<k2;i++)
    {
        printf("%3d ",m_p[i]);
    }
    printf("\n");
    assign_destory(k2,flag,m_p);
    return 0x00;
}

//return 0:ok 1:fail
int Destory_xx_block(unsigned int sum_param,char * param_buf)
{
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;
	
	int * p_blk=&param[2];//�õ������Ϣ
	int destoryblk_num=0;
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//��ɲ�����ת����char-->int

	//NAND_Init();

	switch (param[0])
	{
	case 0:
		/////////ָ���ƻ�ģ�麯��
		printf("<Test Info> assign_destory begin...\n");
		destoryblk_num=sum_param-2;//�õ�Ҫ�ƻ��Ŀ���
		assign_destory(destoryblk_num,param[1],p_blk);
		break;
	case 1:
		/////////ָ����Χ�ƻ�����
		printf("<Test Info> range_destory begin...\n");
		destoryblk_num=param[3]-param[2]+1;
		range_destory(destoryblk_num,param[1],p_blk);
		break;
	case 2:
		////////����ƻ�����
		printf("<Test Info> random_destory begin...\n");
        unsigned int p[6];
        random_destory(param[1],p);
		break;
	default:
		break;
	}

	return 0x00;

}