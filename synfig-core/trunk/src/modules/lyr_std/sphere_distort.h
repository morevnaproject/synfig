/* === S Y N F I G ========================================================= */
/*!	\file sphere_distort.h
**	\brief Header file for implementation of the "Spherize" layer
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SPHERE_DISTORT_H
#define __SYNFIG_SPHERE_DISTORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite.h>
#include <synfig/vector.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig
{
class Spherize_Trans;

class Layer_SphereDistort : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Spherize_Trans;

private:

	Vector center;
	double radius;

	double percent;

	int		type;

//	static Point sphtrans(const Point &xoff, const Point &center, const Real &radius, const Real &percent, int type);

//	static double spherify(double xoff);
//	static double unspherify(double xoff);

	bool clip;

	synfig::Rect bounds;

	void sync();

public:

	Layer_SphereDistort();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

	virtual Rect get_bounding_rect()const;

	virtual Vocab get_param_vocab()const;
	virtual etl::handle<synfig::Transform> get_transform()const;
}; // END of class Layer_SphereDistort

}

/* === E N D =============================================================== */

#endif
