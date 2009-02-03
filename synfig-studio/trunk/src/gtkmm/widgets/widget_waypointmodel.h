/* === S Y N F I G ========================================================= */
/*!	\file widget_waypointmodel.h
**	\brief Template Header
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_WIDGET_WAYPOINTMODEL_H
#define __SYNFIG_GTKMM_WIDGET_WAYPOINTMODEL_H

/* === H E A D E R S ======================================================= */

//#include <gtk/gtk.h>
//#include <gtkmm/arrow.h>
//#include <gtkmm/image.h>
//#include <gdkmm/pixbufloader.h>
//#include <gtkmm/viewport.h>
//#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
//#include <gtkmm/progressbar.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/combo.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>


#include <synfig/waypoint.h>
#include <synfig/synfig_string.h>
#include <synfig/synfig_time.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Time;
class Widget_WaypointModel;

class Widget_WaypointModel : public Gtk::Table
{
	synfig::Waypoint::Model waypoint_model;

	bool updating;

	Gtk::Combo *in,*out;
	Gtk::OptionMenu *before, *after;
	Gtk::Menu *before_options,*after_options;

	Gtk::SpinButton *spin_tension, *spin_continuity, *spin_bias, *spin_temporal_tension;
	Gtk::Adjustment adj_tension, adj_continuity, adj_bias, adj_temporal_tension;

	Gtk::CheckButton checkbutton_after;
	Gtk::CheckButton checkbutton_before;
	Gtk::CheckButton checkbutton_tension;
	Gtk::CheckButton checkbutton_continuity;
	Gtk::CheckButton checkbutton_bias;
	Gtk::CheckButton checkbutton_temporal_tension;

	void on_change();

public:
	Widget_WaypointModel();
	void set_waypoint_model(synfig::Waypoint::Model &x);
	const synfig::Waypoint::Model &get_waypoint_model()const { return waypoint_model; }
}; // END of class Widget_WaypointModel

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
