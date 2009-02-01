/* === S Y N F I G ========================================================= */
/*!	\file renderer_ducks.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Gerald Young
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

#include "renderer_ducks.h"
#include "workarea.h"
#include "duckmatic.h"
#include <ETL/bezier>
#include <ETL/misc>
#include "widget_color.h"
#include <synfig/distance.h>
#include "app.h"
#ifdef OPENGL_RENDER
	#include "glPlayfield.h"
#endif

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Ducks::~Renderer_Ducks()
{
}

/*
bool
Renderer_Ducks::get_enabled_vfunc()const
{
	return get_work_area()->grid_status();
}
*/

struct ScreenDuck
{
	synfig::Point pos;
	Gdk::Color color;
	bool selected;
	bool hover;
	Real width;

	ScreenDuck():width(0) { }
};

void
Renderer_Ducks::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	const synfig::Vector focus_point(get_work_area()->get_focus_point());


	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(drawable));


	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);

	const float pw(get_pw()),ph(get_ph());

	const std::list<etl::handle<Duckmatic::Bezier> >& bezier_list(get_work_area()->bezier_list());
	const bool solid_lines(get_work_area()->solid_lines);

	const std::list<handle<Duckmatic::Stroke> >& stroke_list(get_work_area()->stroke_list());

	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

#ifdef OPENGL_RENDER
#define ADD_POINT(point) points.push_back(point[0]); points.push_back(point[1]); points.push_back(0);
#define GDK2GL(color) (GLushort)color.get_red(), color.get_green(), color.get_blue()
	glPlayfield *playfield = get_work_area()->get_playfield();
	playfield->setLineWidthGL(1);
	playfield->setFunctionGL(GL_COPY);
#endif

	// Render the strokes
	for(std::list<handle<Duckmatic::Stroke> >::const_iterator iter=stroke_list.begin();iter!=stroke_list.end();++iter)
	{
		Point window_start(window_startx,window_starty);
#ifdef OPENGL_RENDER
		vector<GLfloat> points;
#else
		vector<Gdk::Point> points;
#endif
		std::list<synfig::Point>::iterator iter2;
		Point holder;

		for(iter2=(*iter)->stroke_data->begin();iter2!=(*iter)->stroke_data->end();++iter2)
		{
			holder=*iter2-window_start;
			holder[0]/=pw;holder[1]/=ph;
#ifdef OPENGL_RENDER
			ADD_POINT(holder);
#else
			points.push_back(Gdk::Point(round_to_int(holder[0]),round_to_int(holder[1])));
#endif
		}

#ifdef OPENGL_RENDER
		synfig::Color &c = (*iter)->color;
		// FIXME: Colors are not the same as shown
		//playfield->setColorGL(GDK2GL(colorconv_synfig2gdk((*iter)->color)));
		playfield->setColorGL(c.get_r(), c.get_g(), c.get_b());
		//playfield->setLineWidthGL(1);
		//playfield->setFunctionGL(GL_COPY);
		playfield->drawLines(points);
#else
		gc->set_rgb_fg_color(colorconv_synfig2gdk((*iter)->color));
		gc->set_function(Gdk::COPY);
		gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

		// Draw the stroke
  		drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));
#endif
	}



	// Render the beziers
	for(std::list<handle<Duckmatic::Bezier> >::const_iterator iter=bezier_list.begin();iter!=bezier_list.end();++iter)
	{
		const int NSTEPS = 17;

		Point window_start(window_startx,window_starty);
		Point p1((*iter)->p1->get_trans_point()-window_start);
		Point p2((*iter)->p2->get_trans_point()-window_start);
		Point c1((*iter)->c1->get_trans_point()-window_start);
		Point c2((*iter)->c2->get_trans_point()-window_start);
		p1[0]/=pw;p1[1]/=ph;
		p2[0]/=pw;p2[1]/=ph;
		c1[0]/=pw;c1[1]/=ph;
		c2[0]/=pw;c2[1]/=ph;
#ifdef OPENGL_RENDER
		vector<GLfloat> points;
		ADD_POINT(p1);
		ADD_POINT(c1);
		ADD_POINT(c2);
		ADD_POINT(p2);

		playfield->prepareBezier(points, NSTEPS);
		playfield->setColorGL(GDK2GL(DUCK_COLOR_BEZIER_1));
		//playfield->setFunctionGL(GL_COPY);
		playfield->drawBezier();
		playfield->setColorGL(GDK2GL(DUCK_COLOR_BEZIER_2));
		playfield->drawBezier(GL_LINES);
#else
		bezier<Point> curve(p1,c1,c2,p2);
		vector<Gdk::Point> points;

		float f;
		Point pt;
		for(f=0;f<1.0;f+=1.0/NSTEPS)
		{
			pt=curve(f);
			points.push_back(Gdk::Point(round_to_int(pt[0]),round_to_int(pt[1])));
		}
		points.push_back(Gdk::Point(round_to_int(p2[0]),round_to_int(p2[1])));

		// Draw the curve
/*		if(solid_lines)
		{
			gc->set_rgb_fg_color(DUCK_COLOR_BEZIER_1);
			gc->set_function(Gdk::COPY);
			gc->set_line_attributes(3,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));
			gc->set_rgb_fg_color(DUCK_COLOR_BEZIER_2);
			gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));
		}
		else
*/
		{
//			gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
//			gc->set_function(Gdk::INVERT);
//			gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
//			drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));
			gc->set_rgb_fg_color(DUCK_COLOR_BEZIER_1);
			gc->set_function(Gdk::COPY);
			gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));
			gc->set_rgb_fg_color(DUCK_COLOR_BEZIER_2);
			gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points));

		}
