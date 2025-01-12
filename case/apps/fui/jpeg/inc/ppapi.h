/*------------------------------------------------------------------------------
--																			  --
--		 This software is confidential and proprietary and may be used		  --
--		  only as expressly authorized by a licensing agreement from		  --
--																			  --
--							  Hantro Products Oy.							  --
--																			  --
--					 (C) COPYRIGHT 2006 HANTRO PRODUCTS OY					  --
--							  ALL RIGHTS RESERVED							  --
--																			  --
--				   The entire notice above must be reproduced				  --
--					on all copies and should not be removed.				  --
--																			  --
--------------------------------------------------------------------------------
--
--	Description : The video post processor API
--
--------------------------------------------------------------------------------
--
--	Version control information, please leave untouched.
--
--	$RCSfile: ppapi.h,v $
--	$Date: 2007/03/08 13:17:59 $
--	$Revision: 1.4 $
--
------------------------------------------------------------------------------*/

#ifndef __PPAPI_H__
#define __PPAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"

#define PP_PIPELINE_DISABLED							0U
#define PP_PIPELINED_DEC_TYPE_H264						1U
#define PP_PIPELINED_DEC_TYPE_MPEG4 					2U
#define PP_PIPELINED_DEC_TYPE_JPEG						3U
#define PP_PIPELINED_DEC_TYPE_VC1						4U
#define PP_PIPELINED_DEC_TYPE_MPEG1 					5U
#define PP_PIPELINED_DEC_TYPE_MPEG2 					6U
#define PP_PIPELINED_DEC_TYPE_RV 						7U


#define PP_PIX_FMT_YCBCR_4_0_0							0x080000U
#define PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR				0x080001U

#define PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED				0x010001U
#define PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR				0x010002U
#define PP_PIX_FMT_YCBCR_4_4_0_SEMIPLANAR				0x010003U

#define PP_PIX_FMT_YCBCR_4_2_0_PLANAR					0x020000U
#define PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR				0x020001U
#define PP_PIX_FMT_YCBCR_4_2_0_TILED					0x020002U

#define PP_PIX_FMT_RGB16_CUSTOM 						0x040000U
#define PP_PIX_FMT_RGB16_5_5_5							0x040001U
#define PP_PIX_FMT_RGB16_5_6_5							0x040002U
#define PP_PIX_FMT_BGR16_5_5_5							0x040003U
#define PP_PIX_FMT_BGR16_5_6_5							0x040004U

#define PP_PIX_FMT_RGB32_CUSTOM 						0x041000U
#define PP_PIX_FMT_RGB32								0x041001U
#define PP_PIX_FMT_BGR32								0x041002U

#define PP_YCBCR2RGB_TRANSFORM_CUSTOM					0U
#define PP_YCBCR2RGB_TRANSFORM_BT_601					1U
#define PP_YCBCR2RGB_TRANSFORM_BT_709					2U

#define PP_ROTATION_NONE								0U
#define PP_ROTATION_RIGHT_90							1U
#define PP_ROTATION_LEFT_90 							2U
#define PP_ROTATION_HOR_FLIP							3U
#define PP_ROTATION_VER_FLIP							4U
#define PP_ROTATION_180 								5U

