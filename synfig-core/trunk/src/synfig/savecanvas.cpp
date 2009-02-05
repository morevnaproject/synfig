/* === S Y N F I G ========================================================= */
/*!	\file savecanvas.cpp
**	\brief Writeme
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

#ifdef HAVE_SYS_ERRNO_H
#	include <sys/errno.h>
#endif

#include "savecanvas.h"
#include "general.h"
#include "valuenode.h"
#include "valuenodes/valuenode_subtract.h"
#include "valuenodes/valuenode_animated.h"
#include "valuenodes/valuenode_composite.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_linear.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "valuenodes/valuenode_reference.h"
#include "valuenodes/valuenode_segcalctangent.h"
#include "valuenodes/valuenode_segcalcvertex.h"
#include "valuenodes/valuenode_bline.h"
#include "synfig_time.h"
#include "keyframe.h"
#include "layer.h"
#include "synfig_string.h"
#include "paramdesc.h"

#include <libxml++/libxml++.h>
#include <ETL/stringf>
#include "gradient.h"
#include <errno.h>

extern "C" {
#include <libxml/tree.h>
}

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define COLOR_VALUE_TYPE_FORMAT		"%f"
#define	VECTOR_VALUE_TYPE_FORMAT	"%0.10f"
#define	TIME_TYPE_FORMAT			"%0.3f"
#define	VIEW_BOX_FORMAT				"%f %f %f %f"

/* === G L O B A L S ======================================================= */

ReleaseVersion save_canvas_version = ReleaseVersion(RELEASE_VERSION_END-1);
int valuenode_too_new_count;

/* === P R O C E D U R E S ================================================= */

xmlpp::Element* encode_canvas(xmlpp::Element* root,Canvas::ConstHandle canvas);
xmlpp::Element* encode_value_node(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas);

xmlpp::Element* encode_keyframe(xmlpp::Element* root,const Keyframe &kf, float fps)
{
	root->set_name("keyframe");
 	root->set_attribute("time",kf.get_time().get_string(fps));
	if(!kf.get_description().empty())
		root->set_child_text(kf.get_description());
	return root;
}


xmlpp::Element* encode_real(xmlpp::Element* root,Real v)
{
	root->set_name("real");
 	root->set_attribute("value",strprintf(VECTOR_VALUE_TYPE_FORMAT,v));
	return root;
}

xmlpp::Element* encode_time(xmlpp::Element* root,Synfig_Time t, float /*fps*/=0)
{
	root->set_name("time");
 	//root->set_attribute("value",t.get_string(fps));
 	root->set_attribute("value",t.get_string());
	return root;
}

xmlpp::Element* encode_integer(xmlpp::Element* root,int i)
{
	root->set_name("integer");
	root->set_attribute("value",strprintf("%i",i));
	return root;
}

xmlpp::Element* encode_bool(xmlpp::Element* root,bool b)
{
	root->set_name("bool");
	root->set_attribute("value",b?"true":"false");
	return root;
}

xmlpp::Element* encode_string(xmlpp::Element* root,const String &str)
{
	root->set_name("string");
	root->set_child_text(str);
	return root;
}

xmlpp::Element* encode_vector(xmlpp::Element* root,Vector vect)
{
	root->set_name("vector");
	root->add_child("x")->set_child_text(strprintf(VECTOR_VALUE_TYPE_FORMAT,(float)vect[0]));
	root->add_child("y")->set_child_text(strprintf(VECTOR_VALUE_TYPE_FORMAT,(float)vect[1]));
	return root;
}

xmlpp::Element* encode_color(xmlpp::Element* root,Color color)
{
	root->set_name("color");
	root->add_child("r")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_r()));
	root->add_child("g")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_g()));
	root->add_child("b")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_b()));
	root->add_child("a")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_a()));
	return root;
}

xmlpp::Element* encode_angle(xmlpp::Element* root,Angle theta)
{
	root->set_name("angle");
	root->set_attribute("value",strprintf("%f",(float)Angle::deg(theta).get()));
	return root;
}

