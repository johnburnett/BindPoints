#include "BindPoints.h"

class BindMod : public Modifier
{
	public:
		Matrix3 thisTM;
		Tab<INode*> nodes;
		Tab<PointInfo*> pointInfo;

		static HWND hWnd;

		BindMod();
		virtual ~BindMod();

		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
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
		RefResult NotifyRefChanged(	const Interval& changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message, BOOL propagate);
#endif

		// From Modifier
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		Class_ID InputType() { return defObjectClassID; }


		void Update();
		BOOL AddNode(INode* thisNode, INode* node);
		BOOL RemoveNode(int i);
		int GetNumNodes();
		INode* GetNode(int i);

		int GetNumPoints();
		void SetNumPoints(int numBinds);

		BOOL UnBind(int vi, int bi);
		int GetNumBinds(int vi);
		BOOL GetBindInfo(int pIdx, int bIdx, PointInfo& pInfo);
		float GetBindWeight(int vi, int bi);
		BOOL SetBindWeight(int vi, int bi, float weight);

		void UpdateUI();
};
