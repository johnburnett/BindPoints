#if MAX_VERSION_MAJOR < 14	//Max2012
	#include "maxscrpt\Maxscrpt.h"
	#include "maxscrpt\Strings.h"
	#include "maxscrpt\arrays.h"
	#include "maxscrpt\3DMath.h"
	#include "maxscrpt\Numbers.h"
	#include "maxscrpt\MAXclses.h"
	#include "maxscrpt\definsfn.h"
#else
	#include "maxscript\maxscript.h"
	#include "maxscript\foundation\strings.h"
	#include "maxscript\foundation\arrays.h"
	#include "maxscript\foundation\3dmath.h"
	#include "maxscript\foundation\numbers.h"
	#include "maxscript\maxwrapper\maxclasses.h"
	#include "maxscript\macros\define_instantiation_functions.h"
#endif

#include "ToNode.h"
#include "ToShape.h"
#include "ToFace.h"
#include "ToPoint.h"

#define get_bind_mod()							\
	Modifier *mod = arg_list[0]->to_modifier();	\
	Class_ID id = mod->ClassID();				\
	if (id != TOSHAPE_CLASSID	&&				\
		id != TONODE_CLASSID	&&				\
		id != TOFACE_CLASSID	&&				\
		id != TOPOINT_CLASSID )					\
		throw RuntimeError(GetString(IDS_NOTBINDERROR), arg_list[0]);

#if MAX_VERSION_MAJOR < 17 //Max 2015
#define get_valid_node(_checknode, _node, _fn)								\
	nv = _checknode;														\
	if (is_node(nv))														\
	{																		\
		if(nv->ref_deleted)													\
			throw RuntimeError("Attempted to access to deleted object");	\
	}																		\
	else																	\
	{																		\
		throw RuntimeError (#_fn##" requires a node");						\
	}																		\
	_node = nv->to_node()
#else
#define get_valid_node(_checknode, _node, _fn)								\
	nv = _checknode;														\
	if (is_node(nv))														\
	{																		\
		if(nv->ref_deleted)													\
			throw RuntimeError(_T("Attempted to access to deleted object"));\
	}																		\
	else																	\
	{																		\
		throw RuntimeError (_T(#_fn##_T(" requires a node")));				\
	}																		\
	_node = nv->to_node()
#endif
	
def_struct_primitive( bindOps_update,			bindOps,	"Update");
def_struct_primitive( bindOps_addNode,			bindOps,	"AddNode");
def_struct_primitive( bindOps_removeNode,		bindOps,	"RemoveNode");
def_struct_primitive( bindOps_getNumNodes,		bindOps,	"GetNumNodes");
def_struct_primitive( bindOps_getNode,			bindOps,	"GetNode");
def_struct_primitive( bindOps_getNumPoints,		bindOps,	"GetNumPoints");
def_struct_primitive( bindOps_setNumPoints,		bindOps,	"SetNumPoints");
def_struct_primitive( bindOps_bind,				bindOps,	"Bind");
def_struct_primitive( bindOps_unbind,			bindOps,	"UnBind");
def_struct_primitive( bindOps_getNumBinds,		bindOps,	"GetNumBinds");
def_struct_primitive( bindOps_getBindInfo,		bindOps,	"GetBindInfo");
def_struct_primitive( bindOps_getBindWeight,	bindOps,	"GetBindWeight");
def_struct_primitive( bindOps_setBindWeight,	bindOps,	"SetBindWeight");

Value* bindOps_update_cf(Value** arg_list, int count)
{
	check_arg_count(update, 1, count);

	get_bind_mod();

	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		m->Update();
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		m->Update();
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		m->Update();
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		m->Update();
	}

	return &ok;
}

Value* bindOps_addNode_cf(Value** arg_list, int count)
{
	check_arg_count(addNode, 3, count);

	get_bind_mod();

	MAXNode* nv;
	INode* thisNode;
	get_valid_node((MAXNode*)arg_list[1], thisNode, addNode);
	INode* bindNode;
	get_valid_node((MAXNode*)arg_list[2], bindNode, addNode);

	BOOL res = FALSE;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		res = m->AddNode(thisNode, bindNode);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		res = m->AddNode(thisNode, bindNode);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		res = m->AddNode(thisNode, bindNode);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		res = m->AddNode(thisNode, bindNode);
	}

	if (res)
	{
//		MAXScript_interface->FlushUndoBuffer();
		return &true_value;
	} else
		return &false_value;
}

Value* bindOps_removeNode_cf(Value** arg_list, int count)
{
	check_arg_count(removeNode, 2, count);

	get_bind_mod();

	int i = (arg_list[1]->to_int()) - 1;

	BOOL res = FALSE;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		res = m->RemoveNode(i);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		res = m->RemoveNode(i);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		res = m->RemoveNode(i);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		res = m->RemoveNode(i);
	}

	if (res)
	{
//		MAXScript_interface->FlushUndoBuffer();
		return &true_value;
	} else
		return &false_value;
}