xmlpp::Element* encode_segment(xmlpp::Element* root,Segment seg)
{
	root->set_name("segment");
	encode_vector(root->add_child("p1")->add_child("vector"),seg.p1);
	encode_vector(root->add_child("t1")->add_child("vector"),seg.t1);
	encode_vector(root->add_child("p2")->add_child("vector"),seg.p2);
	encode_vector(root->add_child("t2")->add_child("vector"),seg.t2);
	return root;
}

xmlpp::Element* encode_bline_point(xmlpp::Element* root,BLinePoint bline_point)
{
	root->set_name(ValueBase::type_name(ValueBase::TYPE_BLINEPOINT));

	encode_vector(root->add_child("vertex")->add_child("vector"),bline_point.get_vertex());
	encode_vector(root->add_child("t1")->add_child("vector"),bline_point.get_tangent1());

	if(bline_point.get_split_tangent_flag())
		encode_vector(root->add_child("t2")->add_child("vector"),bline_point.get_tangent2());

	encode_real(root->add_child("width")->add_child("real"),bline_point.get_width());
	encode_real(root->add_child("origin")->add_child("real"),bline_point.get_origin());
	return root;
}

xmlpp::Element* encode_gradient(xmlpp::Element* root,Gradient x)
{
	root->set_name("gradient");

	Gradient::const_iterator iter;
	x.sort();
	for(iter=x.begin();iter!=x.end();iter++)
	{
		xmlpp::Element *cpoint(encode_color(root->add_child("color"),iter->color));
		cpoint->set_attribute("pos",strprintf("%f",iter->pos));
	}
	return root;
}


xmlpp::Element* encode_value(xmlpp::Element* root,const ValueBase &data,Canvas::ConstHandle canvas=0);

xmlpp::Element* encode_list(xmlpp::Element* root,std::list<ValueBase> list, Canvas::ConstHandle canvas=0)
{
	root->set_name("list");

	while(!list.empty())
	{
		encode_value(root->add_child("value"),list.front(),canvas);
		list.pop_front();
	}

	return root;
}

xmlpp::Element* encode_value(xmlpp::Element* root,const ValueBase &data,Canvas::ConstHandle canvas)
{
	switch(data.get_type())
	{
	case ValueBase::TYPE_REAL:
		return encode_real(root,data.get(Real()));
	case ValueBase::TYPE_TIME:
		if(canvas)
			return encode_time(root,data.get(Synfig_Time()),canvas->rend_desc().get_frame_rate());
		else
			return encode_time(root,data.get(Synfig_Time()));
	case ValueBase::TYPE_INTEGER:
		return encode_integer(root,data.get(int()));
	case ValueBase::TYPE_COLOR:
		return encode_color(root,data.get(Color()));
	case ValueBase::TYPE_VECTOR:
		return encode_vector(root,data.get(Vector()));
	case ValueBase::TYPE_ANGLE:
		return encode_angle(root,data.get(Angle()));
	case ValueBase::TYPE_BOOL:
		return encode_bool(root,data.get(bool()));
	case ValueBase::TYPE_STRING:
		return encode_string(root,data.get(String()));
	case ValueBase::TYPE_SEGMENT:
		return encode_segment(root,data.get(Segment()));
	case ValueBase::TYPE_BLINEPOINT:
		return encode_bline_point(root,data.get(BLinePoint()));
	case ValueBase::TYPE_GRADIENT:
		return encode_gradient(root,data.get(Gradient()));
	case ValueBase::TYPE_LIST:
		return encode_list(root,data,canvas);
	case ValueBase::TYPE_CANVAS:
		return encode_canvas(root,data.get(Canvas::Handle()).get());
	case ValueBase::TYPE_NIL:
		synfig::error("Encountered NIL ValueBase");
		root->set_name("nil");
		return root;
	default:
		synfig::error(strprintf(_("Unknown value(%s), cannot create XML representation!"),ValueBase::type_local_name(data.get_type()).c_str()));
		root->set_name("nil");
		return root;
	}
}

