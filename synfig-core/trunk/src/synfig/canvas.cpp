/* === S Y N F I G ========================================================= */
/*!	\file canvas.cpp
**	\brief Canvas Class Member Definitions
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

/* === H E A D E R S ======================================================= */

#define SYNFIG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layer.h"
#include "canvas.h"
#include <cassert>
#include "exception.h"
#include "synfig_time.h"
#include "context.h"
#include "layers/layer_pastecanvas.h"
#include "loadcanvas.h"
#include <sigc++/bind.h>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

namespace synfig { extern Canvas::Handle open_canvas(const String &filename, String &errors, String &warnings); };

/* === M A C R O S ========================================================= */

#define ALLOW_CLONE_NON_INLINE_CANVASES

struct _CanvasCounter
{
	static int counter;
	~_CanvasCounter()
	{
		if(counter)
			synfig::error("%d canvases not yet deleted!",counter);
	}
} _canvas_counter;

int _CanvasCounter::counter(0);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Canvas::Canvas(const String &id):
	id_			(id),
	version_	(CURRENT_CANVAS_VERSION),
	cur_time_	(0),
	is_inline_	(false),
	is_dirty_	(true),
	op_flag_	(false)
{
	_CanvasCounter::counter++;
	clear();
}

void
Canvas::on_changed()
{
	is_dirty_=true;
	Node::on_changed();
}

Canvas::~Canvas()
{
	// we were having a crash where pastecanvas layers were still
	// refering to a canvas after it had been destroyed;  this code
	// will stop the pastecanvas layers from refering to the canvas
	// before the canvas is destroyed

	// the set_sub_canvas(0) ends up deleting the parent-child link,
	// which deletes the current element from the set we're iterating
	// through, so we have to make sure we've incremented the iterator
	// before we mess with the pastecanvas
	std::set<Node*>::iterator iter = parent_set.begin();
	while (iter != parent_set.end())
	{
		Layer_PasteCanvas* paste_canvas = dynamic_cast<Layer_PasteCanvas*>(*iter);
		iter++;
		if(paste_canvas)
			paste_canvas->set_sub_canvas(0);
		else
			warning("destroyed canvas has a parent that is not a pastecanvas - please report if repeatable");
	}

	//if(is_inline() && parent_) assert(0);
	_CanvasCounter::counter--;
	clear();
	begin_delete();
}

Canvas::iterator
Canvas::end()
{
	return CanvasBase::end()-1;
}

Canvas::const_iterator
Canvas::end()const
{
	return CanvasBase::end()-1;
}

Canvas::reverse_iterator
Canvas::rbegin()
{
	return CanvasBase::rbegin()+1;
}

Canvas::const_reverse_iterator
Canvas::rbegin()const
{
	return CanvasBase::rbegin()+1;
}

int
Canvas::size()const
{
	return CanvasBase::size()-1;
}

void
Canvas::clear()
{
	while(!empty())
	{
		Layer::Handle layer(front());
		//if(layer->count()>2)synfig::info("before layer->count()=%d",layer->count());

		erase(begin());
		//if(layer->count()>1)synfig::info("after layer->count()=%d",layer->count());
	}
	//CanvasBase::clear();

	// We need to keep a blank handle at the
	// end of the image list, and acts at
	// the bottom. Without it, the layers
	// would just continue going when polled
	// for a color.
	CanvasBase::push_back(Layer::Handle());

	changed();
}

bool
Canvas::empty()const
{
	return CanvasBase::size()<=1;
}

Layer::Handle &
Canvas::back()
{
	return *(CanvasBase::end()-1);
}

const Layer::Handle &
Canvas::back()const
{
	return *(CanvasBase::end()-1);
}

Context
Canvas::get_context()const
{
	return begin();
}

const ValueNodeList &
Canvas::value_node_list()const
{
	if(is_inline() && parent_)
		return parent_->value_node_list();
	return value_node_list_;
}

KeyframeList &
Canvas::keyframe_list()
{
	if(is_inline() && parent_)
		return parent_->keyframe_list();
	return keyframe_list_;
}

const KeyframeList &
Canvas::keyframe_list()const
{
	if(is_inline() && parent_)
		return parent_->keyframe_list();
	return keyframe_list_;
}

etl::handle<Layer>
Canvas::find_layer(const Point &pos)
{
	return get_context().hit_check(pos);
}

static bool
valid_id(const String &x)
{
	static const char bad_chars[]=" :#@$^&()*";
	unsigned int i;

	if(!x.empty() && x[0]>='0' && x[0]<='9')
		return false;

	for(i=0;i<sizeof(bad_chars);i++)
		if(x.find_first_of(bad_chars[i])!=string::npos)
			return false;

	return true;
}

