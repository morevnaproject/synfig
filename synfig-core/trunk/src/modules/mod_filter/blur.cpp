/* === S Y N F I G ========================================================= */
/*!	\file mod_filter/blur.cpp
**	\brief Implementation of the "Blur" layer
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

#include "blur.h"

#include <synfig/synfig_string.h>
#include <synfig/synfig_time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#include <cstring>
#include <ETL/pen>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Blur_Layer);
SYNFIG_LAYER_SET_NAME(Blur_Layer,"blur");
SYNFIG_LAYER_SET_LOCAL_NAME(Blur_Layer,N_("Blur"));
SYNFIG_LAYER_SET_CATEGORY(Blur_Layer,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Blur_Layer,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Blur_Layer,"$Id$");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Blur_Layer::Blur_Layer():
	Layer_Composite	(1.0,Color::BLEND_STRAIGHT),
	size(0.1,0.1),
	type(Blur::FASTGAUSSIAN)
{
}

bool
Blur_Layer::set_param(const String &param, const ValueBase &value)
{
	IMPORT_PLUS(size,clamp(size));
	IMPORT(type);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Blur_Layer::get_param(const String &param)const
{
	EXPORT(size);
	EXPORT(type);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Blur_Layer::get_color(Context context, const Point &pos)const
{
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return context.get_color(blurpos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	return Color::blend(context.get_color(blurpos),context.get_color(pos),get_amount(),get_blend_method());
}

bool
Blur_Layer::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// don't do anything at quality 10
	if (quality == 10)
		return context.render(surface,quality,renddesc,cb, SOFTWARE);

	// int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);

	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();

	RendDesc	workdesc(renddesc);
	Surface		worksurface,blurred;

	//callbacks depend on how long the blur takes
	if(size[0] || size[1])
	{
		if(type == Blur::DISC)
		{
			stageone = SuperCallback(cb,0,5000,10000);
			stagetwo = SuperCallback(cb,5000,10000,10000);
		}
		else
		{
			stageone = SuperCallback(cb,0,9000,10000);
			stagetwo = SuperCallback(cb,9000,10000,10000);
		}
	}
	else
	{
		stageone = SuperCallback(cb,0,9999,10000);
		stagetwo = SuperCallback(cb,9999,10000,10000);
	}

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
		halfsizey = (int) (abs(size[1]*.5/ph) + 3);

	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			if(quality < 4)
			{
				halfsizex*=2;
				halfsizey*=2;
			}
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
			break;
		}
		case Blur::GAUSSIAN:
		{
		#define GAUSSIAN_ADJUSTMENT		(0.05)
			Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);

			pw=pw*pw;
			ph=ph*ph;

			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );

			break;
		}
	}

	//render the background onto the expanded surface
	if(!context.render(&worksurface,quality,workdesc,&stageone, SOFTWARE))
		return false;

	//blur the image
	Blur(size,type,&stagetwo)(worksurface,workdesc.get_br()-workdesc.get_tl(),blurred);

	//be sure the surface is of the correct size
	surface->set_wh(renddesc.get_w(),renddesc.get_h());

	{
		Surface::pen pen(surface->begin());
		worksurface.blit_to(pen,halfsizex,halfsizey,renddesc.get_w(),renddesc.get_h());
	}
	{
		Surface::alpha_pen pen(surface->begin());
		pen.set_alpha(get_amount());
		pen.set_blend_method(get_blend_method());
		blurred.blit_to(pen,halfsizex,halfsizey,renddesc.get_w(),renddesc.get_h());
	}
	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	return true;
}

Layer::Vocab
Blur_Layer::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of Blur"))
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	return ret;
}

Rect
Blur_Layer::get_full_bounding_rect(Context context)const
{
	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	Rect bounds(context.get_full_bounding_rect().expand_x(size[0]).expand_y(size[1]));

	return bounds;
}
