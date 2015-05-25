#include "BindPoints.h"

class ShapeBind
{
	public:
		Point3 basePos;
		Point3 baseTan;
		float lengthParam;
		float weight;
		int nodeIndex;
		int splineIndex;
		BOOL absolute;

		ShapeBind() :
			basePos(0.0f, 0.0f, 0.0f),
			baseTan(0.0f, 0.0f, 0.0f),
			nodeIndex(0),
			splineIndex(0),
			lengthParam(0.0f),
			weight(0.0f),
			absolute(FALSE)
		{ }
		ShapeBind(Point3 pos, Point3 btan, int shpIdx, int splIdx, float lp, float w, BOOL a) :
			basePos(pos),
			baseTan(btan),
			nodeIndex(shpIdx),
			splineIndex(splIdx),
			lengthParam(lp),
			weight(w),
			absolute(a)
		{ }
};

class ShapePoint
{
	public:
		Tab<ShapeBind*> binds;

		ShapePoint() { binds.ZeroCount(); }
		~ShapePoint() { for (int i=0; i<binds.Count(); i++) delete binds[i]; }

		BOOL AddBind(Point3 basePos, Point3 baseTan, int nodeIndex, int splineIndex, float lengthParam, float weight, BOOL absolute)
		{
			for (int i=0; i<binds.Count(); i++)
			{
				if (binds[i]->nodeIndex == nodeIndex &&
					binds[i]->splineIndex == splineIndex)
					return FALSE;
			}

			ShapeBind* b = new ShapeBind(basePos, baseTan, nodeIndex, splineIndex, lengthParam, weight, absolute);
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
