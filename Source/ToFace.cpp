/**********************************************************************
 *<
	FILE: ToFace.cpp

	DESCRIPTION:	Bind points on one object to faces on another

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "ToFace.h"
#include "AboutRollup.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class ToFaceClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()						{ return 1; }
	void *			Create(BOOL loading = FALSE)	{ return new ToFace(); }
	const TCHAR *	ClassName()						{ return GetString(IDS_TOFACE_CLASSNAME); }
	SClass_ID		SuperClassID()					{ return OSM_CLASS_ID; }
	Class_ID		ClassID()						{ return TOFACE_CLASSID; }
	const TCHAR* 	Category()						{ return GetString(IDS_CATEGORY); }
	const TCHAR*	InternalName()					{ return _T("BindToFace"); }
	HINSTANCE		HInstance()						{ return hInstance; }
};

static ToFaceClassDesc ToFaceDesc;
ClassDesc2* GetToFaceDesc() { return &ToFaceDesc; }

///////////////////////////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 bind_param_blk
(
	bind_params, _T("Parameters"),  0, &ToFaceDesc,
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

///////////////////////////////////////////////////////////////////////////////////////////////////

class ToFaceParamMapDlgProc : public ParamMap2UserDlgProc
{
public:
	ToFaceParamMapDlgProc	(ToFace* toFace) : m_toFace(toFace) { }

#if (MAX_RELEASE >= 9000)
	INT_PTR	DlgProc			(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else
	BOOL	DlgProc			(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
	void	Update			(TimeValue t);
	void	DeleteThis		();

private:
	ToFace* m_toFace;
};

#if (MAX_RELEASE >= 9000)
INT_PTR ToFaceParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
BOOL ToFaceParamMapDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
#endif
{
	switch (msg)
	{
		case WM_INITDIALOG :
		{
			m_toFace->m_hWnd = hWnd;

			// Update everything else...
			m_toFace->UpdateUI();
			break;
		}
		case WM_PAINT:
		{
			break;
		}
		case WM_DESTROY :
			m_toFace->m_hWnd = NULL;
			break;
	}
	return FALSE;
}

void ToFaceParamMapDlgProc::Update(TimeValue t)
{
	if (m_toFace) m_toFace->UpdateUI();
}

void ToFaceParamMapDlgProc::DeleteThis()
{
	delete this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Utility class used in ModifyObject that just wraps each bind node after making sure it's
// a valid one to use, and takes care of freeing resources when its not needed anymore.

class ToFaceNode
{
public:
	static ToFaceNode* CreateToFaceNode(TimeValue t, INode* inode);
	~ToFaceNode();

	Matrix3		objectToWorld;
	Mesh*		mesh;

private:
	ToFaceNode	(TriObject* triObject, BOOL needDelete);

	TriObject*	m_triObject;
	BOOL		m_needDelete;
};

ToFaceNode::ToFaceNode(TriObject* triObject, BOOL needDelete) :
	m_triObject		(triObject),
	m_needDelete	(needDelete)
{
}

ToFaceNode::~ToFaceNode()
{
	if (m_triObject && m_needDelete)
	{
		delete m_triObject;
		m_triObject = NULL;
	}
}

ToFaceNode* ToFaceNode::CreateToFaceNode(TimeValue t, INode* inode)
{
	ToFaceNode* toFaceNode = NULL;

	BOOL needDelete = FALSE;
	TriObject* triObject = ToFace::GetTriObject(t, inode, needDelete);

	if (triObject)
	{
		toFaceNode = new ToFaceNode(triObject, needDelete);

		toFaceNode->mesh = &(triObject->GetMesh());
		toFaceNode->mesh->checkNormals(TRUE);
		toFaceNode->objectToWorld = inode->GetObjectTM(t);
	}

	return toFaceNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

HWND ToFace::m_hWnd = NULL;

ToFace::ToFace()
{

#if MAX_RELEASE >= 9000	//max 9
	m_pblock = NULL;		//set reference to NULL
#endif

	ToFaceDesc.MakeAutoParamBlocks(this);

	m_objectToWorld.IdentityMatrix();
	m_nodes.ZeroCount();
	m_pointInfo.ZeroCount();
	m_version = CURRENT_VERSION;
	m_hAboutRollup = NULL;
}

ToFace::~ToFace()
{
	for (int i=0; i<m_pointInfo.Count(); i++)
		delete m_pointInfo[i];

	DeleteAllRefsFromMe();
}

// Animatable /////////////////////////////////////////////////////////////////////////////////

void ToFace::GetClassName(TSTR& s)
{
	s = GetString(IDS_TOFACE_CLASSNAME);
}

Class_ID ToFace::ClassID()
{
	return TOFACE_CLASSID;
}

void ToFace::DeleteThis()
{
	delete this;
}

void ToFace::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
	ToFaceDesc.BeginEditParams(ip, this, flags, prev);
	bind_param_blk.SetUserDlgProc(new ToFaceParamMapDlgProc(this));
	m_hAboutRollup = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));
}

void ToFace::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	ToFaceDesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(m_hAboutRollup);
}

int ToFace::NumParamBlocks()
{
	return 1;
}

IParamBlock2* ToFace::GetParamBlock(int i)
{
	return ((i == 0) ? m_pblock : NULL);
}

IParamBlock2* ToFace::GetParamBlockByID(BlockID id)
{
	return ((m_pblock->ID() == id) ? m_pblock : NULL);
}

// ReferenceMaker /////////////////////////////////////////////////////////////////////////////////

int ToFace::NumRefs()
{
	return (NUM_REFS + m_nodes.Count());
}

RefTargetHandle ToFace::GetReference(int i)
{
	if (i == PBLOCK_REF)
		return m_pblock;
	else
		return m_nodes[i-NUM_REFS];
}

void ToFace::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF)
		m_pblock = (IParamBlock2*)rtarg;
	else
		m_nodes[i-NUM_REFS] = (INode*)rtarg;
}

int ToFace::RemapRefOnLoad(int iref)
{
	if (m_version<3)
		return (iref+NUM_REFS);
	else
		return iref;
}

RefResult ToFace::NotifyRefChanged(
	Interval changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message)
{
	switch (message)
	{
		case REFMSG_TARGET_DELETED:
		{
			for (int i=m_nodes.Count()-1; i>=0; i--)
			{
				if (m_nodes[i] == hTarget) RemoveNode(i);
			}
			break;
		}
	}
	return REF_SUCCEED;
}

// ReferenceTarget ////////////////////////////////////////////////////////////////////////////////

RefTargetHandle ToFace::Clone(RemapDir& remap)
{
	ToFace* toFace = new ToFace();

	toFace->ReplaceReference(PBLOCK_REF, m_pblock->Clone(remap));

	toFace->m_objectToWorld = m_objectToWorld;
	toFace->m_nodes.SetCount(m_nodes.Count());
	for (int i=0; i < m_nodes.Count(); i++)
	{
		toFace->m_nodes[i] = NULL;
		toFace->ReplaceReference(i+NUM_REFS, m_nodes[i]);
	}

	int pointCount = m_pointInfo.Count();
	toFace->m_pointInfo.SetCount(pointCount);
	for (int pIdx=0; pIdx<pointCount; pIdx++)
	{
		int bindCount = m_pointInfo[pIdx]->binds.Count();

		toFace->m_pointInfo[pIdx] = new FacePoint;
		toFace->m_pointInfo[pIdx]->binds.SetCount(bindCount);

		for (int bIdx=0; bIdx<bindCount; bIdx++)
		{
			FaceBind* b = new FaceBind;
			b->bindCoord	= m_pointInfo[pIdx]->binds[bIdx]->bindCoord;
			b->nodeIndex	= m_pointInfo[pIdx]->binds[bIdx]->nodeIndex;
			b->faceIndex	= m_pointInfo[pIdx]->binds[bIdx]->faceIndex;
			b->weight		= m_pointInfo[pIdx]->binds[bIdx]->weight;
			toFace->m_pointInfo[pIdx]->binds[bIdx] = b;
		}
	}

#if MAX_RELEASE > 3100
	BaseClone(this, toFace, remap);
#endif

	return(toFace);
}

// BaseObject /////////////////////////////////////////////////////////////////////////////////////

CreateMouseCallBack* ToFace::GetCreateMouseCallBack()
{
	return NULL;
}

TCHAR* ToFace::GetObjectName()
{
	return GetString(IDS_TOFACE_CLASSNAME);
}

// Modifier ///////////////////////////////////////////////////////////////////////////////////////

Interval ToFace::LocalValidity(TimeValue t)
{
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval valid = FOREVER;

	for (int i=0; i<m_nodes.Count(); i++)
	{
		ObjectState os = m_nodes[i]->EvalWorldState(t);
		valid &= os.obj->ObjectValidity(t);
		m_nodes[i]->GetNodeTM(t, &valid);
	}

	m_pblock->GetValidity(t, valid);

	return valid;
}

ChannelMask ToFace::ChannelsUsed()
{
	return PART_GEOM|PART_TOPO;
}

ChannelMask ToFace::ChannelsChanged()
{
	return GEOM_CHANNEL;
}

#define VERSION_CHUNK		0x0000
#define THISTM_CHUNK		0x0001
#define BINDINDEX_CHUNK		0x0002 // obsolete
#define BINDCOORD_CHUNK		0x0003 // obsolete
#define NUMNODES_CHUNK		0x0004
#define POINTINFO_CHUNK		0x0005

IOResult ToFace::Save(ISave* isave)
{
	ULONG nb;
	IOResult res;

	res = Modifier::Save(isave);
    if (res != IO_OK) return res;

	m_version = CURRENT_VERSION;
	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&m_version, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(THISTM_CHUNK);
	m_objectToWorld.Save(isave);
	isave->EndChunk();

	int numNodes = m_nodes.Count();
	isave->BeginChunk(NUMNODES_CHUNK);
	isave->Write(&numNodes, sizeof(int), &nb);
	isave->EndChunk();

	int pointCount = m_pointInfo.Count();
	if (pointCount)
	{
		isave->BeginChunk(POINTINFO_CHUNK);

		isave->Write(&pointCount, sizeof(int), &nb);
		for (int pIdx=0; pIdx<pointCount; pIdx++)
		{
			int bindCount = m_pointInfo[pIdx]->binds.Count();
			isave->Write(&bindCount, sizeof(int), &nb);
			for (int bIdx=0; bIdx<bindCount; bIdx++)
			{
				isave->Write(&m_pointInfo[pIdx]->binds[bIdx]->bindCoord, sizeof(Point3), &nb);
				isave->Write(&m_pointInfo[pIdx]->binds[bIdx]->nodeIndex, sizeof(int), &nb);
				isave->Write(&m_pointInfo[pIdx]->binds[bIdx]->faceIndex, sizeof(int), &nb);
				isave->Write(&m_pointInfo[pIdx]->binds[bIdx]->weight, sizeof(float), &nb);
			}
		}

		isave->EndChunk();
	}

	return IO_OK;
}

IOResult ToFace::Load(ILoad* iload)
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
				iload->Read(&m_version,sizeof(int), &nb);
				break;
			}
			case THISTM_CHUNK: {
				m_objectToWorld.Load(iload);
				break;
			}
			case NUMNODES_CHUNK: {
				iload->Read(&numNodes,sizeof(int), &nb);
				m_nodes.SetCount(numNodes);
				for (int i=0; i<numNodes; i++) m_nodes[i] = NULL;
				break;
			}
			case POINTINFO_CHUNK:
			{
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);
				m_pointInfo.SetCount(pointCount);

				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					m_pointInfo[pIdx] = new FacePoint;

					int bindCount;
					iload->Read(&bindCount, sizeof(int), &nb);
					m_pointInfo[pIdx]->binds.SetCount(bindCount);

					for (int bIdx=0; bIdx<bindCount; bIdx++)
					{
						FaceBind* b = new FaceBind;
						iload->Read(&b->bindCoord, sizeof(Point3), &nb);
						iload->Read(&b->nodeIndex, sizeof(int), &nb);
						iload->Read(&b->faceIndex, sizeof(int), &nb);
						iload->Read(&b->weight, sizeof(float), &nb);

						if (b->faceIndex == -1) // check if it's a "null" bind from old version
						{
							b->faceIndex = 0;
							b->weight = 0.0;
						}

						m_pointInfo[pIdx]->binds[bIdx] = b;
					}
				}
				break;
			}
			case BINDINDEX_CHUNK: // old version
			{
				// make room for the one bind object
				m_nodes.SetCount(1);
				m_nodes[0] = NULL;

				// set point count to number of binds
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);
				m_pointInfo.SetCount(pointCount);

				// make initial binds (one for each point), setting everything
				// except the bindCoords (which are loaded later)
				for (int pIdx=0; pIdx<pointCount; pIdx++)
				{
					m_pointInfo[pIdx] = new FacePoint;
					m_pointInfo[pIdx]->binds.SetCount(1);
					FaceBind* b = new FaceBind;

					iload->Read(&b->faceIndex, sizeof(int), &nb);
					if (b->faceIndex == -1) // check if it's a "null" bind from old version
					{
						b->faceIndex = 0;
						b->weight = 0.0;
					} else {
						b->weight = 1.0;
					}

					b->nodeIndex = 0;
					m_pointInfo[pIdx]->binds[0] = b;
				}
				break;
			}
			case BINDCOORD_CHUNK: // old version
			{
				// finish setting up the binds for each point
				int pointCount;
				iload->Read(&pointCount, sizeof(int), &nb);

				for (int pIdx=0; pIdx<pointCount; pIdx++)
					iload->Read(&m_pointInfo[pIdx]->binds[0]->bindCoord, sizeof(Point3), &nb);

				break;
			}
		}
		res = iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}

	return IO_OK;
}

void ToFace::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	int nodeCount = m_nodes.Count();
	if (!nodeCount) return;

	// Go through each of the nodes in the list of picked nodes, and create a ToFaceNode out of it
	// NOTE: some toFaceNodes may be NULL, in which case the given node couldn't convert itself into
	// something suitable for bind to use.
	Tab<ToFaceNode*> toFaceNodes;
	for (int ii=0; ii<nodeCount; ++ii)
	{
		ToFaceNode* toFaceNode = ToFaceNode::CreateToFaceNode(t, m_nodes[ii]);
		toFaceNodes.Append(1, &toFaceNode);
	}

	int numPoints =	(os->obj->NumPoints() < GetNumPoints()) ?
					os->obj->NumPoints() : GetNumPoints();

	Matrix3 worldToObject = Inverse(m_objectToWorld);
	float strength = m_pblock->GetFloat(pb_strength, t);

	// TODO: transform bind meshes into world space to save some matrix multiplies later?
	for (int pointIndex=0; pointIndex<numPoints; ++pointIndex)
	{
		if (
			os->obj->GetSubselState() == 0 ||	// entire object passed up stack or
			os->obj->PointSelection(pointIndex)	// point is selected
		) {
			Point3 Pw = os->obj->GetPoint(pointIndex) * m_objectToWorld;

			// offset collects the influences of each bind that is applied to this point, in world space
			Point3 offset(0.0f, 0.0f, 0.0f);

			int numBinds = GetNumBinds(pointIndex);
			for (int bindIndex=0; bindIndex<numBinds; ++bindIndex)
			{
				FaceBind* faceBind = m_pointInfo[pointIndex]->binds[bindIndex];

				ToFaceNode* toFaceNode = toFaceNodes[faceBind->nodeIndex];
				// toFaceNode might be NULL if the corresponding INode couldn't convert to a mesh
				if (toFaceNode)
				{
					Mesh* faceMesh = toFaceNode->mesh;
					int faceIndex = faceBind->faceIndex;

					if (!faceMesh || faceIndex >= faceMesh->numFaces) continue;

					// facePoint is the position on the bind mesh's face that we originally bound to, in object space
					Point3 facePoint =	faceMesh->verts[faceMesh->faces[faceIndex].v[0]] * (1.0f - faceBind->bindCoord[1] - faceBind->bindCoord[2]) +
										faceMesh->verts[faceMesh->faces[faceIndex].v[1]] * faceBind->bindCoord[1] +
										faceMesh->verts[faceMesh->faces[faceIndex].v[2]] * faceBind->bindCoord[2];

					// projectedPoint is facePoint pushed off the face by the distance the point was originally from the face when it was bound
					Point3 projectedPoint = facePoint + (faceBind->bindCoord[0] * faceMesh->getFaceNormal(faceIndex));

					// add in the weighted position of this bind
					offset += ((projectedPoint * toFaceNode->objectToWorld) - Pw) * faceBind->weight * strength;
				}
			}

			if (os->obj->GetSubselState() != 0)
			{
				Pw += offset * os->obj->PointSelection(pointIndex);
			} else {
				Pw += offset;
			}

			os->obj->SetPoint(pointIndex, (Pw * worldToObject));
		}
	}

	int toFaceNodeCount = toFaceNodes.Count();
	for (int ii=0; ii<toFaceNodeCount; ++ii)
	{
		if (toFaceNodes[ii])
		{
			delete toFaceNodes[ii];
			toFaceNodes[ii] = NULL;
		}
	}
	toFaceNodes.SetCount(0);

	os->obj->UpdateValidity(GEOM_CHAN_NUM, LocalValidity(t));
	os->obj->PointsWereChanged();
}

Class_ID ToFace::InputType()
{
	return defObjectClassID;
}

// ToFace /////////////////////////////////////////////////////////////////////////////////////////

TriObject* ToFace::GetTriObject(TimeValue t, INode* node, BOOL &needsDel)
{
	if (node)
	{
		ObjectState os = node->EvalWorldState(t);

		if (os.obj->IsSubClassOf(triObjectClassID))
		{
			needsDel = FALSE;
			TriObject* obj = (TriObject*)os.obj;

			return obj;
		} else {
			if (os.obj->CanConvertToType(triObjectClassID))
			{
				Object* oldObj = os.obj;
				TriObject* obj = (TriObject*)os.obj->ConvertToType(t,triObjectClassID);
				needsDel = (obj != oldObj);

				return obj;
			}
		}
	}
	return NULL;
}

void ToFace::UpdateUI()
{
	if (m_hWnd)
	{
		// Print num points and binds
		int i, nCount, pCount, bCount;
		TSTR str;
		HWND hTextWnd;

		nCount = m_nodes.Count();
		pCount = m_pointInfo.Count();
		bCount = 0;
		for (i=0; i<pCount; i++)
			bCount += m_pointInfo[i]->binds.Count();

		str.printf("%d", nCount);
		hTextWnd = GetDlgItem(m_hWnd,IDC_NUMNODES);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

		str.printf("%d", pCount);
		hTextWnd = GetDlgItem(m_hWnd,IDC_NUMPOINTS);
		SetWindowText(hTextWnd, str);
		str.Resize(0);

		str.printf("%d", bCount);
		hTextWnd = GetDlgItem(m_hWnd,IDC_NUMBINDS);
		SetWindowText(hTextWnd, str);

		// Update node list
		SendDlgItemMessage(m_hWnd, IDC_NODES, LB_RESETCONTENT, 0, 0);
        for (i=0; i<GetNumNodes(); i++)
        {
			SendDlgItemMessage(m_hWnd, IDC_NODES, LB_ADDSTRING, 0, (LPARAM)GetNode(i)->GetName());
        }
	}
}

void ToFace::Update()
{
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

BOOL ToFace::AddNode(INode* thisNode, INode* node)
{
	TimeValue t = GetCOREInterface()->GetTime();

	BOOL needsDel;
	TriObject* obj = GetTriObject(t, node, needsDel);
	if (!obj) return FALSE;
	if (needsDel) obj->DeleteThis();

	for (int i=0; i<m_nodes.Count(); i++)
	{
		if (m_nodes[i] == node)
			return FALSE;
	}

	if (!m_nodes.Count())
	{
		m_objectToWorld = thisNode->GetObjectTM(t);
	}

	m_nodes.Append(1, &node);

#if (MAX_RELEASE >= 9000)	//max 9
	SetReference((NUM_REFS + m_nodes.Count()-1), node);
#else	//max 8 and earlier
	MakeRefByID(FOREVER, (NUM_REFS + m_nodes.Count()-1), node);
#endif

	UpdateUI();

	return TRUE;
}

BOOL ToFace::RemoveNode(int i)
{
	if (i<0 || i>=m_nodes.Count())
		return FALSE;

	DeleteReference(NUM_REFS + i);
	m_nodes.Delete(i,1);

	// Remap all binds to use new node indexes
	for (int pIdx=0; pIdx<m_pointInfo.Count(); pIdx++)
	{
		for (int bIdx=(m_pointInfo[pIdx]->binds.Count()-1); bIdx>=0; bIdx--)
		{
			int ni = m_pointInfo[pIdx]->binds[bIdx]->nodeIndex;

			if (ni == i)
				UnBind(pIdx, bIdx);
			else if (ni > i)
			{
				m_pointInfo[pIdx]->binds[bIdx]->nodeIndex = (ni-1);
			}
		}
	}

	UpdateUI();

	return TRUE;
}

int ToFace::GetNumNodes()
{
	return m_nodes.Count();
}

INode* ToFace::GetNode(int i)
{
	if (i>=0 && i<m_nodes.Count())
		return m_nodes[i];
	else
		return NULL;
}

int ToFace::GetNumPoints()
{
	return m_pointInfo.Count();
}

void ToFace::SetNumPoints(int numBinds)
{
	if (numBinds > m_pointInfo.Count())
	{
		for (int i=m_pointInfo.Count(); i<numBinds; i++)
		{
			FacePoint* p = new FacePoint;
			m_pointInfo.Append(1, &p);
		}
	} else {
		for (int i=(m_pointInfo.Count()-1); i>=numBinds; i--)
			delete m_pointInfo[i];
		m_pointInfo.SetCount(numBinds);
	}
	UpdateUI();
}

BOOL ToFace::Bind(INode* thisNode, int pointIndex, int nodeIndex, int faceIndex, float weight)
{
	if (pointIndex < 0	||	pointIndex >= m_pointInfo.Count()	||
		nodeIndex < 0	||	nodeIndex >= m_nodes.Count()	||
		faceIndex < 0
	)
		return FALSE;

	Interface* ip = GetCOREInterface();
	if (!ip) return FALSE;
	TimeValue t = ip->GetTime();

	ObjectState thisOS = thisNode->EvalWorldState(t);
	if (pointIndex >= thisOS.obj->NumPoints()) return FALSE;

	INode* bindNode = m_nodes[nodeIndex];

	BOOL needsDel;
	TriObject* bindObj = GetTriObject(t, bindNode, needsDel);
	if (!bindObj) return FALSE;
	Mesh& bindMesh = bindObj->GetMesh();

	if (faceIndex > bindMesh.numFaces) {
		if (needsDel) bindObj->DeleteThis();
		return FALSE;
	}

	// transform this point into the bind node's coord sys
	Point3 p = thisOS.obj->GetPoint(pointIndex);
	p = p * thisNode->GetObjectTM(t) * Inverse(bindNode->GetObjectTM(t));

	// project the point onto the plane of the bind face
	Point3 a = bindMesh.verts[bindMesh.faces[faceIndex].v[0]];
	Point3 b = bindMesh.verts[bindMesh.faces[faceIndex].v[1]];
	Point3 c = bindMesh.verts[bindMesh.faces[faceIndex].v[2]];

	Point3 v1 = b - a;
	Point3 v2 = c - a;
	Point3 pn = -CrossProd(v1, v2);
	pn = Normalize(pn);

	Point3 pa = a - p;
	Point3 npa = Normalize(pa);

	float dp = DotProd(pn, npa);
	float dist = Length(pa) * dp;
	Point3 projP = p + (pn * dist);

	// get the barycentric coords of the projected point
	Point3 bc = bindMesh.BaryCoords(faceIndex, projP);
	if (bc == Point3(-1,1,1))
	{
		if (needsDel) bindObj->DeleteThis();
		return FALSE;
	}

	bc.x = dist;

	if (needsDel) bindObj->DeleteThis();

	BOOL res = m_pointInfo[pointIndex]->AddBind(bc, nodeIndex, faceIndex, weight);

	UpdateUI();

	return res;
}

BOOL ToFace::UnBind(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= m_pointInfo.Count()) return FALSE;

	BOOL res = m_pointInfo[pIdx]->RemoveBind(bIdx);

	UpdateUI();

	return res;
}

int ToFace::GetNumBinds(int pIdx)
{
	if (pIdx < 0 || pIdx >= m_pointInfo.Count()) return -1;

	int cnt = m_pointInfo[pIdx]->binds.Count();

	return cnt;
}

BOOL ToFace::GetBindInfo(int pIdx, int bIdx, int& nIdx, int& idx, float& weight)
{
	if (pIdx < 0 || pIdx >= m_pointInfo.Count() ||
		bIdx < 0 || bIdx >= m_pointInfo[pIdx]->binds.Count()) return FALSE;

	nIdx = m_pointInfo[pIdx]->binds[bIdx]->nodeIndex;
	idx = m_pointInfo[pIdx]->binds[bIdx]->faceIndex;
	weight = m_pointInfo[pIdx]->binds[bIdx]->weight;

	return TRUE;
}

float ToFace::GetBindWeight(int pIdx, int bIdx)
{
	if (pIdx < 0 || pIdx >= m_pointInfo.Count() ||
		bIdx < 0 || bIdx >= m_pointInfo[pIdx]->binds.Count()) return -1.0f;

	return m_pointInfo[pIdx]->binds[bIdx]->weight;
}

BOOL ToFace::SetBindWeight(int pIdx, int bIdx, float weight)
{
	if (pIdx < 0 || pIdx >= m_pointInfo.Count() ||
		bIdx < 0 || bIdx >= m_pointInfo[pIdx]->binds.Count()) return FALSE;

	m_pointInfo[pIdx]->binds[bIdx]->weight = weight;

	return TRUE;
}