void
Canvas::set_id(const String &x)
{
	if(is_inline() && parent_)
		throw runtime_error("Inline Canvas cannot have an ID");

	if(!valid_id(x))
		throw runtime_error("Invalid ID");
	id_=x;
	signal_id_changed_();
}

void
Canvas::set_name(const String &x)
{
	name_=x;
	signal_meta_data_changed()("name");
	signal_meta_data_changed("name")();
}

void
Canvas::set_author(const String &x)
{
	author_=x;
	signal_meta_data_changed()("author");
	signal_meta_data_changed("author")();
}

void
Canvas::set_description(const String &x)
{
	description_=x;
	signal_meta_data_changed()("description");
	signal_meta_data_changed("description")();
}

void
Canvas::set_time(Time t)const
{
	if(is_dirty_ || !get_time().is_equal(t))
	{
#if 0
		if(is_root())
		{
			synfig::info("is_dirty_=%d",is_dirty_);
			synfig::info("get_time()=%f",(float)get_time());
			synfig::info("t=%f",(float)t);
		}
#endif

		// ...questionable
		const_cast<Canvas&>(*this).cur_time_=t;

		is_dirty_=false;
		get_context().set_time(t);
	}
	is_dirty_=false;
}

Canvas::LooseHandle
Canvas::get_root()const
{
	return parent_?parent_->get_root().get():const_cast<synfig::Canvas *>(this);
}

int
Canvas::get_depth(etl::handle<Layer> layer)const
{
	const_iterator iter;
	int i(0);
	for(iter=begin();iter!=end();++iter,i++)
	{
		if(layer==*iter)
			return i;
	}
	return -1;
}

String
Canvas::get_relative_id(etl::loose_handle<const Canvas> x)const
{
	if(x->get_root()==this)
		return ":";
	if(is_inline() && parent_)
		return parent_->_get_relative_id(x);
	return _get_relative_id(x);
}

String
Canvas::_get_relative_id(etl::loose_handle<const Canvas> x)const
{
	if(is_inline() && parent_)
		return parent_->_get_relative_id(x);

	if(x.get()==this)
		return String();

	if(parent()==x.get())
		return get_id();

	String id;

	const Canvas* canvas=this;

	for(;!canvas->is_root();canvas=canvas->parent().get())
		id=':'+canvas->get_id()+id;

	if(x && get_root()!=x->get_root())
	{
		//String file_name=get_file_name();
		//String file_path=x->get_file_path();

		String file_name;
		if(is_absolute_path(get_file_name()))
			file_name=etl::relative_path(x->get_file_path(),get_file_name());
		else
			file_name=get_file_name();

		// If the path of X is inside of file_name,
		// then remove it.
		//if(file_name.size()>file_path.size())
		//	if(file_path==String(file_name,0,file_path.size()))
		//		file_name.erase(0,file_path.size()+1);

		id=file_name+'#'+id;
	}

	return id;
}

ValueNode::Handle
Canvas::find_value_node(const String &id)
{
	return
		ValueNode::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_value_node(id)
		);
}

ValueNode::ConstHandle
Canvas::find_value_node(const String &id)const
{
	if(is_inline() && parent_)
		return parent_->find_value_node(id);

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos && id.find_first_of('#')==string::npos)
		return value_node_list_.find(id);

	String canvas_id(id,0,id.rfind(':'));
	String value_node_id(id,id.rfind(':')+1);
	if(canvas_id.empty())
		canvas_id=':';
	//synfig::warning("constfind:value_node_id: "+value_node_id);
	//synfig::warning("constfind:canvas_id: "+canvas_id);

	String warnings;
	return find_canvas(canvas_id, warnings)->value_node_list_.find(value_node_id);
}

ValueNode::Handle
Canvas::surefind_value_node(const String &id)
{
	if(is_inline() && parent_)
		return parent_->surefind_value_node(id);

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos && id.find_first_of('#')==string::npos)
		return value_node_list_.surefind(id);

	String canvas_id(id,0,id.rfind(':'));
	String value_node_id(id,id.rfind(':')+1);
	if(canvas_id.empty())
		canvas_id=':';

	String warnings;
	return surefind_canvas(canvas_id,warnings)->value_node_list_.surefind(value_node_id);
}

void
Canvas::add_value_node(ValueNode::Handle x, const String &id)
{
	if(is_inline() && parent_)
		return parent_->add_value_node(x,id);
//		throw runtime_error("You cannot add a ValueNode to an inline Canvas");

	if(x->is_exported())
		throw runtime_error("ValueNode is already exported");

	if(id.empty())
		throw Exception::BadLinkName("Empty ID");

	if(id.find_first_of(':',0)!=string::npos)
		throw Exception::BadLinkName("Bad character");

	try
	{
		if(PlaceholderValueNode::Handle::cast_dynamic(value_node_list_.find(id)))
			throw Exception::IDNotFound("add_value_node()");

		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound)
	{
		x->set_id(id);

		x->set_parent_canvas(this);

		if(!value_node_list_.add(x))
		{
			synfig::error("Unable to add ValueNode");
			throw std::runtime_error("Unable to add ValueNode");
		}

		return;
	}
}

