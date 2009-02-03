/* === S Y N F I G ========================================================= */
/*!	\file synfig/module.h
**	\brief writeme
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

#ifndef __SYNFIG_MODULE_H
#define __SYNFIG_MODULE_H

/* === H E A D E R S ======================================================= */

#include "general.h"
#include <ETL/handle>
#include <map>
#include "synfig_string.h"
#include "releases.h"
#include <utility>
#include "vector.h"
#include "color.h"
#include "layer.h"
#include "canvas.h"

//#include "value.h"

/* === M A C R O S ========================================================= */

//! Marks the start of a module description
#define MODULE_DESC_BEGIN(x) struct x##_modclass : public synfig::Module { x##_modclass(synfig::ProgressCallback *callback=NULL);

//! Sets the localized name of the module
#define MODULE_NAME(x) 			virtual const char * Name() { return x; }

//! Sets a localized description of the module
#define MODULE_DESCRIPTION(x)	virtual const char * Desc() { return x; }

//! Sets the name of the module's author
#define MODULE_AUTHOR(x)		virtual const char * Author() { return x; }

//! Sets the version string for the module
#define MODULE_VERSION(x)		virtual const char * Version() { return x; }

//! Sets the copyright string for the module
#define MODULE_COPYRIGHT(x)		virtual const char * Copyright() { return x; }

//! Describes the module's construction function
#define MODULE_CONSTRUCTOR(x)	bool constructor_(synfig::ProgressCallback *cb) { return x(cb); }

//! Describes the module's destruction function
#define MODULE_DESTRUCTOR(x)	virtual void destructor_() { return x(); }

//! Marks the end of a module description
#define MODULE_DESC_END };

//#if 0
#ifdef __APPLE__
//! Marks the start of a module's inventory
#define MODULE_INVENTORY_BEGIN(x)  extern "C" {		\
	synfig::Module* _##x##_LTX_new_instance(synfig::ProgressCallback *cb) \
	{ if(SYNFIG_CHECK_VERSION()){x##_modclass *mod=new x##_modclass(cb); mod->constructor_(cb); return mod; }\
	if(cb)cb->error(#x": Unable to load module due to version mismatch."); return NULL; } \
	}; x##_modclass::x##_modclass(synfig::ProgressCallback */*cb*/) {
#else
//! Marks the start of a module's inventory
#define MODULE_INVENTORY_BEGIN(x)  extern "C" {		\
	synfig::Module* x##_LTX_new_instance(synfig::ProgressCallback *cb) \
	{ if(SYNFIG_CHECK_VERSION()){x##_modclass *mod=new x##_modclass(cb); mod->constructor_(cb); return mod; }\
	if(cb)cb->error(#x": Unable to load module due to version mismatch."); return NULL; } \
	}; x##_modclass::x##_modclass(synfig::ProgressCallback */*cb*/) {
#endif

//! Marks the start of the layers in the module's inventory
#define BEGIN_LAYERS {

//! DEPRECATED - use #INCLUDE_LAYER(class)
#define LAYER(class)																			\
	synfig::Layer::register_in_book(															\
		synfig::Layer::BookEntry(class::create,													\
								 class::name__,													\
								 dgettext("synfig", class::local_name__),						\
								 class::category__,												\
								 class::cvs_id__,												\
								 class::version__));

#define LAYER_ALIAS(class,alias)																\
	synfig::Layer::register_in_book(															\
		synfig::Layer::BookEntry(class::create,													\
								 alias,															\
								 alias,															\
								 CATEGORY_DO_NOT_USE,											\
								 class::cvs_id__,												\
								 class::version__));

//! Marks the end of the layers in the module's inventory
#define END_LAYERS }

//! Marks the start of the targets in the module's inventory
#define BEGIN_TARGETS {

#define TARGET(x)																				\
	synfig::Target::book()[synfig::String(x::name__)]=											\
		std::pair<synfig::Target::Factory,synfig::String>										\
			(x::create,synfig::String(x::ext__));												\
	synfig::Target::ext_book()[synfig::String(x::ext__)]=x::name__;

#define TARGET_EXT(x,y) synfig::Target::ext_book()[synfig::String(y)]=x::name__;

//! Marks the end of the targets in the module's inventory
#define END_TARGETS }

//! Marks the start of the importers in the module's inventory
#define BEGIN_IMPORTERS {

#define IMPORTER(x) synfig::Importer::book()[synfig::String(x::ext__)]=x::create;

#define IMPORTER_EXT(x,y) synfig::Importer::book()[synfig::String(y)]=x::create;

//! Marks the end of the importers in the module's inventory
#define END_IMPORTERS }

//! Marks the start of the valuenodes in the module's inventory
#define BEGIN_VALUENODES { synfig::LinkableValueNode::Book &book(synfig::LinkableValueNode::book());

#define VALUENODE(class,name,local,version)														\
	book[name].factory=reinterpret_cast<synfig::LinkableValueNode::Factory>(&class::create);	\
	book[name].check_type=&class::check_type;													\
	book[name].local_name=local;																\
	book[name].release_version=version;

//! Marks the end of the valuenodes in the module's inventory
#define END_VALUENODES }

//! Marks the end of a module's inventory
#define MODULE_INVENTORY_END	}

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ProgressCallback;

/*!	\class Module
**	\todo writeme
*/
class Module : public etl::shared_object
{
public:
	bool constructor_(synfig::ProgressCallback */*cb*/) { return true; }
	virtual void destructor_() { }

	typedef etl::handle<Module> Handle;
	typedef etl::loose_handle<Module> LooseHandle;
	typedef etl::handle<const Module> ConstHandle;

public:
	typedef Module*(*constructor_type)(ProgressCallback *);
	typedef std::map<String, Handle > Book;
private:
	static Book* book_;
public:
	static Book& book();

	static bool subsys_init(const String &prefix);
	static bool subsys_stop();
	static void register_default_modules(ProgressCallback *cb=NULL);

	static void Register(Handle mod);
	static bool Register(const String &module_name, ProgressCallback *cb=NULL);
	static inline void Register(Module *mod) { Register(Handle(mod)); }

	virtual const char * Name() { return " "; }
	virtual const char * Desc() { return " "; }
	virtual const char * Author() { return " "; }
	virtual const char * Version() { return " "; }
	virtual const char * Copyright() { return SYNFIG_COPYRIGHT; }

	virtual ~Module() { destructor_(); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
