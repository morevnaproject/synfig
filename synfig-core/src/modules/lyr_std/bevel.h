/* === S Y N F I G ========================================================= */
/*!	\file bevel.h
**	\brief Header file for implementation of the "Bevel" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_BEVEL_H__
#define __SYNFIG_LAYER_BEVEL_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite_fork.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>
#include <synfig/angle.h>

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Layer_Bevel : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (int) type of blur to use
	ValueBase param_type;
	//!Parameter: (Real) amount of blur
	ValueBase param_softness;
	//!Parameter: (Color) light color
	ValueBase param_color1;
	//!Parameter: (Color) dark color
	ValueBase param_color2;
	//!Parameter: (Angle) angle of the light source
	ValueBase param_angle;
	//!Parameter: (Real) depth of the bevel
	ValueBase param_depth;
	//!Parameter: (bool) use luma
	ValueBase param_use_luma;
	//!Parameter: (bool) solid
	ValueBase param_solid;
	
	//Vector	offset;
	//Vector	offset45;


	void calc_offset();
    rendering::TaskBlend::Handle draw_sample(rendering::Task::Handle sub_task, Vector offset1, Vector offset2, Color color )const;

public:
	Layer_Bevel();

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	//virtual RendDesc get_sub_renddesc_vfunc(const RendDesc &renddesc) const;
    rendering::Task::Handle build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle sub_task)const override;
}; // END of class Layer_Bevel

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