/*
void
Canvas::rename_value_node(ValueNode::Handle x, const String &id)
{
	if(id.empty())
		throw Exception::BadLinkName("Empty ID");

	if(id.find_first_of(": ",0)!=string::npos)
		throw Exception::BadLinkName("Bad character");

	try
	{
		if(PlaceholderValueNode::Handle::cast_dynamic(value_node_list_.find(id)))
			throw Exception::IDNotFound("rename_value_node");
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound)
	{
		x->set_id(id);

		return;
	}
}
*/

void
Canvas::remove_value_node(ValueNode::Handle x)
{
	if(is_inline() && parent_)
		return parent_->remove_value_node(x);
//		throw Exception::IDNotFound("Canvas::remove_value_node() was called from an inline canvas");

	if(!x)
		throw Exception::IDNotFound("Canvas::remove_value_node() was passed empty handle");

	if(!value_node_list_.erase(x))
		throw Exception::IDNotFound("Canvas::remove_value_node(): ValueNode was not found inside of this canvas");

	//x->set_parent_canvas(0);

	x->set_id("");
}

Canvas::Handle
Canvas::surefind_canvas(const String &id, String &warnings)
{
	if(is_inline() && parent_)
		return parent_->surefind_canvas(id,warnings);

	if(id.empty())
		return this;

	// If the ID contains a "#" character, then a filename is
	// expected on the left side.
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return surefind_canvas(String(id,1),warnings);

		//! \todo This needs a lot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);

		file_name=unix_to_local_path(file_name);

		Canvas::Handle external_canvas;

		if(!is_absolute_path(file_name))
			file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;

		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			String errors;
			external_canvas=open_canvas(file_name, errors, warnings);
			if(!external_canvas)
				throw runtime_error(errors);
			externals_[file_name]=external_canvas;
		}

		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id, warnings));
	}

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos)
	{
		Children::iterator iter;

		// Search for the image in the image list,
		// and return it if it is found
		for(iter=children().begin();iter!=children().end();iter++)
			if(id==(*iter)->get_id())
				return *iter;

		// Create a new canvas and return it
		//synfig::warning("Implicitly creating canvas named "+id);
		return new_child_canvas(id);
	}

	// If the first character is the separator, then
	// this references the root canvas.
	if(id[0]==':')
		return get_root()->surefind_canvas(string(id,1),warnings);

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and
	// call "find_canvas" on it, and return the result.

	String canvas_name=string(id,0,id.find_first_of(':'));

	Canvas::Handle child_canvas=surefind_canvas(canvas_name,warnings);

	return child_canvas->surefind_canvas(string(id,id.find_first_of(':')+1),warnings);
}

Canvas::Handle
Canvas::find_canvas(const String &id, String &warnings)
{
	return
		Canvas::Handle::cast_const(
			const_cast<const Canvas*>(this)->find_canvas(id, warnings)
		);
}

Canvas::ConstHandle
Canvas::find_canvas(const String &id, String &warnings)const
{
	if(is_inline() && parent_)
		return parent_->find_canvas(id, warnings);

	if(id.empty())
		return this;

	// If the ID contains a "#" character, then a filename is
	// expected on the left side.
	if(id.find_first_of('#')!=string::npos)
	{
		// If '#' is the first character, remove it
		// and attempt to parse the ID again.
		if(id[0]=='#')
			return find_canvas(String(id,1), warnings);

		//! \todo This needs a lot more optimization
		String file_name(id,0,id.find_first_of('#'));
		String external_id(id,id.find_first_of('#')+1);

		file_name=unix_to_local_path(file_name);

		Canvas::Handle external_canvas;

		if(!is_absolute_path(file_name))
			file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;

		// If the composition is already open, then use it.
		if(externals_.count(file_name))
			external_canvas=externals_[file_name];
		else
		{
			String errors, warnings;
			external_canvas=open_canvas(file_name, errors, warnings);
			if(!external_canvas)
				throw runtime_error(errors);
			externals_[file_name]=external_canvas;
		}

		return Handle::cast_const(external_canvas.constant()->find_canvas(external_id, warnings));
	}

	// If we do not have any resolution, then we assume that the
	// request is for this immediate canvas
	if(id.find_first_of(':')==string::npos)
	{
		Children::const_iterator iter;

		// Search for the image in the image list,
		// and return it if it is found
		for(iter=children().begin();iter!=children().end();iter++)
			if(id==(*iter)->get_id())
				return *iter;

		throw Exception::IDNotFound("Child Canvas in Parent Canvas: (child)"+id);
	}

	// If the first character is the separator, then
	// this references the root canvas.
	if(id[0]==':')
		return get_root()->find_canvas(string(id,1), warnings);

	// Now we know that the requested Canvas is in a child
	// of this canvas. We have to find that canvas and
	// call "find_canvas" on it, and return the result.

	String canvas_name=string(id,0,id.find_first_of(':'));

	Canvas::ConstHandle child_canvas=find_canvas(canvas_name, warnings);

	return child_canvas->find_canvas(string(id,id.find_first_of(':')+1), warnings);
}

