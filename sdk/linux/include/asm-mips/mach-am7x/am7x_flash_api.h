#ifndef _ACTIONS_FLASH_API_H_
#define _ACTIONS_FLASH_API_H_
/*
*@ mach-am7x/include/flash_api.h
*
*@author: onlyfish
*@date: 2010-07-12
*@version: 0.1
*/
/**
*******************************************************************************
* @file	flash_drv_api.h
* @brief  д���ļ��ļ�Ҫ˵��
*
* ���ļ�������ϸ˵��,\n
* �ڶ�����ϸ˵��\n
* ��������ϸ˵��\n
* @version  Version 1.1
* @author: onlyfish
*@date: 2010-07-12
* @warning д�Ϲ��ھ����˵��
* @bug д�Ϲ���bug��˵��
* @deprecated д�Ϲ�ʱ��˵��������˵�����ļ���ʲôʱ��͹�ʱ��û����
* @todo д�Ͻ�Ҫ������
* @see д��Ҫ�ο����ļ����߱���
* @ingroup nand_drv
********************************************************************************/
#include <linux/types.h>
#include <am_types.h>

#define MAX__PARTS   16 
#define NAME_SIZE_MAX  10
#define PART_Str_SIZE   128 
struct Flash_Partiton {
    char      partName[NAME_SIZE_MAX];
    uint32_t part_off;   /* LBA of first sector in the partition */
    uint32_t part_size;  /* number of blocks in partition */
}__attribute__ ((packed));//,aligned(4)


/**
* @addtogroup nand_drv
* @{
*/

/*driver info*/
/**
* @brief ���ܣ�д���̶�Ӧsectors����
*
* @param:offset��д��ʼoffset��ַ��
*
* @param:buffer   д���ݻ���
*
* @param:sectors дsectors��Ŀ��sectorΪ�߼���λ������ֵΪ512
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return 
*         ��дʧ�ܣ�����ʧ��err code�����ɹ�����д��ʵ��sectors��Ŀ��  
*             
*/
INT32S flash_write_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );

/**
* @brief ���ܣ������̶�Ӧsectors����
*
* @param:offset������ʼoffset��ַ��
*
* @param:buffer   �����ݻ���
*
* @param:sectors ��sectors��Ŀ��sectorΪ�߼���λ������ֵΪ512
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return 
*         ��дʧ�ܣ�����ʧ��err code�����ɹ�����д��ʵ��sectors��Ŀ��  
*             
*/
INT32S flash_read_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );
/**
* @brief ���ܣ����ظ�����������Ϣ
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return :
*        partition Capacity(��SectorΪ��λ) 
*             
*/
INT32S flash_getcapacity(INT8U *Part );


/*
*@brief ���ܣ�
*        ��Flash��ָ����lbaλ�ö���һ�����������ݵ�sramaddress
*        
*@param��lba           �߼���ַ
*                      
*@param��sramaddress   ������RAM�е��׵�ַָ��
* 
*@param��SecCnt  ,��ʾ�����ȡSector��Ŀ
*

*@return:0     - read ok
*        ��0   - read error
*                1 -   LBA ERROR
*                2 -   FLASH ECC ERROR  
*
*/
INT32S nand_adfu_read(INT32U lba, INT8U* sramaddress,INT32U SecCnt);


/*
*@brief ���ܣ�
*        ��sramaddress������д�뵽Flash��lba��
*        
*@param��lba           �߼���ַ
*                      
*@param��sramaddress   ������RAM�е��׵�ַָ��
* 
*@param��SecCnt  ,��ʾ�����ȡSector��Ŀ
*

*@return:0     - write ok
*        ��0   - write error
*                1 -   LBA ERROR
*                2 -   FLASH ECC ERROR  
*
*/
INT32S nand_adfu_write(INT32U lba, INT8U* sramaddress,INT32U SecCnt);

