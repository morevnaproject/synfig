/* === S Y N F I G ========================================================= */
/*!	\file duckmatic.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DUCKMATIC_H
#define __SYNFIG_STUDIO_DUCKMATIC_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <map>
#include <set>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <synfig/vector.h>
#include <synfig/synfig_string.h>
#include <synfig/real.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <synfig/synfig_time.h>
#include <synfig/color.h>
#include <ETL/smart_ptr>

#include "duck.h"
#include <synfig/color.h>
#include <synfig/guidset.h>

/* === M A C R O S ========================================================= */

#ifdef HASH_MAP_H
#include HASH_MAP_H
#include FUNCTIONAL_H

#ifndef __STRING_HASH__
#define __STRING_HASH__
class StringHash
{
# ifdef FUNCTIONAL_HASH_ON_STRING
	HASH_MAP_NAMESPACE::hash<synfig::String> hasher_;
# else  // FUNCTIONAL_HASH_ON_STRING
	HASH_MAP_NAMESPACE::hash<const char*> hasher_;
# endif  // FUNCTIONAL_HASH_ON_STRING
public:
	size_t operator()(const synfig::String& x)const
	{
# ifdef FUNCTIONAL_HASH_ON_STRING
		return hasher_(x);
# else  // FUNCTIONAL_HASH_ON_STRING
		return hasher_(x.c_str());
# endif  // FUNCTIONAL_HASH_ON_STRING
	}
};
#endif
#else
#include <map>
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp { class ValueDesc; }
namespace synfig { class ParamDesc; }

namespace studio
{

class CanvasView;
class Duckmatic;

class DuckDrag_Base : public etl::shared_object
{
public:
	virtual void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin)=0;
	virtual bool end_duck_drag(Duckmatic* duckmatic)=0;
	virtual void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector)=0;
};

class DuckDrag_Translate : public DuckDrag_Base
{
	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;
	synfig::Vector snap;
	std::vector<synfig::Vector> positions;

public:
	void begin_duck_drag(Duckmatic* duckmatic, const synfig::Vector& begin);
	bool end_duck_drag(Duckmatic* duckmatic);
	void duck_drag(Duckmatic* duckmatic, const synfig::Vector& vector);
};

/*! \class Duckmatic
**
**	This class helps organize any of the devices displayed in
**	the work area that the user may want to interact with.
**	This includes ducks, beziers, and strokes
**
**	\note At some point I'll probably rename this class to "DuckOMatic".
*/
class Duckmatic
{
	friend class DuckDrag_Base;
	friend class DuckDrag_Translate;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

#ifdef HASH_MAP_H
typedef HASH_MAP_CLASS<synfig::GUID,etl::smart_ptr<synfig::Point>,synfig::GUIDHash> DuckDataMap;
#else
typedef std::map<synfig::GUID,etl::smart_ptr<synfig::Point> > DuckDataMap;
#endif

	typedef studio::DuckMap DuckMap;

	typedef studio::Duck Duck;

	struct Stroke;

	struct Bezier;

	class Push;

	friend class Push;

	typedef Duck::Type Type;

	typedef std::list<float> GuideList;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Type type_mask;

	DuckMap duck_map;

	DuckDataMap duck_data_share_map;

	std::list<etl::handle<Stroke> > stroke_list_;

	std::list<etl::handle<Stroke> > persistent_stroke_list_;

	synfig::GUIDSet selected_ducks;

	synfig::GUID last_duck_guid;

	std::list<etl::handle<Bezier> > bezier_list_;

	//! I cannot recall what this is for
	//synfig::Vector snap;

	etl::handle<DuckDrag_Base> duck_dragger_;

	sigc::signal<void> signal_duck_selection_changed_;

	sigc::signal<void> signal_strokes_changed_;

	sigc::signal<void> signal_grid_changed_;

	mutable sigc::signal<void> signal_sketch_saved_;

	GuideList guide_list_x_;
	GuideList guide_list_y_;

	mutable synfig::String sketch_filename_;

	/*
 -- ** -- P R O T E C T E D   D A T A -----------------------------------------
	*/

protected:

	etl::handle<Bezier> selected_bezier;

	synfig::Time cur_time;

	//! This flag is set if operations should snap to the grid
	/*! \todo perhaps there should be two of these flags, one for each axis?
	**	\see show_grid, grid_size */
	bool grid_snap;

	bool guide_snap;

	//! This vector describes the grid size.
	/*! \see grid_snap, show_grid */
	synfig::Vector grid_size;

	bool show_persistent_strokes;

	bool axis_lock;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	synfig::Vector last_translate_;
	synfig::Vector drag_offset_;

	//etl::handle<Duck> selected_duck;


	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	Duckmatic();
	virtual ~Duckmatic();

	sigc::signal<void>& signal_duck_selection_changed() { return signal_duck_selection_changed_; }
	sigc::signal<void>& signal_strokes_changed() { return signal_strokes_changed_; }
	sigc::signal<void>& signal_grid_changed() { return signal_grid_changed_; }
	sigc::signal<void>& signal_sketch_saved() { return signal_sketch_saved_; }

	GuideList& get_guide_list_x() { return guide_list_x_; }
	GuideList& get_guide_list_y() { return guide_list_y_; }
	const GuideList& get_guide_list_x()const { return guide_list_x_; }
	const GuideList& get_guide_list_y()const { return guide_list_y_; }

	void set_guide_snap(bool x=true);
	bool get_guide_snap()const { return guide_snap; }
	void toggle_guide_snap() { set_guide_snap(!get_guide_snap()); }

	//! Sets the state of the grid snap flag
	void set_grid_snap(bool x=true);

	//! Gets the state of the grid snap flag
	bool get_grid_snap()const { return grid_snap; }

	void enable_grid_snap() { set_grid_snap(true); }

	void disable_grid_snap() { set_grid_snap(false); }

	void toggle_grid_snap() { set_grid_snap(!grid_snap); }

	synfig::Point snap_point_to_grid(const synfig::Point& x, float radius=0.1)const;

	bool get_show_persistent_strokes()const { return show_persistent_strokes; }
	void set_show_persistent_strokes(bool x);

	//! Sets the size of the grid
	void set_grid_size(const synfig::Vector &s);

	//! Returns the size of the grid
	const synfig::Vector &get_grid_size()const { return grid_size; }


	const synfig::Time &get_time()const { return cur_time; }

	bool get_axis_lock()const { return axis_lock; }
	void set_axis_lock(bool x) { axis_lock=x; }

	void set_time(synfig::Time x) { cur_time=x; }

	bool is_duck_group_selectable(const etl::handle<Duck>& x)const;

	//const DuckMap& duck_map()const { return duck_map; }
	DuckList get_duck_list()const;

	const std::list<etl::handle<Bezier> >& bezier_list()const { return bezier_list_; }

	const std::list<etl::handle<Stroke> >& stroke_list()const { return stroke_list_; }

	const std::list<etl::handle<Stroke> >& persistent_stroke_list()const { return persistent_stroke_list_; }

	std::list<etl::handle<Stroke> >& persistent_stroke_list() { return persistent_stroke_list_; }

	//! \todo We should modify this to support multiple selections
	etl::handle<Duck> get_selected_duck()const;

	DuckList get_selected_ducks()const;

	//! Returns \a true if the given duck is currently selected
	bool duck_is_selected(const etl::handle<Duck> &duck)const;


	void refresh_selected_ducks();

	void clear_selected_ducks();

	int count_selected_ducks()const;

	void toggle_select_duck(const etl::handle<Duck> &duck);

	void select_duck(const etl::handle<Duck> &duck);