Canvas::Handle
Canvas::create()
{
	return new Canvas("Untitled");
}

void
Canvas::push_back(etl::handle<Layer> x)
{
//	int i(x->count());
	insert(end(),x);
	//if(x->count()!=i+1)synfig::info("push_back before %d, after %d",i,x->count());
}

void
Canvas::push_front(etl::handle<Layer> x)
{
//	int i(x->count());
	insert(begin(),x);
	//if(x->count()!=i+1)synfig::error("push_front before %d, after %d",i,x->count());
}

void
Canvas::insert(iterator iter,etl::handle<Layer> x)
{
//	int i(x->count());
	CanvasBase::insert(iter,x);

	/*if(x->count()!=i+1)
	{
		synfig::error(__FILE__":%d: Canvas::insert(): ***FAILURE*** before %d, after %d",__LINE__,i,x->count());
		return;
		//throw runtime_error("Canvas Insertion Failed");
	}*/

	x->set_canvas(this);

	add_child(x.get());

	LooseHandle correct_canvas(this);
	//while(correct_canvas->is_inline())correct_canvas=correct_canvas->parent();
	Layer::LooseHandle loose_layer(x);

	add_connection(loose_layer,
				   sigc::connection::connection(
					   x->signal_added_to_group().connect(
						   sigc::bind(
							   sigc::mem_fun(
								   *correct_canvas,
								   &Canvas::add_group_pair),
							   loose_layer))));
	add_connection(loose_layer,
				   sigc::connection::connection(
					   x->signal_removed_from_group().connect(
						   sigc::bind(
							   sigc::mem_fun(
								   *correct_canvas,
								   &Canvas::remove_group_pair),
							   loose_layer))));

	if(!x->get_group().empty())
		add_group_pair(x->get_group(),x);

	changed();
}

void
Canvas::push_back_simple(etl::handle<Layer> x)
{
	CanvasBase::insert(end(),x);
	changed();
}

void
Canvas::erase(iterator iter)
{
	if(!(*iter)->get_group().empty())
		remove_group_pair((*iter)->get_group(),(*iter));

	// HACK: We really shouldn't be wiping
	// out these signals entirely. We should
	// only be removing the specific connections
	// that we made. At the moment, I'm too
	// lazy to add the code to keep track
	// of those connections, and no one else
	// is using these signals, so I'll just
	// leave these next two lines like they
	// are for now - darco 07-30-2004

	// so don't wipe them out entirely
	// - dooglus 09-21-2007
	disconnect_connections(*iter);

	if(!op_flag_)remove_child(iter->get());

	CanvasBase::erase(iter);
	if(!op_flag_)changed();
}

Canvas::Handle
Canvas::clone(const GUID& deriv_guid)const
{
	synfig::String name;
	if(is_inline())
		name="inline";
	else
	{
		name=get_id()+"_CLONE";

#ifndef ALLOW_CLONE_NON_INLINE_CANVASES
		throw runtime_error("Cloning of non-inline canvases is not yet supported");
#endif	// ALLOW_CLONE_NON_INLINE_CANVASES
	}

	Handle canvas(new Canvas(name));

	if(is_inline())
	{
		canvas->is_inline_=true;
		// \todo this was setting parent_=0 - is there a reason for that?
		// this was causing bug 1838132, where cloning an inline canvas that contains an imported image fails
		// it was failing to ascertain the absolute pathname of the imported image, since it needs the pathname
		// of the canvas to get that, which is stored in the parent canvas
		canvas->parent_=parent();
		canvas->rend_desc() = rend_desc();
		//canvas->set_inline(parent());
	}

	canvas->set_guid(get_guid()^deriv_guid);

	const_iterator iter;
	for(iter=begin();iter!=end();++iter)
	{
		Layer::Handle layer((*iter)->clone(deriv_guid));
		if(layer)
		{
			assert(layer.count()==1);
			int presize(size());
			canvas->push_back(layer);
			if(!(layer.count()>1))
			{
				synfig::error("Canvas::clone(): Cloned layer insertion failure!");
				synfig::error("Canvas::clone(): \tlayer.count()=%d",layer.count());
				synfig::error("Canvas::clone(): \tlayer->get_name()=%s",layer->get_name().c_str());
				synfig::error("Canvas::clone(): \tbefore size()=%d",presize);
				synfig::error("Canvas::clone(): \tafter size()=%d",size());
			}
			assert(layer.count()>1);
		}
		else
		{
			synfig::error("Unable to clone layer");
		}
	}

	canvas->signal_group_pair_removed().clear();
	canvas->signal_group_pair_added().clear();

	return canvas;
}

