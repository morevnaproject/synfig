/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblendsw.h
**	\brief TaskBlendSW Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TASKBLENDSW_H
#define __SYNFIG_RENDERING_TASKBLENDSW_H

/* === H E A D E R S ======================================================= */

#include "tasksw.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskBlendSW: public TaskSW
{
public:
	typedef etl::handle<TaskBlendSW> Handle;

	Color::BlendMethod blend_method;

	TaskBlendSW(): blend_method(Color::BLEND_COMPOSITE) { }
	Task::Handle clone() const { return clone_pointer(this); }

	const Task::Handle& sub_task_a() const { return sub_task(0); }
	Task::Handle& sub_task_a() { return sub_task(0); }

	const Task::Handle& sub_task_b() const { return sub_task(1); }
	Task::Handle& sub_task_b() { return sub_task(1); }

	virtual bool run(RunParams &params) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
