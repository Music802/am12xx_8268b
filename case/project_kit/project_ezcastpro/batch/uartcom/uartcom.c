/**
 * drivers/ScalerCom/ScalerCom.c
 *
 *this driver is for communication with Scaler by uart
 *
 *author: Nick
 *date:2010-11-25
 *version:0.1
 */
/*
#include <linux/interrupt.h>
#include "actions_regs.h"
#include <linux/cdev.h>
#include <linux/syscalls.h>
#include "linux/fs.h"
#include <linux/file.h> 
#include "linux/module.h"
#include "linux/init.h"
#include <linux/syscalls.h>
#include <linux/major.h>
#include <linux/poll.h>
#include "am7x-freq.h"
#include <am7x_board.h>
#include <sys_msg.h>
#include "actions_io.h"
#include "am7x_gpio.h"
#include <am7x_dev.h>
#include <asm/mipsregs.h>
#include <sys_cfg.h>
//#include "swfdec.h"
*/

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/major.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <sys_msg.h>
#include <linux/delay.h>

#include "actions_regs.h"
#include "actions_io.h"
#include "am7x-freq.h"
#include "am7x_gpio.h"
#include "../../../case/include/ezcastpro.h"//richardyu 070414
#include "uartcom.h"

#define AM_CHIP_ID 1213

#define HDMI_Output 0

typedef enum
{  
	IOCTL_SET_ATOP_VERSION = 1,
	IOCTL_GET_SCALER_VERSION,//2
	IOCTL_GET_ACTIONS_INFO,//3
	IOCTL_GET_LANGUAGE,//4
	IOCTL_GET_VOLUME,//5
	IOCTL_GET_NOW_IN_MODE,//6
	IOCTL_GET_WIFI_STATUS,//7
	IOCTL_GET_POWER_ON_MESSAGE,//8
	IOCTL_GEL_MODEL_NAME,//9
	IOCTL_GET_CLIENT_IP,//10
	IOCTL_SET_LAN_WIFI_OSD,//11
	IOCTL_SET_FREEZE,//12
	IOCTL_SET_HIDE,//13
	IOCTL_AM_VERSION,//14
	IOCTL_SETUP_OSD,//15
	IOCTL_GET_SOURCE,//16
	IOCTL_OPEN_SPECTRUM,//17
	IOCTL_OTA_BLOCK,//18
	SEND_RS232_CMD,//19
	CMD_GET_RESULT,//20
	EZCASTPRO_USB1_OFF,//21,richardyu 061714
	IOCTL_SET_ATOP_VOLUME, //22
	CMD_START_GETVAL,//23 liangrao 
	CMD_GET_INITVAL,//24 liangrao
	IOCTL_WATCHDOG,//25 watchdog
	CMD_GET_FACTRESULT,//26
}UARTCOM_IOCTL_ID;

#define PROJECTOR_UART 1
#define EZCASTPRO_UART_ON 0//richardyu 061714
#define DELTA_CMD
#define IR_REMOTE  0
#define PRINT_RECEIVE 1
#define ACK_EN 0

#define DBG_AM_MSG		1
#if  DBG_AM_MSG
#define DBG_UARTCOM_MSG(format,args...)   printk(format,##args)
#else
#define DBG_UARTCOM_MSG(format,args...)   do {} while (0)
#endif

#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define act_readl(port)  (*(volatile unsigned int *)(port))

//#ifdef DELTA_CMD
//#define Serial_Brg 9600 //19200
//#else
#define Serial_Brg 19200//9600 //19200
//#endif

#define COM_UART_CTL		UART2_CTL
#define UARTCOM_CTL			UART2_CTL
#define COM_UART_STAT		UART2_STAT
#define COM_UART_TXDAT		UART2_TXDAT
#define COM_UART_RXDAT		UART2_RXDAT
#define COM_CMU_UARTCLK 	CMU_UART2CLK
#define COM_IRQ_UART		IRQ_UART2

#define SCAN_TIME	10//20 // 10*4 = 40ms

#define BUF_SIZE 4096//512

//展頻！！！
#define AFU_TEST_ON 0


INT8U factestbuf[BUF_SIZE];
INT8U uartbuf[BUF_SIZE];
INT8U uartsendbuf[BUF_SIZE];
INT8U uartreadbuf[BUF_SIZE];
int overflow;
int recv_idx; // for uartbuf
int read_idx; // for uartbuf
int busy;
//int sendsize = 0;
//int flag_send_req = 0;
int send_idx = 0; // for uartsendbuf 
int out_idx = 0; // for uartsendbuf
int check_idx = 0;
int readidx_lan = 0; //for uartreadbuf
int recvidx_lan = 0; //for uartreadbuf
//int readbuf_idx = 0;
int outsem = 0;
int FF_FB_multi = 0;
int reverse_mode = 0;
int forward_mode = 0;
int ack_wait =0;
int Enter_lock = 0;

//unsigned char ack_cmd=0;
unsigned char ack_cmd[5]={0};

char atop_ver[20];
//char actions_info[17]="  0.23:2013-04-12";//{'A','.','0','.','0',':','2','0','1','2','D','E',''};
char actions_info[20]="1.53:2014-04-30";

