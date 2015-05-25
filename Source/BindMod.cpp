/**********************************************************************
 *<
	FILE: BindMod.cpp

	DESCRIPTION:	Bind modifiers base class

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "BindMod.h"

HWND BindMod::hWnd = NULL;

BindMod::BindMod()
{
	thisTM.IdentityMatrix();
	nodes.ZeroCount();
	pointInfo.ZeroCount();
}

ToPoint::~ToPoint()
{
	for (int i=0; i<pointInfo.Count(); i++)
		delete pointInfo[i];

	DeleteAllRefsFromMe();
}

void ToPoint::UpdateUI()
{
	if (hWnd)
	{
		// Print num points and binds
		int i, pCount, bCount;
		TSTR str;
		HWND hTextWnd;

		pCount = pointInfo.Count();
		bCount = 0;
		for (i=0; i<pCount; i++)
			bCount += pointInfo[i]->binds.Count();

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

int ToPoint::NumRefs()
{
	return nodes.Count();
}

RefTargetHandle ToPoint::GetReference(int i)
{
	return nodes[i];
}

void ToPoint::SetReference(int i, RefTargetHandle rtarg)
{
	nodes[i] = (INode*)rtarg;
}

RefResult ToPoint::NotifyRefChanged(
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

RefTargetHandle ToPoint::Clone(RemapDir& remap)
{
	ToPoint* newmod = new ToPoint();

	newmod->thisTM = thisTM;
	newmod->nodes.SetCount(nodes.Count());
	for (int i=0; i < nodes.Count(); i++)
	{
		newmod->nodes[i] = NULL;
		newmod->ReplaceReference(i, nodes[i]);
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

#define CURRENT_VERSION		3
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

	int ver = CURRENT_VERSION;
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
				int ver;
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

//////////////////////////////////////////////////////////////////////////////////////////////

void BindMod::Update()
{
	NotifyDependents
}

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
	MakeRefByID(FOREVER, (nodes.Count()-1), node);

	UpdateUI();

	return TRUE;
}

BOOL ToPoint::RemoveNode(int i)
{
	if (i<0 || i>=nodes.Count())
		return FALSE;

	DeleteReference(i);
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
