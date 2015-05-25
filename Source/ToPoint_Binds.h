#include "BindPoints.h"

class PointBind
{
	public:
		Point3 basePos;
		float weight;
		int nodeIndex;
		int pointIndex;

		PointBind() :
			basePos(0.0f, 0.0f, 0.0f),
			nodeIndex(0),
			pointIndex(0),
			weight(0.0f)
		{ }
		PointBind(Point3 pos, int nIdx, int pIdx, float w) :
			basePos(pos),
			nodeIndex(nIdx),
			pointIndex(pIdx),
			weight(w)
		{ }
};

class PointPoint
{
	public:
		Tab<PointBind*> binds;

		PointPoint() { binds.ZeroCount(); }
		~PointPoint() { for (int i=0; i<binds.Count(); i++) delete binds[i]; }

		BOOL AddBind(Point3 basePos, int nodeIndex, int pointIndex, float weight)
		{
			for (int i=0; i<binds.Count(); i++)
			{
				if (binds[i]->nodeIndex == nodeIndex &&
					binds[i]->pointIndex == pointIndex)
					return FALSE;
			}

			PointBind* b = new PointBind(basePos, nodeIndex, pointIndex, weight);
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