#define PP_MASK_ALPHA_COLOR_DEPTH_32					0
#define PP_MASK_ALPHA_COLOR_DEPTH_16					1

	typedef const void *PPInst;

	typedef enum PPResult_
	{
		PP_OK = 0,
		PP_PARAM_ERROR = -1,
		PP_MEMFAIL = -4,

		PP_SET_IN_SIZE_INVALID = -64,
		PP_SET_IN_ADDRESS_INVALID = -65,
		PP_SET_IN_FORMAT_INVALID = -66,
		PP_SET_CROP_INVALID = -67,
		PP_SET_ROTATION_INVALID = -68,
		PP_SET_OUT_SIZE_INVALID = -69,
		PP_SET_OUT_ADDRESS_INVALID = -70,
		PP_SET_OUT_FORMAT_INVALID = -71,
		PP_SET_VIDEO_ADJUST_INVALID = -72,
		PP_SET_RGB_BITMASK_INVALID = -73,
		PP_SET_FRAMEBUFFER_INVALID = -74,
		PP_SET_MASK1_INVALID = -75,
		PP_SET_MASK2_INVALID = -76,
		PP_SET_SLICE_HEIGHT_INVALID = -77,
		PP_SET_SLICE_AND_PIPELINE = -78,

		PP_BUSY = -128,
		PP_HW_BUS_ERROR = -256,
		PP_HW_TIMEOUT = -257,
		PP_DWL_ERROR = -258,
		PP_SYSTEM_ERROR = -259,
		PP_DEC_PIPELINE_ERROR = -512
	} PPResult;

	typedef struct PPInitParam
	{
		void *pHeap;
		u32 reg_rw_trace;
		u32 mem_usage_trace;
		u32 hw_reset_enable;
	} PPInitParam_t;

	typedef struct PPInImage_
	{
		u32 pixFormat;
		u32 videoRange;
		u32 width;
		u32 height;
		u32 bufferBusAddr;
		u32 bufferCbBusAddr;
		u32 bufferCrBusAddr;
		u32 vc1MultiResEnable;
		u32 vc1RangeRedFrm;
		u32 deinterlaceEnable;
		u32 deinterlceThreshold;
		u32 sliceMode;
		u32 sliceHeight;
	} PPInImage;

	typedef struct PPOutImage_
	{
		u32 pixFormat;
		u32 width;
		u32 height;
		u32 bufferBusAddr;
		u32 bufferChromaBusAddr;
		u32 maskAlphaColorDepth;
	} PPOutImage;

	typedef struct PPRgbTransform_
	{
		u32 a;
		u32 b;
		u32 c;
		u32 d;
		u32 e;
	} PPRgbTransform;

	typedef struct PPRgbBitmask_
	{
		u32 maskR;
		u32 maskG;
		u32 maskB;
		u32 maskAlpha;
	} PPRgbBitmask;

	typedef struct PPOutRgb_
	{
		u32 rgbTransform;
		i32 contrast;
		i32 brightness;
		i32 saturation;
		u32 alpha;
		u32 transparency;
		PPRgbTransform rgbTransformCoeffs;
		PPRgbBitmask rgbBitmask;
	} PPOutRgb;

	typedef struct PPInCropping_
	{
		u32 enable;
		/* NOTE: these are coordinates relative to the input picture */
		u32 originX;
		u32 originY;
		u32 height;
		u32 width;
	} PPInCropping;

	typedef struct PPOutMask1_
	{
		u32 enable;
		/* NOTE: these are coordinates relative to the output image */
		i32 originX;
		i32 originY;
		u32 height;
		u32 width;
		u32 alphaBlendEna;
		u32 blendComponentBase;
		u32 fixedAlpha;
		u32 alpha;
	} PPOutMask1;

	typedef struct PPOutMask2_
	{
		u32 enable;
		/* NOTE: these are coordinates relative to the output image */
		i32 originX;
		i32 originY;
		u32 height;
		u32 width;
		u32 alphaBlendEna;
		u32 blendComponentBase;
		u32 fixedAlpha;
		u32 alpha;
	} PPOutMask2;

	typedef struct PPOutFrameBuffer_
	{
		u32 enable;
		/* NOTE: these are coordinates relative to the framebuffer */
		i32 writeOriginX;
		i32 writeOriginY;
		u32 frameBufferWidth;
		u32 frameBufferHeight;
	} PPOutFrameBuffer;

	typedef struct PPInRotation_
	{
		u32 rotation;
	} PPInRotation;

	typedef struct PPConfig_
	{
		PPInImage ppInImg;
		PPInCropping ppInCrop;
		PPInRotation ppInRotation;
		PPOutImage ppOutImg;
		PPOutRgb ppOutRgb;
		PPOutMask1 ppOutMask1;
		PPOutMask2 ppOutMask2;
		PPOutFrameBuffer ppOutFrmBuffer;
	} PPConfig;

	/* Version information */
	typedef struct PPApiVersion_
	{
		u32 major;	/* PP API major version */
		u32 minor;	/* PP API minor version */
	} PPApiVersion;

	typedef struct PPBuild_
	{
		u32 swBuild;	/* Software build ID */
		u32 hwBuild;	/* Hardware build ID */
	} PPBuild;

/*------------------------------------------------------------------------------
	4. Prototypes of PP API functions
------------------------------------------------------------------------------*/
	PPResult PPInit(PPInst *pPostPInst, PPInitParam_t *param);

	PPResult PPDecPipelineEnable(PPInst postPInst, const void *pDecInst,
								 u32 decType);

	PPResult PPDecPipelineDisable(PPInst postPInst, const void *pDecInst);

	PPResult PPGetConfig(PPInst postPInst, PPConfig * pPpConf);

	PPResult PPSetConfig(PPInst postPInst, PPConfig * pPpConf);

	void PPRelease(PPInst pPpInst);

	PPResult PPGetResult(PPInst postPInst);

	PPApiVersion PPGetAPIVersion(void);

	PPBuild PPGetBuild(void);

	void PPTrace(const char *string);

	PPResult PPSliceStart(PPInst postPInst);
	PPResult PPSliceEnd(PPInst postPInst);
	PPResult PPSlice(PPInst postPInst);

	void *PPGetDWL(PPInst ppInst);

#ifdef __cplusplus
}
#endif

#endif /* __PPAPI_H__ */