//temp mark
//int language = 0;
int language = -1; 
//temp mark
int volume_value = 20;
int temp_volume = 20;
char scaler_status[20];
int mode_name = -1;
INT8U Source_once = 0;
INT8U scaler_ver[3];
//INT8U AM_ver[9] = {0x2a,0x20,0x01,0x04,0x03,0x00,0x00,0x17,0x00};
//INT8U AM_ver[9] = {0x2a,0x20,0x01,0x04,0x03,0x01,0x01,0x35,0x00};	//add by Shane
//INT8U AM_ver[9] = {0x2a,0x20,0x01,0x04,0x03,0x01,0x1b,0x12,0x00};	//add by 
INT8U AM_ver[9] = {0x2a,0x20,0x01,0x04,0x03,0x01,0x2f,0x14,0x04};	//14720004
INT8U Now_in_mode[8] = {0x2a,0x20,0x01,0x03,0x02,0x10,0x00,0x00};
INT8U send_scaler_message[15] ={0x2a,0x20,0x01,0x03,0x02,0x04,0x00,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//default setting
int Now_in = -1;
int temp_Now_in = -1;
int Wifi_Status = -1;
int Volume_lock = 0;
INT8U source_now = -1;
//int ACK_Volume_lock = 0;
int wait_flag = 0;
int second_times = 0;
//int tempbuf[50];
//int templen = 0;
int Lan_WiFi_OSD = 1;
int ota_block = 0;
int lock_err_flag = 0;


static dev_t uartcom_dev;
static struct cdev  *uartcom_cdev=NULL;
struct timer_list N_timer;

int start_getval_flag=-1;
int get_testresult_flag=-1;


typedef enum
{
	UART_IO_CMD_SEND = 0,
	UART_IO_CMD_RECEIVE,
}UART_IO_CMD;


typedef struct{
	INT8U header[2];
	INT8U atop[2];
	INT8U command[4];
	INT8U data[22];
} UART_PACKET;


typedef enum
{
	COM_CMD_KEY, //-- 0
	COM_CMD_GET_VERSION, //-- 1
	COM_CMD_LANGUAGE, //-- 2
	COM_CMD_RESET, //-- 3
	COM_CMD_DEBUG, //-- 4
	COM_CMD_STATUS, //-- 5
	COM_CMD_CALIBRATION_PATTERN, //-- 6
	COM_CMD_VOLUME, //-- 7
	COM_CMD_SOURCE, //-- 8
}COM_CMD_ID;

#define FUI_KEYACTION_DOWN_MASK (KEY_ACTION_DOWN<<KEY_ACTION_MASK_SHIFT)

static const INT8U UART_HEAD[2]={0x0d,0x0a};
static const INT8U UART_TAIL[2]={0x23,0x0d};

static const INT8U ACK_HEADER[2]={0xa2,0x02};

static const INT8U UART_HEADER[2]={0x2a,0x20};
static const INT8U ATOP_HEADER1[2]={0xf5,0x14}; 
static const INT8U ATOP_HEADER2[2]={0x05,0x14};
static const INT8U ATOP_HEADER3[2]={0x2b,0x31}; 

static void _PutC(char c);
static void comResponseCommand(INT8U *cmd, INT8U len);

INT32U act_get_cclk_div()
{
#if AM_CHIP_ID == 1203
	return 1;
#elif AM_CHIP_ID == 1211
	return ((act_readl(CMU_BUSCLK)&(1<<2))+1);
#elif AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213 
	INT32U sclk_div =2,sclk_val=*(volatile INT32U*)(CMU_BUSCLK);

	sclk_val = (sclk_val>>28)&0x7;

	switch(sclk_val){
		case 1: sclk_div=2; break;
		case 2: sclk_div=5; break;
		case 3: sclk_div=6; break;
		case 4: sclk_div=3; break;
		case 5: sclk_div=7; break;
		case 6: sclk_div=8; break;
		default:;
		//__asm__ volatile ("break 0x19");break;
	}
	return sclk_div;
#endif
}

INT32U act_getCorePll(void)
{
	unsigned long val = act_readl(CMU_COREPLL);
	unsigned int spread = val&(1<<24);

#if AM_CHIP_ID == 1203
	val >>= 2;
	val &= 0x3f;
	val *= 6;
#elif AM_CHIP_ID == 1211
	unsigned long bus_val = act_readl(CMU_BUSCLK);

	if(bus_val&(1<<11)){	//DDRPLL
		val = act_readl(CMU_DDRPLL);

		val >>=2;
		val &= 0x3f;
		val *= 8;
	}else{
		val >>= 2;
		val &= 0x3f;
		val *= 6;
	}
	if(spread)
		val += 6*6;
#elif AM_CHIP_ID == 1220 ||AM_CHIP_ID == 1213
		val >>= 2;
		val &= 0x7f;
		val *= 6;
#endif

	return val;
}

INT32U act_get_sclk_div(void)
{
	INT32U sclk_div=*(volatile INT32U*)(CMU_BUSCLK);

	sclk_div = (sclk_div>>4)&1;
#if AM_CHIP_ID == 1203
	sclk_div = (sclk_div+1)*act_get_cclk_div();
#elif AM_CHIP_ID == 1211
	sclk_div ++;
#elif AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213
	sclk_div = act_get_cclk_div();
#endif
	return sclk_div;
}





#if AM_CHIP_ID == 1220 ||AM_CHIP_ID == 1213
static void serial_setbrg (INT32U corePll,INT32U cclk_div)
{
	INT32U tmp;
	volatile INT32U *uart1_clk = (volatile INT32U*)(CMU_UART1CLK);
	volatile INT32U *uart2_clk = (volatile INT32U*)(CMU_UART2CLK);
	INT32U *uart2_ctl = (INT32U*)(UARTCOM_CTL);
	DBG_UARTCOM_MSG("ori *uart1_ctl==0x%08x, \n",act_readl(UART1_CTL));				
	DBG_UARTCOM_MSG("ori *uart2_ctl==0x%08x, \n",*uart2_ctl);			
	DBG_UARTCOM_MSG("ori *uart1_clk==0x%08x, \n",*uart1_clk);				
	DBG_UARTCOM_MSG("ori *uart2_clk==0x%08x, \n",*uart2_clk);				
	DBG_UARTCOM_MSG("ori get_hclk==0x%08x, \n",get_hclk());					
//	tmp = corePll*1000000/(16*Serial_Brg*cclk_div);
//	tmp = corePll*1000000*2/(16*Serial_Brg*cclk_div);	
//	tmp = (get_hclk()/(16*Serial_Brg))-1;
	*uart2_ctl = *uart2_ctl&~(1<<15);
//	tmp = *uart1_clk;
//	DBG_UARTCOM_MSG("tmp 0 ==0x%08x, ",tmp);
#if Serial_Brg==115200
	tmp = 0x0000006a; //115200						
#elif Serial_Brg==19200
	tmp = 0x00000281; //19200
#elif Serial_Brg==9600
	tmp = 0x00000503; //9600
#endif	
//	DBG_UARTCOM_MSG("tmp 1 ==0x%08x, ",tmp);							
//	tmp = tmp/6;
//	DBG_UARTCOM_MSG("tmp 2 ==0x%08x, ",tmp);						
	tmp |= 0x10000;
//	*uart1_clk = 
	*uart2_clk = tmp;//0x1004d(24M)/0x10185(120M)/0x10138(96M)/0x100c3(60M)
	*uart2_ctl = *uart2_ctl|(1<<15);
//	*uart2_ctl =act_readl(UART1_CTL);
	DBG_UARTCOM_MSG("new *uart2_clk==0x%08x, ",*uart2_clk);				
	DBG_UARTCOM_MSG("new *uart2_ctl==0x%08x, \n",*uart2_ctl);			
}
#if 0
static void serial_setbrg (INT32U corePll,INT32U cclk_div)
{
	INT32U tmp;
//	volatile INT32U *uart1_clk = (volatile INT32U*)(CMU_UART1CLK);
	volatile INT32U *uart2_clk = (volatile INT32U*)(CMU_UART2CLK);
	DBG_UARTCOM_MSG("ori uart2_clk==%08x\n",tmp);

//	tmp = corePll*1000000/(16*Serial_Brg*cclk_div);
	tmp = corePll*1000000*2/(16*Serial_Brg*cclk_div);	
	tmp |= 0x10000;
//	*uart1_clk = 
	DBG_UARTCOM_MSG("new uart2_clk==%08x\n",tmp);
	*uart2_clk = tmp;//0x1004d(24M)/0x10185(120M)/0x10138(96M)/0x100c3(60M)
}
#endif
#else
static void serial_setbrg (unsigned int baud)
{
    unsigned int tmp;
	INT32U *uart_clk = (volatile INT32U*)(COM_CMU_UARTCLK);
	INT32U *uart_ctl = (INT32U*)(COM_UART_CTL);

	*uart_ctl = *uart_ctl&~(1<<15);
	tmp = get_hclk()/(16*baud);
	tmp |= 0x10000;
	*uart_clk = tmp;
	*uart_ctl = *uart_ctl|(1<<15);
}
#endif
static void _PutC(char c)
{
	volatile INT32U *uart_stat = (volatile INT32U*)(COM_UART_STAT);
	volatile INT32U *uart_txdata = (volatile INT32U*)(COM_UART_TXDAT);
    INT32U tmp;

	tmp = *uart_stat;
	/* Wait for fifo to shift out some bytes */
#if IR_REMOTE
	DBG_UARTCOM_MSG("IR remote: _putC\n");
#else
	while( *uart_stat & 0x00000040 ){
		DBG_UARTCOM_MSG("UART2_STAT=%x\n",*uart_stat);
//		DBG_UARTCOM_MSG("1")  ;
	}
	*uart_txdata = (INT32U)c;
#endif
}

static int en_uart2_rx_irq (void)
{

	volatile INT32U *uart_config = (volatile INT32U*)(COM_UART_CTL);

	// change uart interrupt mode to at least one
	*uart_config = 0x0000cc03 |(1<<18); // Enable UART1 RX IRQ

	if(act_readl(COM_UART_STAT) & 1)
		act_writel(1,COM_UART_STAT); /* clear IRQ pending bit */

	while((act_readl(COM_UART_STAT) & 0x20) == 0)
		act_readl(COM_UART_RXDAT) ;
	return 0;
}

static irqreturn_t comISR(unsigned int irq, void *dev_id)
{
	unsigned int status;
//	DBG_UARTCOM_MSG("do uart isr\n");
	status = act_readl(COM_UART_STAT);
	if(status & (1<<4)){ // rx error
		goto rx_error;
	}
	
	//if(act_readl(COM_UART_STAT) & 1)
		//act_writel(1,COM_UART_STAT); // clear IRQ pending bit 
	while(!(act_readl(COM_UART_STAT) &0x20)) 
	{
		uartbuf[recv_idx] = act_readl(COM_UART_RXDAT) & 0xff;
		//DBG_UARTCOM_MSG("%d::%02x",recv_idx,uartbuf[recv_idx]);
	  	recv_idx++;
		if (recv_idx== read_idx) 
		{
			overflow = 1;
			read_idx = 0; 
			recv_idx = 0; 
		}
		else if(recv_idx >= BUF_SIZE) 
		{
			recv_idx = 0;
		}
	}
	DBG_UARTCOM_MSG("\n");
rx_error:
	IRQ_CLEAR_PENDING(status, COM_UART_STAT);
	return IRQ_HANDLED;
}

void uartcomInit(void){
	INT32U temp;
	int rc;
	
	volatile INT32U* uartconf = (volatile INT32U*)(COM_UART_CTL);

#if AM_CHIP_ID==1213
	// dport->save_config = act_readl(UARTCOM_CTL); // for restore after shutdown this module
	INT32U* gpio3 = (INT32U*)(GPIO_MFCTL3);
	INT32U* gpio4 = (INT32U*)(GPIO_MFCTL4);

	*gpio3 = (*gpio3 & 0x3fffffff) | 0x40000000;
	*gpio4 = (*gpio4 & 0xfffffff8) | 0x00000001;

#elif AM_CHIP_ID==1211

	
	INT32U* gpio5 = (INT32U*)(GPIO_MFCTL5);
	INT32U* gpio6 = (INT32U*)(GPIO_MFCTL6);

	temp = *gpio5 & 0x0fffffff;
	*gpio5 = temp | 0x20000000;
	temp = *gpio6 & 0xfffffff0;
	*gpio6 = temp | 0x00000002;
#endif
	
	
	*uartconf = 0x0000cc03;	 	  // 03 ==> 9600,n,8,1
                                  // 0~1: data bits 00:5, 01:6, 10:7, 11:8
                                  // 2: stop bit 0:0, 1:1
                                  // 3 -
                                  // 4~6: parity bit 0xx: none 100: odd, 101:even
								  
#if AM_CHIP_ID==1213
	serial_setbrg(act_getCorePll(), act_get_sclk_div() );
#else
	serial_setbrg(Serial_Brg);
#endif
	en_uart2_rx_irq();
	rc = request_irq(COM_IRQ_UART, comISR, IRQF_DISABLED, "puart1", uartbuf );
	if(rc < 0)
	{
		DBG_UARTCOM_MSG("Core UART request irq error\n");
	}
	else
		DBG_UARTCOM_MSG("Core UART request irq ok\n");
}


static int uart_enable(INT8U enable)
{

	volatile INT32U *uart_config = (volatile INT32U*)(COM_UART_CTL);

	// change uart interrupt mode to at least one
	if (enable==0)
		*uart_config = 0x0000cc03 & (~(1<<18)); // Disable UART1 RX IRQ
	else
		*uart_config = 0x0000cc03 |(1<<18); // Enable UART1 RX IRQ
}

void sendbuf(char* buf, int num){
	int i;
	for(i = 0; i < num; i++){
		_PutC(buf[i]);
	}
}

static INT8U comGetRxBufLength(void)
{
	int len;
	if (recv_idx < read_idx) {
	   len = BUF_SIZE - read_idx + recv_idx;   
	}
    else if (recv_idx > read_idx) {
    	len = recv_idx - read_idx;
    }
	else
		len = 0;	
	
	return len;
}

#define MAX_CMD_BUFF 50 
#define RESULT_SIZE 10


INT8U cmd_buff[MAX_CMD_BUFF*10]={0};
INT16U cmd_start=0;
INT16U cmd_end=0;

typedef struct rs232_cmd_result_t{
	INT8U readed;
	INT8U len;
	char buf[MAX_CMD_BUFF];
}rs232_cmd_result;

rs232_cmd_result result_set[RESULT_SIZE];
INT8U residx=0;

INT8U parseCmd(INT8U *cmd,INT8U *buf,INT8U len){
	int i=0;
	int idx=-1;
	INT16U size=0;
	if(!(len==1 && buf[0]==0x00)){
		for(i=0;i<len;i++){
			cmd_buff[cmd_end]=buf[i];
			cmd_end++;
			if(cmd_end==MAX_CMD_BUFF){
				cmd_end==0;
			}
			if(cmd_end==cmd_start){
				printk("cmd overflow , clear buffer\n");
				memset(cmd_buff,0,MAX_CMD_BUFF);
				cmd_start=0;
				cmd_end=0;
				return 0;
			}
		}
	
		if(len!=0){
			printk("try to write %x to buff cmd_start=%x , cmd_end=%x\n",len,cmd_start,cmd_end);
#if 1
    printk("cmd_start=%x\n",cmd_start);
    idx=cmd_start;
    for(i=0;i<MAX_CMD_BUFF*10;i++){
        printk("[%2x]",cmd_buff[idx+i]);

        if(idx+i==MAX_CMD_BUFF*10){
            idx=-MAX_CMD_BUFF*10;
        }
        if(idx+i==cmd_end){
            break;
        }
    }
#endif

		}
	}
	int get_head=-1;
	int get_tail=-1;

	idx=cmd_start;

	for(i=0;i<MAX_CMD_BUFF*10;i++){
		if(cmd_buff[(idx+i)%(MAX_CMD_BUFF*10)]==UART_HEAD[0] && cmd_buff[(idx+i+1)%(MAX_CMD_BUFF*10)]==UART_HEAD[1]){
			get_head=(idx+i+2)%(MAX_CMD_BUFF*10);
			break;
		}

		if(idx+i==MAX_CMD_BUFF*10){
			idx=-MAX_CMD_BUFF*10;
		}
		if(idx+i==cmd_end){
			break;
		}
	}
	
	if(get_head!=-1){
		cmd_start=get_head;
		idx=cmd_start+2;
		for(i=0;i<MAX_CMD_BUFF*10;i++){
	        if(cmd_buff[idx+i]==UART_TAIL[0] && cmd_buff[(idx+i+1)%(MAX_CMD_BUFF*10)]==UART_TAIL[1]){
        	    get_tail=idx+i;
				break;
			}
			if(idx+i==MAX_CMD_BUFF*10){
				idx=-MAX_CMD_BUFF*10;
			}
	        if(idx+i==cmd_end){
    	        break;
       		}
		}
		if(get_tail!=-1){
			//printk("get_tail=%x , cmd_end=%x size=%x\n",get_tail,cmd_end,size);
			for(i=0;i<MAX_CMD_BUFF;cmd_start++){
				if(cmd_start==MAX_CMD_BUFF*10){
					cmd_start=0;
				}
				if(cmd_start==get_tail){
					cmd_start+=2;
					break;
				}
				cmd[i]=cmd_buff[cmd_start];
				i++;
				cmd_buff[cmd_start]=0x00;
				size++;
			}
			
		}
	}
	//printk("return size=%x cmd_start=%x\n",size,cmd_start);
	return size;
}

INT8U get_cmd_result(char *buff){
	INT8U size=0;
	if(result_set[residx].readed==1){
		printk("get cmd at %d\n",residx);
		memcpy(buff,result_set[residx].buf,result_set[residx].len);
		size = result_set[residx].len;
		result_set[residx].len=0;
		result_set[residx].readed=0;
		memset(&result_set[residx],0,sizeof(rs232_cmd_result));
		residx++;
		if(residx==RESULT_SIZE){
			residx=0;
		}
	}
	return size;
}

INT8U save_cmd_result(char *buff,INT8U size){
	int idx=residx;
	int i;
	for(i=0;i<RESULT_SIZE;i++){
		if(idx+i==RESULT_SIZE){
			idx-=RESULT_SIZE;	
		}
		if(result_set[idx+i].readed==0){
			result_set[idx+i].readed=1;
			result_set[idx+i].len=size;
			memcpy(result_set[idx+i].buf,buff,size);
			printk("save result to %d\n",idx+i);
			break;
		}		
	}
	if(i==RESULT_SIZE){
		printk("result is full");
	}
	return 0;
}

int id=-1;

static INT8U comGetCommand(INT8U *cmd)
{
	int len, cmdlen;
	int ack_length;
	INT8U i;
	int idx = read_idx, cmd_idx;
	INT8U checksum;
	
	INT8U check_packet = 0;
	
	len = comGetRxBufLength();
	
	if(len==0){
		return 0;
	}else{
		DBG_UARTCOM_MSG("\nlen=%d\n",len);
	}
	cmdlen = 0;

	DBG_UARTCOM_MSG("scalercom: print raw data: \n");

#if PROJECTOR_UART

	#if PRINT_RECEIVE //print all bytes receive
	char RX_buf[256]="";
	if(start_getval_flag==1 &&id==-1){
		id=read_idx;

		
	}
	for(i=0;i<len;i++){
		if( (idx + i) >= BUF_SIZE )//check BUF_SIZE by charlie on 0613
			DBG_UARTCOM_MSG("%d:<0x%02x>,",idx+i-BUF_SIZE,uartbuf[idx+i-BUF_SIZE]);
		else{
			DBG_UARTCOM_MSG("%d:<0x%02x>,",idx+i,uartbuf[idx+i]);
			
			RX_buf[i]=uartbuf[idx+i];
			
		}
			
		if(i==len-1)DBG_UARTCOM_MSG("\n");
	}
	        DBG_UARTCOM_MSG("data:%s\n",RX_buf);
		if(strstr(RX_buf,"factest"))
		{
                	sprintf(factestbuf,"%s",RX_buf);
			DBG_UARTCOM_MSG("uartcom:factestbuf=%s\n",factestbuf);
			get_testresult_flag=0;
		}						//init buf
		if(strstr(RX_buf,"<actuarttest>") ||(strstr(&uartbuf[id],"*sharp=?")&&start_getval_flag==1) ){
			
			struct file *fp;
			printk("am7x_com uartbuf=%s %d\n",&uartbuf[id],id);
			start_getval_flag=0;
			
			mm_segment_t fs;
			loff_t pos;
			fp=filp_open("/tmp/uart.testbuf",O_RDWR|O_CREAT,0644);
			
			if(IS_ERR(fp)){
				printk("create file err\n");
				return -1;
			}
			
			fs=get_fs();
			set_fs(KERNEL_DS);
			
			pos=0;
				vfs_write(fp,RX_buf,sizeof(RX_buf),&pos);
			filp_close(fp,NULL);
			
			set_fs(fs);
		}
		
		
	
	
	
	#endif
//	if(uartbuf[idx]== ACK_HEADER[0] && (len - uartbuf[idx+4]) > 6){
//		ack_length = 6 + uartbuf[idx+4];
//		idx += ack_length;
//		if( idx > BUF_SIZE )
//			idx -= BUF_SIZE;
//	}
	/*0613 Charlie add uartbuf[idx] check upperbound size 512!!*/
	//if((uartbuf[idx]== ATOP_HEADER1[0]) && (uartbuf[idx+1]== ATOP_HEADER1[1]) ){
	if( uartbuf[idx]== ATOP_HEADER1[0] ){
		if( uartbuf[idx+1]== ATOP_HEADER1[1] || (idx == BUF_SIZE-1 && uartbuf[0]== ATOP_HEADER1[1]) ){//check if idx == BUF_SIZE, idx+1 will be wrong address
			if( idx+6 >= BUF_SIZE ){
				check_packet = uartbuf[idx+6-BUF_SIZE];
			}else{
				check_packet = uartbuf[idx+6];
			}
			if( len >= 7 + check_packet + 1 + 4 )
				len = 8 + check_packet;
			else
				wait_flag = 1;
		}
	//}else if((uartbuf[idx]== ATOP_HEADER2[0]) && (uartbuf[idx+1]== ATOP_HEADER2[1]) ){
	}else if( uartbuf[idx]== ATOP_HEADER2[0] ){
		if( uartbuf[idx+1]== ATOP_HEADER2[1] || (idx == BUF_SIZE-1 && uartbuf[0]== ATOP_HEADER2[1]) ){//check if idx == BUF_SIZE, idx+1 will be wrong address
			if( idx+3 >= BUF_SIZE ){
				check_packet = uartbuf[idx+3-BUF_SIZE];
			}else{
				check_packet = uartbuf[idx+3];
			}
			if( len >= 4 + check_packet + 2 + 4 )
				len -= 8;
			else
				wait_flag = 1;
		}
	}else if(uartbuf[idx]== ATOP_HEADER3[0]) {
		if( uartbuf[idx+1]== ATOP_HEADER3[1] || (idx == BUF_SIZE-1 && uartbuf[0]== ATOP_HEADER3[1]) ){//check if idx == BUF_SIZE, idx+1 will be wrong address
			if( idx+4 >= BUF_SIZE ){ // the fifth byte is the total length
				check_packet = uartbuf[idx+4-BUF_SIZE];
			}else{
				check_packet = uartbuf[idx+4];
			}
			DBG_UARTCOM_MSG("Robert=================<0x%02x>,",check_packet);
			if( len > check_packet)
				len = check_packet;
			else
				wait_flag = 1;
		}
	}
	if( wait_flag == 0 ){
		if(uartbuf[idx]== UART_HEADER[0]){
			if (len < 6) {	// receive not enough to find out real length of this command
				wait_flag = 1;
			} else {
				if((idx + 4 > BUF_SIZE)) {
					check_packet = 6 + uartbuf[idx+4-BUF_SIZE];
				} else {
					check_packet = 6 + uartbuf[idx+4];
				}
					if (len > check_packet)
						len = check_packet;
					else
						wait_flag = 1;	// total packet not received			
			}
		}
		#if 0
		//if(uartbuf[idx]== UART_HEADER[0] && (len - uartbuf[idx+4]) > 6){
		if(uartbuf[idx]== UART_HEADER[0]){
			if((idx + 4 > BUF_SIZE) && (len - uartbuf[idx+4-BUF_SIZE]) > 6){
				len = 6 + uartbuf[idx+4-BUF_SIZE];
			}else if(len - uartbuf[idx+4] > 6){
				len = 6 + uartbuf[idx+4];
		//}else	if(uartbuf[idx]== UART_HEADER[0] && (len <6 || ((len - uartbuf[idx+4]) != 6))){
			}else if(len <6){
				wait_flag = 1;
			}else if((idx + 4 > BUF_SIZE) && ((len - uartbuf[idx+4-BUF_SIZE]) != 6)){
				wait_flag = 1;
			}
		}
		#endif
	}
	if( second_times == 1){
		second_times = 0;
		wait_flag = 0;
	}
/*	for(i=0;i<len;i++){
		if( (idx + i) >= BUF_SIZE )//check BUF_SIZE by charlie on 0613
			DBG_UARTCOM_MSG("<0x%02x>,",uartbuf[idx+i-BUF_SIZE]);
		else
			DBG_UARTCOM_MSG("<0x%02x>,",uartbuf[idx+i]);
		if(i==len-1)DBG_UARTCOM_MSG("\n");
	}*/
	if( wait_flag == 1 ){//&& uartbuf[idx]== UART_HEADER[0]&& i == 0){//wait for next commande
		second_times = 1;
		goto WAIT_NEXT;
	}
	/*0613 Charlie add uartbuf[idx] check upperbound size 512!!*/
	/*else if( wait_flag == 1 && tempbuf[0] != 0){
		wait_flag = 0;
		for( i = 0; i < len ; i++ ){
			tempbuf[templen + i] = uartbuf[idx + templen + i];
		}
		for( i = 0; i < (templen + len) ; i++ ){
			 uartbuf[idx + i] = tempbuf[i];
		}
		tempbuf[0] = 0;
	}*/

	for (i=0;i<len;i++) {
		cmd_idx = idx;
		if (uartbuf[idx]== UART_HEADER[0]&& i == 0)
		{	
			//DBG_UARTCOM_MSG("OUR<0x%02x>,",uartbuf[idx]);
			idx++;
			if (idx>=BUF_SIZE) idx = 0;
			if(idx >= (read_idx + len)){ //??
				idx--;
				break;
			}
			if (uartbuf[idx]== UART_HEADER[1]){
				idx++;
				if (idx>=BUF_SIZE) idx = 0;
				cmdlen = len - i;
				break;
			}else{
				if(idx == 0){
					idx = BUF_SIZE-1;
				}
				else{
					idx = idx -1;
				}
				if(check_idx < idx){
					uartreadbuf[recvidx_lan] = uartbuf[idx];
					recvidx_lan++;
					idx++;
					if (idx>=BUF_SIZE) idx = 0;
					if (recvidx_lan>=BUF_SIZE) recvidx_lan = 0;
					uartreadbuf[recvidx_lan] = uartbuf[idx];
					check_idx = idx;
					recvidx_lan++;
					if (recvidx_lan>=BUF_SIZE) recvidx_lan = 0;
				}
			}
		}else{
			// nick add for lan control
			//DBG_UARTCOM_MSG("ATOP<0x%02x>,",uartbuf[idx]);
			if(check_idx <= idx){
				//DBG_UARTCOM_MSG("ATOP<0x%02x>,",uartbuf[idx]);
				uartreadbuf[recvidx_lan] = uartbuf[idx];
				recvidx_lan++;
				if(recvidx_lan >= BUF_SIZE)
					recvidx_lan = 0;
				check_idx = idx;
			}
			idx++;
			if ((read_idx>=BUF_SIZE) || (idx>=BUF_SIZE))  {
				idx = 0;
				check_idx = 0;
				//printk("[%s-%d]check_idx and idx is 0\n",__FUNCTION__,__LINE__);
			}
		}	
	}
	if (cmdlen==0){ // have no uart command, for lan control command
		read_idx = idx;//recvidx_lan;
		return 0;
	}
	//printk("cmdlen=%d,cmd_idx=%d,check_idx=%d,recvidx_lan=%d\n",cmdlen,cmd_idx,check_idx,recvidx_lan);
#ifdef DELTA_CMD
	DBG_UARTCOM_MSG("scalercom line:623\n");
	for (i=0;i<cmdlen;i++) {
		if(cmdlen <= 14){
			cmd[i] = uartbuf[cmd_idx];
			DBG_UARTCOM_MSG("0x%02x",cmd[i]);
		}
		else{
			if(check_idx < cmd_idx){
				uartreadbuf[recvidx_lan] = uartbuf[cmd_idx];
				recvidx_lan++;
				if(recvidx_lan >= BUF_SIZE)
					recvidx_lan = 0;
				check_idx = cmd_idx;
			}
		}
		/*
		// nick add for lan control
		if(i > 5){ // fix 6 byte for key command
			if(check_idx < cmd_idx){
			uartreadbuf[recvidx_lan] = uartbuf[cmd_idx];
			recvidx_lan++;
			if(recvidx_lan >= BUF_SIZE)
				recvidx_lan = 0;
			check_idx = cmd_idx;
			}
		}
		*/
		cmd_idx++;
		if (cmd_idx>=BUF_SIZE) cmd_idx = 0;
} 
	//printk("\n end cmd_idx=%d\n",cmd_idx);
	read_idx = cmd_idx;
	if(cmdlen > 14){
		return 0;
	}
#else
	for (i=0;i<cmdlen;i++) {		
		cmd[i] = uartbuf[cmd_idx]; 
		// nick add for lan control
		if(i > 3){ // fix 4 byte for key command
			if(check_idx < cmd_idx){
			uartreadbuf[recvidx_lan] = uartbuf[cmd_idx];
			recvidx_lan++;
			if(recvidx_lan >= BUF_SIZE)
				recvidx_lan = 0;
			check_idx = cmd_idx;
			}
		}
		cmd_idx++;
		if (cmd_idx>=BUF_SIZE) cmd_idx = 0;
	} 
	read_idx = cmd_idx;
#endif
	return cmdlen;

WAIT_NEXT:
	cmdlen = 0;
	return cmdlen;

#else
	for(i=0;i<len;i++){
		DBG_UARTCOM_MSG("<0x%02x>,",uartbuf[idx+i]);
		cmd[i]=uartbuf[idx+i];
		cmdlen++;
		read_idx++;
		if(read_idx==BUF_SIZE){
			read_idx==0;
		}
		if(i==len-1)DBG_UARTCOM_MSG("\n");
	}

	return cmdlen;
#endif

}

static void send_key_press(INT32U type,INT32U key){
	struct am_sys_msg uMsg;
	key = key & 0x000000ff;
	//key  = key | 0x100; // for ir
	//printk("type = %x, key = %x\n",type,key);
	uMsg.type = SYSMSG_KEY;
	uMsg.subtype = KEY_SUBTYPE_IR;
	uMsg.dataload = key | type;
	//--printk("dataload = %x\n",uMsg.dataload);
	am_put_sysmsg(uMsg);
}

void writeOut(){
	int i;
	int cur_send_idx = send_idx;
	int cur_out_idx = out_idx;
	int out_num = 0;
	if(cur_send_idx < cur_out_idx){
		out_num = BUF_SIZE-cur_out_idx+cur_send_idx;
	}
	else{
		out_num = cur_send_idx - cur_out_idx;
	}
	out_idx = cur_send_idx;
	while(outsem){
		i = 0;
        /* Mos: add msleep to avoid Dead lock when multiple write occured */
        msleep(1);
	}
	outsem = 1;
	DBG_UARTCOM_MSG("writeOut %d: ",out_num);//richardyu 042814
	for(i = 0; i < out_num; i++){
		DBG_UARTCOM_MSG("0x%02x, ",uartsendbuf[cur_out_idx+i]);		
		_PutC(uartsendbuf[cur_out_idx+i]);
	}
	outsem = 0;
	
}

typedef enum
{      
    KEY_UP = 0x1, //-- 0
	KEY_DOWN ,
	KEY_LEFT ,
	KEY_RIGHT ,
	KEY_ENTER ,
	KEY_EXIT ,
	KEY_ZOOMIN ,
	KEY_ZOOMOUT, //8
}DELTA_CMD_ID;
typedef enum
{      
    SOURCE_UDISK = 0x9, // 9 ,
    SOURCE_USB_DISPLAY ,
    SOURCE_NETWORK_DISPLAY ,
    SOURCE_WIFI_SETTING ,
	SOURCE_LEAVE,
	SOURCE_PC, // 14
}VS_SOURCE_ID;



typedef enum
{      
    VS_HINT = 0x10, // 16
	SNAP_SHOT = 0xFF, //255
}VS_UNIQUE_ID;


int ParseCmd(char* cmdbuf){ //if header is for AM
	int i = 0;
	char DL =0;
	char temp_cmd=0;
	char buf[6]={0,0,0,0,0,0};
	DL=*(cmdbuf+2);
	temp_cmd=*(cmdbuf+1);
	printk("from Scaler, DL=%d, command=%d\n",DL,temp_cmd);
	if(DL<1){
		printk("DL is wrong!!\n");
		return 0;
	}
#if ACK_EN
	if(ack_wait==1){ //for check ACK from scaler
		if(temp_cmd==ack_cmd[3]){ //ack_cmd[3] is "cmd", receive ACK 
			DBG_UARTCOM_MSG("scalercom receive ACK from scaler");
			memset(ack_cmd,0,5);
			ack_wait=0;
			return 0;
		}
		return 0;
	}
#endif
	//printk("input cmdbuf = %s\n",cmdbuf);
	//strcpy(buf, cmdbuf);
//	memcpy(buf,cmdbuf+2,DL-1);
	buf[0]=*(cmdbuf+3);
	printk("data = %d\n",buf[0]);
	switch(temp_cmd){
		case 'V': //viewsonic only
			switch(buf[0]){
			printk("from Scaler, VS only\n");
				case 1:
					printk("VS HINT\n");
					return VS_HINT;
					break;
				case 0xF:
					printk("SNAP_SHOT\n");
					return SNAP_SHOT;
					break;				
				default:
					printk("wrong data\n");
					return 0;
					break;					
			}
			break;
		case 1:
			printk("from Scaler, source probe\n");
			switch(buf[0]){
				case 1:
					printk("USB disk\n");
					return SOURCE_UDISK;
					break;
				case 2:
					printk("USB display\n");
					return SOURCE_USB_DISPLAY;
					break;
				case 3:
					printk("Network display\n");
					return SOURCE_NETWORK_DISPLAY;
					break;
				case 4:
					printk("WiFi setting\n");
					return SOURCE_WIFI_SETTING;
					break;
				case 5:
					printk("LEAVE\n");
					return SOURCE_LEAVE;
					break;
				case 6:
					printk("connectPC\n");
					return SOURCE_PC;
					break;
				default:
					printk("wrong data\n");
					return 0;
					break;
			}
			break;
//		case 2: //for AM develop only
		case 0:
			printk("from Scaler, key\n");
			switch(buf[0]){
				case 1:
					printk("enter\n");
					return KEY_ENTER;
					break;
				case 2:
					printk("up\n");
					return KEY_UP;
					break;
				case 3:
					printk("left\n");
					return KEY_LEFT;
					break;
				case 4:
					printk("down\n");
					return KEY_DOWN;
					break;
				case 5:
					printk("right\n");
					return KEY_RIGHT;
					break;
				case 6:
					printk("exit\n");
					return KEY_EXIT;
					break;
				default:
					printk("wrong data\n");
					break;
			}
			break;
	}
	return 0;
}

#if 1
typedef enum {
	KEY_CODE_UP = 0x01,
	KEY_CODE_DOWN = 0x02,
	KEY_CODE_LEFT = 0x03,
	KEY_CODE_RIGHT = 0x04,
	KEY_CODE_BACK = 0x05,
	KEY_CODE_ENTER = 0x06,
	KEY_CODE_HOME = 0x07,
	KEY_CODE_SETUP = 0x08,
	KEY_CODE_REVERSE = 0x09,
	KEY_CODE_FAST_FORWARD = 0x0A,
	KEY_CODE_PREVIOUS = 0x0B,
	KEY_CODE_NEXT = 0x0C,
	KEY_CODE_PLAY_PAUSE = 0x0D,
	KEY_CODE_STOP = 0x0E,
	KEY_CODE_PAGE_UP = 0x10,
	KEY_CODE_PAGE_DOWN = 0x11,
} KeyType;

typedef enum {
	Source_CODE_MEDIA = 0x01,
	Source_CODE_LAN_WIFI = 0x02,
	Source_CODE_USB_B = 0x03,
	Source_CODE_HOME_SHARED = 0x04,
	Source_CODE_DLNA = 0x04,
	Source_CODE_VGA1 = 0x05,
	Source_CODE_VGA2 = 0x06,
	Source_CODE_HDMI1 = 0x07,
	Source_CODE_HDMI2 = 0x08,
	Source_CODE_AV = 0x09,
	Source_CODE_S_Video = 0x0A,
	Source_CODE_SysSetting = 0xB,
} SourceType;

typedef enum {
	Info_Language = 0x01,
	Info_CODE_VOLUME = 0x02,
	Info_CODE_SCALER_STATUS = 0x03,
//	Info_Volume_Get = 0x04,
//	Info_WiFi_Stutas = 0x05,
	Info_POWER_ON = 0x06,
//	Info_WiFi_IP = 0x07,
	Info_Lan_WiFi_OSD = 0x08,
	Info_Key_Info = 0x0b,
	Info_Key_A_a = 0x0c,
	Info_now_in_mode = 0x10,
	Filesystem_Off = 0x12,
} InfoType;
#define AFU_TEST_ADDR_1 CMU_DDRPLL2
#define AFU_TEST_ADDR_2 CMU_LCDPLL
#define AFU_TEST_ADDR_3 CMU_COREPLL

#endif

static void uartcom_scan(){ //every 40ms

#if PROJECTOR_UART
	int i=0;
	int temp_volume = 20;
	int len;
	UART_PACKET cmd_packet;
	INT8U *pbuf = (INT8U*)&cmd_packet;
	INT8U cmd_data;
	INT32U idx0,idx1;
	INT8U key = 0;	
	INT8U ack_buff[13]={0};
	int cmd;
	//INT8U vol_cmd1[7] ={0x2a,0x20,0x01,0x03,0x02,0x02,0x32};
	//INT8U vol_cmd2[7] ={0x2a,0x20,0x01,0x03,0x02,0x02,0x5f};
	//int vol_change = 0;
//	if(ACK_Volume_lock == 0 ){
//		comResponseCommand(AM_ver,9);
//	}
	memset(&cmd_packet, 0, sizeof(cmd_packet));
	//if( vol_change % 100 == 97 ){
	//	comResponseCommand(vol_cmd1,7);
	//}else if(vol_change % 100 == 71 ){
	//	comResponseCommand(vol_cmd2,7);
	//}
	//vol_change++;
	//if( vol_change > 4294967290 ) vol_change = 0;
#if ACK_EN
	if(ack_wait==1){ //send ack_cmd to scaler
		comResponseCommand(ack_cmd,5);
	}
#endif
	len = comGetCommand((INT8U*)&cmd_packet);
	if(len<=0 ){
		goto SCAN_OUT;
	}else{
		DBG_UARTCOM_MSG("len == %d \n", len);
	}
	ack_buff[0]=cmd_packet.header[0];
	ack_buff[1]=cmd_packet.header[1];
	ack_buff[2]=cmd_packet.atop[0];
	ack_buff[3]=cmd_packet.atop[1];
	ack_buff[4]=cmd_packet.command[0];
	ack_buff[5]=cmd_packet.command[1];
	ack_buff[6]=cmd_packet.command[2];
	ack_buff[7]=cmd_packet.command[3];
	ack_buff[8]=cmd_packet.data[0];
	ack_buff[9]=cmd_packet.data[1];
	ack_buff[10]=cmd_packet.data[2];
	ack_buff[11]=cmd_packet.data[3];
	ack_buff[12]=cmd_packet.data[4];
	//DBG_UARTCOM_MSG("\n\nack_buff:%x\n\n",ack_buff);
	
	if(cmd_packet.atop[1] == 0x01){
		DBG_UARTCOM_MSG("Key_type-->ack_buff[3]: 0x%02x\n", ack_buff[3]);
		switch(cmd_packet.command[1]) {		
			case KEY_CODE_UP: // 0x01
				//key = 0x16;
				key = 0x01;
				break;
			case KEY_CODE_DOWN: // 0x02
				//key = 0x18;
				key = 0x02;
				break;
			case KEY_CODE_LEFT: // 0x03
				//key = 0x19;
				key = 0x03;
				break;
			case KEY_CODE_RIGHT: // 0x04
				//key = 0x1b;
				key = 0x04;
				break;
			case KEY_CODE_BACK: // 0x05
				//key = 0x14;
				key = 0x13;
				break;				
			case KEY_CODE_ENTER: // 0x06
				//key = 0x1a;
				key = 0x06;
				break;
			case KEY_CODE_HOME: // 0x07
				key = 0xb;
				break;
			case KEY_CODE_SETUP: // 0x08 // reserve to use for furture
				/* KenoHuang masks, because no usage
				key = ???;
				*/
				break;
			case KEY_CODE_REVERSE: // 0x09
				FF_FB_multi--;
				DBG_UARTCOM_MSG("\nreverse_mode key Get :%d\n\n",FF_FB_multi);
				key = 0x40;
				break;
			case KEY_CODE_FAST_FORWARD: //0x0a
				FF_FB_multi++;
				DBG_UARTCOM_MSG("\nforward_mode key Get :%d\n\n",FF_FB_multi);
				key = 0x41;
				break;
			case KEY_CODE_PREVIOUS: //0x0b
				DBG_UARTCOM_MSG("\nreverse_mode key Get :%d\n\n",FF_FB_multi);
				if( FF_FB_multi != 0 ){
					if( FF_FB_multi > 0 || Now_in == 2 )
						FF_FB_multi = 0;
					else
						FF_FB_multi--;
					key = 0x40;
				}else
					key = 0x17;
				break;
			case KEY_CODE_NEXT: // 0x0c
				DBG_UARTCOM_MSG("\nforward_mode key Get :%d\n\n",FF_FB_multi);
				if( FF_FB_multi != 0 ){
					if( FF_FB_multi < 0 || Now_in == 2 )
						FF_FB_multi = 0;
					else
						FF_FB_multi++;
					key = 0x41;
				}else
					key = 0x1c;
				break;
			case KEY_CODE_PLAY_PAUSE:
				FF_FB_multi = 0;
				key = 0x42;
				break;
			case KEY_CODE_STOP:
				key = 0x43;
				break;
			case KEY_CODE_PAGE_UP:
				key = 0x50;//144
				break;
			case KEY_CODE_PAGE_DOWN:
				key = 0x51;//145
				break;
		}
	}else if(cmd_packet.atop[1] == 0x02){
		DBG_UARTCOM_MSG("source_type-->ack_buff[3]: 0x%02x\n", ack_buff[3]);
		switch(cmd_packet.command[1]) {
			case Source_CODE_MEDIA:
				key = 0x30; //224
				break;
			case Source_CODE_LAN_WIFI:  //eDisplay pro
				DBG_UARTCOM_MSG("\n\nTo Lan_Wifi\n\n");
				key = 0x31; //225
				break;				
			case Source_CODE_USB_B:
				key = 0x32; //226
				break;				
			case Source_CODE_HOME_SHARED:
				key = 0x33; //227
				break;
			case Source_CODE_VGA1:
				key = 0x34; //228
				break;
			case Source_CODE_VGA2:
				key = 0x35; //229
				break;
			case Source_CODE_HDMI1:
				key = 0x36; //230
				break;
			case Source_CODE_HDMI2:
				key = 0x37; //231
				break;
			case Source_CODE_AV:
				key = 0x38; //232
				break;
			case Source_CODE_S_Video:
				key = 0x39; //233
				break;
			case Source_CODE_SysSetting:
				DBG_UARTCOM_MSG("\n\nTo EZmedia Setting System\n\n");
				key = 0x40; //234
				break;
		}
	}else if(cmd_packet.atop[1] == 0x03){
		DBG_UARTCOM_MSG("\n\nInfo key Get\n\n");
		switch(cmd_packet.command[1]) {	
			case Info_Language://1
				language = cmd_packet.command[2];
				DBG_UARTCOM_MSG("\n\nTo language_Get:%d\n\n",language);
				key = 0x44; //229
				break;
			case Info_CODE_VOLUME://2
				temp_volume = cmd_packet.command[2];
				//判斷是否在正確的mode，若是不正確的mode則給上層正確的值並且把回傳正確該有的行為~~
				if( mode_name != 0 && mode_name != 1 && mode_name != 2 && mode_name != 3 && Now_in != 2 && Now_in != 4 && Now_in != 6 && Now_in != 12 && Now_in != 15 && Now_in != 16 && Now_in != 17 && Now_in != 19 ){
					if( temp_volume > volume_value ){//送右鍵
						send_key_press(FUI_KEYACTION_DOWN_MASK,0x1b);
					}else if( temp_volume < volume_value ){//送左鍵
						send_key_press(FUI_KEYACTION_DOWN_MASK,0x19);
					}
					//解鎖
					DBG_UARTCOM_MSG("Now_out!!\n");
					send_scaler_message[4] = 0x02;
					send_scaler_message[5] = 0x04;
					send_scaler_message[6] = 0x01;
					send_scaler_message[7] = 0x55;
					comResponseCommand(send_scaler_message,8);//Volume_unlock
					volume_value = temp_volume;
					key = 0x00;
				}else{
					volume_value = temp_volume;
					ack_buff[7] = 0;
					for( i = 0 ; i < 7 ; i++)
						ack_buff[7] += ack_buff[i];
					comResponseCommand(ack_buff,8);
					DBG_UARTCOM_MSG("\n\nTo Volume_Get\n\n");
					key = 0x45; //228
				}
				break;
			case Info_CODE_SCALER_STATUS://3
				key = 0x46; //238
				break;
			case Info_POWER_ON://6
				//ACK_Volume_lock = 1;
				AM_ver[8] = 0x00;
				for( i = 0; i < 8 ; i++ ){
					AM_ver[8] += AM_ver[i];
				}
				language = cmd_packet.command[2];
				send_key_press(FUI_KEYACTION_DOWN_MASK,0x44);
				comResponseCommand(AM_ver,9);
				mode_name = cmd_packet.command[3];
				scaler_ver[0] = cmd_packet.data[0];
				scaler_ver[1] = cmd_packet.data[1];
				scaler_ver[2] = cmd_packet.data[2];
				source_now = cmd_packet.data[3]+0x2f;
				DBG_UARTCOM_MSG("\n\n!!!!!!!!!!To source_now ::::0x%02x\n",cmd_packet.data[3]);
				DBG_UARTCOM_MSG("!!!!!!!!!!To source_now ::::0x%02x\n\n",source_now);
				Lan_WiFi_OSD = cmd_packet.data[4];
				send_key_press(FUI_KEYACTION_DOWN_MASK,0x90);
				key = source_now; //source
				//test
				printk("%s lang=%d\n", __func__, language);
				printk("%s mode_name=%d\n", __func__, mode_name);
				printk("%s scaler_ver1=%d\n", __func__, scaler_ver[0]);
				printk("%s scaler_ver2=%d\n", __func__, scaler_ver[1]);
				printk("%s scaler_ver3=%d\n", __func__, scaler_ver[2]);
				printk("%s source_now=%d\n", __func__, source_now);
				printk("%s Lan_WiFi_OSD=%d\n", __func__, Lan_WiFi_OSD);
				break;
			case Info_Lan_WiFi_OSD://8
				Lan_WiFi_OSD = cmd_packet.command[2];
				DBG_UARTCOM_MSG("\nLan_WiFi_OSD: 0x%02x\n", Lan_WiFi_OSD);
				key = 0x90;
				break;
			case Info_Key_Info://11
				key = 0x48; //239
				break;
			case Info_Key_A_a://12
				key = 0x49; //240
				break;
			case Info_now_in_mode://16 scaler request a mode
			DBG_UARTCOM_MSG("!!!!!!!!!!Now_in mode from scaler ::::0x%02x\n\n",Now_in);
				Now_in_mode[7] = Now_in;
				for( i = 0; i < 7 ; i++ ){
					Now_in_mode[8] += Now_in_mode[i];
				}
				comResponseCommand(Now_in_mode,8);
				break;
			case Filesystem_Off://18
				key = 0x52; //241
				break;	//shane
		}
	}
	/*if( Volume_lock == 1 && lock_err_flag == 0 ){//Prevent Volume lock uneffective
		if( key == 0x19 || key == 0x1b ){
			send_scaler_message[4] = 0x02;
			send_scaler_message[5] = 0x04;
			send_scaler_message[6] = 0x00;
			send_scaler_message[7] = 0x54;
			comResponseCommand(send_scaler_message,8);//Volume_lock
			lock_err_flag = 1;
		}
	}*/
	if(key!=0){
		DBG_UARTCOM_MSG("key: 0x%02x\n", key);	
		send_key_press(FUI_KEYACTION_DOWN_MASK,key);
	}
#else
	int i=0;
	INT8U len;
	INT8U buff[50];
	INT8U cmd_data;
	INT32U idx0,idx1;
	INT8U key = 0;	
	INT8U ack_buff[13]={0};
	int cmd;
	//INT8U vol_cmd1[7] ={0x2a,0x20,0x01,0x03,0x02,0x02,0x32};
	//INT8U vol_cmd2[7] ={0x2a,0x20,0x01,0x03,0x02,0x02,0x5f};
	//int vol_change = 0;
//	if(ACK_Volume_lock == 0 ){
//		comResponseCommand(AM_ver,9);
//	}
	//if( vol_change % 100 == 97 ){
	//	comResponseCommand(vol_cmd1,7);
	//}else if(vol_change % 100 == 71 ){
	//	comResponseCommand(vol_cmd2,7);
	//}
	//vol_change++;
	//if( vol_change > 4294967290 ) vol_change = 0;
#if ACK_EN
	if(ack_wait==1){ //send ack_cmd to scaler
		comResponseCommand(ack_cmd,5);
	}
#endif
#if 0//richardyu 032814
//	INT8U test_cmd[]={0x28,0x41,0x42,0x43,0x29};
	static char tmp_count=0;	
	INT8U test_cmd[]={0x0d,0x2a,'p','o','w','=','o','n',0x23,0x0d};	
	if(tmp_count>0)tmp_count=2;
	if(tmp_count++==0){
		printk("send richardyu test cmd\n");
		comResponseCommand(test_cmd,10);
	}
#endif

	uart_enable(0);
	len = comGetCommand(buff);
	uart_enable(1);	
	//DBG_UARTCOM_MSG("len == %d \n", len);
	INT8U temp[MAX_CMD_BUFF]={0};
	INT8U size = parseCmd(temp,buff,len);
	if(size==0){
		goto SCAN_OUT;
	}else{
		printk("save cmd to buffer\n");
		save_cmd_result(temp,size);
	}
	/*
	if(notify!=NULL){
		(*notify)(temp,size);
	}else{
		printk("notify is null\n");
	}
	*/

	send_key_press(FUI_KEYACTION_DOWN_MASK,0x99);	
#endif

SCAN_OUT:
	mod_timer(&N_timer,jiffies +SCAN_TIME);
}


static ssize_t am_uartcom_read(struct file *pfile, char __user *buf, size_t size, loff_t *offset)
{
	int i;
	int readsize;
	int datalen = 0;
	if(recvidx_lan < readidx_lan){
		datalen = BUF_SIZE-readidx_lan+recvidx_lan;
	}
	else{
		datalen = recvidx_lan - readidx_lan;
	}
	if(size > datalen)
		readsize  = datalen;
	else
		readsize = size;
	if(readsize!=0){
		DBG_UARTCOM_MSG("AM: ATOP read!!\n");
		DBG_UARTCOM_MSG("recv_idx = %d,read_idx = %d,size = %d, readsize = %d\n",recvidx_lan,readidx_lan,size,readsize);
	}
	/*
	read_idx += readsize;
		
	if(read_idx >= BUF_SIZE){
		read_idx = read_idx - BUF_SIZE;
	}
	*/
	for(i = 0; i < readsize; i++){
		if((readidx_lan+i) < BUF_SIZE){
			buf[i] = uartreadbuf[readidx_lan+i];
		}
		else{
			buf[i] = uartreadbuf[(readidx_lan+i)-BUF_SIZE];
		}
	}
	/*
	if((readbuf_idx+i) < BUF_SIZE){
		readbuf_idx = i;
	}
	else{
		readbuf_idx = readbuf_idx+i-BUF_SIZE;
	}
	*/
	readidx_lan += readsize;
		
	if(readidx_lan >= BUF_SIZE){
		readidx_lan = readidx_lan - BUF_SIZE;
	}
	return readsize;
}

static ssize_t  am_uartcom_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)
{
	int i;
	/*
	for(i = sendsize; i < (sendsize+size); i++){
		uartsendbuf[sendsize+i] = buf[i];
	}
	flag_send_req = 1;
	sendsize += size;
	*/
	char tmpbuf[300];
	memset(tmpbuf,0,300);
	copy_from_user(tmpbuf, buf, size);	
	DBG_UARTCOM_MSG("write to buf = %s(%d)\n",tmpbuf,size);
	DBG_UARTCOM_MSG("[ ");
	for(i = 0;i < size; i++){
		uartsendbuf[send_idx] = tmpbuf[i];
		DBG_UARTCOM_MSG("%x",uartsendbuf[send_idx]);
		send_idx++;
		if(send_idx >= BUF_SIZE){
			send_idx = BUF_SIZE - send_idx;
		}
	}
	DBG_UARTCOM_MSG(" ]");
	DBG_UARTCOM_MSG(" will be writen to scaler\n\n");
	//flag_send_req = 1;
	//sendsize = 0;
	writeOut();
	
	DBG_UARTCOM_MSG("writed %d bytes\n",i);
	return i;
}