#endif
	}

	const DuckList duck_list(get_work_area()->get_duck_list());
	//Gtk::StateType state = Gtk::STATE_ACTIVE;
	Gtk::ShadowType shadow=Gtk::SHADOW_OUT;
	std::list<ScreenDuck> screen_duck_list;
	const float radius((abs(pw)+abs(ph))*4);

	etl::handle<Duck> hover_duck(get_work_area()->find_duck(get_work_area()->get_cursor_pos(),radius, get_work_area()->get_type_mask()));

	// Render the ducks
	for(std::list<handle<Duck> >::const_iterator iter=duck_list.begin();iter!=duck_list.end();++iter)
	{

		// If this type of duck has been masked, then skip it
		if((*iter)->get_type() && (!(get_work_area()->get_type_mask() & (*iter)->get_type())))
			continue;

//		Real x,y;
	//	Gdk::Rectangle area;
		Point sub_trans_point((*iter)->get_sub_trans_point());
		Point sub_trans_origin((*iter)->get_sub_trans_origin());

		if (App::restrict_radius_ducks &&
			(*iter)->is_radius())
		{
			if (sub_trans_point[0] < sub_trans_origin[0])
				sub_trans_point[0] = sub_trans_origin[0];
			if (sub_trans_point[1] < sub_trans_origin[1])
				sub_trans_point[1] = sub_trans_origin[1];
		}

		Point point((*iter)->get_transform_stack().perform(sub_trans_point));
		Point origin((*iter)->get_transform_stack().perform(sub_trans_origin));

		point[0]=(point[0]-window_startx)/pw;
		point[1]=(point[1]-window_starty)/ph;

		bool has_connect(false);
		if((*iter)->get_tangent() || (*iter)->get_type()&Duck::TYPE_ANGLE)
		{
			has_connect=true;
		}
		if((*iter)->get_connect_duck())
		{
			has_connect=true;
			origin=(*iter)->get_connect_duck()->get_trans_point();
		}

		origin[0]=(origin[0]-window_startx)/pw;
		origin[1]=(origin[1]-window_starty)/ph;

		bool selected(get_work_area()->duck_is_selected(*iter));
		bool hover(*iter==hover_duck || (*iter)->get_hover());

		shadow = selected?Gtk::SHADOW_IN:Gtk::SHADOW_OUT;

		if(get_work_area()->get_selected_value_node())
		{
			synfigapp::ValueDesc value_desc((*iter)->get_value_desc());
			if (value_desc.is_valid() &&
				((value_desc.is_value_node()		&& get_work_area()->get_selected_value_node() == value_desc.get_value_node()) ||
				 (value_desc.parent_is_value_node()	&& get_work_area()->get_selected_value_node() == value_desc.get_parent_value_node())))
			{
#ifdef OPENGL_RENDER
				playfield->setColorGL(GDK2GL(DUCK_COLOR_SELECTED));
				playfield->setFunctionGL(GL_COPY);
				playfield->setLineWidthGL(2);
				playfield->drawRectangle(point[0] - 5, point[1] - 5, point[0] + 5, point[1] + 5);
#else
				gc->set_function(Gdk::COPY);
				gc->set_rgb_fg_color(DUCK_COLOR_SELECTED);
				//gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				gc->set_line_attributes(2,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

				drawable->draw_rectangle(gc,false,
					round_to_int(point[0]-5),
					round_to_int(point[1]-5),
					10,
					10
				);
#endif
			}

		}

		if((*iter)->get_box_duck())
		{
			Point boxpoint((*iter)->get_box_duck()->get_trans_point());
			boxpoint[0]=(boxpoint[0]-window_startx)/pw;
			boxpoint[1]=(boxpoint[1]-window_starty)/ph;
			Point tl(min(point[0],boxpoint[0]),min(point[1],boxpoint[1]));

#ifdef OPENGL_RENDER
#else
			gc->set_function(Gdk::COPY);
			gc->set_rgb_fg_color(DUCK_COLOR_BOX_1);
			gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_rectangle(gc,false,
				round_to_int(tl[0]),
				round_to_int(tl[1]),
				round_to_int(abs(boxpoint[0]-point[0])),
				round_to_int(abs(boxpoint[1]-point[1]))
			);
			gc->set_function(Gdk::COPY);
			gc->set_rgb_fg_color(DUCK_COLOR_BOX_2);
			gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
			drawable->draw_rectangle(gc,false,
				round_to_int(tl[0]),
				round_to_int(tl[1]),
				round_to_int(abs(boxpoint[0]-point[0])),
				round_to_int(abs(boxpoint[1]-point[1]))
			);
#endif
		}

		ScreenDuck screen_duck;
		screen_duck.pos=point;
		screen_duck.selected=selected;
		screen_duck.hover=hover;

		if(!(*iter)->get_editable())
			screen_duck.color=(DUCK_COLOR_NOT_EDITABLE);
		else if((*iter)->get_tangent())
			screen_duck.color=((*iter)->get_scalar()<0 ? DUCK_COLOR_TANGENT_1 : DUCK_COLOR_TANGENT_2);
		else if((*iter)->get_type()&Duck::TYPE_VERTEX)
			screen_duck.color=DUCK_COLOR_VERTEX;
		else if((*iter)->get_type()&Duck::TYPE_RADIUS)
			screen_duck.color=DUCK_COLOR_RADIUS;
		else if((*iter)->get_type()&Duck::TYPE_WIDTH)
			screen_duck.color=DUCK_COLOR_WIDTH;
		else if((*iter)->get_type()&Duck::TYPE_ANGLE)
			screen_duck.color=(DUCK_COLOR_ANGLE);
		else
			screen_duck.color=DUCK_COLOR_OTHER;

		screen_duck_list.push_front(screen_duck);

		if(has_connect)
		{
			if(solid_lines)
			{
#ifdef OPENGL_RENDER
				playfield->setColorGL(GDK2GL(DUCK_COLOR_CONNECT_OUTSIDE));
				playfield->setLineWidthGL(3);
				playfield->setFunctionGL(GL_COPY);
				playfield->drawLine(origin[0], origin[1], point[0], point[1]);
				playfield->setColorGL(GDK2GL(DUCK_COLOR_CONNECT_INSIDE));
				playfield->setLineWidthGL(1);
				playfield->drawLine(origin[0], origin[1], point[0], point[1]);
#else
				gc->set_line_attributes(3,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				gc->set_rgb_fg_color(DUCK_COLOR_CONNECT_OUTSIDE);
				gc->set_function(Gdk::COPY);
				drawable->draw_line(gc, (int)origin[0],(int)origin[1],(int)(point[0]),(int)(point[1]));
				gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				gc->set_rgb_fg_color(DUCK_COLOR_CONNECT_INSIDE);
				drawable->draw_line(gc, (int)origin[0],(int)origin[1],(int)(point[0]),(int)(point[1]));
#endif
			}
			else
			{
#ifdef OPENGL_RENDER
#else
//				gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
//				gc->set_function(Gdk::INVERT);
//				drawable->draw_line(gc, (int)origin[0],(int)origin[1],(int)(point[0]),(int)(point[1]));
				gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				gc->set_rgb_fg_color(DUCK_COLOR_CONNECT_OUTSIDE);
				gc->set_function(Gdk::COPY);
				drawable->draw_line(gc, (int)origin[0],(int)origin[1],(int)(point[0]),(int)(point[1]));
				gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				gc->set_rgb_fg_color(DUCK_COLOR_CONNECT_INSIDE);
				drawable->draw_line(gc, (int)origin[0],(int)origin[1],(int)(point[0]),(int)(point[1]));
#endif
			}
		}

		if((*iter)->is_radius())
		{
			const Real mag((point-origin).mag());
#ifdef OPENGL_RENDER
			if(solid_lines)
			{
				playfield->setColorGL(0.0f, 0.0f, 0.0f);
				playfield->setLineWidthGL(3);
				playfield->setFunctionGL(GL_COPY);
				playfield->drawCircle(origin[0], origin[1], mag);
				playfield->setColorGL((GLubyte)0xAF, 0xAF, 0xAF);
			}
			else
			{
				playfield->setColorGL(1.0f, 1.0f, 1.0f);
				playfield->setFunctionGL(GL_INVERT);
			}
			playfield->setLineWidthGL(1);
			playfield->drawCircle(origin[0], origin[1], mag);
#else
			const int d(round_to_int(mag*2));
			const int x(round_to_int(origin[0]-mag));
			const int y(round_to_int(origin[1]-mag));

			if(solid_lines)
			{
				gc->set_rgb_fg_color(Gdk::Color("#000000"));
				gc->set_function(Gdk::COPY);
				gc->set_line_attributes(3,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
				drawable->draw_arc(
					gc,
					false,
					x,
					y,
					d,
					d,
					0,
					360*64
				);
				gc->set_rgb_fg_color(Gdk::Color("#afafaf"));
			}
			else
			{
				gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
				gc->set_function(Gdk::INVERT);
			}
			gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

			drawable->draw_arc(
				gc,
				false,
				x,
				y,
				d,
				d,
				0,
				360*64
			);
#endif

			if(hover)
			{
				Real mag;
				if (App::restrict_radius_ducks)
				{
					Point sub_trans_point((*iter)->get_sub_trans_point());
					Point sub_trans_origin((*iter)->get_sub_trans_origin());

					if (sub_trans_point[0] < sub_trans_origin[0])
						sub_trans_point[0] = sub_trans_origin[0];
					if (sub_trans_point[1] < sub_trans_origin[1])
						sub_trans_point[1] = sub_trans_origin[1];

					Point point((*iter)->get_transform_stack().perform(sub_trans_point));
					Point origin((*iter)->get_transform_stack().perform(sub_trans_origin));

					mag = (point-origin).mag();
				}
				else
					mag = ((*iter)->get_trans_point()-(*iter)->get_trans_origin()).mag();

				Distance real_mag(mag, Distance::SYSTEM_UNITS);
				real_mag.convert(App::distance_system,get_work_area()->get_rend_desc());
#ifdef OPENGL_RENDER
				playfield->setColorGL(GDK2GL(DUCK_COLOR_WIDTH_TEXT_1));
				playfield->drawText(real_mag.get_string().c_str(), point[0]+1+6, point[1]+1+8);
				playfield->setColorGL(GDK2GL(DUCK_COLOR_WIDTH_TEXT_2));
				playfield->drawText(real_mag.get_string().c_str(), point[0]+6, point[1]+8);
#else
				layout->set_text(real_mag.get_string());

				gc->set_rgb_fg_color(DUCK_COLOR_WIDTH_TEXT_1);
				drawable->draw_layout(
					gc,
					round_to_int(point[0])+1+6,
					round_to_int(point[1])+1-8,
					layout
				);
				gc->set_rgb_fg_color(DUCK_COLOR_WIDTH_TEXT_2);
				drawable->draw_layout(
					gc,
					round_to_int(point[0])+6,
					round_to_int(point[1])-8,
					layout
				);
#endif
			}

		}

	}


	for(;screen_duck_list.size();screen_duck_list.pop_front())
	{
		int radius=4;
		int outline=1;
		Gdk::Color color(screen_duck_list.front().color);

		if(!screen_duck_list.front().selected)
		{
			color.set_red(color.get_red()*2/3);
			color.set_green(color.get_green()*2/3);
			color.set_blue(color.get_blue()*2/3);
		}

		if(screen_duck_list.front().hover && !screen_duck_list.back().hover && screen_duck_list.size()>1)
		{
			screen_duck_list.push_back(screen_duck_list.front());
			continue;
		}

		if(screen_duck_list.front().hover)
		{
			radius+=2;
			outline++;
		}

#ifdef OPENGL_RENDER
		playfield->setFunctionGL(GL_COPY);
		playfield->setColorGL(GDK2GL(DUCK_COLOR_OUTLINE));
		playfield->setPointSizeGL(radius*2);
		playfield->drawPoint(screen_duck_list.front().pos[0], screen_duck_list.front().pos[1]);
		playfield->setColorGL(GDK2GL(color));
		playfield->setPointSizeGL((radius-outline)*2);
		playfield->drawPoint(screen_duck_list.front().pos[0], screen_duck_list.front().pos[1]);
#else
		gc->set_function(Gdk::COPY);
		gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
		gc->set_rgb_fg_color(DUCK_COLOR_OUTLINE);
		drawable->draw_arc(
			gc,
			true,
			round_to_int(screen_duck_list.front().pos[0]-radius),
			round_to_int(screen_duck_list.front().pos[1]-radius),
			radius*2,
			radius*2,
			0,
			360*64
		);


		gc->set_rgb_fg_color(color);

		drawable->draw_arc(
			gc,
			true,
			round_to_int(screen_duck_list.front().pos[0]-radius+outline),
			round_to_int(screen_duck_list.front().pos[1]-radius+outline),
			radius*2-outline*2,
			radius*2-outline*2,
			0,
			360*64
		);
#endif
	}
}
