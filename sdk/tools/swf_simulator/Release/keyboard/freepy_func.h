/*
 * Copyright (C) 1999.4  Li ZhenChun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License; or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that is will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, M A 02139, USA.
 *
 * Author: Li ZhenChun  email: zhchli@163.net or zhchli@126.com
 * 
 */

#ifndef _FREEPY_FUNC_H_
#define _FREEPY_FUNC_H_

#define MAX_PY_LEN 8	//ȫƴ����ĸ��������
#define MAX_INPUT_BUF_SIZE 128
#define MAX_CZ_WIDTH 10	//�����к��ֵ�������

typedef struct tag_text_select	//��ѡ����ͺ��ֵĽṹ
{
	int index;	//��ѡ�������
	int text_len;	//������ߺ��ֵĸ����������n�����֣���ռ��2*n��byte
	char text[MAX_CZ_WIDTH*2];	//������ߺ��ֵ�buffer
}text_select;

/**********************************************************************/
/*                                                                    */
/*      Functions                                                     */
/*                                                                    */
/**********************************************************************/
//freepy.c
int  freepy_start ();
int  freepy_stop ();

//global.c
void global_vars_init();

int  freepy_search (char *input_string, short *py_num, char *TranBuf, text_select * select_buffer, int buffer_num);


#endif
/* _FREEPY_H_ */

