/**********************************************************************
 *<
	FILE: ToNode.cpp

	DESCRIPTION:	Bind points on one object to object transforms.

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "ToNode.h"
#include "AboutRollup.h"

class ToNodeClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return 1; }
		void *			Create(BOOL loading = FALSE) { return new ToNode(); }
		const TCHAR *	ClassName() { return GetString(IDS_TONODE_CLASSNAME); }
		SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
		Class_ID		ClassID() { return TONODE_CLASSID; }
		const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
		const TCHAR*	InternalName() { return _T("BindToNode"); }
		HINSTANCE		HInstance() { return hInstance; }
};

static ToNodeClassDesc ToNodeDesc;
ClassDesc2* GetToNodeDesc() { return &ToNodeDesc; }

IObjParam* ToNode::ip = NULL;
HWND ToNode::hWnd = NULL;

#if MAX_VERSION_MAJOR < 15	//Max 2013
 #define p_end end
#endif

static ParamBlockDesc2 bind_param_blk ( bind_params, _T("Parameters"),  0, &ToNodeDesc,
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BIND, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_strength, 	_T("strength"), TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_STRENGTH,
		p_default, 	1.0f,
		p_range, 	-9999999.0f,	9999999.0f,
		p_ui, 		TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_STRENGTH_EDIT,	IDC_STRENGTH_SPIN, 0.01f,
		p_end,
	p_end
);

class ToNodeParamMapDlgProc : public ParamMap2UserDlgProc {
	public:
		ToNode *tn;

		ToNodeParamMapDlgProc(ToNode *mod) { tn = mod; }
		void Update(TimeValue t) { if (tn) tn->UpdateUI(); }
#if MAX_VERSION_MAJOR < 9	//Max 9
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#else
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#endif
		void DeleteThis() { delete this; }
};

#if MAX_VERSION_MAJOR < 9	//Max 9
BOOL ToNodeParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
INT_PTR ToNodeParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			tn->hWnd = hWnd;

			// Update everything else...
			tn->UpdateUI();
			break;
		}
		case WM_PAINT:
		{
			break;
		}
		case WM_DESTROY:
			tn->hWnd = NULL;
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ToNode::ToNode()
{

#if MAX_RELEASE >= 9000
	pblock = NULL;	//set reference to NULL
#endif

	ToNodeDesc.MakeAutoParamBlocks(this);

	thisTM.IdentityMatrix();
	nodes.ZeroCount();
	baseTM.ZeroCount();
	pointInfo.ZeroCount();
	ver = CURRENT_VERSION;
	hAboutRollup = NULL;
}

ToNode::~ToNode()
{
	for (int i=0; i<pointInfo.Count(); i++)
		delete pointInfo[i];

	DeleteAllRefsFromMe();
}

void ToNode::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	ToNodeDesc.BeginEditParams(ip, this, flags, prev);
	bind_param_blk.SetUserDlgProc(new ToNodeParamMapDlgProc(this));
	hAboutRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));
}

void ToNode::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ToNodeDesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(hAboutRollup);

	this->ip = NULL;
}

void ToNode::UpdateUI()
{
	if (hWnd)
	{
		// Print num points and binds
		int i, nCount, pCount, bCount;
		TSTR str;
		HWND hTextWnd;

		nCount = nodes.Count();
		pCount = pointInfo.Count();
		bCount = 0;
		for (i=0; i<pCount; i++)
			bCount += pointInfo[i]->binds.Count();

#if MAX_VERSION_MAJOR < 15 //Max 2013
		str.printf("%d", nCount);
#else
		str.printf(_T("%d"), nCount);
#endif
		hTextWnd = GetDlgItem(hWnd,IDC_NUMNODES);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

#if MAX_VERSION_MAJOR < 15 //Max 2013
		str.printf("%d", pCount);
#else
		str.printf(_T("%d"), pCount);
#endif
		hTextWnd = GetDlgItem(hWnd,IDC_NUMPOINTS);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

#if MAX_VERSION_MAJOR < 15 //Max 2013
		str.printf("%d", bCount);
#else
		str.printf(_T("%d"), bCount);
#endif
		hTextWnd = GetDlgItem(hWnd,IDC_NUMBINDS);
		SetWindowText(hTextWnd, str);

		// Update node list
		SendDlgItemMessage(hWnd, IDC_NODES, LB_RESETCONTENT, 0, 0);
        for (i=0; i<GetNumNodes(); i++)
        {
			SendDlgItemMessage(hWnd, IDC_NODES, LB_ADDSTRING, 0, (LPARAM)GetNode(i)->GetName());
        }
	}
}

int ToNode::NumRefs()
{
	return (NUM_REFS + nodes.Count());
}

RefTargetHandle ToNode::GetReference(int i)
{
	if (i == PBLOCK_REF)
		return pblock;
	else
		return nodes[i-NUM_REFS];
}

void ToNode::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF)
		pblock = (IParamBlock2*)rtarg;
	else
		nodes[i-NUM_REFS] = (INode*)rtarg;
}

#if MAX_VERSION_MAJOR < 17 //Max 2015
RefResult ToNode::NotifyRefChanged(
	Interval changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message)
#else
RefResult ToNode::NotifyRefChanged(
	const Interval& changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message, BOOL propagate)
#endif
{
	switch (message)
	{
		case REFMSG_TARGET_DELETED:
		{
			for (int i=nodes.Count()-1; i>=0; i--)
			{
				if (nodes[i] == hTarget) RemoveNode(i);
			}
			break;
		}
	}
	return REF_SUCCEED;
}

RefTargetHandle ToNode::Clone(RemapDir& remap)
{
	ToNode* newmod = new ToNode();

	newmod->ReplaceReference(PBLOCK_REF, pblock->Clone(remap));

	newmod->thisTM = thisTM;
	newmod->nodes.SetCount(nodes.Count());
	for (int i=0; i < nodes.Count(); i++)
	{
		newmod->nodes[i] = NULL;
		newmod->ReplaceReference(i+NUM_REFS, nodes[i]);
	}

	newmod->baseTM.SetCount(baseTM.Count());
	for (int i=0; i < baseTM.Count(); i++)
		newmod->baseTM[i] = baseTM[i];

	int pointCount = pointInfo.Count();
	newmod->pointInfo.SetCount(pointCount);
	for (int pIdx=0; pIdx<pointCount; pIdx++)
	{
		int bindCount = pointInfo[pIdx]->binds.Count();

		newmod->pointInfo[pIdx] = new NodePoint;
		newmod->pointInfo[pIdx]->binds.SetCount(bindCount);

		for (int bIdx=0; bIdx<bindCount; bIdx++)
		{
			NodeBind* b = new NodeBind;
			b->basePos		= pointInfo[pIdx]->binds[bIdx]->basePos;
			b->nodeIndex	= pointInfo[pIdx]->binds[bIdx]->nodeIndex;
			b->weight		= pointInfo[pIdx]->binds[bIdx]->weight;
			newmod->pointInfo[pIdx]->binds[bIdx] = b;
		}
	}

#if MAX_RELEASE > 3100
	BaseClone(this, newmod, remap);
#endif

	return(newmod);
}

#define VERSION_CHUNK		0x0000
#define THISTM_CHUNK		0x0001
#define NUMNODES_CHUNK		0x0002
#define BASETM_CHUNK		0x0003
#define POINTINFO_CHUNK		0x0004

IOResult ToNode::Save(ISave *isave)
{
	ULONG nb;
	IOResult res;

	res = Modifier::Save(isave);
    if (res != IO_OK) return res;

	ver = CURRENT_VERSION;
	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&ver, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(THISTM_CHUNK);
	thisTM.Save(isave);
	isave->EndChunk();

	int numNodes = nodes.Count();
	isave->BeginChunk(NUMNODES_CHUNK);
	isave->Write(&numNodes, sizeof(int), &nb);
	isave->EndChunk();

	for (int i=0; i<numNodes; i++)
	{
		isave->BeginChunk(BASETM_CHUNK);
		baseTM[i].Save(isave);
		isave->EndChunk();
	}

	int pointCount = pointInfo.Count();
	if (pointCount)
	{
		isave->BeginChunk(POINTINFO_CHUNK);

		isave->Write(&pointCount, sizeof(int), &nb);
		for (int pIdx=0; pIdx<pointCount; pIdx++)
		{
			int bindCount = pointInfo[pIdx]->binds.Count();
			isave->Write(&bindCount, sizeof(int), &nb);
			for (int bIdx=0; bIdx<bindCount; bIdx++)
			{
				isave->Write(&pointInfo[pIdx]->binds[bIdx]->basePos, sizeof(Point3), &nb);
				isave->Write(&pointInfo[pIdx]->binds[bIdx]->nodeIndex, sizeof(int), &nb);
				isave->Write(&pointInfo[pIdx]->binds[bIdx]->weight, sizeof(float), &nb);
			}
		}

		isave->EndChunk();
	}

	return IO_OK;
}

IOResult ToNode::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

    res = Modifier::Load(iload);
    if (res != IO_OK) return res;

	int numNodes = 0;
	int cnt = 0;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
			case VERSION_CHUNK: {
				iload->Read(&ver,sizeof(int), &nb);
				break;
			}
			case THISTM_CHUNK:
				thisTM.Load(iload);
				break;
			case NUMNODES_CHUNK: {
				iload->Read(&numNodes,sizeof(int), &nb);
				nodes.SetCount(numNodes);
				baseTM.SetCount(numNodes);
				for (int i=0; i<numNodes; i++) nodes[i] = NULL;
				break;
			}
			case BASETM_CHUNK: {
				assert(numNodes); // numNodes should always be non-zero if we get here
				baseTM[cnt].Load(iload);
				cnt++;
				break;
			}
			case POINTINFO_CHUNK:
			{
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);
				pointInfo.SetCount(pointCount);

				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					pointInfo[pIdx] = new NodePoint;

					int bindCount;
					iload->Read(&bindCount, sizeof(int), &nb);
					pointInfo[pIdx]->binds.SetCount(bindCount);

					for (int bIdx=0; bIdx<bindCount; bIdx++)
					{
						NodeBind* b = new NodeBind;
						iload->Read(&b->basePos, sizeof(Point3), &nb);
						iload->Read(&b->nodeIndex, sizeof(int), &nb);
						iload->Read(&b->weight, sizeof(float), &nb);
						pointInfo[pIdx]->binds[bIdx] = b;
					}
				}

				break;
			}
		}
		res = iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}

	return IO_OK;
}

int ToNode::RemapRefOnLoad(int iref)
{
	if (ver<2)
		return (iref+NUM_REFS);
	else
		return iref;
}

Interval ToNode::LocalValidity(TimeValue t)
{
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval valid = FOREVER;

	for (int i=0; i<nodes.Count(); i++)
		nodes[i]->GetNodeTM(t, &valid);

	pblock->GetValidity(t, valid);

	return valid;
}

void ToNode::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	if (!nodes.Count()) return;

	int numPoints =	(os->obj->NumPoints() < GetNumPoints()) ?
					os->obj->NumPoints() : GetNumPoints();

	Matrix3 iThisTM = Inverse(thisTM);
	float strength = pblock->GetFloat(pb_strength, t);

	Tab<Matrix3> offsetTM;
	offsetTM.SetCount(baseTM.Count());
	for (int i=0; i<baseTM.Count(); i++) {
		Matrix3 m = Inverse(baseTM[i]);
		offsetTM[i] = m * nodes[i]->GetObjectTM(t);
	}

	for (int pIdx=0; pIdx<numPoints; pIdx++)
	{
		if (os->obj->GetSubselState() == 0 || os->obj->PointSelection(pIdx))
		{
			Point3 thisP = os->obj->GetPoint(pIdx) * thisTM;
			Point3 offset(0.0f, 0.0f, 0.0f);

			int numBinds = GetNumBinds(pIdx);
			for (int bIdx=0; bIdx<numBinds; bIdx++)
			{
				NodeBind* b = pointInfo[pIdx]->binds[bIdx];

				offset += (thisP * offsetTM[b->nodeIndex] - thisP) * b->weight * strength;
			}

			if (os->obj->GetSubselState() != 0)
				thisP += offset * os->obj->PointSelection(pIdx);
			else
				thisP += offset;

			os->obj->SetPoint(pIdx, (thisP*iThisTM));
		}
	}

	os->obj->UpdateValidity(GEOM_CHAN_NUM, LocalValidity(t));
	os->obj->PointsWereChanged();
}

int ToNode::GetNumPoints()
{
	return pointInfo.Count();
}

void ToNode::SetNumPoints(int numBinds)
{
	if (numBinds > pointInfo.Count())
	{
		for (int i=pointInfo.Count(); i<numBinds; i++)
		{
			NodePoint* p = new NodePoint;
			pointInfo.Append(1, &p);
		}
	} else {
		for (int i=(pointInfo.Count()-1); i>=numBinds; i--)
			delete pointInfo[i];
		pointInfo.SetCount(numBinds);
	}
	UpdateUI();
}

BOOL ToNode::AddNode(INode* thisNode, INode* node)
{
	TimeValue t = GetCOREInterface()->GetTime();

	for (int i=0; i<nodes.Count(); i++)
	{
		if (nodes[i] == node)
			return FALSE;
	}

	if (!nodes.Count())
		thisTM = thisNode->GetObjectTM(t);

	nodes.Append(1, &node);

#if MAX_VERSION_MAJOR < 9	//Max 9
	MakeRefByID(FOREVER, (NUM_REFS + nodes.Count()-1), node);
#else
	#if MAX_VERSION_MAJOR < 14	//Max 2012
	SetReference((NUM_REFS + nodes.Count()-1), node);
	#else
	ReplaceReference((NUM_REFS + nodes.Count()-1), node);
	#endif
#endif

	Matrix3 nodeTM = node->GetObjectTM(t);
	baseTM.Append(1, &nodeTM);

	UpdateUI();

	return TRUE;
}

BOOL ToNode::RemoveNode(int i)
{
	if (i<0 || i>=nodes.Count())
		return FALSE;

	DeleteReference(NUM_REFS + i);
	nodes.Delete(i,1);
	baseTM.Delete(i,1);

	// Remap all binds to use new node indexes
	for (int pIdx=0; pIdx<pointInfo.Count(); pIdx++)
	{
		for (int bIdx=(pointInfo[pIdx]->binds.Count()-1); bIdx>=0; bIdx--)
		{
			int ni = pointInfo[pIdx]->binds[bIdx]->nodeIndex;

			if (ni == i)
				UnBind(pIdx, bIdx);
			else if (ni > i)
			{
				pointInfo[pIdx]->binds[bIdx]->nodeIndex = (ni-1);
			}
		}
	}

	UpdateUI();

	return TRUE;
}

int ToNode::GetNumNodes()
{
	return nodes.Count();
}

INode* ToNode::GetNode(int i)
{
	if (i>=0 && i<nodes.Count())
		return nodes[i];
	else
		return NULL;
}

BOOL ToNode::Bind(INode* thisNode, int pIdx, int nodeIndex, float weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return FALSE;
	if (nodeIndex < 0 || nodeIndex >= nodes.Count()) return FALSE;

	Interface* ip = GetCOREInterface();
	if (!ip) return FALSE;
	TimeValue t = ip->GetTime();

	ObjectState thisOS = thisNode->EvalWorldState(t);
	if (pIdx >= thisOS.obj->NumPoints()) return FALSE;

	Point3 basePos = thisOS.obj->GetPoint(pIdx) * thisNode->GetObjectTM(t);

	BOOL res = pointInfo[pIdx]->AddBind(basePos, nodeIndex, weight);

	UpdateUI();

	return res;
}

BOOL ToNode::UnBind(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return FALSE;

	BOOL res = pointInfo[pIdx]->RemoveBind(bIdx);

	UpdateUI();

	return res;
}

int ToNode::GetNumBinds(int pIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return -1;

	int cnt = pointInfo[pIdx]->binds.Count();

	return cnt;
}

BOOL ToNode::GetBindInfo(int pIdx, int bIdx, int& nIdx, float& weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	nIdx = pointInfo[pIdx]->binds[bIdx]->nodeIndex;
	weight = pointInfo[pIdx]->binds[bIdx]->weight;

	return TRUE;
}

float ToNode::GetBindWeight(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return -1.0f;

	return pointInfo[pIdx]->binds[bIdx]->weight;
}

BOOL ToNode::SetBindWeight(int pIdx, int bIdx, float weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	pointInfo[pIdx]->binds[bIdx]->weight = weight;

	return TRUE;
}

void ToNode::Update()
{
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}