Value* bindOps_getNumNodes_cf(Value** arg_list, int count)
{
	check_arg_count(getNumNodes, 1, count);

	get_bind_mod();

	int cnt = 0;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		cnt = m->GetNumNodes();
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		cnt = m->GetNumNodes();
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		cnt = m->GetNumNodes();
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		cnt = m->GetNumNodes();
	}

	return Integer::intern(cnt);
}

Value* bindOps_getNode_cf(Value** arg_list, int count)
{
	check_arg_count(getNode, 2, count);

	get_bind_mod();

	int i = (arg_list[1]->to_int()) - 1;

	INode* node = NULL;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		node = m->GetNode(i);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		node = m->GetNode(i);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		node = m->GetNode(i);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		node = m->GetNode(i);
	}

	if (node)
		return MAXNode::intern(node);
	else
		return &undefined;
}

Value* bindOps_getNumPoints_cf(Value** arg_list, int count)
{
	check_arg_count(getNumPoints, 1, count);

	get_bind_mod();

	int cnt = 0;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		cnt = m->GetNumPoints();
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		cnt = m->GetNumPoints();
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		cnt = m->GetNumPoints();
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		cnt = m->GetNumPoints();
	}

	return Integer::intern(cnt);
}

Value* bindOps_setNumPoints_cf(Value** arg_list, int count)
{
	check_arg_count(setNumVerts, 2, count);

	get_bind_mod();

	int i = arg_list[1]->to_int();

	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		m->SetNumPoints(i);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		m->SetNumPoints(i);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		m->SetNumPoints(i);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		m->SetNumPoints(i);
	}

	return &ok;
}

#if MAX_MAJOR_VERSION < 9	//Max 9
#define check_arg_count(fn, w, g)	if ((w) != (g)) throw ArgCountError (_T(#fn), w, g)
#endif

Value* bindOps_bind_cf(Value** arg_list, int count)
{
	if (count < 1) throw ArgCountError (_T("bind"), 1, 0);

	get_bind_mod();

	BOOL res = FALSE;
	if (id == TOSHAPE_CLASSID) {
		check_arg_count(bind, 8, count);

// not really using thisNode, but include in args for consistency
//		MAXNode* nv;
//		INode* thisNode;
//		get_valid_node((MAXNode*)arg_list[1], thisNode, bind);

		int		pointIndex	= (arg_list[2]->to_int()) - 1;
		int		nodeIndex	= (arg_list[3]->to_int()) - 1;
		int		splineIndex	= (arg_list[4]->to_int()) - 1;
		float	lengthParam	= arg_list[5]->to_float();
		BOOL	absolute	= arg_list[6]->to_bool();
		float	weight		= arg_list[7]->to_float();

		ToShape* m = (ToShape*)mod;
		res = m->Bind(pointIndex, nodeIndex, splineIndex, lengthParam, weight, absolute);
	} else
	if (id == TONODE_CLASSID) {
		check_arg_count(bind, 5, count);

		MAXNode* nv;
		INode* thisNode;
		get_valid_node((MAXNode*)arg_list[1], thisNode, bind);

		int		pointIndex	= (arg_list[2]->to_int()) - 1;
		int		nodeIndex	= (arg_list[3]->to_int()) - 1;
		float	weight		= arg_list[4]->to_float();

		ToNode* m = (ToNode*)mod;
		res = m->Bind(thisNode, pointIndex, nodeIndex, weight);
	} else
	if (id == TOFACE_CLASSID) {
		check_arg_count(bind, 6, count);

		MAXNode* nv;
		INode* thisNode;
		get_valid_node((MAXNode*)arg_list[1], thisNode, bind);

		int		pointIndex	= (arg_list[2]->to_int()) - 1;
		int		nodeIndex	= (arg_list[3]->to_int()) - 1;
		int		faceIndex	= (arg_list[4]->to_int()) - 1;
		float	weight		= arg_list[5]->to_float();

		ToFace* m = (ToFace*)mod;
		res = m->Bind(thisNode, pointIndex, nodeIndex, faceIndex, weight);
	} else
	if (id == TOPOINT_CLASSID) {
		check_arg_count(bind, 6, count);

// not really using thisNode, but include in args for consistency
//		MAXNode* nv;
//		INode* thisNode;
//		get_valid_node((MAXNode*)arg_list[1], thisNode, bind);

		int		thisIndex	= (arg_list[2]->to_int()) - 1;
		int		nodeIndex	= (arg_list[3]->to_int()) - 1;
		int		pointIndex	= (arg_list[4]->to_int()) - 1;
		float	weight		= arg_list[5]->to_float();

		ToPoint* m = (ToPoint*)mod;
		res = m->Bind(thisIndex, nodeIndex, pointIndex, weight);
	}

	return (res ? &true_value : &false_value);
}

