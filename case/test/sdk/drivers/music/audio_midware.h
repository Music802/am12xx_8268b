#ifndef AUDIO_MIDWARE_H
# define	AUDIO_MIDWARE_H


//
// �ļ���Ϣ������ʱ�䣬�����ʵ�


#define			PLAY				0x01
#define			STOP				0x02
#define			pause				0x03
#define			SET_SEEK_TIME		0x04
#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF			0x08
#define			CANCEL_FB			0x09
                                                             	
#define			GET_MEDIA_INFO			0x13    	
#define			GET_PLAYER_STATUS		0x14    	//��ȡplayer��ǰ��״̬                                                               	

#define			SET_FILE			0x16		//��ָ����������ʱ,Ϊ�ļ���(����·��);

/**
 *@brief player tag struct
 */

/**
 *@brief player status enumeration.
 */
typedef enum{
		PLAYER_PLAYING,
		PLAYER_FF,
		PLAYER_BF,
		PLAYER_PAUSED,
		PLAYER_STOPPED,
		PLAYER_ERROR,
		FM_SEARCHING						//��radio����ʱ�õ���״̬
}player_status_t;							//��ǰdecoder״̬��Ϣ�ĳ���
/**
 *@brief play mode enumeration.
 */
typedef  enum{
		NORMAL_PLAY,						//��������					
		TAG_PLAY,						//TAG����
		SEEK_PLAY						//�ϵ�����/������seek����
} play_mode_t;
typedef struct{
			play_mode_t mode;					//��ǰ�Ĳ���ģʽ
			unsigned int param;					//��ͬ����ģʽ�µ��������
		}play_param_t;	
								//����ʱ�Ĳ���ģʽ	
/**
 *@brief playe parameter struct
 */
	
//��ͬ����ģʽʱ���������
typedef enum{
		ERR_OPEN_FILE,
		ERR_FILE_NOT_SUPPORT,
		ERR_DECODER_ERROR,
		ERR_NO_LICENSE,                    			//��license,���Ŵ��������򲥷�ʱ�䵽��
      		ERR_SECURE_CLOCK,			                        //DRMʱ�Ӵ���
      		ERR_LICENSE_INFO,                        			//DRM��Ϣ����
      		ERR_OTHER						//��������
}err_constant_t;



/**@}*/
/*********���ݽṹ����**********************/
/**
 *@brief music file infomation struct
 */
 #define FULL_PATH_SIZE	128

typedef struct{
		/*�ļ���*/	
		char filepath[FULL_PATH_SIZE];			/**< the file path*/
		char filename[FULL_PATH_SIZE];		/**< the file name */
		unsigned int file_index;				/**< the file index */
		unsigned int file_len;    				/**< length of the file */
		
		/*ý����Ϣ*/
		unsigned int total_time;				/**< record the total time of music*/
		unsigned int bitrate;					/**< record the bitrate of music */
		unsigned int sample_rate;				/**< record the sample rate of music */
		unsigned int channels;					/**< record the channels of music */
	
}music_file_info_t;
/**


 *@brief music status struct
 */
typedef struct{
		int file_index;						/**< index for current music */ //��ǰ���Ÿ���������
		int cur_time;						      /**< real time for current music */ //������ǰ�Ĳ���ʱ��		
		player_status_t status;				/**< player status */ //codec��ǰ��״̬		
		unsigned int err_no;					/**< error number */
	
}music_status_t;							//decoder��ǰ��Ҫ���ظ�ap��״̬��Ϣ,��Щ״̬flag����ֻ������


/*�����ӿ�*/
/**
 * @brief  Open the audio decoder.
 *
 * 
 * @param[in] param the Param is not used.
 * @return return the result of audio decoder open.
 * - open successfully:return a music_dec_engine_t struct with  (void *)type.
 * - open fail: return NULL.
 */ 
void *audioDecOpen(void *param);

int audioDecCmd(void *handle,unsigned int cmd,unsigned int param);

int audioDecClose(void *handle);

#endif

