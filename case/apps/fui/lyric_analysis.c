#ifdef MODULE_CONFIG_LYRICS

#include "lyric_analysis.h"

unsigned char Buffer[BUFFER_SIZE];


static char fs_open(struct lyric_stream_input_s *input,char* file)
{
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	f->fp=fopen(file,"rb");
	if(f->fp==NULL){
		printf(ERROR_MSG_OPEN);
		return 0;
	}
	printf("open file oK\n");
	return 1;
}
static int fs_read(struct lyric_stream_input_s *input,unsigned int buf,unsigned int len)
{
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	return fread((unsigned char *)buf, sizeof(char),len,f->fp);
}

static int fs_write(struct lyric_stream_input_s *input,unsigned int buf,unsigned int len)
{
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	return fwrite((char *)buf,sizeof(char),len,f->fp);
}

static int fs_seek(struct lyric_stream_input_s *input,int offset,int original)
{
	static const int seek_cmd[4] = {
		0,
		SEEK_SET, //DSEEK_SET
		SEEK_END, //DSEEK_END
		SEEK_CUR, //DSEEK_CUR
	};
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	return fseek(f->fp,offset,seek_cmd[original]);
}

static int fs_tell(struct lyric_stream_input_s *input)
{
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	return ftell(f->fp);
}

static int fs_dispose(struct lyric_stream_input_s *input)
{
	lyric_local_stream_input_t * f = (lyric_local_stream_input_t*)input;
	fclose(f->fp);
	return 0;
}



