
#ifndef _TTFONT_H_
#define _TTFONT_H_

#include "swf.h"
#include <ft2build.h>
#include FT_FREETYPE_H

/** Truetype���λ����С */
#if 0
#define SBIT_HASH_SIZE (1024-1)
#else
#define SBIT_HASH_SIZE (1024)
#endif
#define PATH_MAX 1024

#define SBIT_CACHE_MAX 1024*1024

#ifndef true
#define true 1
#endif

#ifndef bool
#define bool	int
#endif

#ifndef false
#define false 0
#endif
#ifndef byte
typedef unsigned char byte;
#endif
#ifndef word
typedef unsigned short word;
#endif
#ifndef dword
typedef unsigned long dword;
#endif

typedef dword pixel;

/** ��������λͼ�ṹ */
typedef struct Cache_Bitmap_
{
	int width;
	int height;
	int left;
	int top;
	char format;
	short max_grays;
	int pitch;
	unsigned char *buffer;
} Cache_Bitmap;

/** �������νṹ */
typedef struct SBit_HashItem_
{
	unsigned long ucs_code;
	int glyph_index;
	int size;
	bool anti_alias;
	bool cleartype;
	bool embolden;
	int xadvance;
	int yadvance;
	Cache_Bitmap bitmap;
} SBit_HashItem;

/** Truetype����ṹ */
typedef struct _ttf
{
	FT_Library library;
	FT_Face face;

	char *fontName;
	bool antiAlias;
	bool cleartype;
	bool embolden;
	int pixelSize;

	SBit_HashItem sbitHashRoot[SBIT_HASH_SIZE];
	int cacheSize;
	int cachePop;

	char fnpath[PATH_MAX];
	int fileSize;
	byte *fileBuffer;
} t_ttf, *p_ttf;

/**
 * ��TTF����
 *
 * @param filename TTF�ļ���
 * @param size Ԥ��������С
 * @param load2mem �Ƿ���ص��ڴ�
 *
 * @return ����TTF��ָ��
 * - NULL ʧ��
 */
extern p_ttf ttf_open(const char *filename, int size, bool load2mem);

/**
 * ��ָ�������д�TTF����
 *
 * @param ttfBuf TTF��������
 * @param ttfLength TTF�������ݴ�С�����ֽڼ�
 * @param pixelSize Ԥ��������С
 * @param ttfName TTF������
 *
 * @return ����TTF��ָ��
 * - NULL ʧ��
 */
extern p_ttf ttf_open_buffer(void *ttfBuf, size_t ttfLength, int pixelSize,
							 const char *ttfName);

/**
 * �ͷ�TTF����
 *
 * @param ttf ttfָ��
 */
extern void ttf_close(p_ttf ttf);

/**
 * ����TTF�����С
 *
 * @param ttf ttfָ��
 * @param size �����С
 *
 * @return �Ƿ�ɹ�
 */
extern bool ttf_set_pixel_size(p_ttf ttf, int size);

/**
 * ����TTF�Ƿ����ÿ����Ч��
 *
 * @param ttf ttfָ��
 * @param aa �Ƿ����ÿ����Ч��
 */
extern void ttf_set_anti_alias(p_ttf ttf, bool aa);

/**
 * ����TTF�Ƿ�����ClearType(Sub-Pixel LCD�Ż�Ч��)
 *
 * @param ttf ttfָ��
 * @param cleartype �Ƿ�����cleartype
 */
extern void ttf_set_cleartype(p_ttf ttf, bool cleartype);

/**
 * ����TTF�Ƿ���������Ӵ�
 *
 * @param ttf ttfָ��
 * @param embolden �Ƿ���������Ӵ�
 */
extern void ttf_set_embolden(p_ttf ttf, bool embolden);



/** TTF���� */
extern void ttf_lock(void);

/** TTF���� */
extern void ttf_unlock(void);

/** ��ʼ��TTFģ�� */
extern void ttf_init(void);

/** �ͷ�TTFģ�� */
extern void ttf_free(void);



//int show_string_in_osd(char *gb, char *string_bitmap, int w, int h);

void Set_vars(FT_Face *face);
int set_pixel_size_ex(int size);

#endif