/*
*@brief ���ܣ�
*        ����U����ADFU����ʱ��ȡ���������ϵ�Mbrec��Brec���룬
*        Ϊ��ʹADFU�����ľ���Flash���ڲ�����֯�ṹ����Driver
*        �ڲ��������д��һ����װ��ʹ��ADFU�ܹ��Լ�õĲ���
*        ����Ķ�д���������ϵĴ���.
*
*@param��Brec_Block_Num - ADFU�Ƕȿ����Ŀ�ţ�Mbrecռ���ĸ��飬
*                           ÿһ��ռ��һ���飻Bbrecÿ��ռ��һ���飬
*
*@param��Brec_Sector_Num - ADFU�Ƕȿ����������ţ�ÿ��Mbrec��
*                          Brec�����ڲ������ƫ����������
*
*@return .  ��ȡ�Ľ��������ֵΪ0��ʾ��ȡ�ɹ���      
*
*/
INT32S brec_sector_read(INT32S Brec_Block_Num,INT32S Brec_Sector_Num,INT8S *Buffer_Addr);


/*
*@brief ���ܣ�
*        ����U����ADFU����ʱд���������ϵ�Mbrec��Brec���룬
*        Ϊ��ʹADFU�����ľ���Flash���ڲ�����֯�ṹ����Driver
*        �ڲ��������д��һ����װ��ʹ��ADFU�ܹ��Լ�õĲ���
*        ����Ķ�д���������ϵĴ���.
*@param��Brec_Block_Num - ADFU�Ƕȿ����Ŀ�ţ�Mbrecռ���ĸ��飬
*                           ÿһ��ռ��һ���飻Bbrecÿ��ռ��һ���飬
*@param��Brec_Sector_Num - ADFU�Ƕȿ����������ţ�ÿ��Mbrec��
*                          Brec�����ڲ������ƫ����������
*
*@param��Buffer_Addr     - ����Bufferָ�롣
*
*@return:д��Ľ��������ֵΪ0��ʾд��ɹ���    
*
*/
INT32S brec_sector_write(INT32S Brec_Block_Num,INT32S Brec_Sector_Num,INT8S *Buffer_Addr);

/*
*@brief function��get flash type 
*
* @param��null
*
* @return .  nand flash type   
*            0x38343646 - string "F648"  
*
*/
int Flash_GetPartNum(struct Flash_Partiton *Fparts);


INT32U get_nand_flash_type(void);

#ifndef DEFINE_StorageInfo
#define PLATFORM_IS_32BIT
struct StorageInfo{
    INT8U                       ChipEnableInfo;                     //chip connect information
    INT8U                       ChipCnt;                            //the count of chip current connecting nand flash
    INT8U                       BankCntPerChip;                     //the banks count in chip
    INT8U                       DieCntPerChip;                      //the die count in a chip
    INT8U                       PlaneCntPerDie;                     //the count of plane in one die
    INT8U                       SectorNumPerPage;                   //page size,based on 0.5k
    INT16U                      PageNumPerPhyBlk;                   //the page number of physic block
    INT16U                      TotalBlkNumPerDie;                  //total number of the physic block in a die
    INT16U                      OperationOpt;                       //the operation support bitmap
    INT8U                       FrequencePar;                       //frequence parameter,may be based on xMHz
    INT8U                       NandChipId[5];                      //nand flash id
    #ifdef PLATFORM_IS_32BIT
    INT8U                       Reserved[2];                        //Reserved
    #endif
    INT16U            CodeDataBlk;
};
#endif
/*
*@brief function��get NAND FLASH DRIVERS Strage information for 
     chip id,Block size,page size 
*
* @param��null
*
* @return .   
*           
*
*/
struct StorageInfo * GetNandStorageInfo(void);

/*
*@brief function��update flash cache buffer to nand flash device 
*
* @param��null
*
* @return .   
*           
*
*/
struct StorageInfo * nand_logical_update(void);

/*
*@brief function��update flash cache buffer to nand flash device 
*@fish add 2011-05-06  for Stop am7x_recover.ko kernel thread
* @param��null
*
* @return .   
*           
*
*/
#define CHECK_STOP_VAL     (0x01)
#define CHECK_START_VAL    (0x00)
INT32U Stop_Check_Recover(INT8U Flag);

INT32U   ReadWrite_Entry(INT8U entrynum, INT32S lba, INT8U* buffer,INT32U Pagetmp);

/**
 * @}
 */
#endif /*_ACTIONS_GPIO_H_*/