static unsigned int am_uartcom_poll(struct file *pfile, struct poll_table_struct *ptable)
{
	return 0;
}


static int am_uartcom_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	INT8U *arg_str;
	
	char power_on[20] = {0};
	int Client_IP[4];
	int SetupOSD_Lock = 0;
	int i= 0;
	int temp_volume;

	INT32U* spread_enable = (INT32U*)(AFU_TEST_ADDR_1);
	INT32U* lcd_test = (INT32U*)(AFU_TEST_ADDR_2);
	unsigned int  rtc_reg=0;//richardyu 060914	
	
	switch(cmd){
		case IOCTL_SET_ATOP_VERSION: //1
			memset(atop_ver, 0, 20);
			copy_from_user(atop_ver,(void*)arg,20);
			printk("atop ver = %s\n",atop_ver);
			break;
		case IOCTL_GET_SCALER_VERSION: //2
			copy_to_user((void*)arg,(void*)scaler_ver,3);
			break;
		case IOCTL_GET_ACTIONS_INFO: //3
			copy_to_user((void*)arg,(void*)actions_info,15);
			break;
		case IOCTL_GET_LANGUAGE: //4
			DBG_UARTCOM_MSG("IOCTL_GET_LANGUAGE::::%d!!\n",language);
			/*temp mark*/
			//while( language == -1){if( language != -1)break;}
			/*temp mark*/
			copy_to_user((void*)arg,&language,sizeof(int));
			DBG_UARTCOM_MSG("\n\n!!!!!!!!!!To source_now ::::0x%02x\n\n",source_now);
			//scaler 切過來
			send_scaler_message[4] = 0x01;
			send_scaler_message[5] = 0x0f;
			send_scaler_message[6] = 0x00;
			for( i = 0 ; i < 6 ; i++ )
			{
				send_scaler_message[6] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,7);
			DBG_UARTCOM_MSG("\n\n!!!!!!!!!!After source_ready ::::0x%02x\n\n",source_now);
			//if( Source_once == 0 && source_now != -1 ){
			//	DBG_UARTCOM_MSG("Source_onceSource_onceSource_onceSource_oncesource_nowsource_nowsource_now::::0x%02x!!\n",source_now);
			//	send_key_press(FUI_KEYACTION_DOWN_MASK,source_now);
			//}
			send_key_press(FUI_KEYACTION_DOWN_MASK,0x90);
			break;
		case IOCTL_GET_VOLUME:   //5
			DBG_UARTCOM_MSG("IOCTL_GET_VOLUME::::%d!!\n",volume_value);
			copy_to_user((void*)arg,&volume_value,sizeof(int));
			break;
		case IOCTL_GET_NOW_IN_MODE://6
			Now_in = arg;
//			copy_from_user((int*)Now_in,(void*)arg,4);
			DBG_UARTCOM_MSG("Now_in_Mode::::%d!!\n",Now_in);
			if( Now_in == 2 || Now_in == 4 || Now_in == 6 || Now_in == 12 || Now_in == 15 || Now_in == 16 ){//add Now_in == 1x  USB_Display or WiFi_Display
				DBG_UARTCOM_MSG("Now_in!!\n");
				send_scaler_message[4] = 0x02;
				send_scaler_message[5] = 0x04;
				if( Volume_lock == 0 ){
					DBG_UARTCOM_MSG("SendMessage!!\n");
					send_scaler_message[6] = 0x00;
					send_scaler_message[7] = 0x54;
					comResponseCommand(send_scaler_message,8);//Volume_lock
					Volume_lock = 1;
				}
			}else{
				if( Volume_lock == 1 ){
					DBG_UARTCOM_MSG("Now_out!!\n");
					send_scaler_message[4] = 0x02;
					send_scaler_message[5] = 0x04;
					send_scaler_message[6] = 0x01;
					send_scaler_message[7] = 0x55;
					comResponseCommand(send_scaler_message,8);//Volume_unlock
					Volume_lock = 0;
					lock_err_flag = 0;
				}
			}
			if(Now_in == 17){
				send_scaler_message[4] = 0x02;
				send_scaler_message[5] = 0x0d;
				send_scaler_message[6] = 0x01;
				send_scaler_message[7] = 0x00;
				for( i = 0 ; i < 7 ; i++ ){
					send_scaler_message[7] += send_scaler_message[i];
				}
				comResponseCommand(send_scaler_message,8);//Photo_enter_lock
				Enter_lock = 1;
			}else{
				if( Enter_lock == 1 ){
					send_scaler_message[4] = 0x02;
					send_scaler_message[5] = 0x0d;
					send_scaler_message[6] = 0x00;
					send_scaler_message[7] = 0x00;
					for( i = 0 ; i < 7 ; i++ ){
						send_scaler_message[7] += send_scaler_message[i];
					}
					comResponseCommand(send_scaler_message,8);//Photo_enter_lock
					Enter_lock = 0;
				}
			}
			break;
		case IOCTL_GET_WIFI_STATUS://7
			//Wifi_Status = arg;
			copy_from_user(&Wifi_Status,(void*)arg, sizeof(int));
			if(Wifi_Status == 8 )
				Wifi_Status = 1;
			DBG_UARTCOM_MSG("Wifi_Status::::%d!!\n",Wifi_Status);
#if PROJECTOR_UART
			send_scaler_message[0] = 0x2a;
			send_scaler_message[1] = 0x20;
			send_scaler_message[2] = 0x01;
			send_scaler_message[3] = 0x03;
#endif
			send_scaler_message[4] = 0x02;
			send_scaler_message[5] = 0x05;
			send_scaler_message[6] = Wifi_Status;
			send_scaler_message[7] = 0x00;//0x55+Wifi_Status;
			for( i = 0; i < 7 ; i++ ){
				send_scaler_message[7] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,8);
			if( Wifi_Status == 2 ){
				send_scaler_message[4] = 0x09;
				send_scaler_message[5] = 0x07;
				send_scaler_message[6] = 0xc0;
				send_scaler_message[7] = 0xa8;
				send_scaler_message[8] = 0x64;
				send_scaler_message[9] = 0x0a;
				send_scaler_message[10] = 0xff;
				send_scaler_message[11] = 0xff;
				send_scaler_message[12] = 0xff;
				send_scaler_message[13] = 0x00;
				send_scaler_message[14] = 0x00;
				for( i = 0; i < 14 ; i++ ){
					send_scaler_message[14] += send_scaler_message[i];
				}
				send_scaler_message[14] %= 256;
				comResponseCommand(send_scaler_message,15);
			}
			break;
		case IOCTL_GET_POWER_ON_MESSAGE://8
			power_on[0] = language;
			power_on[1] = mode_name;
			power_on[2] = scaler_ver[0];
			power_on[3] = scaler_ver[1];
			power_on[4] = scaler_ver[2];
			power_on[5] = source_now;
			power_on[6] = Lan_WiFi_OSD;
			printk("%s lang=%d\n", __func__, power_on[0]);
			printk("%s mode_name=%d\n", __func__, power_on[1]);
			printk("%s scaler_ver1=%d\n", __func__, power_on[2]);
			printk("%s scaler_ver2=%d\n", __func__, power_on[3]);
			printk("%s scaler_ver3=%d\n", __func__, power_on[4]);
			printk("%s source_now=%d\n", __func__, power_on[5]);
			printk("%s Lan_WiFi_OSD=%d\n", __func__, power_on[6]);
			copy_to_user((void*)arg,(void*)power_on,7);
			break;

		case IOCTL_GEL_MODEL_NAME://9
			DBG_UARTCOM_MSG("mode_name:%d!!!\n",mode_name);
			copy_to_user((void*)arg,&mode_name,sizeof(int));
			break;
		case IOCTL_GET_CLIENT_IP://10
			copy_from_user((void*)Client_IP,(void*)arg,4);
#if PROJECTOR_UART
			send_scaler_message[0] = 0x2a;
			send_scaler_message[1] = 0x20;
			send_scaler_message[2] = 0x01;
			send_scaler_message[3] = 0x03;
#endif
			send_scaler_message[4] = 0x09;
			send_scaler_message[5] = 0x07;
			send_scaler_message[6] = Client_IP[0];
			send_scaler_message[7] = Client_IP[1];
			send_scaler_message[8] = Client_IP[2];
			send_scaler_message[9] = Client_IP[3];
			send_scaler_message[10] = 0xff;
			send_scaler_message[11] = 0xff;
			send_scaler_message[12] = 0xff;
			send_scaler_message[13] = 0x00;
			send_scaler_message[14] = 0x00;
			DBG_UARTCOM_MSG("Client_IP[0]:%d!Client_IP[1]:%d!Client_IP[2]:%d!Client_IP[3]:%d!!!\n",Client_IP[0],Client_IP[1],Client_IP[2],Client_IP[3]);
			for( i = 0; i < 14 ; i++ ){
				send_scaler_message[14] += send_scaler_message[i];
			}
			send_scaler_message[14] %= 256;
			comResponseCommand(send_scaler_message,15);
			break;
		case IOCTL_SET_LAN_WIFI_OSD://11
			//Lan_WiFi_OSD =1;
			//DBG_UARTCOM_MSG("Lan_WiFi_OSD:%d!!!\n",Lan_WiFi_OSD);
			//DBG_UARTCOM_MSG("Lan_WiFi_OSD:%d!!!\n",Lan_WiFi_OSD);
			DBG_UARTCOM_MSG("Lan_WiFi_OSD:%d!!!\n",Lan_WiFi_OSD);
			copy_to_user((void*)arg,&Lan_WiFi_OSD,sizeof(int));
			break;
		case IOCTL_SET_FREEZE://12
			send_scaler_message[4] = 0x01;
			send_scaler_message[5] = 0x0a;
			send_scaler_message[6] = 0;
			for( i = 0; i < 6 ; i++ ){
				send_scaler_message[6] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,7);
			break;
		case IOCTL_SET_HIDE://13
			send_scaler_message[4] = 0x01;
			send_scaler_message[5] = 0x09;
			send_scaler_message[6] = 0;
			for( i = 0; i < 6 ; i++ ){
				send_scaler_message[6] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,7);
			break;
		case IOCTL_AM_VERSION://14
			AM_ver[8] = 0x00;
			for( i = 0; i < 8 ; i++ ){
				AM_ver[8] += AM_ver[i];
			}
			DBG_UARTCOM_MSG("AM_ver=%d\n",AM_ver[8]);
			comResponseCommand(AM_ver,9);
			break;
		case IOCTL_SETUP_OSD://15
			SetupOSD_Lock = arg;
			DBG_UARTCOM_MSG("SetupOSD_Lock=%d\n",SetupOSD_Lock);
			DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
			if( Now_in == 17 ){
				send_scaler_message[4] = 0x02;
				send_scaler_message[5] = 0x0d;
				if(SetupOSD_Lock == 0){
					send_scaler_message[6] = 0x00;
					temp_Now_in = Now_in;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
					Now_in = 18;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
				}else if(SetupOSD_Lock == 1){
					send_scaler_message[6] = 0x01;
					Now_in = temp_Now_in;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
				}
				send_scaler_message[7] = 0x00;
				for( i = 0; i < 7 ; i++ ){
					send_scaler_message[7] += send_scaler_message[i];
				}
			}else{
				send_scaler_message[4] = 0x02;
				send_scaler_message[5] = 0x04;
				if(SetupOSD_Lock == 0){
					send_scaler_message[6] = 0x03;
					temp_Now_in = Now_in;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
					Now_in = 18;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
				}else if(SetupOSD_Lock == 1){
					send_scaler_message[6] = 0x00;
					Now_in = temp_Now_in;
					DBG_UARTCOM_MSG("Now_in=%d\n",Now_in);
				}
				send_scaler_message[7] = 0x54+send_scaler_message[6];
			}
			comResponseCommand(send_scaler_message,8);
			break;
		case IOCTL_GET_SOURCE://16
			DBG_UARTCOM_MSG("\n\n!!!!!!!!!!To source_now ::::%d\n\n",source_now);
			copy_to_user((void*)arg,&source_now,sizeof(int));
			break;
		case IOCTL_OPEN_SPECTRUM://17
			DBG_UARTCOM_MSG(KERN_INFO "richardyu spread spectrum...\n");
			DBG_UARTCOM_MSG(KERN_INFO "original  CMU_DDRPLL2 == 0x%x\n",act_readl(AFU_TEST_ADDR_1));	
			DBG_UARTCOM_MSG(KERN_INFO "original  CMU_LCDPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_2));
			DBG_UARTCOM_MSG(KERN_INFO "original  CMU_COREPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_3));
			INT32U* spread_enable = (INT32U*)(AFU_TEST_ADDR_1);
			spread_enable = (INT32U*)(AFU_TEST_ADDR_2);
			*spread_enable = (*spread_enable & 0xfffffffd) | 0x00000002; //LCD, 2 enable, 0 disable
			INT32U* lcd_test = (INT32U*)(AFU_TEST_ADDR_2);
			*lcd_test = (*lcd_test & 0xff3fffff) | 0x00000000; // 0->0.5 4->1
			DBG_UARTCOM_MSG(KERN_INFO "richardyu changing value...\n");
			DBG_UARTCOM_MSG(KERN_INFO "new CMU_DDRPLL2 == 0x%x\n",act_readl(AFU_TEST_ADDR_1));
			DBG_UARTCOM_MSG(KERN_INFO "new CMU_LCDPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_2));
			DBG_UARTCOM_MSG(KERN_INFO "new CMU_COREPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_3));
			break;
		case IOCTL_OTA_BLOCK://18
			ota_block = arg;
			send_scaler_message[4] = 0x02;
			send_scaler_message[5] = 0x11;
			send_scaler_message[6] = ota_block;
			for( i = 0; i < 7 ; i++ ){
					send_scaler_message[7] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,8);
			break;
		case CMD_GET_RESULT:
			printk("CMD_GET_RESULT\n");
			char result[50];
			memset(result,0,50);
			int len = get_cmd_result(result);
			if(len!=0){
				copy_to_user((void *)arg,(void *)result,len);
			}
			break;
		case CMD_START_GETVAL:
			
			start_getval_flag=1;
			id=-1;
			break;
		case CMD_GET_INITVAL:
			printk("get benq cur init value!\n");	
			if(start_getval_flag==0){
				copy_to_user((void *)arg,(void *)&uartbuf[id],strlen(&uartbuf[id])+1);

			}
			else
				copy_to_user((void *)arg,(void *)NULL,0);	
			break;
		case CMD_GET_FACTRESULT:
			DBG_UARTCOM_MSG("get uart test result! get_testresult_flag=%d factestbuf=%s\n",get_testresult_flag,factestbuf);	
			if(get_testresult_flag==0){
				copy_to_user((void *)arg,(void *)&factestbuf,strlen(factestbuf));
				get_testresult_flag=-1;

			}
			else
				copy_to_user((void *)arg,(void *)NULL,0);	
			break;
		case EZCASTPRO_USB1_OFF://21
			// power off USB P1
			rtc_reg = RTC_READ(RTC_ALARM);
			printk("EZCASTPRO_USB1_OFF RTC_ALARM=%0x\n",rtc_reg);
			RTC_WRITE(((RTC_READ(RTC_ALARM)&0xffffdeff)|0x200),RTC_ALARM);
			break;
		case IOCTL_SET_ATOP_VOLUME: //22
			copy_from_user(&temp_volume,(void*)arg, sizeof(int));
			DBG_UARTCOM_MSG("\n\n!!!!!!!!send volume %d\n", temp_volume);
			//temp_volume = arg;
#if PROJECTOR_UART
			send_scaler_message[0] = 0x2a;
			send_scaler_message[1] = 0x20;
			send_scaler_message[2] = 0x01;
			send_scaler_message[3] = 0x03;
#endif
			send_scaler_message[4] = 0x02;
			send_scaler_message[5] = 0x02;
			send_scaler_message[6] = (temp_volume==1)?0x01:0xFF;
			send_scaler_message[7] = 0x00; //checksum

			for( i = 0; i < 7 ; i++ ){
				send_scaler_message[7] += send_scaler_message[i];
			}			
			comResponseCommand(send_scaler_message,8);
			break;
		case IOCTL_WATCHDOG://25
			DBG_UARTCOM_MSG("[soft watch dog] Dog growl\n");
			send_scaler_message[0] = 0x2a;
			send_scaler_message[1] = 0x20;
			send_scaler_message[2] = 0x01;
			send_scaler_message[3] = 0xff;
			send_scaler_message[4] = 0x01;
			send_scaler_message[5] = 0x00;
			send_scaler_message[6] = 0x00; //checksum
			for(i = 0; i < 6;i++){
				send_scaler_message[6] += send_scaler_message[i];
			}
			comResponseCommand(send_scaler_message,7);
			break;
	}
	return 0;
}

static void comResponseCommand(INT8U *cmd, INT8U len)   //return msg to scaler
{
	INT8U i;
	//--for(i = 0; i<len ; i++) printk("%02X ",cmd[i]);
	while(outsem){
		i = 0;
        /* Mos: add msleep to avoid Dead lock when multiple write occured */
        msleep(1);
	}
	outsem = 1;
	DBG_UARTCOM_MSG("len=%d, do _PutC ",len);
	for(i = 0; i<len ; i++){	
		DBG_UARTCOM_MSG("%d, ",cmd[i]);
		_PutC(cmd[i]);
	}
	DBG_UARTCOM_MSG("!!\n");
	outsem = 0;
}

static int am_uartcom_open(struct inode *pinode, struct file *pfile)
{
	DBG_UARTCOM_MSG("am_uartcom_open\n");
	return 0;
}

static int am_uartcom_release(struct inode *pinode, struct file *pfile)
{
	DBG_UARTCOM_MSG("am_uartcom_release\n");
	return 0;
}

static struct file_operations uartcom_drv_fops=
{
	.owner  = THIS_MODULE,
	.read = am_uartcom_read,
	.write = am_uartcom_write,
	.poll = am_uartcom_poll,
	.ioctl = am_uartcom_ioctl,
	.open = am_uartcom_open,
	.release = am_uartcom_release,
};

static vga_OFN_to_GPIO18(void)
{
	INT32U* gpio18 = (INT32U*)(GPIO_MFCTL0);
	*gpio18=(*gpio18&0xFFFFFF0F)|0x00000040;
	//for IR remote
#if IR_REMOTE
	INT32U* IR_pin = (INT32U*)(GPIO_MFCTL6);
	*IR_pin=(*IR_pin&0xFFFFFFF0)|0x00000003;
#endif
	printk("<0> set gpio18\n");
}

static INT32S  __init Scaler_uartcom_init(void)
{
	INT32S result=0;
	int regddrpll,regcorepll;

	DBG_UARTCOM_MSG("*********Scaler_uartcom_init\n");

	#if 0
	result = alloc_chrdev_region(&sys_msg_dev, 0, SYSMSG_MAX_DEVS, "sysmsgdrv");
	#else
	uartcom_dev =MKDEV(AM_UARTCOM_MAJOR,0);
	result = register_chrdev_region(uartcom_dev,1,"uartcomdrv");
	#endif
	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for uartcom\n");
		return -EIO;
	}
	DBG_UARTCOM_MSG("uartcom major=%d, minor=%d\n",MAJOR(uartcom_dev),MINOR(uartcom_dev));

	uartcom_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!uartcom_cdev){
		printk(KERN_ERR "malloc memory  fails for uartcom device\n");
		unregister_chrdev_region(uartcom_dev,SYSMSG_MAX_DEVS);
		return -ENOMEM;
	}
  	cdev_init(uartcom_cdev, &uartcom_drv_fops);
	if(cdev_add(uartcom_cdev, uartcom_dev, 1))
		goto out_err;




#if AFU_TEST_ON

	DBG_UARTCOM_MSG(KERN_INFO "richardyu spread spectrum...\n");
	DBG_UARTCOM_MSG(KERN_INFO "original  CMU_DDRPLL2 == 0x%x\n",act_readl(AFU_TEST_ADDR_1));	
	DBG_UARTCOM_MSG(KERN_INFO "original  CMU_LCDPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_2));
	DBG_UARTCOM_MSG(KERN_INFO "original  CMU_COREPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_3));
	DBG_UARTCOM_MSG(KERN_INFO "richardyu enable/disable...\n");	
	INT32U* spread_enable = (INT32U*)(AFU_TEST_ADDR_1);
//	*spread_enable = (*spread_enable & 0xffffff2f) | 0x00000000; //DDR 1 -> 0.5% 5 -> 1% 0 -> disable  
	
	spread_enable = (INT32U*)(AFU_TEST_ADDR_2);
	*spread_enable = (*spread_enable & 0xfffffffd) | 0x00000002; //LCD, 2 enable, 0 disable
//	spread_enable = (INT32U*)(AFU_TEST_ADDR_3);
//	*spread_enable = (*spread_enable & 0xfffffffd) | 0x00000002; //CORE, 2 enable, 0 disable

	DBG_UARTCOM_MSG(KERN_INFO "modified  CMU_DDRPLL2 == 0x%x\n",act_readl(AFU_TEST_ADDR_1));	
	DBG_UARTCOM_MSG(KERN_INFO "modified  CMU_LCDPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_2));
	DBG_UARTCOM_MSG(KERN_INFO "modified  CMU_COREPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_3));
	INT32U* lcd_test = (INT32U*)(AFU_TEST_ADDR_2);
	*lcd_test = (*lcd_test & 0xff3fffff) | 0x00000000; // 0->0.5 4->1
	
//	INT32U* core_test = (INT32U*)(AFU_TEST_ADDR_3);
//	*core_test = (*core_test & 0xff3fffff) | 0x00400000;  // 0->0.5 4->1
	DBG_UARTCOM_MSG(KERN_INFO "richardyu changing value...\n");
	DBG_UARTCOM_MSG(KERN_INFO "new CMU_DDRPLL2 == 0x%x\n",act_readl(AFU_TEST_ADDR_1));
	DBG_UARTCOM_MSG(KERN_INFO "new CMU_LCDPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_2));
	DBG_UARTCOM_MSG(KERN_INFO "new CMU_COREPLL == 0x%x\n",act_readl(AFU_TEST_ADDR_3));
#endif
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=8251//richardyu 070414
	DBG_UARTCOM_MSG(KERN_INFO "ezcastpro 8250, start uart, power on usb p1\n");
	uartcomInit();
	init_timer(&N_timer);
	N_timer.expires = jiffies +SCAN_TIME;
	N_timer.function = (void*)uartcom_scan;
	add_timer(&N_timer);
	regddrpll = act_readl(CMU_DDRPLL);
	printk("cmu ddrpll = %x\n",regddrpll);
	//turn on USB P1
	unsigned int rtc_reg=0;
	rtc_reg = RTC_READ(RTC_ALARM);
	RTC_WRITE(((RTC_READ(RTC_ALARM)&0xffffdfff)|0x300),RTC_ALARM);		
#else
	DBG_UARTCOM_MSG(KERN_INFO "ezcastpro not 8250, no uart, no usb p1\n");
#endif

	
//	vga_OFN_to_GPIO18(); //richardyu 0829
	return 0;
out_err:
	printk(KERN_ERR "register failed  for sysmsg device\n");
	kfree(uartcom_cdev);
	unregister_chrdev_region(uartcom_dev,SYSMSG_MAX_DEVS);
	return -ENODEV;

}

static void __exit Scaler_uartcom_exit(void)
{
	if(uartcom_cdev){
		cdev_del(uartcom_cdev);
		kfree(uartcom_cdev);
	}
	del_timer(&N_timer);
	free_irq(COM_IRQ_UART,uartbuf);
	unregister_chrdev_region(uartcom_dev,1);
}

/*
void set_callback(NOTIFY delegate){
	notify = delegate;
}
*/
//EXPORT_SYMBOL(set_callback);

module_init(Scaler_uartcom_init);
module_exit(Scaler_uartcom_exit);

MODULE_AUTHOR("Nick");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Actions-micro uart communication");