	void toggle_select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br);

	void select_ducks_in_box(const synfig::Vector& tl,const synfig::Vector& br);

	void unselect_duck(const etl::handle<Duck> &duck);

	void start_duck_drag(const synfig::Vector& offset);
	void translate_selected_ducks(const synfig::Vector& vector);
	bool end_duck_drag();

	void signal_edited_selected_ducks();

	void signal_user_click_selected_ducks(int button);


	etl::handle<Duck> find_similar_duck(etl::handle<Duck> duck);
	etl::handle<Duck> add_similar_duck(etl::handle<Duck> duck);

	void add_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color=synfig::Color(0,0,0));

	void add_persistent_stroke(etl::smart_ptr<std::list<synfig::Point> > stroke_point_list, const synfig::Color& color=synfig::Color(0,0,0));

	void clear_persistent_strokes();

	void add_duck(const etl::handle<Duck> &duck);

	void add_bezier(const etl::handle<Bezier> &bezier);

	void erase_duck(const etl::handle<Duck> &duck);

	void erase_bezier(const etl::handle<Bezier> &bezier);

	//! Returns the last duck added
	etl::handle<Duck> last_duck()const;

	etl::handle<Bezier> last_bezier()const;

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Duck> find_duck(synfig::Point pos, synfig::Real radius=0, Duck::Type type=Duck::TYPE_DEFAULT);

	GuideList::iterator find_guide_x(synfig::Point pos, float radius=0.1);
	GuideList::iterator find_guide_y(synfig::Point pos, float radius=0.1);
	GuideList::const_iterator find_guide_x(synfig::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_x(pos,radius); }
	GuideList::const_iterator find_guide_y(synfig::Point pos, float radius=0.1)const { return const_cast<Duckmatic*>(this)->find_guide_y(pos,radius); }

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	//etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real radius=0);

	//! \note parameter is in canvas coordinates
	/*!	A radius of "zero" will have an unlimited radius */
	etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real radius=0, float* location=0);

	etl::handle<Bezier> find_bezier(synfig::Point pos, synfig::Real scale, synfig::Real radius, float* location=0);

	bool add_to_ducks(const synfigapp::ValueDesc& value_desc,etl::handle<CanvasView> canvas_view, const synfig::TransformStack& transform_stack_, synfig::ParamDesc *param_desc=0, int multiple=0);

	//! \writeme
	void set_type_mask(Type x) { type_mask=x; }

	//! \writeme
	Type get_type_mask()const { return type_mask; }

	void select_all_ducks();
	void unselect_all_ducks();

	void clear_ducks();

	bool save_sketch(const synfig::String& filename)const;
	bool load_sketch(const synfig::String& filename);
	const synfig::String& get_sketch_filename()const { return sketch_filename_; }

	void set_duck_dragger(etl::handle<DuckDrag_Base> x) { duck_dragger_=x; }
	etl::handle<DuckDrag_Base> get_duck_dragger()const { return duck_dragger_; }
	void clear_duck_dragger() { duck_dragger_=new DuckDrag_Translate(); }
}; // END of class Duckmatic


/*! \class Duckmatic::Push
**	\writeme */
class Duckmatic::Push
{
	Duckmatic *duckmatic_;
	DuckMap duck_map;
	std::list<etl::handle<Bezier> > bezier_list_;
	std::list<etl::handle<Stroke> > stroke_list_;
	DuckDataMap duck_data_share_map;
	etl::handle<DuckDrag_Base> duck_dragger_;

	bool needs_restore;

public:
	Push(Duckmatic *duckmatic_);
	~Push();
	void restore();
}; // END of class Duckmatic::Push

/*! \struct Duckmatic::Bezier
**	\writeme */
struct Duckmatic::Bezier : public etl::shared_object
{
private:
	sigc::signal<void,float> signal_user_click_[5];
public:

	etl::handle<Duck> p1,p2,c1,c2;
	bool is_valid()const { return p1 && p2 && c1 && c2; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
}; // END of struct Duckmatic::Bezier

/*! \struct Duckmatic::Stroke
**	\writeme */
struct Duckmatic::Stroke : public etl::shared_object
{
private:
	sigc::signal<void,float> signal_user_click_[5];
public:

	etl::smart_ptr<std::list<synfig::Point> > stroke_data;

	synfig::Color color;

	bool is_valid()const { return (bool)stroke_data; }

	sigc::signal<void,float> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }
}; // END of struct Duckmatic::Stroke

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
