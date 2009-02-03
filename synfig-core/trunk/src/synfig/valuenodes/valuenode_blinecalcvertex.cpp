/* === S Y N F I G ========================================================= */
/*!	\file valuenode_blinecalcvertex.cpp
**	\brief Implementation of the "BLine Vertex" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode_blinecalcvertex.h"
#include "valuenode_bline.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "../general.h"
#include "../exception.h"
#include <ETL/hermite>

#endif


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BLineCalcVertex::ValueNode_BLineCalcVertex(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	if(x!=ValueBase::TYPE_VECTOR)
		throw Exception::BadType(ValueBase::type_local_name(x));

	ValueNode_BLine* value_node(new ValueNode_BLine());
	set_link("bline",value_node);
	set_link("loop",ValueNode_Const::create(bool(false)));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
}

LinkableValueNode*
ValueNode_BLineCalcVertex::create_new()const
{
	return new ValueNode_BLineCalcVertex(ValueBase::TYPE_VECTOR);
}

ValueNode_BLineCalcVertex*
ValueNode_BLineCalcVertex::create(const ValueBase &x)
{
	return new ValueNode_BLineCalcVertex(x.get_type());
}

ValueNode_BLineCalcVertex::~ValueNode_BLineCalcVertex()
{
	unlink_all();
}

ValueBase
ValueNode_BLineCalcVertex::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	const std::vector<ValueBase> bline((*bline_)(t).get_list());
	handle<ValueNode_BLine> bline_value_node(bline_);
	const bool looped(bline_value_node->get_loop());
	int size = bline.size(), from_vertex;
	bool loop((*loop_)(t).get(bool()));
	Real amount((*amount_)(t).get(Real()));
	BLinePoint blinepoint0, blinepoint1;

	if (!looped) size--;
	if (size < 1) return Vector();
	if (loop)
	{
		amount = amount - int(amount);
		if (amount < 0) amount++;
	}
	else
	{
		if (amount < 0) amount = 0;
		if (amount > 1) amount = 1;
	}

	vector<ValueBase>::const_iterator iter, next(bline.begin());

	iter = looped ? --bline.end() : next++;
	amount = amount * size;
	from_vertex = int(amount);
	if (from_vertex > size-1) from_vertex = size-1;
	blinepoint0 = from_vertex ? *(next+from_vertex-1) : *iter;
	blinepoint1 = *(next+from_vertex);

	etl::hermite<Vector> curve(blinepoint0.get_vertex(),   blinepoint1.get_vertex(),
							   blinepoint0.get_tangent2(), blinepoint1.get_tangent1());
	return curve(amount-from_vertex);
}







String
ValueNode_BLineCalcVertex::get_name()const
{
	return "blinecalcvertex";
}

String
ValueNode_BLineCalcVertex::get_local_name()const
{
	return _("BLine Vertex");
}

bool
ValueNode_BLineCalcVertex::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bline_,  ValueBase::TYPE_LIST);
	case 1: CHECK_TYPE_AND_SET_VALUE(loop_,   ValueBase::TYPE_BOOL);
	case 2: CHECK_TYPE_AND_SET_VALUE(amount_, ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BLineCalcVertex::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return bline_;
		case 1: return loop_;
		case 2: return amount_;
	}

	return 0;
}

int
ValueNode_BLineCalcVertex::link_count()const
{
	return 3;
}

String
ValueNode_BLineCalcVertex::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "bline";
		case 1: return "loop";
		case 2: return "amount";
	}
	return String();
}

String
ValueNode_BLineCalcVertex::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("BLine");
		case 1: return _("Loop");
		case 2: return _("Amount");
	}
	return String();
}

int
ValueNode_BLineCalcVertex::get_link_index_from_name(const String &name)const
{
	if(name=="bline")  return 0;
	if(name=="loop")   return 1;
	if(name=="amount") return 2;
	throw Exception::BadLinkName(name);
}

bool
ValueNode_BLineCalcVertex::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}
