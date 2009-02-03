/* === S Y N F I G ========================================================= */
/*!	\file layer_composite.cpp
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

#include "layer_composite.h"
#include "../context.h"
#include "../synfig_time.h"
#include "../color.h"
#include "../surface.h"
#include "../renddesc.h"
#include "../target.h"

#include "layer_bitmap.h"

#include "../general.h"
#include "../render.h"
#include "../paramdesc.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
Layer_Composite::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc_, ProgressCallback *cb)  const
{
	RendDesc renddesc(renddesc_);

	if(!amount_)
		return context.accelerated_render(surface,quality,renddesc,cb);

	CanvasBase image;

	SuperCallback stageone(cb,0,50000,100000);
	SuperCallback stagetwo(cb,50000,100000,100000);

	Layer_Bitmap::Handle surfacelayer(new class Layer_Bitmap());

	Context iter;

	for(iter=context;*iter;iter++)
		image.push_back(*iter);

	image.push_front(surfacelayer.get());

	// We want to go ahead and schedule any other
	// layers...
//	while(dynamic_cast<Layer_Composite*>(context->get()))
//	while(context->get() &&
//		&context->get()->AcceleratedRender==
//		&Layer_Composite::AcceleratedRender)
//		image.push_back(*(context++));

	image.push_back(0);	// Alpha black

	// Render the backdrop
	if(!context.accelerated_render(&surfacelayer->surface,quality,renddesc,&stageone))
		return false;

	if(quality<=4)surfacelayer->c=3;else
	if(quality<=5)surfacelayer->c=2;
	else if(quality<=6)surfacelayer->c=1;
	else surfacelayer->c=0;
	surfacelayer->tl=renddesc.get_tl();
	surfacelayer->br=renddesc.get_br();
	surfacelayer->set_blend_method(Color::BLEND_STRAIGHT);

	image.push_front(const_cast<synfig::Layer_Composite*>(this));

	// Set up a surface target
	Target::Handle target(surface_target(surface));

	if(!target)
	{
		if(cb)cb->error(_("Unable to create surface target"));
		return false;
	}

	RendDesc desc(renddesc);

	target->set_rend_desc(&desc);

	// Render the scene
	return render(Context(image.begin()),target,desc,&stagetwo);
	//return render_threaded(Context(image.begin()),target,desc,&stagetwo,2);
}

Rect
Layer_Composite::get_full_bounding_rect(Context context)const
{
	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	return context.get_full_bounding_rect()|get_bounding_rect();
}

Layer::Vocab
Layer_Composite::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc(amount_,"amount")
		.set_local_name(_("Amount"))
	);
	ret.push_back(ParamDesc(blend_method_,"blend_method")
		.set_local_name(_("Blend Method"))
	);

	return ret;
}

bool
Layer_Composite::set_param(const String & param, const ValueBase &value)
{
	if(param=="amount" && value.same_type_as(amount_))
		amount_=value.get(amount_);
	else
	if(param=="blend_method" && value.same_type_as(int()))
	{
		blend_method_ = static_cast<Color::BlendMethod>(value.get(int()));

		if (blend_method_ < 0 || blend_method_ >= Color::BLEND_END)
		{
			warning("illegal value (%d) for blend_method - using Composite instead", blend_method_);
			blend_method_ = Color::BLEND_COMPOSITE;
			return false;
		}

		if (blend_method_ == Color::BLEND_STRAIGHT && !reads_context())
		{
			Canvas::Handle canvas(get_canvas());
			if (canvas)
			{
				String version(canvas->get_version());

				if (version == "0.1" || version == "0.2")
				{
					if (get_name() == "PasteCanvas")
						warning("loaded a version %s canvas with a 'Straight' blended PasteCanvas (%s) - check it renders OK",
								version.c_str(), get_non_empty_description().c_str());
					else
					{
						blend_method_ = Color::BLEND_COMPOSITE;
						converted_blend_ = true;

						// if this layer has a transparent color, go back and set the color again
						// now that we know we are converting the blend method as well.  that will
						// make the color non-transparent, and change the blend method to alpha over
						if (transparent_color_)
							set_param("color", get_param("color"));
					}
				}
			}
		}
	}
	else
		return Layer::set_param(param,value);
	return true;
}

ValueBase
Layer_Composite::get_param(const String & param)const
{
	if(param=="amount")
		return get_amount();
	if(param=="blend_method")
		return static_cast<int>(get_blend_method());
	return Layer::get_param(param);
}