xmlpp::Element* encode_animated(xmlpp::Element* root,ValueNode_Animated::ConstHandle value_node,Canvas::ConstHandle canvas=0)
{
	assert(value_node);
	root->set_name("animated");

	root->set_attribute("type",ValueBase::type_name(value_node->get_type()));

	const ValueNode_Animated::WaypointList &waypoint_list=value_node->waypoint_list();
	ValueNode_Animated::WaypointList::const_iterator iter;

	for(iter=waypoint_list.begin();iter!=waypoint_list.end();++iter)
	{
		xmlpp::Element *waypoint_node=root->add_child("waypoint");
		//waypoint_node->set_attribute("time",iter->get_time().get_string(canvas->rend_desc().get_frame_rate()));
		waypoint_node->set_attribute("time",iter->get_time().get_string());

		//waypoint_node->add_child(encode_value(iter->get_value(),canvas));
		if(iter->get_value_node()->is_exported())
			waypoint_node->set_attribute("use",iter->get_value_node()->get_relative_id(canvas));
		else {
			ValueNode::ConstHandle value_node = iter->get_value_node();
			if(ValueNode_Const::ConstHandle::cast_dynamic(value_node)) {
				const ValueBase data = ValueNode_Const::ConstHandle::cast_dynamic(value_node)->get_value();
				if (data.get_type() == ValueBase::TYPE_CANVAS)
					waypoint_node->set_attribute("use",data.get(Canvas::Handle()).get()->get_relative_id(canvas));
				else
					encode_value_node(waypoint_node->add_child("value_node"),iter->get_value_node(),canvas);
			}
			else
				encode_value_node(waypoint_node->add_child("value_node"),iter->get_value_node(),canvas);
		}

		switch(iter->get_before())
		{
		case INTERPOLATION_HALT:
			waypoint_node->set_attribute("before","halt");
			break;
		case INTERPOLATION_LINEAR:
			waypoint_node->set_attribute("before","linear");
			break;
		case INTERPOLATION_MANUAL:
			waypoint_node->set_attribute("before","manual");
			break;
		case INTERPOLATION_CONSTANT:
			waypoint_node->set_attribute("before","constant");
			break;
		case INTERPOLATION_TCB:
			waypoint_node->set_attribute("before","auto");
			break;
		default:
			error("Unknown waypoint type for \"before\" attribute");
		}

		switch(iter->get_after())
		{
		case INTERPOLATION_HALT:
			waypoint_node->set_attribute("after","halt");
			break;
		case INTERPOLATION_LINEAR:
			waypoint_node->set_attribute("after","linear");
			break;
		case INTERPOLATION_MANUAL:
			waypoint_node->set_attribute("after","manual");
			break;
		case INTERPOLATION_CONSTANT:
			waypoint_node->set_attribute("after","constant");
			break;
		case INTERPOLATION_TCB:
			waypoint_node->set_attribute("after","auto");
			break;
		default:
			error("Unknown waypoint type for \"after\" attribute");
		}

		if(iter->get_tension()!=0.0)
			waypoint_node->set_attribute("tension",strprintf("%f",iter->get_tension()));
		if(iter->get_temporal_tension()!=0.0)
			waypoint_node->set_attribute("temporal-tension",strprintf("%f",iter->get_temporal_tension()));
		if(iter->get_continuity()!=0.0)
			waypoint_node->set_attribute("continuity",strprintf("%f",iter->get_continuity()));
		if(iter->get_bias()!=0.0)
			waypoint_node->set_attribute("bias",strprintf("%f",iter->get_bias()));

	}

	return root;
}

