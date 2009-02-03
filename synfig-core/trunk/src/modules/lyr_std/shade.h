/* === S Y N F I G ========================================================= */
/*!	\file shade.h
**	\brief Header file for implementation of the "Shade" layer
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

#ifndef __SYNFIG_LAYER_SHADE_H__
#define __SYNFIG_LAYER_SHADE_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>

using namespace synfig;
using namespace std;
using namespace etl;

class Layer_Shade : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
private:
	synfig::Vector 	size;
	int				type;
	synfig::Color	color;
	synfig::Vector	origin;
	bool invert;

public:
	Layer_Shade();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }
}; // END of class Layer_Shade

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
