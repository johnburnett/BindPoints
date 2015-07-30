/**********************************************************************
 *<
	FILE: BindPoints.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __BINDPOINTS__H
#define __BINDPOINTS__H

#include "Max.h"

#if MAX_VERSION_MAJOR < 9	//Max 9
	#include "max_mem.h"	//max 8 and earlier
#else
	#include "maxheapdirect.h"
#endif

#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#define NUM_REFS	1
#define PBLOCK_REF	0

#define CURRENT_VERSION		4

enum {
	bind_params,
};

enum {
	pb_strength,
};

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __BINDPOINTS__H
