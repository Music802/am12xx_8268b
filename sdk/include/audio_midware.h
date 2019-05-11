#ifndef AUDIO_MIDWARE_H
#define	AUDIO_MIDWARE_H


#ifdef  _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif


/**
* @decode cmd definition
*/
#define			PLAY				0x01
#define			STOP				0x02
#define			pause				0x03
#define			SET_SEEK_TIME			0x04
#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF			0x08
#define			CANCEL_FB			0x09
#define			GET_MEDIA_INFO			0x13    	
#define			GET_PLAYER_STATUS		0x14    	//��ȡplayer��ǰ��״̬                                                               	
#define			SET_FILE			0x16		//��ָ����������ʱ,Ϊ�ļ���(����·��);
#define			SET_FLASH			0x17
#define			SET_PCM_INFO		0x18
#define			SET_NOTIFY_FUNC		0x19
#define			TERMINAL			0x20
#define			SET_EQ				0x21
#define			GET_EQ				0x22

/**@}*/

#define			FLASHLOOP_FLAG			-55

/**
 *@brief player status enumeration.
 */
typedef enum{
		PLAYER_PLAYING=1,
		PLAYER_PAUSED,
		PLAYER_FF,
		PLAYER_BF,
		PLAYER_STOPPED
}player_status_t;							//��ǰdecoder״̬��Ϣ�ĳ���
/**
 *@brief play mode enumeration.
 */
typedef  enum{
		NORMAL_PLAY,						//��������					
		TAG_PLAY,						//TAG����
		SEEK_PLAY						//�ϵ�����/������seek����
} play_mode_t;

/**
 *@brief playe parameter struct
 */
	typedef struct{
			play_mode_t mode;					//��ǰ�Ĳ���ģʽ
			unsigned int param;					//��ͬ����ģʽ�µ��������
		}play_param_t;	
								//����ʱ�Ĳ���ģʽ	
/**
 *@brief error constant enumeration.
 */

typedef enum{
		ERR_OPEN_FILE=1,
		ERR_FILE_NOT_SUPPORT,
		ERR_DECODER_ERROR,
		ERR_NO_LICENSE,                    			//��license,���Ŵ��������򲥷�ʱ�䵽��
      	ERR_SECURE_CLOCK,			                        //DRMʱ�Ӵ���
      	ERR_LICENSE_INFO,                        			//DRM��Ϣ����
      	ERR_OTHER						//��������
}err_constant_t;

								/**
 *@brief enum for notify msg, used for notify_func.  
 */ 
typedef enum _audio_notify_msg_e
{
	AUDIO_FILE_INFO_READY_MSG,	/**< file information is filled by audio player */
	AUDIO_NO_SUPPORT_MSG,			/**< not support */
	AUDIO_STOP_OK_MSG,			/**< audio player is stopped */
}audio_notify_msg_e;




/*********���ݽṹ����**********************/
/**
 *@brief music file infomation struct
 */
 #define AUDIO_FULL_PATH_SIZE	768

typedef struct{
		/*�ļ���*/	
		char filepath[AUDIO_FULL_PATH_SIZE];			/**< the file path*/
		char filename[AUDIO_FULL_PATH_SIZE];		/**< the file name */
		unsigned int file_index;				/**< the file index */
		unsigned int file_len;    				/**< length of the file */
		
		/*ý����Ϣ*/
		unsigned int total_time;				/**< record the total time of music*/
		unsigned int bitrate;					/**< record the bitrate of music */
		unsigned int sample_rate;				/**< record the sample rate of music */
		unsigned int channels;					/**< record the channels of music */
	
}music_file_info_t;

typedef struct{
	unsigned int sample_rate;				/**< record the sample rate of music */
	unsigned int channels;					/**< record the channels of music */
	unsigned int bits_per_sample;
}pcm_info_t;
/**


 *@brief music status struct
 */
typedef struct{
		int file_index;						/**< index for current music */ //��ǰ���Ÿ���������
		int cur_time;						      /**< real time for current music */ //������ǰ�Ĳ���ʱ��		
		player_status_t status;				/**< player status */ //codec��ǰ��״̬		
		unsigned int err_no;					/**< error number */
		int cur_time_ms;			      /**< real time for current music */ //������ǰ�Ĳ���ʱ��		
	
}music_status_t;							//decoder��ǰ��Ҫ���ظ�ap��״̬��Ϣ,��Щ״̬flag����ֻ������
/**
 *@brief file opration function struct
 */
typedef struct _file_iocontext_s
{	
	char *file_name;
	void *handle;
	long (*read)(void *, unsigned char *, unsigned long);
	long (*seek_set)(void *, long);
	long (*seek_cur)(void *, long);
	long (*seek_end)(void *, long);
	long (*tell)(void *);
	long (*get_file_length)(void *);
}file_iocontext_s;

/**
 *@brief struct of set file, used for setting file information.  
 */
typedef struct _a_set_file_s
{
	file_iocontext_s *f_io;  /**< file io operation func*/
	music_file_info_t *pInfo;     /**< file information*/
}a_set_file_s;


/**
 *@brief equalizer struct
 */
typedef struct
{
	char name[32];
	unsigned char sliderpos[10];	//range from 0 to 100
	unsigned char preamppos;		//no use now
	unsigned char reserved;			//no use now
}eq_info_t;




/*�����ӿ�*/
/**
 * @brief  Open the audio decoder.
 *
 * 
 * @param[in] param the Param is used for play device select.
 * @return return the result of audio decoder open.
 * - open successfully:return a music_dec_engine_t struct with  (void *)type.
 * - open fail: return NULL.
 */ 
EXPORT void *audioDecOpen(void *param);
/**
 * @brief  send cmd to audio decoder.
 *
 * 
 * @param[in] handle the handle is a music_dec_engine_t type struct from the return of audioDecOpen().
 * @param[in] cmd the cmd is used for send control cmd to decoder
 * - PLAY:start music play
 * - STOP:stop music play
 * - PAUSE:pause music play
 * - CONTINUE:continue music play
 * - FAST_FORWARD:fast forward music play
 * - FAST_BACKWARD:fast backward music play
 * - CANCEL_FF:cancel fastforward
 * - CANCEL_FB:cancel fastbackward
 * - SET_FILE:set file play  
 * - GET_MEDIA_INFO:get music info
 * - GET_PLAYER_STATUS:get player status
 * @param[in] param the param in used for some cmd to get or send ext info.
 * @return return the result.
 * - 0:send cmd successfully.
 * - -1: failed.
 */ 
EXPORT int audioDecCmd(void *handle,unsigned int cmd,unsigned int param);
/**
 * @brief  close the audio decoder.
 *
 * 
 * @param[in] handle the handle is a music_dec_engine_t type struct from the return of audioDecOpen().
 * @return return the result.
 * - 0: close successfully.
 * - !0: close failed.
 */ 
EXPORT int audioDecClose(void *handle);
/**

 * @}

 */


#endif