void
Canvas::set_inline(LooseHandle parent)
{
	if(is_inline_ && parent_)
	{

	}

	id_="inline";
	is_inline_=true;
	parent_=parent;

	// Have the parent inherit all of the group stuff

	std::map<String,std::set<etl::handle<Layer> > >::const_iterator iter;

	for(iter=group_db_.begin();iter!=group_db_.end();++iter)
	{
		parent->group_db_[iter->first].insert(iter->second.begin(),iter->second.end());
	}

	rend_desc()=parent->rend_desc();
}

Canvas::Handle
Canvas::create_inline(Handle parent)
{
	assert(parent);
	//if(parent->is_inline())
	//	return create_inline(parent->parent());

	Handle canvas(new Canvas("inline"));
	canvas->set_inline(parent);
	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas()
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas();
//		runtime_error("You cannot create a child Canvas in an inline Canvas");

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());

	canvas->parent_=this;

	canvas->rend_desc()=rend_desc();

	return canvas;
}

Canvas::Handle
Canvas::new_child_canvas(const String &id)
{
	if(is_inline() && parent_)
		return parent_->new_child_canvas(id);
//		runtime_error("You cannot create a child Canvas in an inline Canvas");

	// Create a new canvas
	children().push_back(create());
	Canvas::Handle canvas(children().back());

	canvas->set_id(id);
	canvas->parent_=this;
	canvas->rend_desc()=rend_desc();

	return canvas;
}

Canvas::Handle
Canvas::add_child_canvas(Canvas::Handle child_canvas, const synfig::String& id)
{
	if(is_inline() && parent_)
		return parent_->add_child_canvas(child_canvas,id);

	if(child_canvas->parent() && !child_canvas->is_inline())
		throw std::runtime_error("Cannot add child canvas because it belongs to someone else!");

	if(!valid_id(id))
		throw runtime_error("Invalid ID");

	try
	{
		String warnings;
		find_canvas(id, warnings);
		throw Exception::IDAlreadyExists(id);
	}
	catch(Exception::IDNotFound)
	{
		if(child_canvas->is_inline())
			child_canvas->is_inline_=false;
		child_canvas->id_=id;
		children().push_back(child_canvas);
		child_canvas->parent_=this;
	}

	return child_canvas;
}

void
Canvas::remove_child_canvas(Canvas::Handle child_canvas)
{
	if(is_inline() && parent_)
		return parent_->remove_child_canvas(child_canvas);

	if(child_canvas->parent_!=this)
		throw runtime_error("Given child does not belong to me");

	if(find(children().begin(),children().end(),child_canvas)==children().end())
		throw Exception::IDNotFound(child_canvas->get_id());

	children().remove(child_canvas);

	child_canvas->parent_=0;
}

void
Canvas::set_file_name(const String &file_name)
{
	if(parent())
		parent()->set_file_name(file_name);
	else
	{
		String old_name(file_name_);
		file_name_=file_name;

		// when a canvas is made, its name is ""
		// then, before it's saved or even edited, it gets a name like "Synfig Animation 23", in the local language
		// we don't want to register the canvas' filename in the canvas map until it gets a real filename
		if (old_name != "")
		{
			file_name_=file_name;
			std::map<synfig::String, etl::loose_handle<Canvas> >::iterator iter;
			for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
				if(iter->second==this)
					break;
			if (iter == get_open_canvas_map().end())
				CanvasParser::register_canvas_in_map(this, file_name);
			else
				signal_file_name_changed_();
		}
	}
}

sigc::signal<void>&
Canvas::signal_file_name_changed()
{
	if(parent())
		return parent()->signal_file_name_changed();
	else
		return signal_file_name_changed_;
}

String
Canvas::get_file_name()const
{
	if(parent())
		return parent()->get_file_name();
	return file_name_;
}

String
Canvas::get_file_path()const
{
	if(parent())
		return parent()->get_file_path();
	return dirname(file_name_);
}

String
Canvas::get_meta_data(const String& key)const
{
	if(!meta_data_.count(key))
		return String();
	return meta_data_.find(key)->second;
}

