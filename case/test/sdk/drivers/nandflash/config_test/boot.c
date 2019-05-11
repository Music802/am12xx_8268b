
//#define Multi_FLAG "MultiSectorBoot" //size of 16 

#include "boot.h"

 
INT32U bSet_Value_ECC(INT8U *Buffer)
{
    INT8U bVaildMbrcNum,bVaildBrecNum;
    INT16U iLoop;
    struct BootBlk_Info  BlkInfo;
    INT8S   MbrcECCBit,BrecECCBit;
    INT8U   Recalimval,BCHTypeVal;
    INT8S   bModfiyValue,bVaildBootCnt;
    INT8S   bMaxSetECCBit;
    bVaildMbrcNum =0;
    bVaildBrecNum = 0;
    //��ȷ�ϵ�ǰ�ķ�Mbrec ,Brec ��Ч����
    BrecECCBit=MbrcECCBit =0x00;
    printf("name    chksum   chksum  MaxBit TtalBit BCHType Reclaim \n");
    for(iLoop =0x00 ;iLoop <4 ;iLoop++)
    {
         
         memcpy(&BlkInfo,Buffer+iLoop*sizeof(struct BootBlk_Info),sizeof(struct BootBlk_Info));
         if(BlkInfo.check ==0x00)
             bVaildMbrcNum++;
         if(MbrcECCBit<BlkInfo.ECC_Bit_Max)
         {
             MbrcECCBit = BlkInfo.ECC_Bit_Max;
         }
         Recalimval =BlkInfo.RecalaimVal;
         BCHTypeVal = BlkInfo.BCHType;
         printf("%s%d   0x%2x   0x%2x ",BlkInfo.name,BlkInfo.blknum,BlkInfo.Flashchecksum,BlkInfo.Calchecksum);
         printf(" %2d     %3d     %3d     %4d  \n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BCHTypeVal,Recalimval);
         
    }
    for(iLoop =0x04 ;iLoop <8 ;iLoop++)
    {
         memcpy(&BlkInfo,Buffer+iLoop*sizeof(struct BootBlk_Info),sizeof(struct BootBlk_Info));
         
         if(BlkInfo.check ==0x00)
             bVaildBrecNum++;
         if(BrecECCBit<BlkInfo.ECC_Bit_Max)
         {
             BrecECCBit = BlkInfo.ECC_Bit_Max;
         }  
         Recalimval =BlkInfo.RecalaimVal;
         BCHTypeVal = BlkInfo.BCHType;
         printf("%s%d   0x%2x   0x%2x ",BlkInfo.name,BlkInfo.blknum,BlkInfo.Flashchecksum,BlkInfo.Calchecksum);
         printf(" %2d     %3d     %3d     %4d  \n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BCHTypeVal,Recalimval);
    }
    printf("VaildBrecNum:%d,VaildMbrcNum:%d\n",bVaildBrecNum,bVaildMbrcNum);

    //bVaildBootCnt ����MPupdate Ĭ�������ļ���ȡ�ã���Сֵ1�� ���ֵ4, Ĭ��ֵ��ʱ��3
    //������ʾ�����ʾ������ֹͣ������
    bVaildBootCnt = 3 ; 
    if(bVaildBootCnt<1 && bVaildBootCnt>4)
    {
         printf("");
         return 0x01;
    }
    /*�ж�ÿ��boot ������ECC error bit ��Ŀ
    bModfiyValue �˱���������MPUpdate �����ļ���ȡ�á�
    */
    bModfiyValue = 0;  //�޷��ű���
    bMaxSetECCBit= Recalimval + bModfiyValue;
    //�ж������ļ������Ƿ�����Դ�ֵ��Ч�����������ʾ������ֹͣ������
    if(bMaxSetECCBit >=BCHTypeVal)
    {        
         printf("");
         return 0x02;
    }

    ////��ʾ����Mbrec,Brec ��Ч�ݵ���������Ч������
    if(bVaildBrecNum <=bVaildBootCnt  || bVaildMbrcNum <=bVaildBootCnt)
    {
         printf("");
         return 0x03;
    }

    //��ʾMbrec Brec ������Bit ���� �����������ʾ������ֹͣ������
    if( BrecECCBit >=bMaxSetECCBit || MbrcECCBit >bMaxSetECCBit )
    {
         printf("");
         return 0x04;
    }

    //��ʾ�ɹ���
    return 0x00;    
}


/*******************************************************************************
* Function:       // INT32S cmp_brec_check_sum(unsigned int)
* Description:
* Input��         // �������
* Output��        // ���� У���,-1 FAIL
* Other��         //
*******************************************************************************/
INT32S static brec_read_check_sum(unsigned short BlockStartNum, unsigned short SectorStartNum, INT8U *BufferAddr,unsigned int MAXSec)
{
	unsigned short n,CheckSum = 0;
	unsigned int m, CurBrecCheckSum;

	/*����brec��У���*/
	for (m = SectorStartNum; m < MAXSec; m++)
	{
		//NAND_PhyBoot_Read(N_PHY_READ,BlockStartNum ,m,0, BufferAddr);
		
		
		
		NAND_PhyBoot_Read(BlockStartNum,m,1,BufferAddr);
		if (m==MAXSec-1)  	/*last Brec sector*/
		{
			for (n = 0;n < 255; n++)       
				CheckSum += BufferAddr[2*n] + BufferAddr[2*n+1] * 0x100;
		}
		else
		{
			for (n = 0;n < 256; n++)       
				CheckSum += BufferAddr[2*n] + BufferAddr[2*n+1] * 0x100;
		}
		//Str_printf("", BufferAddr, SECT_SIZE);
	}
	//printf("CheckSum:$%x,%x,%x\n",CheckSum,MAXSec,m);
	/*�ж��ۼ�������brecУ�����ԭ��brec�д�ŵ�У����Ƿ�һ��*/
	if ((BufferAddr[508] + BufferAddr[509] * 0x100) != 0x55aa)
	{
		return -1;
	}
	if (CheckSum != BufferAddr[510] + BufferAddr[511] * 0x100) 
	{
		return -1;
	}
	CurBrecCheckSum = (unsigned int)CheckSum ;//+ 0x55aa0000
	return  CurBrecCheckSum;
}

//return 0: fail   ��0: ��ȷ�ķ���
int static m_mbrccheck(void)
{
	unsigned int iloop,jloop;
	unsigned char *TmpBuf=(void *)malloc(8*512);
	unsigned int MBRC_SECTOR_NUM,MBRC_Total_Size;
	unsigned int mbrc_tab_checksum=0;
	unsigned int mbrc_checksum=0;
	unsigned int tmpmbrc_checksum=0;
    unsigned int correct_mbrcnum=0;
	unsigned int *t_p=(unsigned int*)TmpBuf;
	int flag=0;
	for (iloop=0;iloop<4;iloop++)
	{
		NAND_PhyBoot_Read(iloop,0,1,TmpBuf);
		if(!strncmp(Multi_FLAG, TmpBuf,16))
        {
            MBRC_SECTOR_NUM = *(volatile unsigned int*)(TmpBuf + 0x28);//must before createTbl()
            MBRC_Total_Size = MBRC_SECTOR_NUM + 1;                     
        }
        else
        {
            MBRC_Total_Size = 1;
            MBRC_SECTOR_NUM = 1;
        }
		for (jloop=0;jloop<512/4-1;jloop++)
		{
			mbrc_tab_checksum+=*t_p++;
		}
        mbrc_tab_checksum+=0x1234;
		if (mbrc_tab_checksum!=*t_p)//˵��mbrc�Ǵ��
		{
			continue;
		}
		else
		{
			flag=1;
			break;
		}
		/*else
		{
			if (MBRC_Total_Size==0x01)
			{
				return mbrc_tab_checksum;
			}
		}
		return brec_read_check_sum(iloop,1,TmpBuf,MBRC_Total_Size);*/
	}
	 if (flag==1)
	 {
		 //˵���ҵ���һ����ȷ��mbrc,�����iloop
		 mbrc_checksum=brec_read_check_sum(iloop,1,TmpBuf,MBRC_Total_Size);

	 } 
	 else
	 {
		 //˵��û��һ��mbrc�ǶԵ�
		 free(TmpBuf);
		 printf("<Test Info> no mbrc is ok !!\n");
		 return 0;
	 }

	 for (jloop=0;jloop<4;jloop++)
	 {
		 tmpmbrc_checksum=brec_read_check_sum(jloop,1,TmpBuf,MBRC_Total_Size);
		 printf("<Test Info> mbrc data blk: %d,curchecksum: %5d,checksum: %5d,%3dkb\n",jloop,tmpmbrc_checksum,mbrc_checksum,MBRC_Total_Size/2);
         if(tmpmbrc_checksum==mbrc_checksum)
            correct_mbrcnum++;
     }
	 free(TmpBuf);
	 return correct_mbrcnum;
}


//return 0: fail   ��0: ��ȷ�ķ���
int static m_breccheck(void)
{
	unsigned int iloop,jloop;
	unsigned char *TmpBuf=(void *)malloc(8*512);
	unsigned short BlockStartNum,SectorStartNum;
	unsigned int BrecSrcCheckSum, tmpBrecCheckSum;
    unsigned int correct_brecnum=0;
	int flag=0;
	for (iloop=0;iloop<4;iloop++)
	{
		BlockStartNum=4+iloop;
		SectorStartNum=0;
		BrecSrcCheckSum=brec_read_check_sum(BlockStartNum,SectorStartNum,TmpBuf,256);
		if (BrecSrcCheckSum!=-1)
		{
			//˵���ҵ�����ȷ��һ��brec
			flag=1;
			break;
		}
	}
	if (flag==0)
	{
		//˵��4��brec���Ǵ����
		printf("<Test Info> no brec is ok!!\n");
		free(TmpBuf);
		return 0;
	}
	for (jloop=4;jloop<8;jloop++)
	{
	    ///����ѭ�����ж�����checksum�Ƿ���ͬ
        SectorStartNum=0;
		tmpBrecCheckSum=brec_read_check_sum(jloop,SectorStartNum,TmpBuf,256);
		printf("<Test Info> brec data blk: %d,curchecksum: %5d,checksum: %5d,%3dkb\n",jloop-4,tmpBrecCheckSum,BrecSrcCheckSum,128);
        if(tmpBrecCheckSum==BrecSrcCheckSum)
            correct_brecnum++;
    }
	
	free(TmpBuf);

    /////����ѭ���еıȽϽ���������ѷ���ֵ��Ϊ��ȷbrec�ķ���
	return correct_brecnum;
}


//return 0:ok   1:fail
int m_bootcheck(void)
{
    int i,j;
    i=m_mbrccheck();
    j=m_breccheck();
    
    if(0==i)
    return 1;
    if(0==j)
	return 1;
    printf("<Test Info> correct_mbrcnum: %3d, correct_brecnum: %3d\n",i,j);

    if(i==4&&j==4)
    return 0;//˵��bootdata��ȫ��ȷ
    else 
    return 1;
    
}

