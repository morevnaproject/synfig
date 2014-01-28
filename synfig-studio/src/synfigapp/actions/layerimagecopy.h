/* === S Y N F I G ========================================================= */
/*!	\file layerimagecopy.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_APP_ACTION_LAYERIMAGECOPY_H
#define __SYNFIG_APP_ACTION_LAYERIMAGECOPY_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/layer_bitmap.h>
#include <synfig/layer_switch.h>
#include <synfigapp/action.h>
#include <list>
#include <synfig/guid.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class LayerImageCopy :
	public Super
{
private:
	synfig::Time time;
	synfig::String filename;
	etl::handle<synfig::Layer_Bitmap> layer_bitmap;
	etl::handle<synfig::Layer_Switch> layer_switch;

public:

	LayerImageCopy();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
