1.ȷ��make menconfig �Ѿ�ѡ��MMC SD 
   Device drivers-->MMC/SD/Card support ѡ��<M>
   �����ѡ��
   ȷ������ѡ��OK
    <M> MMC block device drivers
	<M> Action AM7x Card Reader MMC/SD function support
2.��������make
3.����ļ�:
   drivers/mmc/core/mmc_core.ko
   drivers/mmc/card/mmc_block.ko
   drivers//mmc/host/am7x_sd.ko
4.����linux-tmp/Copy-mods.sh ���˽ű�������������ļ�copy 
   initrd/lib/modules
   rootfs/lib/modules
5.������Ӧ�����ļ���
6.���ն�˳��insmod .ko 
 insmod /lib/modules/mmc_core.ko
 insmod /lib/modules/mmc_block.ko
 insmod /lib/modules/am7x_sd.ko
7.����SD/MMC ��SD drivers���Զ���ʼ��
 ����mount /dev/mmc /mnt/
 cd /mnt 
 ls -l
 �����Կ���SD��������