//��������Ϣ������
static char insertLyricline(lyric_line_t *lyricline,lyric_info_t* lyric)
{
	lyric_line_t * tmp=NULL,*prev=NULL,*next=NULL;
	char isinsert=0;
	
	tmp = lyric->head;
	if(tmp==NULL){
		lyric->head = lyricline;
		return 1;
	}
	while(tmp){//��������
		if(tmp->index==-1){//����tag��������һ���
			tmp->offset = lyricline->offset;
			tmp->len = lyricline->len;
			tmp->index = lyricline->index;
		}
		tmp = tmp->next;
	}
	tmp=lyric->head;
	while(tmp!=NULL){
		if(lyricline->time.min<tmp->time.min){
			isinsert = 1;
		}
		else if(lyricline->time.min==tmp->time.min){
			if(lyricline->time.sec<tmp->time.sec)
				isinsert = 1;
			else if(lyricline->time.sec==tmp->time.sec){
				if(lyricline->time.millisec<lyricline->time.millisec)
					isinsert = 1;
				else if(lyricline->time.millisec==lyricline->time.millisec)
					isinsert = 2;
			}
		}
		if(isinsert==1){//��Ҫ����
			lyricline->next= tmp;
			if(tmp==lyric->head)
				lyric->head = lyricline;
			else
				prev->next = lyricline;
			return 1;
		}
		if(isinsert==2){//�����ظ�����Ҫȥ��֮ǰ��ͬ��ʱ���
			if(tmp==lyric->head)
				lyric->head = lyricline;
			else
				prev->next = lyricline;
			lyricline->next = tmp->next;
			free(tmp);
			return 1;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	prev->next = lyricline;//ֱ�Ӳ��뵽���
	return 1;
}

//�����봫���ʱ��Tag���������Ϣ,�Ҳ����򷵻�
static lyric_line_t* findLyricline(lyric_tag_time_t *tagtime,lyric_info_t* lyric)
{
	lyric_line_t *tmp=NULL,*prev=NULL;
	
	tmp=lyric->head;
	while(tmp!=NULL){
		if(tagtime->min>tmp->time.min){
			goto NEXT;
		}
		else if(tagtime->min==tmp->time.min){
			if(tagtime->sec>tmp->time.sec){
				goto NEXT;
			}
			else{
				break;
			}
		}
		else{
			break;
		}
NEXT:		
		prev = tmp;
		tmp=tmp->next;
	}
	return prev;
}


//��������Tag��Ϣ
static char saveTaginfo(lyric_info_t *lyric,int tagindex,char *str,int len)
{
	char * tmp=NULL;
	
	tmp=(char*)malloc(sizeof(char)*len);
	if(tmp==NULL || str ==NULL){
		printf(ERROR_MSG_SAVE);
		return 0;
	}
	strcpy(tmp,str);
	switch(tagindex){
		case LYRIC_TAG_AL:
			lyric->album = tmp;
		break;
		
		case LYRIC_TAG_AR:
			lyric->ar = tmp;
		break;
		
		case LYRIC_TAG_BY:
			lyric->by = tmp;
		break;
			
		case LYRIC_TAG_OFFSET:
			lyric->offset = tmp;
		break;
			
		case LYRIC_TAG_RE:
			lyric->re = tmp;
		break;
			
		case LYRIC_TAG_TI:
			lyric->ti = tmp;
		break;
			
		case LYRIC_TAG_VE:
			lyric->ve = tmp;
		break;
			
		default:
			free(tmp);
			printf(ERROR_MSG_TAG);
			return 0;
		break;
	}
	return 1;
}

//����ʱ��Tag
static char storeTagTime(lyric_line_t *lyricline,char* str,int len,char tag)
{
	unsigned short t=0;
	int i=0;
	
	if(lyricline==NULL){
		printf(ERROR_MSG_TIME_STORE);
		return 0;
	}
	
	for(i=0;i<len;i++){
		t = t*10+(unsigned short)(str[i]-0x30);
	}
	
	switch(tag){
		case TAG_TIME_MIN:
			lyricline->time.min = t;//�洢����
		break;
			
		case TAG_TIME_SEC:
			lyricline->time.sec = t%60;
			lyricline->time.min += t/60;
		break;
			
		case TAG_TIME_MILSEC:
			lyricline->time.millisec=t;
		break;
			
		default:
			printf(ERROR_MSG_TIME_STORE);
			return 0;
		break;
	}
	return 1;
}

//�򿪸���ļ�����ʼ���������
lyric_info_t openLyric(char * filename)
{
	lyric_info_t lyric;
	lyric_stream_input_t *input=&(lyric.stream_in.input);
	lyric.head = NULL;
	lyric.album=NULL;
	lyric.ar=NULL;
	lyric.by=NULL;
	lyric.offset=NULL;
	lyric.re=NULL;
	lyric.ti=NULL;
	lyric.ve=NULL;
	lyric.head=NULL;
	lyric.curline=NULL;
	input->read=fs_read;
	input->seek=fs_seek;	
	input->tell=fs_tell;
	input->write=fs_write;
	input->dispose=fs_dispose;
	input->open=fs_open;
	input->open((lyric_stream_input_t*)&lyric.stream_in,filename);
	init_tag_table();
	return lyric;
}


//�ر�lyric�ļ������ͷ���Դ
void closeLyric(lyric_info_t*lyric)
{
	lyric_stream_input_t *stream_in=(lyric_stream_input_t*)&lyric->stream_in;
	lyric_line_t * tmp=NULL,*next=NULL;
	
	if(lyric->stream_in.fp==NULL){
		printf(ERROR_MSG_FILE);
		return;
	}
	
	if(lyric->album){
		free(lyric->album);
	}
	
	if(lyric->ar){
		free(lyric->ar);
	}
	
	if(lyric->by){
		free(lyric->by);
	}
	
	if(lyric->offset){
		free(lyric->offset);
	}
	
	if(lyric->re){
		free(lyric->re);
	}
	
	if(lyric->ti){
		free(lyric->ti);
	}
	
	if(lyric->ve){
		free(lyric->ve);
	}
	
	tmp = lyric->head;
	while(tmp){
		next=tmp->next;
		free(tmp);
		tmp=next;
	}
	
	lyric->stream_in.input.dispose(stream_in);//�ر��ļ�
	lyric->stream_in.fp=NULL;
}

//��ȡ�ļ������ͣ�����MBCS or others
char checkFileType(lyric_info_t * lyric)
{
	lyric->filetype = LYRIC_MBCS;
	return 1;
}

//����lyric�ļ�����������Ϣ�����󷵻�0
char analyseLyric(lyric_info_t * lyric)
{
	unsigned char c;
	unsigned char *str=Buffer;
	char tagstart=0,tagend=0;
	lyric_line_t *lyricline=NULL,*lyriclinesibling=NULL;
	lyric_stream_input_t *stream_in=(lyric_stream_input_t*)&lyric->stream_in;
	char partchar=1;
	unsigned int bufaddr = (unsigned int)Buffer;
	unsigned char isdigital=0;//�Ƿ���ʱ��Tag
	int relnum=0;
	int strlen=0;      //�����Ϣ��tepbuf��ʵ�ʳ���
	int charnum=0;     //��¼��ǰBuffer�±�
	int tagindex=-1;   //��¼��ǰ����ʱ��tag��������Tag����
	char tagtime=0;    //��¼ʱ��Tag�е�ǰ�Ƿ֣��뻹�Ǻ���
	char tmpbuf[TEMPER_BUF_SIZE];
	int contentcnt=-1; //������¼ʱ��Tag֮������ݳ���
	int tagtimeindex=0;
	char filetype=0;
	char tagsibling=0;
	
	if(lyric->stream_in.fp==NULL){
		printf(ERROR_MSG_FILE);
		return 0;
	}
	
	checkFileType(lyric);
	filetype = lyric->filetype;
	
	while(1)
	{
		str=Buffer;
		memset(Buffer,0,BUFFER_SIZE);
		//printf("cur=%d\n",lyric->stream_in.input.tell(stream_in));
		relnum=lyric->stream_in.input.read(stream_in,bufaddr,BUFFER_SIZE);
		charnum=0;
		while(charnum<relnum){
			c = str[charnum];
			if(tagstart==2){//�����������tag��tagstart��Ϊ2
				if(tagsibling==1)//��Ҫ��]�����ж�
					goto ISRIGHTBRACIEK;
				goto ISNEWLINE;
			}
			if(tagstart==3)//������[Ƕ�ף��������������������������Tag
				goto ISRIGHTBRACIEK;
			if(filetype==LYRIC_MBCS){//Ŀǰֻ����ĵ�ΪMBCS���д���
				if(c>ASCII_CODE){//Ϊ���룬�������ֽںϳ�һ����
					charnum++;
					if(contentcnt>=0)
						contentcnt +=2;//��Ҫ��2���ֽ�
					if(charnum>=BUFFER_SIZE){//�������½��ӵĵط�
						relnum=lyric->stream_in.input.read(stream_in,bufaddr,BUFFER_SIZE);
						charnum=0;
					}
					if(tagstart==1){ //����ط�������Ҫ�����ݽ�������
						tmpbuf[strlen]=c;
						if(IS_DIGITAL(c))
							isdigital++;
						tmpbuf[strlen+1]=str[charnum];
						if(IS_DIGITAL(tmpbuf[strlen+1]))
							isdigital++;
						strlen+=2;
					}
					goto NEXT;
				}
				else{
					if(contentcnt>=0)//��������ʱ��Tag������
						contentcnt++;
				}
			}
			if(tagstart==1){//ֻ������[�ŻῪʼ����
				if(c==']'){
					if(strlen>=TEMPER_BUF_SIZE){
						printf(ERROR_MSG_SPACE);
						return 0;
					}
					if(isdigital>0 && isdigital==strlen){//����ʱ��tag����Ҫת��Ϊʱ��
						tmpbuf[strlen]='\0';
						if(tagtime==1){
							storeTagTime(lyricline,tmpbuf,strlen,TAG_TIME_SEC);
						}
						else if(tagtime==2){
							storeTagTime(lyricline,tmpbuf,strlen,TAG_TIME_MILSEC);
						}
						else{
							tagstart = 2;
							printf(ERROR_MSG_TAG);
							tagtimeindex--;
							free(lyricline);
							lyricline = NULL;
						}
						if(tagtime!=0){//�Ѿ���¼��ʱ�䣬��Ҫ��¼����
							lyricline->offset=lyric->stream_in.input.tell(stream_in)-relnum+charnum+1;//��¼��ǰ�ļ�ָ��λ��
							contentcnt=0;//׼����ʼ��¼���ݵĳ���
						}
						tagtime=0;
					}
					else if(tagindex!=-1){//������Tag
						tmpbuf[strlen+1]='\0';
						saveTaginfo(lyric,tagindex,tmpbuf,strlen+1);
						tagindex=-1;
						tagtimeindex--;
						free(lyricline);
						lyricline = NULL;
					}
					else{//�����tag
						tagstart = 2;
						printf(ERROR_MSG_TAG);
						tagtimeindex--;
						free(lyricline);
						lyricline = NULL;
					}
					strlen = 0;
					memset(tmpbuf,0,TEMPER_BUF_SIZE);
					tagstart = 0;
					goto NEXT;
				}
				if(c==':'){
					if(strlen>=TEMPER_BUF_SIZE){
						printf(ERROR_MSG_SPACE);
						return 0;
					}
					if(isdigital>0 && isdigital==strlen){//��ʱ��tag����Ҫת��Ϊʱ��
						tmpbuf[strlen]='\0';
						storeTagTime(lyricline,tmpbuf,strlen,TAG_TIME_MIN);
						tagtime++;
					}
					else if(isdigital==0){//����tag
						tmpbuf[strlen]='\0';
						tagindex = get_tag_index(tmpbuf);
						if(tagindex==-1)
							tagstart = 2;//�����tag
					}
					else{//�����tag
						tagstart = 2;
					}
					if(tagstart==2){
						printf(ERROR_MSG_TAG);
						tagtimeindex--;
						free(lyricline);
						lyricline = NULL;
						tagtime=0;
					}
					isdigital = 0;
					strlen = 0;
					memset(tmpbuf,0,TEMPER_BUF_SIZE);
					goto NEXT;
				}
				if(c=='.'){
					if(tagtime!=1){
						tagstart = 2;
						printf(ERROR_MSG_TAG);
						tagtimeindex--;
						free(lyricline);
						lyricline = NULL;
						goto NEXT;
					}
					if(strlen>=TEMPER_BUF_SIZE){
						printf(ERROR_MSG_SPACE);
						return 0;
					}
					if(isdigital==strlen){//����ʱ��tag����Ҫת��Ϊʱ��
						tmpbuf[strlen]='\0';
						storeTagTime(lyricline,tmpbuf,strlen,TAG_TIME_SEC);
						tagtime++;
					}
					else{//�����tag
						tagstart = 2;
						printf(ERROR_MSG_TAG);
						tagtimeindex--;
						free(lyricline);
						lyricline = NULL;
						tagtime=0;
					}
					isdigital = 0;
					strlen = 0;
					memset(tmpbuf,0,TEMPER_BUF_SIZE);
					goto NEXT;
				}
				if(c==0x20){//��ʼ����tag����Ҫȥ���ո�
					goto NEXT;
				}
				if(c==0x0A){//[]���ɶԳ���
					printf(ERROR_MSG_TAG);
					free(lyricline);
					lyricline = NULL;
					tagtimeindex--;
					memset(tmpbuf,0,TEMPER_BUF_SIZE);
					goto ISNEWLINE;
				}
				tmpbuf[strlen]=c;
				if(IS_DIGITAL(c)){
					isdigital++;
				}
				strlen++;
				goto NEXT;
			}
			if(c=='['){
				if(contentcnt>=0){//�����Ѿ�������ʱ��Tag����������ͬ���еĵڶ���[
					lyricline->len = contentcnt-1;//����¼�õ���ʱ��Tag���ݳ��ȱ���,��Ҫ�������[ȥ��
					if(lyricline->len==0){//����[����]֮����Ҫ�ȴ������Tag���ݽ���,��Ҫ�������[ȥ��
						lyricline->index = -1;//�Ƚ�index��עΪ-1�Ⱥ���tag����֮�����޸ġ�
						tagsibling = 1;
						lyriclinesibling = lyricline;
					}
					insertLyricline(lyricline,lyric);
					contentcnt = -1;
				}
				strlen=0;
				isdigital = 0;
				if(tagstart==1){//��������[[ʱ����Ҫfreeǰ������lyricline������Ҫ�ٷ���
					printf(ERROR_MSG_TAG);
					tagtimeindex--;
					free(lyricline);
					lyricline = NULL;
					tagstart = 3;
					goto NEXT;
				}
				tagstart++;
				tagtimeindex++;
				lyricline = (lyric_line_t*)malloc(sizeof(lyric_line_t));//��ʼ����ռ�
				if(lyricline==NULL){
					printf(ERROR_MSG_MALLOC);
					return 0;
				}
				lyricline->next=NULL;
				lyricline->len=0;
				lyricline->offset=0;
				lyricline->time.min=0;
				lyricline->time.sec=0;
				lyricline->time.millisec=0;
				lyricline->index=tagtimeindex;
			/*	if(tagtimeindex==23)
					tagtimeindex=tagtimeindex;
				printf("index=%d,addr=0x%x\n",tagtimeindex,lyricline);*/
				goto NEXT;
			}
ISRIGHTBRACIEK:
			if(c==']'){//�������������ж��[��������Ƕ��ʱ��Tag������һ��
				tagstart = 0;
				memset(tmpbuf,0,strlen);
				strlen=0;
				isdigital=0;
				if(tagsibling==1){//ֻ��ʱ��tag�����п��ܽ�tagsibling��Ϊ1�������������ݳ��ȼ���
					contentcnt=0;
					lyricline = lyriclinesibling;//�ָ�֮ǰ��ָ��
				}
			}
ISNEWLINE:	if(c==0x0A){//ֻ�ж�0A����
				if(contentcnt>=0){//��Ҫ����ʱ��Tag�����ݳ���
					lyricline->len = contentcnt;//����¼�õ���ʱ��Tag���ݳ��ȱ���
					insertLyricline(lyricline,lyric);
				}
				tagsibling = 0;
				lyriclinesibling=NULL;
				tagstart = 0;
				contentcnt = -1;
				tagtime=0;
				isdigital =0;
				tagend = 0;
				strlen = 0;
			}
NEXT:		charnum++;
		}
		
		if(relnum<BUFFER_SIZE){
			printf("File read over\n");
			break;
		}		
	}
	lyric->curline = lyric->head;
	return 1;
}


//��ȡ��һ�и��
int getNextLine(lyric_info_t* lyric,char *buf,int len,lyric_tag_time_t *tagtime)
{
	int realnum;
	char c=0;
	lyric_stream_input_t *stream_in=(lyric_stream_input_t*)&lyric->stream_in;
	if(lyric->curline==NULL || buf==NULL){
		return RTN_MSG_LASTLINE;
	}
	if(len<lyric->curline->len+1){
		printf(ERROR_MSG_BUFFER);
		return RTN_MSG_BUFFER;
	}
	tagtime->min=lyric->curline->time.min;
	tagtime->sec=lyric->curline->time.sec;
	tagtime->millisec=lyric->curline->time.millisec;
	lyric->stream_in.input.seek(stream_in,lyric->curline->offset,1);//��ͷ����
	//printf("cur=%d\n",lyric->stream_in.input.tell(stream_in));
	if(tagtime->sec==53)
		c=c;
	realnum=lyric->stream_in.input.read(stream_in,(unsigned int)buf,lyric->curline->len);
	lyric->curline = lyric->curline->next;
	c = buf[realnum-1];
	if(c==0x0A){
		buf[realnum-1]='\0';
		buf[realnum]='\0';
	}
	else{
		buf[realnum]='\0';
	}
	return realnum;
}

char changLine(lyric_info_t* lyric,lyric_tag_time_t *tagtimecmp)
{
	lyric->curline=findLyricline(tagtimecmp,lyric);
	if(lyric->curline==NULL){
		return 0;
	}
	else{
		return 1;
	}
}

void showLyric(lyric_info_t	*lyric)
{
	char Buf[64];
	lyric_tag_time_t tagtime;
	char rtn;
	lyric_line_t * tmp=NULL,*next=NULL;
	
	if(lyric->album)
		printf("Alubm:%s\n",lyric->album);
	if(lyric->ar)
		printf("Ar:%s\n",lyric->ar);
	if(lyric->by)
		printf("By:%s\n",lyric->by);
	if(lyric->offset)
		printf("Offset:%s\n",lyric->offset);
	if(lyric->re)
		printf("Re:%s\n",lyric->re);
	if(lyric->ti)
		printf("Ti:%s\n",lyric->ti);
	if(lyric->ve)
		printf("Ve:%s\n",lyric->ve);
	
	tmp = lyric->head;
	/**	
	while(tmp){
		next=tmp->next;
		printf("result=%02d,%02d,%02d,off=0x%x,len=0x%x,index=%d\n",tmp->time.min,tmp->time.sec,\
		tmp->time.millisec,tmp->offset,tmp->len,tmp->index);
		tmp=next;
	}
	*/
	
	while(1){
		rtn=getNextLine(lyric,Buf,64,&tagtime);
		if(rtn<0)
			break;
		printf("[%02d:%02d.%02d]%s\n",tagtime.min,tagtime.sec,tagtime.millisec,Buf);
	}
}

#endif
