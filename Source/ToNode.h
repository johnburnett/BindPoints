#include "BindPoints.h"
#include "ToNode_Binds.h"

#define TONODE_CLASSID Class_ID(0x59177a17, 0x739b4ff1)

// version (now using global CURRENT_VERSION):
// 1 - initial version
// 4 - added pblock as ref 0 (skipped versions to get in sync)
//#define NODE_VERSION		4

class ToNode : public Modifier
{
	public:
		IParamBlock2* pblock;
		Matrix3 thisTM;
		Tab<INode*> nodes;
		Tab<Matrix3> baseTM;
		Tab<NodePoint*> pointInfo;
		int ver;

		static IObjParam* ip;
		static HWND hWnd;
		HWND hAboutRollup;

		ToNode();
		virtual ~ToNode();

		Class_ID ClassID() { return TONODE_CLASSID; }
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_TONODE_CLASSNAME); }
		TCHAR *GetObjectName() { return GetString(IDS_TONODE_CLASSNAME); }
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

		BOOL Bind(INode* thisNode, int pIdx, int nodeIndex, float weight);
		BOOL UnBind(int vi, int bi);
		int GetNumBinds(int vi);
		BOOL GetBindInfo(int pIdx, int bIdx, int& nIdx, float& weight);
		float GetBindWeight(int vi, int bi);
		BOOL SetBindWeight(int vi, int bi, float weight);

		void Update();
		void UpdateUI();
};