void
Canvas::set_meta_data(const String& key, const String& data)
{
	if(meta_data_[key]!=data)
	{
		meta_data_[key]=data;
		signal_meta_data_changed()(key);
		signal_meta_data_changed(key)();
	}
}

void
Canvas::erase_meta_data(const String& key)
{
	if(meta_data_.count(key))
	{
		meta_data_.erase(key);
		signal_meta_data_changed()(key);
		signal_meta_data_changed(key)();
	}
}

std::list<String>
Canvas::get_meta_data_keys()const
{
	std::list<String> ret;

	std::map<String,String>::const_iterator iter;

	for(iter=meta_data_.begin();!(iter==meta_data_.end());++iter)
		ret.push_back(iter->first);

	return ret;
}

/* note - the "Motion Blur" and "Duplicate" layers need the dynamic
		  parameters of any PasteCanvas layers they loop over to be
		  maintained.  When the variables in the following function
		  refer to "motion blur", they mean either of these two
		  layers. */
void
synfig::optimize_layers(Time time, Context context, Canvas::Handle op_canvas, bool seen_motion_blur_in_parent)
{
	Context iter;

	std::vector< std::pair<float,Layer::Handle> > sort_list;
	int i, motion_blur_i=0;	// motion_blur_i is for resolving which layer comes first in the event of a z_depth tie
	float motion_blur_z_depth=0; // the z_depth of the least deep motion blur layer in this context
	bool seen_motion_blur_locally = false;
	bool motion_blurred; // the final result - is this layer blurred or not?

	// If the parent didn't cause us to already be motion blurred,
	// check whether there's a motion blur in this context,
	// and if so, calculate its z_depth.
	if (!seen_motion_blur_in_parent)
		for(iter=context,i=0;*iter;iter++,i++)
		{
			Layer::Handle layer=*iter;

			// If the layer isn't active, don't worry about it
			if(!layer->active())
				continue;

			// Any layer with an amount of zero is implicitly disabled.
			ValueBase value(layer->get_param("amount"));
			if(value.get_type()==ValueBase::TYPE_REAL && value.get(Real())==0)
				continue;

			if(layer->get_name()=="MotionBlur" || layer->get_name()=="duplicate")
			{
				float z_depth(layer->get_z_depth()*1.0001+i);

				// If we've seen a motion blur before in this context...
				if (seen_motion_blur_locally)
				{
					// ... then we're only interested in this one if it's less deep...
					if (z_depth < motion_blur_z_depth)
					{
						motion_blur_z_depth = z_depth;
						motion_blur_i = i;
					}
				}
				// ... otherwise we're always interested in it.
				else
				{
					motion_blur_z_depth = z_depth;
					motion_blur_i = i;
					seen_motion_blur_locally = true;
				}
			}
		}

	// Go ahead and start romping through the canvas to paste
	for(iter=context,i=0;*iter;iter++,i++)
	{
		Layer::Handle layer=*iter;
		float z_depth(layer->get_z_depth()*1.0001+i);

		// If the layer isn't active, don't worry about it
		if(!layer->active())
			continue;

		// Any layer with an amount of zero is implicitly disabled.
		ValueBase value(layer->get_param("amount"));
		if(value.get_type()==ValueBase::TYPE_REAL && value.get(Real())==0)
			continue;

		// note: this used to include "&& paste_canvas->get_time_offset()==0", but then
		//		 time-shifted layers weren't being sorted by z-depth (bug #1806852)
		if(layer->get_name()=="PasteCanvas")
		{
			Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));

			// we need to blur the sub canvas if:
			// our parent is blurred,
			// or the child is lower than a local blur,
			// or the child is at the same z_depth as a local blur, but later in the context

#if 0 // DEBUG
			if (seen_motion_blur_in_parent)					synfig::info("seen BLUR in parent\n");
			else if (seen_motion_blur_locally)
				if (z_depth > motion_blur_z_depth)			synfig::info("paste is deeper than BLUR\n");
				else if (z_depth == motion_blur_z_depth) {	synfig::info("paste is same depth as BLUR\n");
					if (i > motion_blur_i)					synfig::info("paste is physically deeper than BLUR\n");
					else									synfig::info("paste is less physically deep than BLUR\n");
				} else										synfig::info("paste is less deep than BLUR\n");
			else											synfig::info("no BLUR at all\n");
#endif	// DEBUG

			motion_blurred = (seen_motion_blur_in_parent ||
							  (seen_motion_blur_locally &&
							   (z_depth > motion_blur_z_depth ||
								(z_depth == motion_blur_z_depth && i > motion_blur_i))));

			Canvas::Handle sub_canvas(Canvas::create_inline(op_canvas));
			Canvas::Handle paste_sub_canvas = paste_canvas->get_sub_canvas();
			if(paste_sub_canvas)
				optimize_layers(time, paste_sub_canvas->get_context(),sub_canvas,motion_blurred);

// \todo: uncommenting the following breaks the rendering of at least examples/backdrop.sifz quite severely
// #define SYNFIG_OPTIMIZE_PASTE_CANVAS
#ifdef SYNFIG_OPTIMIZE_PASTE_CANVAS
			Canvas::iterator sub_iter;

			// Determine if we can just remove the paste canvas altogether
			if (paste_canvas->get_blend_method()	== Color::BLEND_COMPOSITE	&&
				paste_canvas->get_amount()			== 1.0f						&&
				paste_canvas->get_zoom()			== 0						&&
				paste_canvas->get_time_offset()		== 0						&&
				paste_canvas->get_origin()			== Point(0,0)				)
				try {
					for(sub_iter=sub_canvas->begin();sub_iter!=sub_canvas->end();++sub_iter)
					{
						Layer* layer=sub_iter->get();

						// any layers that deform end up breaking things
						// so do things the old way if we run into anything like this
						if(!dynamic_cast<Layer_NoDeform*>(layer))
							throw int();

						ValueBase value(layer->get_param("blend_method"));
						if(value.get_type()!=ValueBase::TYPE_INTEGER || value.get(int())!=(int)Color::BLEND_COMPOSITE)
							throw int();
					}

					// It has turned out that we don't need a paste canvas
					// layer, so just go ahead and add all the layers onto
					// the current stack and be done with it
					while(sub_canvas->size())
					{
						sort_list.push_back(std::pair<float,Layer::Handle>(z_depth,sub_canvas->front()));
						//op_canvas->push_back_simple(sub_canvas->front());
						sub_canvas->pop_front();
					}
					continue;
				}
				catch(int)
				{ }
#endif	// SYNFIG_OPTIMIZE_PASTE_CANVAS

			Layer::Handle new_layer(Layer::create("PasteCanvas"));
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_muck_with_time(false);
			if (motion_blurred)
			{
				Layer::DynamicParamList dynamic_param_list(paste_canvas->dynamic_param_list());
				for(Layer::DynamicParamList::const_iterator iter(dynamic_param_list.begin()); iter != dynamic_param_list.end(); ++iter)
					new_layer->connect_dynamic_param(iter->first, iter->second);
			}
			Layer::ParamList param_list(paste_canvas->get_param_list());
			//param_list.erase("canvas");
			new_layer->set_param_list(param_list);
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_sub_canvas(sub_canvas);
			dynamic_cast<Layer_PasteCanvas*>(new_layer.get())->set_muck_with_time(true);
			layer=new_layer;
		}
		else					// not a PasteCanvas - does it use blend method 'Straight'?
		{
			/* when we use the 'straight' blend method, every pixel on the layer affects the layers underneath,
			 * not just the non-transparent pixels; the following workarea wraps non-pastecanvas layers in a
			 * new pastecanvas to ensure that the straight blend affects the full plane, not just the area
			 * within the layer's bounding box
			 */

			// \todo: this code probably needs modification to work properly with motionblur and duplicate
			etl::handle<Layer_Composite> composite = etl::handle<Layer_Composite>::cast_dynamic(layer);

			/* some layers (such as circle) don't touch pixels that aren't
			 * part of the circle, so they don't get blended correctly when
			 * using a straight blend.  so we encapsulate the circle, and the
			 * encapsulation layer takes care of the transparent pixels
			 * for us.  if we do that for all layers, however, then the
			 * distortion layers no longer work, since they have no
			 * context to work on.  the Layer::reads_context() method
			 * returns true for layers which need to be able to see
			 * their context.  we can't encapsulate those.
			 */
			if (composite &&
				Color::is_straight(composite->get_blend_method()) &&
				!composite->reads_context())
			{
				Canvas::Handle sub_canvas(Canvas::create_inline(op_canvas));
				// don't use clone() because it re-randomizes the seeds of any random valuenodes
				sub_canvas->push_back(composite = composite->simple_clone());
				layer = Layer::create("PasteCanvas");
				composite->set_description(strprintf("Wrapped clone of '%s'", composite->get_non_empty_description().c_str()));
				layer->set_description(strprintf("PasteCanvas wrapper for '%s'", composite->get_non_empty_description().c_str()));
				Layer_PasteCanvas* paste_canvas(static_cast<Layer_PasteCanvas*>(layer.get()));
				paste_canvas->set_blend_method(composite->get_blend_method());
				paste_canvas->set_amount(composite->get_amount());
				sub_canvas->set_time(time); // region and outline don't calculate their bounding rects until their time is set
				composite->set_blend_method(Color::BLEND_STRAIGHT); // do this before calling set_sub_canvas(), but after set_time()
				composite->set_amount(1.0f); // after set_time()
				paste_canvas->set_sub_canvas(sub_canvas);
			}
		}

		sort_list.push_back(std::pair<float,Layer::Handle>(z_depth,layer));
		//op_canvas->push_back_simple(layer);
	}

	//sort_list.sort();
	stable_sort(sort_list.begin(),sort_list.end());
	std::vector< std::pair<float,Layer::Handle> >::iterator iter2;
	for(iter2=sort_list.begin();iter2!=sort_list.end();++iter2)
		op_canvas->push_back_simple(iter2->second);
	op_canvas->op_flag_=true;
}

