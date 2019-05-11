#ifndef _LYRIC_ANALYSIS_H_
#define _LYRIC_ANALYSIS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE      23
#define BUFFER_SIZE          256
#define TEMPER_BUF_SIZE      64
#define ASCII_CODE           0x80


#define ERROR_MSG_OPEN "Error:: File open error\n"
#define ERROR_MSG_FILE "Error:: File is not exist\n"
#define ERROR_MSG_SPACE "Error:: Temp buffer is out of space\n"
#define ERROR_MSG_SAVE "Error:: Save tag info error\n"
#define ERROR_MSG_TAG  "Error:: Tag name is not exist\n"
#define ERROR_MSG_TIME_STORE "Error:: Tag time store error\n"
#define ERROR_MSG_MALLOC "Error:: Malloc line space failed\n"
#define ERROR_MSG_BUFFER "Error:: Buffer for fetching line not enough\n"

#define RTN_MSG_BUFFER          -1
#define RTN_MSG_LASTLINE        -2

enum{
	LYRIC_TAG_AL=0,
	LYRIC_TAG_AR=1,
	LYRIC_TAG_BY=2,
	LYRIC_TAG_OFFSET=3,
	LYRIC_TAG_RE=4,
	LYRIC_TAG_TI=5,
	LYRIC_TAG_VE=6,
};


enum{
	TAG_TIME_MIN=0,
	TAG_TIME_SEC=1,
	TAG_TIME_MILSEC=2,
};

#define LOWER_CASE(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 'a' - 'A') : (c))
#define IS_DIGITAL(c) (((c) >= '0' && (c) <= '9') ? 1:0)

typedef struct lyric_stream_input_s
{
	char (*open)(struct lyric_stream_input_s *input,char* file);
	int (*read)(struct lyric_stream_input_s *input,unsigned int buf,unsigned int len);
	int (*write)(struct lyric_stream_input_s *input,unsigned int buf,unsigned int len);
	int (*seek)(struct lyric_stream_input_s *input,int offset,int original);		
	int (*tell)(struct lyric_stream_input_s *input);
	int (*dispose)(struct lyric_stream_input_s *input);
}lyric_stream_input_t;


typedef struct local_stream_input_s
{
	lyric_stream_input_t input;
	FILE* fp;

} lyric_local_stream_input_t;


typedef struct lyric_tag_time
{
	unsigned short min;         //����
	unsigned short sec;         //���ӣ��������60������Ҫ��λ������
	unsigned short millisec;    //�����ֵֻ�е�ǰ��������ͬʱ�����ã��Ƚϴ�С��
}lyric_tag_time_t;


typedef struct lyric_line
{
	struct lyric_line *next;
	struct lyric_tag_time time;
	int offset;  //��¼��ǰʱ��Tag���������ļ��е�ƫ��λ��
	int len;     //��¼��ǰʱ��Tag���ݳ���
	int index;   //��¼���ļ���ʼ�ĵڸ�ʱ��Tag
}lyric_line_t;

typedef enum
{
	LYRIC_MBCS = 0,
	LYRIC_UNI16_BIG,    //unicode big endian
	LYRIC_UNI16_LIT,    //unicode little endian
	LYRIC_UTF8
}lryic_txt_type_t;

typedef struct lyric_info_s
{
	lyric_local_stream_input_t stream_in;
	lyric_line_t *head;
	lyric_line_t *curline;
	char* album;   //Album where the song is from
	char* ar;      //Lyrics artist
	char* by;      //Creator of the LRC file
	char* offset;  //Overall timestamp adjustment in milliseconds, + shifts time up, - shifts down
	char* re;      //The player or editor that creates LRC file
	char* ti;      //Lyrics (song) title
	char* ve;      //version of program
	char filetype; //MBCS , UNI16_LIT , UNI16_BIG ,UTF8 
}lyric_info_t;

struct hash_table_tagname{
	int item;    //��¼������Ҫ�������ֵ
	int full;    //��¼��ϣ���Ƿ��Ѿ������䣬��ֵ��ʵ�Ǽ���õ��Ĺ�ϣֵ
	int base;
};

/** function definition */
extern void init_tag_table(void);
extern int get_tag_index(char* str);
extern lyric_info_t openLyric(char * filename);
extern void closeLyric(lyric_info_t*lyric);
extern char analyseLyric(lyric_info_t * lyric);
extern int getNextLine(lyric_info_t* lyric,char*buf,int len,lyric_tag_time_t *tagtime);
extern void showLyric(lyric_info_t	*lyric);


#endif



