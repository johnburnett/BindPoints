#include "BindPoints.h"

class FaceBind
{
	public:
		Point3 bindCoord;
		int nodeIndex;
		int faceIndex;
		float weight;

		FaceBind() :
			bindCoord(0.0f, 0.0f, 0.0f),
			nodeIndex(0),
			faceIndex(0),
			weight(0.0f)
		{ }
		FaceBind(Point3 bc, int ni, int fi, float w) :
			bindCoord(bc),
			nodeIndex(ni),
			faceIndex(fi),
			weight(w)
		{ }
};

class FacePoint
{
	public:
		Tab<FaceBind*> binds;

		FacePoint() { binds.ZeroCount(); }
		~FacePoint() { for (int i=0; i<binds.Count(); i++) delete binds[i]; }

		BOOL AddBind(Point3 bindCoord, int nodeIndex, int faceIndex, float weight)
		{
			for (int i=0; i<binds.Count(); i++)
			{
				if (binds[i]->nodeIndex == nodeIndex &&
					binds[i]->faceIndex == faceIndex)
					return FALSE;
			}

			FaceBind* b = new FaceBind(bindCoord, nodeIndex, faceIndex, weight);
			binds.Append(1, &b);

			return TRUE;
		}

		BOOL RemoveBind(int bindIndex)
		{
			if (bindIndex<0 || bindIndex>=binds.Count())
				return FALSE;

			binds.Delete(bindIndex, 1);

			return TRUE;
		}
};
