/**********************************************************************
 *<
	FILE: ToPoint.cpp

	DESCRIPTION:	Bind points on one object to points on another.

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "ToPoint.h"
#include "AboutRollup.h"

class ToPointClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() {return 1;}
		void *			Create(BOOL loading = FALSE) {return new ToPoint();}
		const TCHAR *	ClassName() {return GetString(IDS_TOPOINT_CLASSNAME);}
		SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
		Class_ID		ClassID() {return TOPOINT_CLASSID;}
		const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
		const TCHAR*	InternalName() { return _T("BindToPoint"); }
		HINSTANCE		HInstance() { return hInstance; }
};

static ToPointClassDesc ToPointDesc;
ClassDesc2* GetToPointDesc() { return &ToPointDesc; }

IObjParam* ToPoint::ip = NULL;
HWND ToPoint::hWnd = NULL;

#if MAX_VERSION_MAJOR < 15	//Max 2013
 #define p_end end
#endif

static ParamBlockDesc2 bind_param_blk ( bind_params, _T("Parameters"),  0, &ToPointDesc,
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

class ToPointParamMapDlgProc : public ParamMap2UserDlgProc {
	public:
		ToPoint *tp;

		ToPointParamMapDlgProc(ToPoint *mod) { tp = mod; }
		void Update(TimeValue t) { if (tp) tp->UpdateUI(); }
#if MAX_VERSION_MAJOR < 9	//Max 9
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#else
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#endif
		void DeleteThis() { delete this; }
};

#if MAX_VERSION_MAJOR < 9	//Max 9
BOOL ToPointParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
INT_PTR ToPointParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
	switch (msg)
	{
		case WM_INITDIALOG :
		{
			tp->hWnd = hWnd;

			// Update everything else...
			tp->UpdateUI();
			break;
		}
		case WM_PAINT:
		{
			break;
		}
		case WM_DESTROY :
			tp->hWnd = NULL;
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ToPoint::ToPoint()
{

#if MAX_RELEASE >= 9000
	pblock = NULL;	//set reference to NULL
#endif

	ToPointDesc.MakeAutoParamBlocks(this);

	thisTM.IdentityMatrix();
	nodes.ZeroCount();
	pointInfo.ZeroCount();
	ver = CURRENT_VERSION;
	hAboutRollup = NULL;
}

ToPoint::~ToPoint()
{
	for (int i=0; i<pointInfo.Count(); i++)
		delete pointInfo[i];

	DeleteAllRefsFromMe();
}

void ToPoint::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	ToPointDesc.BeginEditParams(ip, this, flags, prev);
	bind_param_blk.SetUserDlgProc(new ToPointParamMapDlgProc(this));
	hAboutRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));
}

void ToPoint::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	ToPointDesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(hAboutRollup);

	this->ip = NULL;
}

void ToPoint::UpdateUI()
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

int ToPoint::NumRefs()
{
	return (NUM_REFS + nodes.Count());
}

RefTargetHandle ToPoint::GetReference(int i)
{
	if (i == PBLOCK_REF)
		return pblock;
	else
		return nodes[i-NUM_REFS];
}

void ToPoint::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF)
		pblock = (IParamBlock2*)rtarg;
	else
		nodes[i-NUM_REFS] = (INode*)rtarg;
}

#if MAX_VERSION_MAJOR < 17 //Max 2015
RefResult ToPoint::NotifyRefChanged(
	Interval changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message)
#else
RefResult ToPoint::NotifyRefChanged(
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

RefTargetHandle ToPoint::Clone(RemapDir& remap)
{
	ToPoint* newmod = new ToPoint();

	newmod->ReplaceReference(PBLOCK_REF, pblock->Clone(remap));

	newmod->thisTM = thisTM;
	newmod->nodes.SetCount(nodes.Count());
	for (int i=0; i < nodes.Count(); i++)
	{
		newmod->nodes[i] = NULL;
		newmod->ReplaceReference(i+NUM_REFS, nodes[i]);
	}

	int pointCount = pointInfo.Count();
	newmod->pointInfo.SetCount(pointCount);
	for (int pIdx=0; pIdx<pointCount; pIdx++)
	{
		int bindCount = pointInfo[pIdx]->binds.Count();

		newmod->pointInfo[pIdx] = new PointPoint;
		newmod->pointInfo[pIdx]->binds.SetCount(bindCount);

		for (int bIdx=0; bIdx<bindCount; bIdx++)
		{
			PointBind* b = new PointBind;
			b->basePos		= pointInfo[pIdx]->binds[bIdx]->basePos;
			b->nodeIndex	= pointInfo[pIdx]->binds[bIdx]->nodeIndex;
			b->pointIndex	= pointInfo[pIdx]->binds[bIdx]->pointIndex;
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
#define BINDINDEX_CHUNK		0x0002 // obsolete
#define BASEPOINT_CHUNK		0x0003 // obsolete
#define NUMNODES_CHUNK		0x0004
#define POINTINFO_CHUNK		0x0005

IOResult ToPoint::Save(ISave *isave)
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
				isave->Write(&pointInfo[pIdx]->binds[bIdx]->pointIndex, sizeof(int), &nb);
				isave->Write(&pointInfo[pIdx]->binds[bIdx]->weight, sizeof(float), &nb);
			}
		}

		isave->EndChunk();
	}

	return IO_OK;
}

IOResult ToPoint::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

	res = Modifier::Load(iload);
    if (res != IO_OK) return res;

	int numNodes = 0;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
			case VERSION_CHUNK: {
				iload->Read(&ver,sizeof(int), &nb);
				break;
			}
			case THISTM_CHUNK: {
				thisTM.Load(iload);
				break;
			}
			case NUMNODES_CHUNK: {
				iload->Read(&numNodes,sizeof(int), &nb);
				nodes.SetCount(numNodes);
				for (int i=0; i<numNodes; i++) nodes[i] = NULL;
				break;
			}
			case POINTINFO_CHUNK:
			{
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);
				pointInfo.SetCount(pointCount);

				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					pointInfo[pIdx] = new PointPoint;

					int bindCount;
					iload->Read(&bindCount, sizeof(int), &nb);
					pointInfo[pIdx]->binds.SetCount(bindCount);

					for (int bIdx=0; bIdx<bindCount; bIdx++)
					{
						PointBind* b = new PointBind;
						iload->Read(&b->basePos, sizeof(Point3), &nb);
						iload->Read(&b->nodeIndex, sizeof(int), &nb);
						iload->Read(&b->pointIndex, sizeof(int), &nb);
						iload->Read(&b->weight, sizeof(float), &nb);

						if (b->pointIndex == -1) // check if it's a "null" bind from old version
						{
							b->pointIndex = 0;
							b->weight = 0.0;
						}

						pointInfo[pIdx]->binds[bIdx] = b;
					}
				}
				break;
			}
			case BINDINDEX_CHUNK:
			{
				// make room for the one bind object
				nodes.SetCount(1);
				nodes[0] = NULL;

				// set point count to number of binds
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);
				pointInfo.SetCount(pointCount);

				// make initial binds (one for each point), setting everything
				// except the bindCoords (which are loaded later)
				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					pointInfo[pIdx] = new PointPoint;
					pointInfo[pIdx]->binds.SetCount(1);
					PointBind* b = new PointBind;

					iload->Read(&b->pointIndex, sizeof(int), &nb);
					if (b->pointIndex == -1) // check if it's a "null" bind from old version
					{
						b->pointIndex = 0;
						b->weight = 0.0;
					} else {
						b->weight = 1.0;
					}

					b->nodeIndex = 0;
					pointInfo[pIdx]->binds[0] = b;
				}
				break;
			}
			case BASEPOINT_CHUNK:
			{
				// finish setting up the binds for each point
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);

				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					iload->Read(&pointInfo[pIdx]->binds[0]->basePos, sizeof(Point3), &nb);
				}

				break;
			}
		}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}

	return IO_OK;
}

int ToPoint::RemapRefOnLoad(int iref)
{
	if (ver<4)
		return (iref+NUM_REFS);
	else
		return iref;
}

Interval ToPoint::LocalValidity(TimeValue t)
{
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval valid = FOREVER;

	for (int i=0; i<nodes.Count(); i++)
	{
		ObjectState os = nodes[i]->EvalWorldState(t);
		valid &= os.obj->ObjectValidity(t);
		nodes[i]->GetNodeTM(t, &valid);
	}

	pblock->GetValidity(t, valid);

	return valid;
}

void ToPoint::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	if (!nodes.Count()) return;

	int numPoints =	(os->obj->NumPoints() < GetNumPoints()) ?
					os->obj->NumPoints() : GetNumPoints();

	Matrix3 iThisTM = Inverse(thisTM);
	float strength = pblock->GetFloat(pb_strength, t);

	Tab<Object*> nodeObjs;
	nodeObjs.SetCount(nodes.Count());
	for (int i=0; i<nodes.Count(); i++)
		nodeObjs[i] = (nodes[i]->EvalWorldState(t)).obj;

	for (int pIdx=0; pIdx<numPoints; pIdx++)
	{
		if (os->obj->GetSubselState() == 0 || os->obj->PointSelection(pIdx))
		{
			Point3 thisP = os->obj->GetPoint(pIdx) * thisTM;
			Point3 offset(0.0f, 0.0f, 0.0f);

			for (int bIdx=0; bIdx<GetNumBinds(pIdx); bIdx++)
			{
				PointBind* b = pointInfo[pIdx]->binds[bIdx];

				if (b->pointIndex < nodeObjs[b->nodeIndex]->NumPoints())
				{
					Point3 newP = nodeObjs[b->nodeIndex]->GetPoint(b->pointIndex) * nodes[b->nodeIndex]->GetObjectTM(t);
					offset += (newP - b->basePos) * b->weight * strength;
				}
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

//////////////////////////////////////////////////////////////////////////////////////////////

int ToPoint::GetNumPoints()
{
	return pointInfo.Count();
}

void ToPoint::SetNumPoints(int numBinds)
{
	if (numBinds > pointInfo.Count())
	{
		for (int i=pointInfo.Count(); i<numBinds; i++)
		{
			PointPoint* p = new PointPoint;
			pointInfo.Append(1, &p);
		}
	} else {
		for (int i=(pointInfo.Count()-1); i>=numBinds; i--)
			delete pointInfo[i];
		pointInfo.SetCount(numBinds);
	}
	UpdateUI();
}

BOOL ToPoint::AddNode(INode* thisNode, INode* node)
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
	ReplaceReference((NUM_REFS + nodes.Count()-1), node);	//DB 2012
	#endif
#endif

	UpdateUI();

	return TRUE;
}

BOOL ToPoint::RemoveNode(int i)
{
	if (i<0 || i>=nodes.Count())
		return FALSE;

	DeleteReference(NUM_REFS + i);
	nodes.Delete(i,1);

	// Remap all binds to use new node indexes
	for (int pointIndex=0; pointIndex<pointInfo.Count(); pointIndex++)
	{
		for (int bindIndex=(pointInfo[pointIndex]->binds.Count()-1); bindIndex>=0; bindIndex--)
		{
			int ni = pointInfo[pointIndex]->binds[bindIndex]->nodeIndex;

			if (ni == i)
				UnBind(pointIndex, bindIndex);
			else if (ni > i)
			{
				pointInfo[pointIndex]->binds[bindIndex]->nodeIndex = (ni-1);
			}
		}
	}

	UpdateUI();

	return TRUE;
}

int ToPoint::GetNumNodes()
{
	return nodes.Count();
}

INode* ToPoint::GetNode(int i)
{
	if (i>=0 && i<nodes.Count())
		return nodes[i];
	else
		return NULL;
}

BOOL ToPoint::Bind(int thisIndex, int nodeIndex, int pointIndex, float weight)
{
	if (thisIndex < 0 || thisIndex >= pointInfo.Count()) return FALSE;
	if (nodeIndex < 0 || nodeIndex >= nodes.Count()) return FALSE;
	if (pointIndex < 0) return FALSE;

	Interface* ip = GetCOREInterface();
	if (!ip) return FALSE;
	TimeValue t = ip->GetTime();

	ObjectState bindOS = nodes[nodeIndex]->EvalWorldState(t);
	if (pointIndex >= bindOS.obj->NumPoints()) return FALSE;
	Point3 basePos = bindOS.obj->GetPoint(pointIndex) * nodes[nodeIndex]->GetObjectTM(t);

	BOOL res = pointInfo[thisIndex]->AddBind(basePos, nodeIndex, pointIndex, weight);

	UpdateUI();

	return res;
}

BOOL ToPoint::UnBind(int thisIndex, int bindIndex)
{
	if (thisIndex < 0 || thisIndex >= pointInfo.Count()) return FALSE;

	BOOL res = pointInfo[thisIndex]->RemoveBind(bindIndex);

	UpdateUI();

	return res;
}

int ToPoint::GetNumBinds(int thisIndex)
{
	if (thisIndex < 0 || thisIndex >= pointInfo.Count()) return -1;

	int cnt = pointInfo[thisIndex]->binds.Count();

	return cnt;
}

BOOL ToPoint::GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	nIdx = pointInfo[pIdx]->binds[bIdx]->nodeIndex;
	idx = pointInfo[pIdx]->binds[bIdx]->pointIndex;
	weight = pointInfo[pIdx]->binds[bIdx]->weight;

	return TRUE;
}

float ToPoint::GetBindWeight(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return -1.0f;

	return pointInfo[pIdx]->binds[bIdx]->weight;
}

BOOL ToPoint::SetBindWeight(int pIdx, int bIdx, float weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	pointInfo[pIdx]->binds[bIdx]->weight = weight;

	return TRUE;
}

void ToPoint::Update()
{
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}