xmlpp::Element* encode_subtract(xmlpp::Element* root,ValueNode_Subtract::ConstHandle value_node,Canvas::ConstHandle canvas=0)
{
	assert(value_node);
	root->set_name("subtract");

	ValueNode::ConstHandle lhs=value_node->get_lhs();
	ValueNode::ConstHandle rhs=value_node->get_rhs();
	ValueNode::ConstHandle scalar=value_node->get_scalar();

	assert(lhs);
	assert(rhs);

	root->set_attribute("type",ValueBase::type_name(value_node->get_type()));

	if(lhs==rhs)
		warning("LHS is equal to RHS, this <subtract> will always be zero!");

	//if(value_node->get_scalar()!=1)
	//	root->set_attribute("scalar",strprintf(VECTOR_VALUE_TYPE_FORMAT,value_node->get_scalar()));

	if(!scalar->get_id().empty())
		root->set_attribute("scalar",scalar->get_relative_id(canvas));
	else
		encode_value_node(root->add_child("scalar")->add_child("value_node"),scalar,canvas);

	if(!lhs->get_id().empty())
		root->set_attribute("lhs",lhs->get_relative_id(canvas));
	else
		encode_value_node(root->add_child("lhs")->add_child("value_node"),lhs,canvas);

	if(!rhs->get_id().empty())
		root->set_attribute("rhs",rhs->get_relative_id(canvas));
	else
		encode_value_node(root->add_child("rhs")->add_child("value_node"),rhs,canvas);

	return root;
}

xmlpp::Element* encode_dynamic_list(xmlpp::Element* root,ValueNode_DynamicList::ConstHandle value_node,Canvas::ConstHandle canvas=0)
{
	assert(value_node);
	const float fps(canvas?canvas->rend_desc().get_frame_rate():0);

	root->set_name(value_node->get_name());

	root->set_attribute("type",ValueBase::type_name(value_node->get_contained_type()));

	vector<ValueNode_DynamicList::ListEntry>::const_iterator iter;

	ValueNode_BLine::ConstHandle bline_value_node(ValueNode_BLine::ConstHandle::cast_dynamic(value_node));

	if(bline_value_node)
	{
		if(bline_value_node->get_loop())
			root->set_attribute("loop","true");
		else
			root->set_attribute("loop","false");
	}

	for(iter=value_node->list.begin();iter!=value_node->list.end();++iter)
	{
		xmlpp::Element	*entry_node=root->add_child("entry");
		assert(iter->value_node);
		if(!iter->value_node->get_id().empty())
			entry_node->set_attribute("use",iter->value_node->get_relative_id(canvas));
		else
			encode_value_node(entry_node->add_child("value_node"),iter->value_node,canvas);

		// process waypoints
		{
			typedef synfig::ValueNode_DynamicList::ListEntry::Activepoint Activepoint;
			typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;
			String begin_sequence;
			String end_sequence;

			const ActivepointList& timing_info(iter->timing_info);
			ActivepointList::const_iterator entry_iter;

			for(entry_iter=timing_info.begin();entry_iter!=timing_info.end();++entry_iter)
				if(entry_iter->state==true)
				{
					if(entry_iter->priority)
						begin_sequence+=strprintf("p%d ",entry_iter->priority);
					begin_sequence+=entry_iter->time.get_string(fps)+", ";
				}
				else
				{
					if(entry_iter->priority)
						end_sequence+=strprintf("p%d ",entry_iter->priority);
					end_sequence+=entry_iter->time.get_string(fps)+", ";
				}

			// If this is just a plane-jane vanilla entry,
			// then don't bother with begins and ends
			if(end_sequence.empty() && begin_sequence=="SOT, ")
				begin_sequence.clear();

			if(!begin_sequence.empty())
			{
				// Remove the last ", " stuff
				begin_sequence=String(begin_sequence.begin(),begin_sequence.end()-2);
				// Add the attribute
				entry_node->set_attribute("on",begin_sequence);
			}

			if(!end_sequence.empty())
			{
				// Remove the last ", " stuff
				end_sequence=String(end_sequence.begin(),end_sequence.end()-2);
				// Add the attribute
				entry_node->set_attribute("off",end_sequence);
			}
		}
	}

	return root;
}