void
Canvas::get_times_vfunc(Node::time_set &set) const
{
	const_iterator	i = begin(),
				iend = end();

	for(; i != iend; ++i)
	{
		const Node::time_set &tset = (*i)->get_times();
		set.insert(tset.begin(),tset.end());
	}
}

std::set<etl::handle<Layer> >
Canvas::get_layers_in_group(const String&group)
{
	if(is_inline() && parent_)
		return parent_->get_layers_in_group(group);

	if(group_db_.count(group)==0)
		return std::set<etl::handle<Layer> >();
	return group_db_.find(group)->second;
}

std::set<String>
Canvas::get_groups()const
{
	if(is_inline() && parent_)
		return parent_->get_groups();

	std::set<String> ret;
	std::map<String,std::set<etl::handle<Layer> > >::const_iterator iter;
	for(iter=group_db_.begin();iter!=group_db_.end();++iter)
		ret.insert(iter->first);
	return ret;
}

int
Canvas::get_group_count()const
{
	if(is_inline() && parent_)
		return parent_->get_group_count();

	return group_db_.size();
}

void
Canvas::add_group_pair(String group, etl::handle<Layer> layer)
{
	group_db_[group].insert(layer);
	if(group_db_[group].size()==1)
		signal_group_added()(group);
	else
		signal_group_changed()(group);

	signal_group_pair_added()(group,layer);

	if(is_inline()  && parent_)
		return parent_->add_group_pair(group,layer);
}