Value* bindOps_unbind_cf(Value** arg_list, int count)
{
	check_arg_count(unbind, 3, count);

	get_bind_mod();

	int pIdx = (arg_list[1]->to_int()) - 1;
	int bIdx = (arg_list[2]->to_int()) - 1;

	BOOL res = FALSE;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		res = m->UnBind(pIdx, bIdx);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		res = m->UnBind(pIdx, bIdx);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		res = m->UnBind(pIdx, bIdx);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		res = m->UnBind(pIdx, bIdx);
	}

	return (res ? &true_value : &false_value);
}

Value* bindOps_getNumBinds_cf(Value** arg_list, int count)
{
	check_arg_count(getNumBinds, 2, count);

	get_bind_mod();

	int pIdx = arg_list[1]->to_int() - 1;

	int cnt = 0;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		cnt = m->GetNumBinds(pIdx);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		cnt = m->GetNumBinds(pIdx);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		cnt = m->GetNumBinds(pIdx);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		cnt = m->GetNumBinds(pIdx);
	}

	if (cnt < 0)
		return &undefined;
	else
		return Integer::intern(cnt);
}

Value* bindOps_getBindInfo_cf(Value** arg_list, int count)
{
	check_arg_count(getBindInfo, 3, count);

	get_bind_mod();

	int pIdx = arg_list[1]->to_int() - 1;
	int bIdx = arg_list[2]->to_int() - 1;

	int nIdx;
	int idx;
	float weight;

	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		float lenParam;
		BOOL absolute;

		if (m->GetBindInfo(pIdx, bIdx, nIdx, idx, lenParam, weight, absolute))
		{
			init_thread_locals();
			push_alloc_frame();
			one_typed_value_local(Array* res);

			vl.res = new Array(5);

			vl.res->append((Integer::intern(nIdx+1)));
			vl.res->append((Integer::intern(idx+1)));
			vl.res->append((Float::intern(lenParam)));
			vl.res->append(absolute ? &true_value : &false_value);
			vl.res->append((Float::intern(weight)));

			pop_alloc_frame();
			return_value(vl.res);
		}
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;

		if (m->GetBindInfo(pIdx, bIdx, nIdx, weight))
		{
			init_thread_locals();
			push_alloc_frame();
			one_typed_value_local(Array* res);

			vl.res = new Array(2);

			vl.res->append((Integer::intern(nIdx+1)));
			vl.res->append((Float::intern(weight)));

			pop_alloc_frame();
			return_value(vl.res);
		}
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;

		if (m->GetBindInfo(pIdx, bIdx, nIdx, idx, weight))
		{
			init_thread_locals();
			push_alloc_frame();
			one_typed_value_local(Array* res);

			vl.res = new Array(3);

			vl.res->append((Integer::intern(nIdx+1)));
			vl.res->append((Integer::intern(idx+1)));
			vl.res->append((Float::intern(weight)));

			pop_alloc_frame();
			return_value(vl.res);
		}
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;

		if (m->GetBindInfo(pIdx, bIdx, nIdx, idx, weight))
		{
			init_thread_locals();
			push_alloc_frame();
			one_typed_value_local(Array* res);

			vl.res = new Array(3);

			vl.res->append((Integer::intern(nIdx+1)));
			vl.res->append((Integer::intern(idx+1)));
			vl.res->append((Float::intern(weight)));

			pop_alloc_frame();
			return_value(vl.res);
		}
	}

	return &undefined;
}

Value* bindOps_getBindWeight_cf(Value** arg_list, int count)
{
	check_arg_count(getBindWeight, 3, count);

	get_bind_mod();

	int pIdx = arg_list[1]->to_int() - 1;
	int bIdx = arg_list[2]->to_int() - 1;

	float w = -1.0f;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		w = m->GetBindWeight(pIdx, bIdx);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		w = m->GetBindWeight(pIdx, bIdx);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		w = m->GetBindWeight(pIdx, bIdx);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		w = m->GetBindWeight(pIdx, bIdx);
	}

	if (w < 0.0f)
		return &undefined;
	else
		return Float::intern(w);
}

Value* bindOps_setBindWeight_cf(Value** arg_list, int count)
{
	check_arg_count(bind, 4, count);

	get_bind_mod();

	int pIdx = (arg_list[1]->to_int()) - 1;
	int bIdx = (arg_list[2]->to_int()) - 1;
	float weight = arg_list[3]->to_float();

	BOOL res = FALSE;
	if (id == TOSHAPE_CLASSID) {
		ToShape* m = (ToShape*)mod;
		res = m->SetBindWeight(pIdx, bIdx, weight);
	} else
	if (id == TONODE_CLASSID) {
		ToNode* m = (ToNode*)mod;
		res = m->SetBindWeight(pIdx, bIdx, weight);
	} else
	if (id == TOFACE_CLASSID) {
		ToFace* m = (ToFace*)mod;
		res = m->SetBindWeight(pIdx, bIdx, weight);
	} else
	if (id == TOPOINT_CLASSID) {
		ToPoint* m = (ToPoint*)mod;
		res = m->SetBindWeight(pIdx, bIdx, weight);
	}

	return (res ? &true_value : &false_value);
}
