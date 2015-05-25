#include "BindPoints.h"

class Bind
{
	public:
		Bind() { }
};

class PointInfo
{
	public:
		Tab<Bind*> binds;

		PointInfo() {
			binds.ZeroCount();
		}
		~PointInfo() {
			for (int i=0; i<binds.Count(); i++) delete binds[i];
		}

		BOOL RemoveBind(int bindIndex)
		{
			if (bindIndex<0 || bindIndex>=binds.Count())
				return FALSE;

			binds.Delete(bindIndex, 1);

			return TRUE;
		}
};
