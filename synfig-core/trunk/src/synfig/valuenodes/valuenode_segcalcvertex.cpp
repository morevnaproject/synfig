/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalcvertex.cpp
**	\brief Implementation of the "Segment Vertex" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_segcalcvertex.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "../general.h"
#include "../exception.h"
#include <ETL/hermite>
#include "../segment.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_SegCalcVertex::ValueNode_SegCalcVertex(const ValueBase::Type &x):
	LinkableValueNode(x)
{
	if(x!=ValueBase::TYPE_VECTOR)
		throw Exception::BadType(ValueBase::type_local_name(x));

	set_link("segment",ValueNode_Const::create(ValueBase::TYPE_SEGMENT));
	set_link("amount",ValueNode_Const::create(Real(0.5)));
}

ValueNode_SegCalcVertex*
ValueNode_SegCalcVertex::create(const ValueBase &x)
{
	return new ValueNode_SegCalcVertex(x.get_type());
}

ValueNode_SegCalcVertex::~ValueNode_SegCalcVertex()
{
	unlink_all();
}

ValueBase
ValueNode_SegCalcVertex::operator()(Synfig_Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Segment segment((*segment_)(t).get(Segment()));

	etl::hermite<Vector> curve(segment.p1,segment.p2,segment.t1,segment.t2);

	return curve((*amount_)(t).get(Real()));
}


String
ValueNode_SegCalcVertex::get_name()const
{
	return "segcalcvertex";
}

String
ValueNode_SegCalcVertex::get_local_name()const
{
	return _("Segment Vertex");
}

bool
ValueNode_SegCalcVertex::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_VECTOR;
}

bool
ValueNode_SegCalcVertex::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(segment_, ValueBase::TYPE_SEGMENT);
	case 1: CHECK_TYPE_AND_SET_VALUE(amount_,  ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_SegCalcVertex::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return segment_;
	if(i==1)
		return amount_;

	return 0;
}

int
ValueNode_SegCalcVertex::link_count()const
{
	return 2;
}

String
ValueNode_SegCalcVertex::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return "segment";
	if(i==1)
		return "amount";
	return String();
}

String
ValueNode_SegCalcVertex::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return _("Segment");
	if(i==1)
		return _("Amount");
	return String();
}

int
ValueNode_SegCalcVertex::get_link_index_from_name(const String &name)const
{
	if(name=="segment")
		return 0;
	if(name=="amount")
		return 1;

	throw Exception::BadLinkName(name);
}

LinkableValueNode*
ValueNode_SegCalcVertex::create_new()const
{
	return new ValueNode_SegCalcVertex(ValueBase::TYPE_VECTOR);
}
