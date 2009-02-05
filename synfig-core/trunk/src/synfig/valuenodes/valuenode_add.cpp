/* === S Y N F I G ========================================================= */
/*!	\file valuenode_add.cpp
**	\brief Implementation of the "Add" valuenode conversion.
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

#include "../general.h"
#include "valuenode_add.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "../color.h"
#include "../gradient.h"
#include "../vector.h"
#include "../angle.h"
#include "../real.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Add::ValueNode_Add(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	set_link("scalar",ValueNode_Const::create(Real(1.0)));
	ValueBase::Type id(value.get_type());

	switch(id)
	{
	case ValueBase::TYPE_ANGLE:
		set_link("lhs",ValueNode_Const::create(value.get(Angle())));
		set_link("rhs",ValueNode_Const::create(Angle::deg(0)));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("lhs",ValueNode_Const::create(value.get(Color())));
		set_link("rhs",ValueNode_Const::create(Color(0,0,0,0)));
		break;
	case ValueBase::TYPE_GRADIENT:
		set_link("lhs",ValueNode_Const::create(value.get(Gradient())));
		set_link("rhs",ValueNode_Const::create(Gradient()));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("lhs",ValueNode_Const::create(value.get(int())));
		set_link("rhs",ValueNode_Const::create(int(0)));
		break;
	case ValueBase::TYPE_REAL:
		set_link("lhs",ValueNode_Const::create(value.get(Real())));
		set_link("rhs",ValueNode_Const::create(Real(0)));
		break;
	case ValueBase::TYPE_TIME:
		set_link("lhs",ValueNode_Const::create(value.get(Synfig_Time())));
		set_link("rhs",ValueNode_Const::create(Synfig_Time(0)));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("lhs",ValueNode_Const::create(value.get(Vector())));
		set_link("rhs",ValueNode_Const::create(Vector(0,0)));
		break;
	default:
		assert(0);
		throw runtime_error(get_local_name()+_(":Bad type ")+ValueBase::type_local_name(id));
	}
}

LinkableValueNode*
ValueNode_Add::create_new()const
{
	return new ValueNode_Add(get_type());
}

ValueNode_Add*
ValueNode_Add::create(const ValueBase& value)
{
	return new ValueNode_Add(value);
}

synfig::ValueNode_Add::~ValueNode_Add()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Add::operator()(Synfig_Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!ref_a || !ref_b)
		throw runtime_error(strprintf("ValueNode_Add: %s",_("One or both of my parameters aren't set!")));
	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return ((*ref_a)(t).get(Angle())+(*ref_b)(t).get(Angle()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_COLOR:
		return ((*ref_a)(t).get(Color())+(*ref_b)(t).get(Color()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_GRADIENT:
		return ((*ref_a)(t).get(Gradient())+(*ref_b)(t).get(Gradient()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_INTEGER:
		return round_to_int(((*ref_a)(t).get(int())+(*ref_b)(t).get(int()))*(*scalar)(t).get(Real()));
	case ValueBase::TYPE_REAL:
		return ((*ref_a)(t).get(Vector::value_type())+(*ref_b)(t).get(Vector::value_type()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_TIME:
		return ((*ref_a)(t).get(Synfig_Time())+(*ref_b)(t).get(Synfig_Time()))*(*scalar)(t).get(Real());
	case ValueBase::TYPE_VECTOR:
		return ((*ref_a)(t).get(Vector())+(*ref_b)(t).get(Vector()))*(*scalar)(t).get(Real());
	default:
		assert(0);
		break;
	}
	return ValueBase();
}

bool
ValueNode_Add::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(ref_a,  get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(ref_b,  get_type());
	case 2: CHECK_TYPE_AND_SET_VALUE(scalar, ValueBase::TYPE_REAL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Add::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return ref_a;
		case 1: return ref_b;
		case 2: return scalar;
		default: return 0;
	}
}

int
ValueNode_Add::link_count()const
{
	return 3;
}

String
ValueNode_Add::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return _("LHS");
		case 1: return _("RHS");
		case 2: return _("Scalar");
		default: return String();
	}
}

String
ValueNode_Add::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return "lhs";
		case 1: return "rhs";
		case 2: return "scalar";
		default: return String();
	}
}

int
ValueNode_Add::get_link_index_from_name(const String &name)const
{
	if(name=="lhs") return 0;
	if(name=="rhs") return 1;
	if(name=="scalar") return 2;
	throw Exception::BadLinkName(name);
}

String
ValueNode_Add::get_name()const
{
	return "add";
}

String
ValueNode_Add::get_local_name()const
{
	return _("Add");
}

bool
ValueNode_Add::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_ANGLE
		|| type==ValueBase::TYPE_COLOR
		|| type==ValueBase::TYPE_GRADIENT
		|| type==ValueBase::TYPE_INTEGER
		|| type==ValueBase::TYPE_REAL
		|| type==ValueBase::TYPE_TIME
		|| type==ValueBase::TYPE_VECTOR;
}