void
Canvas::remove_group_pair(String group, etl::handle<Layer> layer)
{
	group_db_[group].erase(layer);

	signal_group_pair_removed()(group,layer);

	if(group_db_[group].empty())
	{
		group_db_.erase(group);
		signal_group_removed()(group);
	}
	else
		signal_group_changed()(group);

	if(is_inline() && parent_)
		return parent_->remove_group_pair(group,layer);
}

void
Canvas::add_connection(etl::loose_handle<Layer> layer, sigc::connection connection)
{
	connections_[layer].push_back(connection);
}

void
Canvas::disconnect_connections(etl::loose_handle<Layer> layer)
{
	std::vector<sigc::connection>::iterator iter;
	for(iter=connections_[layer].begin();iter!=connections_[layer].end();++iter)
		iter->disconnect();
	connections_[layer].clear();
}

void
Canvas::rename_group(const String&old_name,const String&new_name)
{
	if(is_inline() && parent_)
		return parent_->rename_group(old_name,new_name);

	{
		std::map<String,std::set<etl::handle<Layer> > >::iterator iter;
		iter=group_db_.find(old_name);
		if(iter!=group_db_.end())
		for(++iter;iter!=group_db_.end() && iter->first.find(old_name)==0;iter=group_db_.find(old_name),++iter)
		{
			String name(iter->first,old_name.size(),String::npos);
			name=new_name+name;
			rename_group(iter->first,name);
		}
	}

	std::set<etl::handle<Layer> > layers(get_layers_in_group(old_name));
	std::set<etl::handle<Layer> >::iterator iter;

	for(iter=layers.begin();iter!=layers.end();++iter)
	{
		(*iter)->remove_from_group(old_name);
		(*iter)->add_to_group(new_name);
	}
}

void
Canvas::register_external_canvas(String file_name, Handle canvas)
{
	if(!is_absolute_path(file_name)) file_name = get_file_path()+ETL_DIRECTORY_SEPARATOR+file_name;
	externals_[file_name] = canvas;
}

#ifdef _DEBUG
void
Canvas::show_externals(String file, int line, String text) const
{
	printf("  .----- (externals for %lx '%s')\n  |  %s:%d %s\n", ulong(this), get_name().c_str(), file.c_str(), line, text.c_str());
	std::map<String, Handle>::iterator iter;
	for (iter = externals_.begin(); iter != externals_.end(); iter++)
	{
		synfig::String first(iter->first);
		etl::loose_handle<Canvas> second(iter->second);
		printf("  |    %40s : %lx (%d)\n", first.c_str(), ulong(&*second), second->count());
	}
	printf("  `-----\n\n");
}
#endif	// _DEBUG