// Generic linkable data node entry
xmlpp::Element* encode_linkable_value_node(xmlpp::Element* root,LinkableValueNode::ConstHandle value_node,Canvas::ConstHandle canvas=0)
{
	assert(value_node);

	String name(value_node->get_name());
	ReleaseVersion saving_version(get_file_version());
	ReleaseVersion feature_version(LinkableValueNode::book()[name].release_version);

	if (saving_version < feature_version)
	{
		valuenode_too_new_count++;
		warning("can't save <%s> valuenodes in this old file format version", name.c_str());

		ValueBase value((*value_node)(0));
		encode_value(root,value,canvas);

		// ValueNode_Const::ConstHandle const_value(ValueNode_Const::create((*value_node)(0)));
		// encode_value_node(root,const_value,canvas);

		return root;
	}

	root->set_name(name);

	root->set_attribute("type",ValueBase::type_name(value_node->get_type()));

	int i;
	for(i=0;i<value_node->link_count();i++)
	{
		ValueNode::ConstHandle link=value_node->get_link(i).constant();
		if(!link)
			throw runtime_error("Bad link");
		if(link->is_exported())
			root->set_attribute(value_node->link_name(i),link->get_relative_id(canvas));
		else
			encode_value_node(root->add_child(value_node->link_name(i))->add_child("value_node"),link,canvas);
	}

	return root;
}

xmlpp::Element* encode_value_node(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas)
{
	assert(value_node);

	if(ValueNode_Animated::ConstHandle::cast_dynamic(value_node))
		encode_animated(root,ValueNode_Animated::ConstHandle::cast_dynamic(value_node),canvas);
	else
	if(ValueNode_Subtract::ConstHandle::cast_dynamic(value_node))
		encode_subtract(root,ValueNode_Subtract::ConstHandle::cast_dynamic(value_node),canvas);
	else
	if(ValueNode_DynamicList::ConstHandle::cast_dynamic(value_node))
		encode_dynamic_list(root,ValueNode_DynamicList::ConstHandle::cast_dynamic(value_node),canvas);
	else if(ValueNode_Const::ConstHandle::cast_dynamic(value_node))
	{
		encode_value(root,ValueNode_Const::ConstHandle::cast_dynamic(value_node)->get_value(),canvas);
	}
	else
	if(LinkableValueNode::ConstHandle::cast_dynamic(value_node))
		encode_linkable_value_node(root,LinkableValueNode::ConstHandle::cast_dynamic(value_node),canvas);
	else
	{
		error(_("Unknown ValueNode Type (%s), cannot create an XML representation"),value_node->get_local_name().c_str());
		root->set_name("nil");
	}

	assert(root);

	if(!value_node->get_id().empty())
		root->set_attribute("id",value_node->get_id());

	if(value_node->rcount()>1)
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());

	return root;
}

xmlpp::Element* encode_layer(xmlpp::Element* root,Layer::ConstHandle layer)
{
	root->set_name("layer");

	root->set_attribute("type",layer->get_name());
	root->set_attribute("active",layer->active()?"true":"false");

	if(!layer->get_version().empty())
		root->set_attribute("version",layer->get_version());
	if(!layer->get_description().empty())
		root->set_attribute("desc",layer->get_description());
	if(!layer->get_group().empty())
		root->set_attribute("group",layer->get_group());

	Layer::Vocab vocab(layer->get_param_vocab());
	Layer::Vocab::const_iterator iter;

	const Layer::DynamicParamList &dynamic_param_list=layer->dynamic_param_list();

	for(iter=vocab.begin();iter!=vocab.end();++iter)
	{
		// Handle dynamic parameters
		if(dynamic_param_list.count(iter->get_name()))
		{
			xmlpp::Element *node=root->add_child("param");
			node->set_attribute("name",iter->get_name());

			handle<const ValueNode> value_node=dynamic_param_list.find(iter->get_name())->second;

			// If the valuenode has no ID, then it must be defined in-place
			if(value_node->get_id().empty())
			{
				encode_value_node(node->add_child("value_node"),value_node,layer->get_canvas().constant());
			}
			else
			{
				node->set_attribute("use",value_node->get_relative_id(layer->get_canvas()));
			}
		}
		else  // Handle normal parameters
		if(iter->get_critical())
		{
			ValueBase value=layer->get_param(iter->get_name());
			if(!value.is_valid())
			{
				error("Layer doesn't know its own vocabulary -- "+iter->get_name());
				continue;
			}

			if(value.get_type()==ValueBase::TYPE_CANVAS)
			{
				// the ->is_inline() below was crashing if the canvas
				// contained a PasteCanvas with the default <No Image
				// Selected> Canvas setting;  this avoids the crash
				if (!value.get(Canvas::LooseHandle()))
					continue;

				if (!value.get(Canvas::LooseHandle())->is_inline())
				{
					Canvas::Handle child(value.get(Canvas::LooseHandle()));

					if(!value.get(Canvas::Handle()))
						continue;
					xmlpp::Element *node=root->add_child("param");
					node->set_attribute("name",iter->get_name());
					node->set_attribute("use",child->get_relative_id(layer->get_canvas()));
					continue;
				}
			}
			xmlpp::Element *node=root->add_child("param");
			node->set_attribute("name",iter->get_name());

			encode_value(node->add_child("value"),value,layer->get_canvas().constant());
		}
	}


	return root;
}

