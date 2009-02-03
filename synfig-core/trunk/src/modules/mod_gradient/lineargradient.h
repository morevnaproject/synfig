/* === S Y N F I G ========================================================= */
/*!	\file lineargradient.h
**	\brief Header file for implementation of the "Linear Gradient" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_INTERPOLATION_LINEARGRADIENT_H
#define __SYNFIG_INTERPOLATION_LINEARGRADIENT_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/layers/layer_composite.h>
#include <synfig/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class LinearGradient : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:

	Point p1,p2;
	Vector diff;
	Gradient gradient;

	bool loop;
	bool zigzag;

	void sync();

	synfig::Color color_func(const synfig::Point &x, float supersample=0)const;

	float calc_supersample(const synfig::Point &x, float pw,float ph)const;

public:
	LinearGradient();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
