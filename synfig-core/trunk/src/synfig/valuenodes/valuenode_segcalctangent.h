/* === S Y N F I G ========================================================= */
/*!	\file valuenode_segcalctangent.h
**	\brief Header file for implementation of the "Segment Tangent" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_SEGCALCTANGENT_H
#define __SYNFIG_VALUENODE_SEGCALCTANGENT_H

/* === H E A D E R S ======================================================= */

#include "../valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_SegCalcTangent : public LinkableValueNode
{
	ValueNode::RHandle segment_;
	ValueNode::RHandle amount_;

	ValueNode_SegCalcTangent(const ValueBase::Type &x=ValueBase::TYPE_VECTOR);

public:

	typedef etl::handle<ValueNode_SegCalcTangent> Handle;
	typedef etl::handle<const ValueNode_SegCalcTangent> ConstHandle;

	//static Handle create(const ValueBase::Type &x=ValueBase::TYPE_VECTOR);


	virtual ValueBase operator()(Synfig_Time t)const;

	virtual ~ValueNode_SegCalcTangent();

	virtual String get_name()const;
	virtual String get_local_name()const;


	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual int link_count()const;
	virtual String link_name(int i)const;

	virtual String link_local_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

protected:
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	LinkableValueNode* create_new()const;

public:
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(ValueBase::Type type);
	static ValueNode_SegCalcTangent* create(const ValueBase &x=ValueBase::TYPE_VECTOR);
}; // END of class ValueNode_SegCalcTangent

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
