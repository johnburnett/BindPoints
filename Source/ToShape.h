#include "BindPoints.h"
#include "ToShape_Binds.h"

#define TOSHAPE_CLASSID Class_ID(0x3ac543c5, 0x39f14454)

// version (now using global CURRENT_VERSION):
// 1 - no baseTan or absolute members in ShapeBind
// 2 - released for about 5 minutes until I figured out I
//     wasn't saving the absolute member at all (oops)
// 3 - save and load baseTan and absolute members (yay)
// 4 - pblock as ref 0
//#define SHAPE_VERSION		4

class ToShape : public Modifier
{
	public:
		IParamBlock2* pblock;
		Matrix3 thisTM;
		Tab<INode*> nodes;
		Tab<ShapePoint*> pointInfo;
		int ver;

		static IObjParam* ip;
		static HWND hWnd;
		HWND hAboutRollup;

		ToShape();
		virtual ~ToShape();

		Class_ID ClassID() { return TOSHAPE_CLASSID; }
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_TOSHAPE_CLASSNAME); }
#if MAX_VERSION_MAJOR < 15 //Max 2013
		TCHAR *GetObjectName() { return GetString(IDS_TOSHAPE_CLASSNAME); }
#else
		const TCHAR *GetObjectName() { return GetString(IDS_TOSHAPE_CLASSNAME); }
#endif
		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; }
		void DeleteThis() { delete this; }
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		RefTargetHandle Clone( RemapDir &remap );
		int NumRefs();
		RefTargetHandle GetReference(int i);
#if MAX_VERSION_MAJOR < 14 //Max 2012
		void SetReference(int i, RefTargetHandle rtarg);
#else
private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
public:
#endif
#if MAX_VERSION_MAJOR < 17 //Max 2015
		RefResult NotifyRefChanged(	Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
#else
		RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message, BOOL propagate);
#endif

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return ((i == 0) ? pblock : NULL); }
		IParamBlock2* GetParamBlockByID(BlockID id) { return ((pblock->ID() == id) ? pblock : NULL); }

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		int RemapRefOnLoad(int iref);

		// From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		Class_ID InputType() { return defObjectClassID; }
		Interval LocalValidity(TimeValue t);
		void ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node);


		BOOL InterpCurveWorld(TimeValue t, int nodeIndex, int splineIndex, float lengthParam, Point3& basePos, Point3& baseTan);

		BOOL AddNode(INode* thisNode, INode* shapeNode);
		BOOL RemoveNode(int i);
		int GetNumNodes();
		INode* GetNode(int i);

		int GetNumPoints();
		void SetNumPoints(int numBinds);

		BOOL Bind(int pIdx, int nodeIndex, int splineIndex, float lengthParam, float weight, BOOL absolute);
		BOOL UnBind(int pIdx, int bIdx);
		int GetNumBinds(int pIdx);
		BOOL GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& lenParam, float& weight, BOOL& absolute);
		float GetBindWeight(int pIdx, int bIdx);
		BOOL SetBindWeight(int pIdx, int bIdx, float weight);

		void Update();
		void UpdateUI();
};
