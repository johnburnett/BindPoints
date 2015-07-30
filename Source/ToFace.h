#include "BindPoints.h"
#include "ToFace_Binds.h"

#define TOFACE_CLASSID Class_ID(0x3ec38e5, 0x5ae14a66)

// version (using global CURRENT_VERSION):
// 1 - initial version
// 2 - weighting
// 4 - m_pblock as ref 0 (skipped versions to get in sync)

class ToFace : public Modifier
{
public:
	ToFace();
	~ToFace();
	
	// Animatable /////////////////////////////////////////////////////////////////////////////////
	
	void					GetClassName(TSTR& s);
	Class_ID				ClassID();
	void					DeleteThis();

	void					BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev);
	void					EndEditParams(IObjParam* ip, ULONG flags, Animatable* next);

	int						NumParamBlocks();
	IParamBlock2*			GetParamBlock(int i);
	IParamBlock2*			GetParamBlockByID(BlockID id);

	// ReferenceMaker /////////////////////////////////////////////////////////////////////////////

	int						NumRefs();
	RefTargetHandle			GetReference(int i);
#if MAX_VERSION_MAJOR < 14 //Max 2012
	void					SetReference(int i, RefTargetHandle rtarg);
#else
private:
	virtual void			SetReference(int i, RefTargetHandle rtarg);
public:
#endif
	int						RemapRefOnLoad(int iref);
#if MAX_VERSION_MAJOR < 17 //Max 2015
	RefResult				NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
#else
	RefResult				NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
#endif

	// ReferenceTarget ////////////////////////////////////////////////////////////////////////////

	RefTargetHandle			Clone(RemapDir &remap);
	
	// BaseObject /////////////////////////////////////////////////////////////////////////////////

	CreateMouseCallBack*	GetCreateMouseCallBack();
#if MAX_VERSION_MAJOR < 15 //Max 2013
	TCHAR*					GetObjectName();
#else
	const TCHAR*			GetObjectName();
#endif

	// Modifier ///////////////////////////////////////////////////////////////////////////////////

	Interval				LocalValidity(TimeValue t);

	ChannelMask				ChannelsUsed();
	ChannelMask				ChannelsChanged();

	IOResult				Save(ISave* isave);
	IOResult				Load(ILoad* iload);

	void					ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node);
	Class_ID				InputType();

	// ToFace //////////////////////////////////////////////////////////////////////////////////'//

	static TriObject*		GetTriObject(TimeValue t, INode* node, BOOL& needsDel);
	void					UpdateUI();

	void					Update();

	BOOL					AddNode(INode* thisNode, INode* node);
	BOOL					RemoveNode(int i);
	int						GetNumNodes();
	INode*					GetNode(int i);

	int						GetNumPoints();
	void					SetNumPoints(int numBinds);

	BOOL					Bind(INode* thisNode, int pIdx, int nodeIndex, int faceIndex, float weight);
	BOOL					UnBind(int vi, int bi);
	int						GetNumBinds(int vi);
	BOOL					GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& weight);
	float					GetBindWeight(int vi, int bi);
	BOOL					SetBindWeight(int vi, int bi, float weight);

	static HWND				m_hWnd;

private:
	IParamBlock2*			m_pblock;
	Matrix3					m_objectToWorld;
	Tab<INode*>				m_nodes;
	Tab<FacePoint*>			m_pointInfo;
	int						m_version;

	HWND					m_hAboutRollup;
};
