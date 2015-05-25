/**********************************************************************
 *<
	FILE: ToShape.cpp

	DESCRIPTION:	Bind points on one object to splines in a shape.

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "ToShape.h"
#include "AboutRollup.h"

class ToShapeClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return 1; }
		void *			Create(BOOL loading = FALSE) { return new ToShape(); }
		const TCHAR *	ClassName() { return GetString(IDS_TOSHAPE_CLASSNAME); }
		SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
		Class_ID		ClassID() { return TOSHAPE_CLASSID; }
		const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
		const TCHAR*	InternalName() { return _T("BindToShape"); }
		HINSTANCE		HInstance() { return hInstance; }
};

static ToShapeClassDesc ToShapeDesc;
ClassDesc2* GetToShapeDesc() { return &ToShapeDesc; }

IObjParam* ToShape::ip = NULL;
HWND ToShape::hWnd = NULL;

static ParamBlockDesc2 bind_param_blk ( bind_params, _T("Parameters"),  0, &ToShapeDesc,
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_BIND, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_strength, 	_T("strength"), TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_STRENGTH,
		p_default, 	1.0f,
		p_range, 	-9999999.0f,	9999999.0f,
		p_ui, 		TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_STRENGTH_EDIT,	IDC_STRENGTH_SPIN, 0.01f,
		end,
	end
);

class ToShapeParamMapDlgProc : public ParamMap2UserDlgProc {
	public:
		ToShape *ts;

		ToShapeParamMapDlgProc(ToShape *mod) { ts = mod; }
		void Update(TimeValue t) { if (ts) ts->UpdateUI(); }
#if (MAX_RELEASE >= 9000)
		INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#else
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#endif
		void DeleteThis() { delete this; }
};

#if (MAX_RELEASE >= 9000)
INT_PTR ToShapeParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
BOOL ToShapeParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
	switch (msg)
	{
		case WM_INITDIALOG :
		{
			ts->hWnd = hWnd;

			// Update everything else...
			ts->UpdateUI();
			break;
		}
		case WM_PAINT:
		{
			break;
		}
		case WM_DESTROY :
			ts->hWnd = NULL;
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////

ToShape::ToShape()
{

#if MAX_RELEASE >= 9000	//max 9
	pblock = NULL;	//set reference to NULL
#endif

	ToShapeDesc.MakeAutoParamBlocks(this);

	thisTM.IdentityMatrix();
	nodes.ZeroCount();
	pointInfo.ZeroCount();
	ver = CURRENT_VERSION;
	hAboutRollup = NULL;
}

ToShape::~ToShape()
{
	for (int i=0; i<pointInfo.Count(); i++)
		delete pointInfo[i];

	DeleteAllRefsFromMe();
}

void ToShape::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	ToShapeDesc.BeginEditParams(ip, this, flags, prev);
	bind_param_blk.SetUserDlgProc(new ToShapeParamMapDlgProc(this));
	hAboutRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));
}

void ToShape::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	ToShapeDesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(hAboutRollup);

	this->ip = NULL;
}

void ToShape::UpdateUI()
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

		str.printf("%d", nCount);
		hTextWnd = GetDlgItem(hWnd,IDC_NUMNODES);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

		str.printf("%d", pCount);
		hTextWnd = GetDlgItem(hWnd,IDC_NUMPOINTS);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

		str.printf("%d", bCount);
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

int ToShape::NumRefs()
{
	return (NUM_REFS + nodes.Count());
}

RefTargetHandle ToShape::GetReference(int i)
{
	if (i == PBLOCK_REF)
		return pblock;
	else
		return nodes[i-NUM_REFS];
}

void ToShape::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF)
		pblock = (IParamBlock2*)rtarg;
	else
		nodes[i-NUM_REFS] = (INode*)rtarg;
}

RefResult ToShape::NotifyRefChanged(
	Interval changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message)
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

RefTargetHandle ToShape::Clone(RemapDir& remap)
{
	ToShape* newmod = new ToShape();

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
	for (int vIdx=0; vIdx<pointCount; vIdx++)
	{
		int bindCount = pointInfo[vIdx]->binds.Count();

		newmod->pointInfo[vIdx] = new ShapePoint;
		newmod->pointInfo[vIdx]->binds.SetCount(bindCount);

		for (int bIdx=0; bIdx<bindCount; bIdx++)
		{
			ShapeBind* b = new ShapeBind;
			b->basePos		= pointInfo[vIdx]->binds[bIdx]->basePos;
			b->baseTan		= pointInfo[vIdx]->binds[bIdx]->baseTan;
			b->lengthParam	= pointInfo[vIdx]->binds[bIdx]->lengthParam;
			b->weight		= pointInfo[vIdx]->binds[bIdx]->weight;
			b->nodeIndex	= pointInfo[vIdx]->binds[bIdx]->nodeIndex;
			b->splineIndex	= pointInfo[vIdx]->binds[bIdx]->splineIndex;
			newmod->pointInfo[vIdx]->binds[bIdx] = b;
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
#define POINTINFO_CHUNK		0x0003

IOResult ToShape::Save(ISave *isave)
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
		for (int vIdx=0; vIdx<pointCount; vIdx++)
		{
			int bindCount = pointInfo[vIdx]->binds.Count();
			isave->Write(&bindCount, sizeof(int), &nb);
			for (int bIdx=0; bIdx<bindCount; bIdx++)
			{
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->basePos, sizeof(Point3), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->baseTan, sizeof(Point3), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->lengthParam, sizeof(float), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->weight, sizeof(float), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->nodeIndex, sizeof(int), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->splineIndex, sizeof(int), &nb);
				isave->Write(&pointInfo[vIdx]->binds[bIdx]->absolute, sizeof(BOOL), &nb);
			}
		}

		isave->EndChunk();
	}

	return IO_OK;
}

IOResult ToShape::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

    res = Modifier::Load(iload);
    if (res != IO_OK) return res;

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
				int numNodes;
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

				for (int vIdx=0; vIdx<pointCount; vIdx++)
				{
					pointInfo[vIdx] = new ShapePoint;

					int bindCount;
					iload->Read(&bindCount, sizeof(int), &nb);
					pointInfo[vIdx]->binds.SetCount(bindCount);

					for (int bIdx=0; bIdx<bindCount; bIdx++)
					{
						ShapeBind* b = new ShapeBind;
						iload->Read(&b->basePos, sizeof(Point3), &nb);
						if (ver >= 2)
							iload->Read(&b->baseTan, sizeof(Point3), &nb);
						iload->Read(&b->lengthParam, sizeof(float), &nb);
						iload->Read(&b->weight, sizeof(float), &nb);
						iload->Read(&b->nodeIndex, sizeof(int), &nb);
						iload->Read(&b->splineIndex, sizeof(int), &nb);
						if (ver >= 3)
							iload->Read(&b->absolute, sizeof(BOOL), &nb);
						pointInfo[vIdx]->binds[bIdx] = b;
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

int ToShape::RemapRefOnLoad(int iref)
{
	if (ver<4)
		return (iref+NUM_REFS);
	else
		return iref;
}

Interval ToShape::LocalValidity(TimeValue t)
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

void ToShape::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	if (!nodes.Count()) return;

	int numPoints =	(os->obj->NumPoints() < GetNumPoints()) ?
						os->obj->NumPoints() : GetNumPoints();

	Matrix3 iThisTM = Inverse(thisTM);
	float strength = pblock->GetFloat(pb_strength, t);

	for (int pIdx=0; pIdx<numPoints; pIdx++)
	{
		if (os->obj->GetSubselState() == 0 || os->obj->PointSelection(pIdx))
		{
			Point3 thisP = os->obj->GetPoint(pIdx) * thisTM;
			Point3 offset(0.0f,0.0f,0.0f);

			for (int bIdx=0; bIdx<GetNumBinds(pIdx); bIdx++)
			{
				ShapeBind* b = pointInfo[pIdx]->binds[bIdx];

				Point3 newPos, newTan;
				if (InterpCurveWorld(t, b->nodeIndex, b->splineIndex, b->lengthParam, newPos, newTan))
				{
					if (b->absolute)
					{
						Matrix3 rot(TRUE);
						if (b->baseTan != newTan)
						{
							float angle = (float)acos(DotProd(b->baseTan, newTan));
							Point3 perp = CrossProd(b->baseTan, newTan);
							rot = RotAngleAxisMatrix(Normalize(perp), angle);
						}

//						Point3 perpOffset = (thisP - b->basePos) * rot;
//						Point3 finalPos = newPos + perpOffset;
//						offset += (finalPos - thisP) * b->weight * strength;

						// same thing as above, but avoiding making temporaries
						offset += ((newPos + ((thisP - b->basePos) * rot)) - thisP) * b->weight * strength;
					} else {
						offset += (newPos - b->basePos) * b->weight * strength;
					}
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

BOOL ToShape::InterpCurveWorld(TimeValue t, int nodeIndex, int splineIndex, float lengthParam, Point3& basePos, Point3& baseTan)
{
	if (nodeIndex < 0 || nodeIndex >= nodes.Count()) return FALSE;

	ObjectState os = nodes[nodeIndex]->EvalWorldState(t);

	if (os.obj->SuperClassID() != SHAPE_CLASS_ID) return FALSE;

	ShapeObject* shp = (ShapeObject*)os.obj;
	if (splineIndex < 0 || splineIndex >= shp->NumberOfCurves()) return FALSE;

	if (lengthParam < 0.0f) lengthParam = 0.0f;
	if (lengthParam > 1.0f) lengthParam = 1.0f;

	Matrix3 tm = nodes[nodeIndex]->GetObjectTM(t);
	basePos = shp->InterpCurve3D(t, splineIndex, lengthParam, PARAM_NORMALIZED);
	basePos = basePos * tm;
	baseTan = shp->TangentCurve3D(t, splineIndex, lengthParam, PARAM_NORMALIZED);
	baseTan = VectorTransform(tm, baseTan);

	return TRUE;
}

int ToShape::GetNumPoints()
{
	return pointInfo.Count();
}

void ToShape::SetNumPoints(int numBinds)
{
	if (numBinds > pointInfo.Count())
	{
		for (int i=pointInfo.Count(); i<numBinds; i++)
		{
			ShapePoint* p = new ShapePoint;
			pointInfo.Append(1, &p);
		}
	} else {
		for (int i=(pointInfo.Count()-1); i>=numBinds; i--)
			delete pointInfo[i];
		pointInfo.SetCount(numBinds);
	}
	UpdateUI();
}

BOOL ToShape::AddNode(INode* thisNode, INode* node)
{
	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t);

	if (os.obj->SuperClassID() != SHAPE_CLASS_ID)
		return FALSE;

	for (int i=0; i<nodes.Count(); i++)
	{
		if (nodes[i] == node)
			return FALSE;
	}

	if (!nodes.Count())
		thisTM = thisNode->GetObjectTM(t);

	nodes.Append(1, &node);

#if (MAX_RELEASE >= 9000)	//max 9
	SetReference((NUM_REFS + nodes.Count()-1), node);
#else	//max 8 and earlier
	MakeRefByID(FOREVER, (NUM_REFS + nodes.Count()-1), node);
#endif

	UpdateUI();

	return TRUE;
}

BOOL ToShape::RemoveNode(int i)
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
			int si = pointInfo[pointIndex]->binds[bindIndex]->nodeIndex;

			if (si == i)
				UnBind(pointIndex, bindIndex);
			else if (si > i)
			{
				pointInfo[pointIndex]->binds[bindIndex]->nodeIndex = (si-1);
			}
		}
	}

	UpdateUI();

	return TRUE;
}

int ToShape::GetNumNodes()
{
	return nodes.Count();
}

INode* ToShape::GetNode(int i)
{
	if (i>=0 && i<nodes.Count())
		return nodes[i];
	else
		return NULL;
}

BOOL ToShape::Bind(int pIdx, int nodeIndex, int splineIndex, float lengthParam, float weight, BOOL absolute)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return FALSE;

	Interface* ip = GetCOREInterface();
	if (!ip) return FALSE;
	TimeValue t = ip->GetTime();

	Point3 basePos;
	Point3 baseTan;
	if (!InterpCurveWorld(t, nodeIndex, splineIndex, lengthParam, basePos, baseTan)) return FALSE;

	BOOL res = pointInfo[pIdx]->AddBind(basePos, baseTan, nodeIndex, splineIndex, lengthParam, weight, absolute);

	UpdateUI();

	return res;
}

BOOL ToShape::UnBind(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return FALSE;

	BOOL res = pointInfo[pIdx]->RemoveBind(bIdx);

	UpdateUI();

	return res;
}

int ToShape::GetNumBinds(int pIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count()) return -1;

	int cnt = pointInfo[pIdx]->binds.Count();

	return cnt;
}

BOOL ToShape::GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& lenParam, float& weight, BOOL& absolute)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	nIdx = pointInfo[pIdx]->binds[bIdx]->nodeIndex;
	idx = pointInfo[pIdx]->binds[bIdx]->splineIndex;
	lenParam = pointInfo[pIdx]->binds[bIdx]->lengthParam;
	weight = pointInfo[pIdx]->binds[bIdx]->weight;
	absolute = pointInfo[pIdx]->binds[bIdx]->absolute;

	return TRUE;
}

float ToShape::GetBindWeight(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return -1.0f;

	return pointInfo[pIdx]->binds[bIdx]->weight;
}

BOOL ToShape::SetBindWeight(int pIdx, int bIdx, float weight)
{
	if (pIdx < 0 || pIdx >= pointInfo.Count() ||
		bIdx < 0 || bIdx >= pointInfo[pIdx]->binds.Count()) return FALSE;

	pointInfo[pIdx]->binds[bIdx]->weight = weight;

	return TRUE;
}

void ToShape::Update()
{
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}