xmlpp::Element* encode_canvas(xmlpp::Element* root,Canvas::ConstHandle canvas)
{
	assert(canvas);
	const RendDesc &rend_desc=canvas->rend_desc();
	root->set_name("canvas");

	if(canvas->is_root())
		root->set_attribute("version",canvas->get_version());

	if(!canvas->get_id().empty() && !canvas->is_root() && !canvas->is_inline())
		root->set_attribute("id",canvas->get_id());

	if(!canvas->parent() || canvas->parent()->rend_desc().get_w()!=canvas->rend_desc().get_w())
		root->set_attribute("width",strprintf("%d",rend_desc.get_w()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_h()!=canvas->rend_desc().get_h())
		root->set_attribute("height",strprintf("%d",rend_desc.get_h()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_x_res()!=canvas->rend_desc().get_x_res())
		root->set_attribute("xres",strprintf("%f",rend_desc.get_x_res()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_y_res()!=canvas->rend_desc().get_y_res())
		root->set_attribute("yres",strprintf("%f",rend_desc.get_y_res()));


	if(!canvas->parent() ||
		canvas->parent()->rend_desc().get_tl()!=canvas->rend_desc().get_tl() ||
		canvas->parent()->rend_desc().get_br()!=canvas->rend_desc().get_br())
	root->set_attribute("view-box",strprintf(VIEW_BOX_FORMAT,
		rend_desc.get_tl()[0],
		rend_desc.get_tl()[1],
		rend_desc.get_br()[0],
		rend_desc.get_br()[1])
	);

	if(!canvas->parent() || canvas->parent()->rend_desc().get_antialias()!=canvas->rend_desc().get_antialias())
		root->set_attribute("antialias",strprintf("%d",rend_desc.get_antialias()));

	if(!canvas->parent())
		root->set_attribute("fps",strprintf(TIME_TYPE_FORMAT,rend_desc.get_frame_rate()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_time_start()!=canvas->rend_desc().get_time_start())
		root->set_attribute("begin-time",rend_desc.get_time_start().get_string(rend_desc.get_frame_rate()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_time_end()!=canvas->rend_desc().get_time_end())
		root->set_attribute("end-time",rend_desc.get_time_end().get_string(rend_desc.get_frame_rate()));

	if(!canvas->is_inline())
	{
		root->set_attribute("bgcolor",strprintf(VIEW_BOX_FORMAT,
			rend_desc.get_bg_color().get_r(),
			rend_desc.get_bg_color().get_g(),
			rend_desc.get_bg_color().get_b(),
			rend_desc.get_bg_color().get_a())
		);

		if(!canvas->get_name().empty())
			root->add_child("name")->set_child_text(canvas->get_name());
		if(!canvas->get_description().empty())
			root->add_child("desc")->set_child_text(canvas->get_description());
		if(!canvas->get_author().empty())
			root->add_child("author")->set_child_text(canvas->get_description());

		std::list<String> meta_keys(canvas->get_meta_data_keys());
		while(!meta_keys.empty())
		{
			xmlpp::Element* meta_element(root->add_child("meta"));
			meta_element->set_attribute("name",meta_keys.front());
			meta_element->set_attribute("content",canvas->get_meta_data(meta_keys.front()));
			meta_keys.pop_front();
		}
		for(KeyframeList::const_iterator iter=canvas->keyframe_list().begin();iter!=canvas->keyframe_list().end();++iter)
			encode_keyframe(root->add_child("keyframe"),*iter,canvas->rend_desc().get_frame_rate());
	}

	// Output the <defs> section
	//! \todo check where the parentheses should really go - around the && or the ||?
	if((!canvas->is_inline() && !canvas->value_node_list().empty()) || !canvas->children().empty())
	{
		xmlpp::Element *node=root->add_child("defs");
		const ValueNodeList &value_node_list(canvas->value_node_list());

		for(ValueNodeList::const_iterator iter=value_node_list.begin();iter!=value_node_list.end();++iter)
		{
			// If the value_node is a constant, then use the shorthand
			if(handle<ValueNode_Const>::cast_dynamic(*iter))
			{
				ValueNode_Const::Handle value_node(ValueNode_Const::Handle::cast_dynamic(*iter));
				reinterpret_cast<xmlpp::Element*>(encode_value(node->add_child("value"),value_node->get_value()))->set_attribute("id",value_node->get_id());
				continue;
			}
			encode_value_node(node->add_child("value_node"),*iter,canvas);
			// writeme
		}

		for(Canvas::Children::const_iterator iter=canvas->children().begin();iter!=canvas->children().end();++iter)
		{
			encode_canvas(node->add_child("canvas"),*iter);
		}
	}

	Canvas::const_reverse_iterator iter;

	for(iter=canvas->rbegin();iter!=canvas->rend();++iter)
		encode_layer(root->add_child("layer"),*iter);

	return root;
}

xmlpp::Element* encode_canvas_toplevel(xmlpp::Element* root,Canvas::ConstHandle canvas)
{
	valuenode_too_new_count = 0;

	xmlpp::Element* ret = encode_canvas(root, canvas);

	if (valuenode_too_new_count)
		warning("saved %d valuenodes as constant values in old file format\n", valuenode_too_new_count);

	return ret;
}

bool
synfig::save_canvas(const String &filename, Canvas::ConstHandle canvas)
{
    ChangeLocale change_locale(LC_NUMERIC, "C");

	synfig::String tmp_filename(filename+".TMP");

	if (filename_extension(filename) == ".sifz")
		xmlSetCompressMode(9);
	else
		xmlSetCompressMode(0);

	try
	{
		assert(canvas);
		xmlpp::Document document;

		encode_canvas_toplevel(document.create_root_node("canvas"),canvas);

		document.write_to_file_formatted(tmp_filename);

#ifdef _WIN32
		// On Win32 platforms, rename() has bad behavior. work around it.
		char old_file[80]="sif.XXXXXXXX";
		mktemp(old_file);
		rename(filename.c_str(),old_file);
		if(rename(tmp_filename.c_str(),filename.c_str())!=0)
		{
			rename(old_file,tmp_filename.c_str());
			synfig::error("synfig::save_canvas(): Unable to rename file to correct filename, errno=%d",errno);
			return false;
		}
		remove(old_file);
#else
		if(rename(tmp_filename.c_str(),filename.c_str())!=0)
		{
			synfig::error("synfig::save_canvas(): Unable to rename file to correct filename, errno=%d",errno);
			return false;
		}
#endif
	}
	catch(...) { synfig::error("synfig::save_canvas(): Caught unknown exception"); return false; }

	return true;
}

String
synfig::canvas_to_string(Canvas::ConstHandle canvas)
{
    ChangeLocale change_locale(LC_NUMERIC, "C");
	assert(canvas);

	xmlpp::Document document;

	encode_canvas_toplevel(document.create_root_node("canvas"),canvas);

	return document.write_to_string_formatted();
}

void
synfig::set_file_version(ReleaseVersion version)
{
	save_canvas_version = version;
}

ReleaseVersion
synfig::get_file_version()
{
	return save_canvas_version;
}
