1. ����Card����˵��

//*****************************************************//
A.  CardType�� ����֧��ʲô���ͣ���SD,MMC,MS,XD,CF
  0x00  :Disable SD,MS,XD ,����ǿ��Disable SD/XD detect pin
  0x01  :֧��SD/MMC,MS ��֧��XD 
  0x02  :֧��SD/MMC, ��֧��MS,XD 
  0x03  :֧��SD/MMC,MS,XD��

//*****************************************************//
B. CFCardtype: ����֧��CF �������
  0x00  :Disable CF ���ܣ�ǿ��Disable CF Detect pin
  0x01  : ֧��16Bit Data Bus,
  0x02  : ֻ֧��8Bit Data Bus, 
 
//*****************************************************//
C. SDMultipin: ����ϵͳSD��multi pin ���ù�ϵ 
   0x00 : ����SD data Bus ����nand flash data Bus ����
   0x01 : ����SD data Bus ��nand flash data Bus ���� 
   0x02 : Reserved

//*****************************************************//
D. MSMultipin: ����ϵͳMS��multi pin ���ù�ϵ 
   0x00 : ����MS data Bus ����nand flash data Bus ����
   0x01 : ����MS data Bus ��nand flash data Bus ���� 
   0x02 : Reserved

//*****************************************************//
E. XDMultipin: ����ϵͳXD��multi pin ���ù�ϵ 
   0x00 : ����XD data Bus ����nand flash data Bus ����
   0x01 : ����xd data Bus ��nand flash data Bus ���� 
   0x02 : Reserved

//*****************************************************//
F.MSStandard: ����ϵͳMS Data1 �Ƿ��ù�ϵ
   0x00 : Ĭ�����MS Data1��SD/nand flash bus ����
   0x01 : ��ʾMS Data1 ����

//*****************************************************//
G.CF_DataBus:�˲�������.
     

2. ����Card GPIO ���ã������GPIO ����


   
 
   