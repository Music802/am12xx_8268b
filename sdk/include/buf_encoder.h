
#ifndef __BUF_ENCODER_H__
#define __BUF_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

//����
/******************************************************************************
//֧�ֵı������ݵĸ�ʽ
#define JPEG_INPUT_PIX_FMT_YUV_420_PLANAR		1
#define JPEG_INPUT_PIX_FMT_YUV_420_SEMIPLANAR	2
#define JPEG_INPUT_PIX_FMT_RGB16_565			3
#define JPEG_INPUT_PIX_FMT_YCbCr422_INTERLEAVED 4
#define JPEG_INPUT_PIX_FMT_RGB24_888			5

int jpeg_encode(void *buffer, void *JPGFileBuffer, char Level, int *size, int w, int h, int type, int dest)

Input:    buffer          -Ҫ��������ݵ�ַ
          Level		      -�����jpg����[0-5]
          size		      -���ɵ�jpg�Ĵ�С
          x			      -buffer(����ͼƬ)�Ŀ�(����Ϊ��λ)
          y			      -buffer(����ͼƬ)�ĸ�(����Ϊ��λ)
          type		      -�μ�����궨�����������
	      dest            -0�������buffer��1��������ļ���
	   			          ��Ӧ��JPGFileBuffer�ֱ�Ϊ���buffer��ַ������ļ�·��
          
Return��0-�ɹ�   -1-ʧ��
*******************************************************************************/

__attribute__ ((visibility("default"))) 
int jpeg_encode(void *JPGFileBuffer,
				void *buffer,
				void *buffer_u,
				void *buffer_v,
				int buffer_stride,
				int pixel_format,
				int crop_x,
				int crop_y,
				int crop_width,
				int crop_height,
				char Level,
				int *size,
				int dest);

#ifdef __cplusplus
}
#endif

#endif//__BUF_ENCODER_H__
