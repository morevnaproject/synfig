/* === S Y N F I G ========================================================= */
/*!	\file context.cpp
**	\brief Template File
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

#include "context.h"
#include "layer.h"
#include "layers/layer_composite.h"
#include "synfig_string.h"
#include "vector.h"
#include "color.h"
#include "surface.h"
#include "renddesc.h"
#include "valuenode.h"

#include "renderers/renderer_opengl.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

// #define SYNFIG_PROFILE_LAYERS
// #define SYNFIG_DEBUG_LAYERS

/* === G L O B A L S ======================================================= */

#ifdef SYNFIG_PROFILE_LAYERS
#include <ETL/clock>
static int depth(0);
static std::map<String,float> time_table;
static std::map<String,int> run_table;
static etl::clock profile_timer;
static String curr_layer;
static void
_print_profile_report()
{
	synfig::info(">>>> Profile Report: (Times are in msecs)");
	std::map<String,float>::iterator iter;
	float total_time(0);
	for(iter=time_table.begin();iter!=time_table.end();++iter)
	{
		String layer(iter->first);
		float time(iter->second);
		int runs(run_table[layer]);
		total_time+=time;
		synfig::info(" Layer \"%s\",\tExecs: %03d, Avg Time: %05.1f, Total Time: %05.1f",layer.c_str(),runs,time/runs*1000,time*1000);
	}
	synfig::info("Total Time: %f seconds", total_time);
	synfig::info("<<<< End of Profile Report");
}
#endif	// SYNFIG_PROFILE_LAYERS

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Color
Context::get_color(const Point &pos)const
{
	Context context(*this);

	while(!context->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return alpha
	if((context)->empty()) return Color::alpha();

	RWLock::ReaderLock lock((*context)->get_rw_lock());

	return (*context)->get_color(context+1, pos);
}

Rect
Context::get_full_bounding_rect()const
{
	Context context(*this);

	while(!context->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return zero-sized rectangle
	if(context->empty()) return Rect::zero();

	return (*context)->get_full_bounding_rect(context+1);
}


/* Profiling will go like this:
	Profile start = +, stop = -

	+
	-

	time diff is recorded

	to get the independent times we need to break at the one inside and record etc...
	so it looks more like this:

	+
	  -
	  +
		-
		+
			...
		-
		+
	  -
	  +
	-

	at each minus we must record all the info for that which we are worried about...
	each layer can do work before or after the other work is done... so both values must be recorded...
*/

bool
Context::render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb, RenderMethod method) const
{
#ifdef SYNFIG_PROFILE_LAYERS
	String layer_name(curr_layer);

	//sum the pre-work done by layer above us... (curr_layer is layer above us...)
	if(depth>0)
	{
		time_table[curr_layer]+=profile_timer();
		//if(run_table.count(curr_layer))run_table[curr_layer]++;
		//	else run_table[curr_layer]=1;
	}
#endif	// SYNFIG_PROFILE_LAYERS

	Renderer_OpenGL &renderer_opengl = Target::renderer_opengl();

	const Rect bbox(renddesc.get_rect());

	// this is going to be set to true if this layer contributes
	// nothing, but it's a straight blend with non-zero amount, and so
	// it has an effect anyway
	bool straight_and_empty = false;
	etl::handle<Layer_Composite> composite;
	Context context(*this);

	for(;!(context)->empty();++context)
	{
		// If we are not active then move on to next layer
		if(!(*context)->active())
			continue;

		const Rect layer_bounds((*context)->get_bounding_rect());
		composite = etl::handle<Layer_Composite>::cast_dynamic(*context);

		// If the box area is less than zero or the boxes do not
		// intersect then move on to next layer, unless the layer is
		// using a straight blend and has a non-zero amount, in which
		// case it will still affect the result
		if(layer_bounds.area() <= 0.0000000000001 || !(layer_bounds && bbox))
		{
			if (composite &&
				Color::is_straight(composite->get_blend_method()) &&
				composite->get_amount() != 0.0f)
			{
				straight_and_empty = true;
				break;
			}
			continue;
		}

		// If this layer has Straight as the blend method and amount
		// is 1.0, and the layer doesn't depend on its context, then
		// we don't want to render the context
		if (composite &&
			composite->get_blend_method() == Color::BLEND_STRAIGHT &&
			composite->get_amount() == 1.0f &&
			!composite->reads_context())
		{
			Layer::Handle layer = *context;
			// TODO: Ask if that's a bug (skipping while NOT empty??)
			while (!context->empty()) context++; // skip the context
			switch (method) {
				case SOFTWARE:
					return layer->accelerated_render(context,surface,quality,renddesc, cb);
					break;
				case OPENGL:
					return layer->opengl_render(context,&renderer_opengl,quality,renddesc, cb);
					break;
				default:
					synfig::info("Context::render(): Unknown rendering method, falling back to software");
					return layer->accelerated_render(context,surface,quality,renddesc, cb);
					break;
			}
		}

		// Break out of the loop--we have found a good layer
		break;
	}

	// If this layer isn't defined, because we've reached the bottom layer, return alpha
	if (context->empty() || (straight_and_empty && composite->get_amount() == 1.0f))
	{
#ifdef SYNFIG_DEBUG_LAYERS
		synfig::info("Context::render(): Hit end of list");
#endif	// SYNFIG_DEBUG_LAYERS
		switch (method) {
			case SOFTWARE:
				surface->set_wh(renddesc.get_w(),renddesc.get_h());
				surface->clear();
				break;
			case OPENGL:
				renderer_opengl.set_wh(renddesc.get_w(),renddesc.get_h());
				// "set_wh" already clears buffers
				//renderer_opengl_->clear();
				break;
			default:
				synfig::info("Context::render(): Unknown rendering method, falling back to software");
				surface->set_wh(renddesc.get_w(),renddesc.get_h());
				surface->clear();
				break;
		}
#ifdef SYNFIG_PROFILE_LAYERS
		profile_timer.reset();
#endif	// SYNFIG_PROFILE_LAYERS
		return true;
	}

#ifdef SYNFIG_DEBUG_LAYERS
	synfig::info("Context::render(): Descending into %s",(*context)->get_name().c_str());
#endif	// SYNFIG_DEBUG_LAYERS

	try {
		RWLock::ReaderLock lock((*context)->get_rw_lock());

#ifdef SYNFIG_PROFILE_LAYERS
	//go down one layer :P
	depth++;
	curr_layer=(*context)->get_name();	//make sure the layer inside is referring to the correct layer outside
	profile_timer.reset(); 										// +
#endif	// SYNFIG_PROFILE_LAYERS

	bool ret;

	// this layer doesn't draw anything onto the canvas we're
	// rendering, but it uses straight blending, so we need to render
	// the stuff under us and then blit transparent pixels over it
	// using the appropriate 'amount'
	if (straight_and_empty)
	{
		if ((ret = Context((context+1)).render(surface,quality,renddesc,cb, method)))
		{

			switch (method) {
				case SOFTWARE:
				{
					Surface clearsurface;
					clearsurface.set_wh(renddesc.get_w(),renddesc.get_h());
					clearsurface.clear();

					Surface::alpha_pen apen(surface->begin());
					apen.set_alpha(composite->get_amount());
					apen.set_blend_method(composite->get_blend_method());

					clearsurface.blit_to(apen);
					break;
				}
				case OPENGL:
					// FIXME //
					break;
				default:
					synfig::info("Context::render(): Unknown rendering method, falling back to software");
					// FIXME //
					break;
			}
		}
	}
	else {
		switch (method) {
			case SOFTWARE:
				ret = (*context)->accelerated_render(context+1,surface,quality,renddesc, cb);
				break;
			case OPENGL:
				ret = (*context)->opengl_render(context+1,&renderer_opengl,quality,renddesc, cb);
				break;
			default:
				synfig::info("Context::render(): Unknown rendering method, falling back to software");
				ret = (*context)->accelerated_render(context+1,surface,quality,renddesc, cb);
				break;
		}
	}

#ifdef SYNFIG_PROFILE_LAYERS
	//post work for the previous layer
	time_table[curr_layer]+=profile_timer();							//-
	if(run_table.count(curr_layer))run_table[curr_layer]++;
		else run_table[curr_layer]=1;

	depth--;
	curr_layer = layer_name; //we are now onto this layer (make sure the post gets recorded correctly...

	//print out the table it we're done...
	if(depth==0) _print_profile_report(),time_table.clear(),run_table.clear();
	profile_timer.reset();												//+
#endif	// SYNFIG_PROFILE_LAYERS

	return ret;
	}
	catch(std::bad_alloc)
	{
		synfig::error("Context::render(): Layer \"%s\" threw a bad_alloc exception!",(*context)->get_name().c_str());
#ifdef _DEBUG
		return false;
#else  // _DEBUG
		++context;
		return context.render(surface, quality, renddesc, cb, method);
#endif	// _DEBUG
	}
	catch(...)
	{
		synfig::error("Context::render(): Layer \"%s\" threw an exception, rethrowing...",(*context)->get_name().c_str());
		throw;
	}
}

void
Context::set_time(Synfig_Time time)const
{
	Context context(*this);
	while(!(context)->empty())
	{
		// If this layer is active, and
		// it either isn't already set to the given time or
		//           it's a time loop layer,
		// then break out of the loop and set its time
		if((*context)->active() &&
		   (!(*context)->dirty_time_.is_equal(time) ||
			(*context)->get_name() == "timeloop"))
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, just return
	if((context)->empty()) return;

	// Set up a writer lock
	RWLock::WriterLock lock((*context)->get_rw_lock());

	//synfig::info("%s: dirty_time=%f",(*context)->get_name().c_str(),(float)(*context)->dirty_time_);
	//synfig::info("%s: time=%f",(*context)->get_name().c_str(),(float)time);

	{
		Layer::ParamList params;
		Layer::DynamicParamList::const_iterator iter;

		for(iter=(*context)->dynamic_param_list().begin();iter!=(*context)->dynamic_param_list().end();iter++)
			params[iter->first]=(*iter->second)(time);

		(*context)->set_param_list(params);

		(*context)->set_time(context+1,time);
		(*context)->dirty_time_=time;

	}
}

void
Context::set_time(Synfig_Time time,const Vector &/*pos*/)const
{
	set_time(time);
/*
	Context context(*this);
	while(!(context)->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, just return
	if((context)->empty()) return;

	else
	{
		Layer::ParamList params;
		Layer::DynamicParamList::const_iterator iter;

		for(iter=(*context)->dynamic_param_list().begin();iter!=(*context)->dynamic_param_list().end();iter++)
			params[iter->first]=(*iter->second)(time);

		(*context)->set_param_list(params);

		(*context)->set_time(context+1,time,pos);
	}
*/
}

etl::handle<Layer>
Context::hit_check(const Point &pos)const
{
	Context context(*this);

	while(!context->empty())
	{
		// If this layer is active, then go
		// ahead and break out of the loop
		if((*context)->active())
			break;

		// Otherwise, we want to keep searching
		// till we find either an active layer,
		// or the end of the layer list
		++context;
	}

	// If this layer isn't defined, return an empty handle
	if((context)->empty()) return 0;

	return (*context)->hit_check(context+1, pos);
}
