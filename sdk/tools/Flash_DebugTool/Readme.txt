   ����Flash_DebugTool ��Ҫ������֤Flash ��ع��ܣ�ͬʱҲ����boot loader ��ع��ܡ�

���ƻ�boot loader reparit Ϊ����
1. ѡ��FireWare�̼�������Debug_FW, ��AM7555_ACM_11_Debug.bin
2. ѡ��fileҪ�����ļ�����
3. ��device ����ADFU 
4. ��CMD list ,
  A. ѡ��erase_phy_block cmd, 
  B. ��дblock ��Ӧ�༭�򣬿�����д0��7,�ֱ�����ķ�mbrec��Brec
  C. ����sector ��1.
  D. ���"DoCmd" ��block number Erase,�ƻ�mbrec, Brec.
  E. ����ͨ����read_root_block������֤�Ƿ���һ��erase �ɹ���
����򣬿�������ȫ����0xFF�����ʾ�����ɹ���
  F. ѭ��B��EƵ�ʣ��ƻ�����block number .
  