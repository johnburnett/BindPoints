#include "BindPoints.h"

class NodeBind
{
	public:
		Point3 basePos;
		int nodeIndex;
		float weight;

		NodeBind() :
			basePos(0.0f, 0.0f, 0.0f),
			nodeIndex(0),
			weight(0.0f)
		{ }
		NodeBind(Point3 pos, int ni, float w) :
			basePos(pos),
			nodeIndex(ni),
			weight(w)
		{ }
};

class NodePoint
{
	public:
		Tab<NodeBind*> binds;

		NodePoint() { binds.ZeroCount(); }
		~NodePoint() { for (int i=0; i<binds.Count(); i++) delete binds[i]; }

		BOOL AddBind(Point3 basePos, int nodeIndex, float weight)
		{
			for (int i=0; i<binds.Count(); i++)
			{
				if (binds[i]->nodeIndex == nodeIndex)
					return FALSE;
			}

			NodeBind* b = new NodeBind(basePos, nodeIndex, weight);
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
