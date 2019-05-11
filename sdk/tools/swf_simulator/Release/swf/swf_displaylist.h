#ifndef SWF_DISPLAYLIST_H
#define SWF_DISPLAYLIST_H

#include "swf_base.h"
#include "swf_action.h"

#define SWF_SCAN_NULL	   0
#define SWF_SCAN_ZBUFFER   1
#define SWF_SCAN_MERGE     2
#define SWF_SCAN_BIG_PIC   4

#define LISTENER_EVENT_MASK     0xFEFF //0xFE0F

typedef struct _Listener
{
	BLIST			BList;
	DISPLAYNODE*	Node;
	SWF_DEC*		Swfdec;
	AS_FUNCTION*	EventCode[AS_EVENT_TYPE_NUM];
	int				Flag;
	int				(*Proc)(struct _Listener * listener, int action, void* param);
} LISTENER;

#define NODE_ATTR_VISIBLE			0x01
#define NODE_ATTR_ENABLED			0x02

#define SET_NODE_VISIBLE(node, flag)\
	(node)->Attribute = (flag == 0) ? ((node)->Attribute & ~NODE_ATTR_VISIBLE) : ((node)->Attribute | NODE_ATTR_VISIBLE)
#define IS_NODE_VISIBLE(node)\
	((node)->Attribute & NODE_ATTR_VISIBLE)

#define SET_NODE_ENALBED(node, flag)\
	(node)->Attribute = (flag == 0) ? ((node)->Attribute & ~NODE_ATTR_ENABLED) : ((node)->Attribute | NODE_ATTR_ENABLED)
#define IS_NODE_ENABLED(node)\
	((node)->Attribute & NODE_ATTR_ENABLED)

#define NODE_ATTR_CHANGE_NONE			0x00
#define NODE_ATTR_CHANGE_COLOR			0x10
#define NODE_ATTR_CHANGE_POSITION		0x20
#define NODE_ATTR_CHANGE_SHAPE			0x40
#define NODE_ATTR_CHANGE_MASK			0x70

#define GET_CHANGE_ATTR(x)\
	((x)->Attribute & NODE_ATTR_CHANGE_MASK)
#define COPY_CHANGE_ATTR(x,y)\
	(x)->Attribute = ((x)->Attribute & ~NODE_ATTR_CHANGE_MASK) | (y & NODE_ATTR_CHANGE_MASK)
#define MARK_SHAPE_CHANGE(x)\
	(x)->Attribute |= NODE_ATTR_CHANGE_SHAPE
#define MARK_POSITION_CHANGE(x)\
	(x)->Attribute |= NODE_ATTR_CHANGE_POSITION
#define MARK_COLOR_CHANGE(x)\
	(x)->Attribute |= NODE_ATTR_CHANGE_COLOR
#define IS_SHAPE_CHANGED(x)\
	((x)->Attribute & NODE_ATTR_CHANGE_SHAPE)
#define IS_POSITION_CHANGED(x)\
	((x)->Attribute & NODE_ATTR_CHANGE_POSITION)
#define IS_NO_ATTR_CHANGED(x)\
	(GET_CHANGE_ATTR(x) == NODE_ATTR_CHANGE_NONE)

#define IS_NODE_DYNAMIC_CREATED(x)\
	((x)->Depth >= 16384)
#define IS_NODE_TIMELINE(x)\
	((x)->Depth < 16384)

#define NODE_ATTR_LOCKROOT		0x100

#define IS_NODE_LOCKROOT(node)\
	((node)->Attribute & NODE_ATTR_LOCKROOT)
#define SET_NODE_LOCKROOT(node, flag)\
	(node)->Attribute = (flag == 0) ? ((node)->Attribute & ~NODE_ATTR_LOCKROOT) : ((node)->Attribute | NODE_ATTR_LOCKROOT)

#define NODE_ATTR_REMOVEWHENJUMP		0x200

#define IS_NODE_REMOVEWHENJUMP(node)\
	((node)->Attribute & NODE_ATTR_REMOVEWHENJUMP)
#define SET_NODE_REMOVEWHENJUMP(node, flag)\
	(node)->Attribute = (flag == 0) ? ((node)->Attribute & ~NODE_ATTR_REMOVEWHENJUMP) : ((node)->Attribute | NODE_ATTR_REMOVEWHENJUMP)


struct _DisplayNode
{
	//DEF_SWF_OBJECT
	BLIST		BList;
	SWF_DEC *	SwfInst;
	SHEAD		UserVars;
	SHEAD		Getters;
	SHEAD		Setters;
	int			Class;
	AS_OBJECT * Prototype;
	AS_OBJECT * Super;
	AS_OBJECT*	SubClassObject;
	int			Valid;

	unsigned short			Depth;//���
	char			Name[AS_MAX_NODENAME];
	int				NameHash;
	
	SWF_MATRIX		Matrix;//�任����
	unsigned short			Ratio;//Morph�ı任����
	unsigned short			MaskDepth;//Mask�����
	SWF_CXFORM		ColorTransform;//��ɫ�任
	unsigned int			ColorRGB;//Color�������õ�RGBֵ,��Ϊ0,���ʾδ����

	DISPLAYLIST*	ParentDisplayList;//����ʾ�б�
	DISPLAYLIST*	ChildDisplayList;//����ʾ�б�
	int				CharacterId;//�ڵ�ID
	int				ResouceId;//��ԴID
	DISPLAYNODE*	MaskNode;

	SWF_MATRIX		DispMatrix;//��ʾ����
	SWF_CXFORM		DispCxForm;//��ʾ��ɫ
	int				DispDepth;//��ʾ���

	SHEAD			ScanLineList;//ɨ�����б�
	unsigned int			Attribute;//����

	LISTENER*		Listener;//������Ӧ����
	void*			Private;//˽�г�Ա
	int				PrevCID;//loadClip֮ǰ��ID
	SWF_DEC*		PrevSWF;//loadClip֮ǰ��Swfdec
	DISPLAYNODE*	hitArea;//������򣬳�ʼ��Ϊnull

	// used by placeobject tag
	SWF_MATRIX		OriginalMatrix;
};

#define SET_AS_STATE(state, x)\
	state = (state & ~0x1) | (x & 0x1)
#define GET_AS_STATE(state)\
	(state & 0x1)

#define AS_STATE_NORM	0x00
#define AS_STATE_STOP	0x01
#define AS_STATE_LOAD	0x02
#define AS_STATE_GOTO	0x04

struct _DisplayList
{
	int				nextFrame;//��һ֡λ��
	int				endFrame;//��ת�յ�֡λ�ã�������תָ�����жϿ���TAG�Ƿ���Ҫִ�У�����StartSound
	int				nextAction;//��һ����Ҫִ�е�Action����
	int				maxAction;//�Ѿ�ִ�й������һ֡�������ж�ĳ�����Ƿ��һ��ִ�д��루���ոմ�����
	int				frameCount;	//֡����
	BLIST			ChildNodeList;//�ó����е�Ԫ��
	DISPLAYNODE *	ParentNode;//�ó����ĸ��ڵ�
	int				AsState;//ʱ����״̬�������Ƿ���stop״̬
	int firstTime;//ֻ����Gotoʱ
};

typedef struct _Walk
{
	SLIST   SList;
	void  * Node;
}SWF_WALK;

#endif
