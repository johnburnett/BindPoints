#include "BindPoints.h"
#include "ToPoint_Binds.h"

#define TOPOINT_CLASSID Class_ID(0xfa8a90f, 0xecaeeb6)

// version (now using global CURRENT_VERSION):
// 1 - initial version
// 2 - weighting
// 4 - pblock as ref 0
//#define POINT_VERSION		4

class ToPoint : public Modifier
{
	public:
		IParamBlock2* pblock;
		Matrix3 thisTM;
		Tab<INode*> nodes;
		Tab<PointPoint*> pointInfo;
		int ver;

		static IObjParam* ip;
		static HWND hWnd;
		HWND hAboutRollup;

		ToPoint();
		virtual ~ToPoint();

		Class_ID ClassID() { return TOPOINT_CLASSID; }
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_TOPOINT_CLASSNAME); }
		TCHAR *GetObjectName() { return GetString(IDS_TOPOINT_CLASSNAME); }
		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; }
		void DeleteThis() { delete this; }
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		RefTargetHandle Clone( RemapDir &remap );
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(	Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);

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


		BOOL AddNode(INode* thisNode, INode* node);
		BOOL RemoveNode(int i);
		int GetNumNodes();
		INode* GetNode(int i);

		int GetNumPoints();
		void SetNumPoints(int numBinds);

		BOOL Bind(int thisIndex, int nodeIndex, int pointIndex, float weight);
		BOOL UnBind(int pIdx, int bIdx);
		int GetNumBinds(int pIdx);
		BOOL GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& weight);
		float GetBindWeight(int pIdx, int bIdx);
		BOOL SetBindWeight(int pIdx, int bIdx, float weight);

		void Update();
		void UpdateUI();
};
