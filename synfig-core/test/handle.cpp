/* === S Y N F I G ========================================================= */
/*! \file handle.cpp
**  \brief Handle Smart Pointer Test
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

#include <list>
#include <map>
#include <vector>

#include "test_base.h"

/* === M A C R O S ========================================================= */

#define NUMBER_OF_OBJECTS	40000

/* === C L A S S E S ======================================================= */

struct MyTestObj : public etl::rshared_object
{
	static int instance_count;
	int my_id;
	explicit MyTestObj(int my_id=0):my_id(my_id)
	{
		instance_count++;
	}

	virtual ~MyTestObj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}

	bool operator<(const MyTestObj &rhs)const
	{
		return my_id<rhs.my_id;
	}
};

struct MyOtherTestObj : public MyTestObj
{
	static int instance_count;
	explicit MyOtherTestObj(int my_id=0):MyTestObj(my_id)
	{
		instance_count++;
	}
	virtual ~MyOtherTestObj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}
};

int MyTestObj::instance_count=0;
int MyOtherTestObj::instance_count=0;

typedef etl::handle<MyTestObj> ObjHandle;
typedef etl::rhandle<MyTestObj> RObjHandle;
typedef etl::handle<MyOtherTestObj> OtherObjHandle;
typedef std::list<ObjHandle> ObjList;
typedef std::list<OtherObjHandle> OtherObjList;
typedef std::list<RObjHandle> RObjList;


/* === P R O C E D U R E S ================================================= */

class BasicSharedObject : public etl::shared_object
{
public:
	typedef etl::handle<BasicSharedObject> Handle;
	typedef etl::loose_handle<BasicSharedObject> LooseHandle;

	BasicSharedObject() {}
};

class RIPSharedObject : public etl::shared_object
{
	int& rip_flag_;

public:
	typedef etl::handle<RIPSharedObject> Handle;
	typedef etl::loose_handle<RIPSharedObject> LooseHandle;

	explicit RIPSharedObject(int& rip_flag)
		: rip_flag_(rip_flag)
	{
		rip_flag_ = 0;
	}

	~RIPSharedObject()
	{
		++rip_flag_;
	}
};

template<class T>
struct DeleteGuard
{
	T* o_ = nullptr;

	explicit DeleteGuard(T* o)
		: o_(o)
	{ }

	~DeleteGuard()
	{
		delete o_;
	}
};

/* === P R O C E D U R E S ================================================= */

void
shared_object_initial_refcount_is_zero()
{
	BasicSharedObject* obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(obj);

	ASSERT_EQUAL(0, obj->count());
}

void
shared_object_ref_increases_refcount()
{
	BasicSharedObject* obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(obj);

	obj->ref();
	ASSERT_EQUAL(1, obj->count());
	obj->ref();
	ASSERT_EQUAL(2, obj->count());
}

void
shared_object_unref_decreases_refcount()
{
	BasicSharedObject* obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(obj);

	obj->ref();
	obj->ref();

	obj->unref();
	ASSERT_EQUAL(1, obj->count());
}

void
shared_object_unref_inactive_does_not_change_refcount()
{
	BasicSharedObject* obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(obj);

	obj->ref();
	obj->ref();

	obj->unref_inactive();
	ASSERT_EQUAL(1, obj->count());

	obj->unref_inactive();
	ASSERT_EQUAL(0, obj->count());
}

void
shared_object_auto_deletes_itself_on_zero_refcount()
{
	int delete_counter = 0;

	RIPSharedObject* obj = new RIPSharedObject(delete_counter);

	obj->ref();
	obj->ref();
	obj->unref();
	ASSERT_EQUAL(1, obj->count());

	obj->unref();
	ASSERT_EQUAL(1, delete_counter);
}


void
handle_default_constructor_means_empty()
{
	BasicSharedObject::Handle obj;

	ASSERT(obj.empty());
}

void
handle_constructor_increases_refcount()
{
	BasicSharedObject::Handle obj(new BasicSharedObject());

	ASSERT_EQUAL(1, obj.count());
	ASSERT_EQUAL(1, obj.get()->count());
}

void
handle_destructor_decreases_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	{
		BasicSharedObject::Handle obj(real_obj);
		ASSERT_EQUAL(2, real_obj->count());
	}

	ASSERT_EQUAL(1, real_obj->count());
}

void
handle_constructor_is_not_empty()
{
	BasicSharedObject::Handle obj(new BasicSharedObject());

	ASSERT_FALSE(obj.empty());
}

void
handle_constructor_is_unique()
{
	BasicSharedObject::Handle obj(new BasicSharedObject());

	ASSERT(obj.unique());
}

void
handle_second_constructor_is_not_unique()
{
	BasicSharedObject::Handle obj1(new BasicSharedObject());
	BasicSharedObject::Handle obj2(obj1.get());

	ASSERT_FALSE(obj1.unique());
	ASSERT_FALSE(obj2.unique());
}

void
handle_constructor_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj(real_obj);

	ASSERT(obj.get() == real_obj);
}

void
handle_destructor_deletes_the_object_if_no_more_references()
{
	int delete_counter = 0;
	RIPSharedObject* real_obj = new RIPSharedObject(delete_counter);

	{
		RIPSharedObject::Handle obj(real_obj);
	}

	ASSERT_EQUAL(1, delete_counter);
}

void
empty_handle_has_refcount_zero()
{
	BasicSharedObject::Handle obj;

	ASSERT_EQUAL(0, obj.count());
}

void
empty_handle_is_not_unique()
{
	BasicSharedObject::Handle obj;

	ASSERT_FALSE(obj.unique());
}


void
handle_detach_decreases_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2(real_obj);

	obj2.detach();
	ASSERT_EQUAL(1, real_obj->count());
}

void
handle_detach_makes_itself_empty()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2(real_obj);

	obj2.detach();
	ASSERT(obj2.empty());
	ASSERT_FALSE(obj1.empty());
	obj1.detach();
	ASSERT(obj1.empty());
}

void
handle_detach_an_already_empty_handle_does_nothing()
{
	BasicSharedObject::Handle obj1(new BasicSharedObject());
	obj1.detach();
	ASSERT(obj1.empty());
	obj1.detach();
	ASSERT(obj1.empty());

	BasicSharedObject::Handle obj2;
	obj2.detach();
	ASSERT(obj2.empty());
}

void
handle_self_assignment_does_not_increase_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj(real_obj);
	obj = obj;
	ASSERT_EQUAL(1, real_obj->count());
}

void
handle_assignment_increases_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2 = obj1;

	ASSERT_EQUAL(2, real_obj->count());
}

void
handle_assignment_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2 = obj1;

	ASSERT(obj2.get() == real_obj);
}

void
handle_assignment_an_object_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2 = real_obj;

	ASSERT(obj2.get() == real_obj);
	ASSERT_EQUAL(2, real_obj->count());
}

void
handle_assignment_from_empty_handle_discards_previous_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	BasicSharedObject::Handle obj1(real_obj);

	obj1 = BasicSharedObject::Handle();
	ASSERT(obj1.empty());
	ASSERT_EQUAL(0, obj1.count());
}

void
handle_assignment_from_empty_handle_decreases_previous_object_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	BasicSharedObject::Handle obj1(real_obj);

	ASSERT_EQUAL(2, real_obj->count());

	obj1 = BasicSharedObject::Handle();
	ASSERT_EQUAL(1, real_obj->count());
}

void
handle_assignment_from_empty_deletes_previous_object_if_it_is_time()
{
	int delete_counter = 0;

	RIPSharedObject* real_obj = new RIPSharedObject(delete_counter);

	RIPSharedObject::Handle obj1(real_obj);

	ASSERT_EQUAL(1, real_obj->count());

	obj1 = RIPSharedObject::Handle();
	ASSERT_EQUAL(1, delete_counter);
}

void
handle_assignment_from_other_handle_discards_previous_object()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	real_obj1->ref();

	BasicSharedObject::Handle obj1(real_obj1);

	BasicSharedObject::Handle obj2(new BasicSharedObject());

	obj1 = obj2;

	ASSERT(obj1 == obj2);
	ASSERT_FALSE(obj1.get() == real_obj1);
}

void
handle_assignment_from_other_handle_decreases_previous_object_refcount()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	real_obj1->ref();

	BasicSharedObject::Handle obj1(real_obj1);

	BasicSharedObject::Handle obj2(new BasicSharedObject());

	obj1 = obj2;

	ASSERT_EQUAL(1, real_obj1->count());
}

void
handle_assignment_from_other_handle_with_same_object_does_not_decrease_object_refcount()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	real_obj1->ref();

	BasicSharedObject::Handle obj1(real_obj1);

	BasicSharedObject::Handle obj2(real_obj1);

	ASSERT_EQUAL(3, obj1->count());
	ASSERT_EQUAL(3, obj2->count());

	obj1 = obj2;

	ASSERT_EQUAL(3, real_obj1->count());
}

void
handle_assignment_from_other_handle_deletes_previous_object_if_it_is_time()
{
	int delete_counter1 = 0;
	int delete_counter2 = 0;
	RIPSharedObject* real_obj1 = new RIPSharedObject(delete_counter1);

	RIPSharedObject::Handle obj1(real_obj1);

	RIPSharedObject::Handle obj2(new RIPSharedObject(delete_counter2));

	ASSERT_EQUAL(1, obj1->count());
	ASSERT_EQUAL(1, obj2->count());

	obj1 = obj2;

	ASSERT_EQUAL(1, delete_counter1);
	ASSERT_EQUAL(0, delete_counter2);
}

void
handle_comparing_to_itself_means_always_true()
{
	BasicSharedObject::Handle obj1(new BasicSharedObject());
	ASSERT(obj1 == obj1);

	BasicSharedObject::Handle obj2;
	ASSERT(obj2 == obj2);
}

void
handle_comparing_to_real_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj(real_obj);
	ASSERT(obj == real_obj);
	ASSERT(real_obj == obj);
}

void
handle_comparing_to_other_handle_with_same_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj1(real_obj);
	BasicSharedObject::Handle obj2(real_obj);
	ASSERT(obj1 == obj2);
	ASSERT(obj2 == obj1);
}

void
handle_comparing_to_other_handle_with_different_object_means_always_false()
{
	BasicSharedObject::Handle obj1(new BasicSharedObject());
	BasicSharedObject::Handle obj2(new BasicSharedObject());
	ASSERT_FALSE(obj1 == obj2);
	ASSERT(obj1 != obj2);
	ASSERT_FALSE(obj2 == obj1);
	ASSERT(obj2 != obj1);

	BasicSharedObject::Handle obj3;
	ASSERT_FALSE(obj1 == obj3);
	ASSERT(obj1 != obj3);
	ASSERT_FALSE(obj3 == obj1);
	ASSERT(obj3 != obj1);
	ASSERT_FALSE(obj2 == obj3);
	ASSERT(obj2 != obj3);
	ASSERT_FALSE(obj3 == obj2);
	ASSERT(obj3 != obj2);
}

void
handle_comparing_to_a_different_object_means_always_false()
{
	BasicSharedObject::Handle obj1(new BasicSharedObject());
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj2);

	ASSERT_FALSE(obj1 == real_obj2);
	ASSERT(obj1 != real_obj2);
	ASSERT_FALSE(real_obj2 == obj1);
	ASSERT(real_obj2 != obj1);
}

void
handle_swap_does_its_job()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	BasicSharedObject::Handle obj1(real_obj1);
	BasicSharedObject::Handle obj2(real_obj2);

	obj1.swap(obj2);
	ASSERT(obj1.get() == real_obj2);
	ASSERT(obj2.get() == real_obj1);
}

void
handle_swap_does_not_change_refcounts()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	BasicSharedObject::Handle obj1(real_obj1);
	BasicSharedObject::Handle obj2(real_obj2);

	obj1.swap(obj2);
	ASSERT_EQUAL(1, real_obj1->count());
	ASSERT_EQUAL(1, real_obj2->count());
}


void
loose_handle_default_constructor_means_empty()
{
	BasicSharedObject::LooseHandle obj;

	ASSERT(obj.empty());
}

void
loose_handle_constructor_does_not_increase_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);
	BasicSharedObject::LooseHandle obj1(real_obj);

	ASSERT_EQUAL(0, obj1.count());
	ASSERT_EQUAL(0, obj1.get()->count());

	real_obj->ref();
	BasicSharedObject::LooseHandle obj2(real_obj);

	ASSERT_EQUAL(1, obj2.count());
	ASSERT_EQUAL(1, obj2.get()->count());
}

void
loose_handle_destructor_does_not_decrease_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	{
		BasicSharedObject::LooseHandle obj(real_obj);
		ASSERT_EQUAL(1, real_obj->count());
	}

	ASSERT_EQUAL(1, real_obj->count());
}

void
loose_handle_constructor_is_not_empty()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);
	BasicSharedObject::LooseHandle obj(real_obj);

	ASSERT_FALSE(obj.empty());
}

void
loose_handle_constructor_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj(real_obj);

	ASSERT(obj.get() == real_obj);
}

void
loose_handle_destructor_does_not_delete_the_object_if_no_more_references()
{
	int delete_counter = 0;
	RIPSharedObject* real_obj = new RIPSharedObject(delete_counter);
	DeleteGuard<RIPSharedObject> guard(real_obj);

	{
		RIPSharedObject::LooseHandle obj(real_obj);
	}

	ASSERT_EQUAL(0, delete_counter);
}

void
empty_loose_handle_has_refcount_zero()
{
	BasicSharedObject::LooseHandle obj;

	ASSERT_EQUAL(0, obj.count());
}

void
loose_handle_detach_does_not_decrease_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2(real_obj);

	obj2.detach();
	ASSERT_EQUAL(0, real_obj->count());
}

void
loose_handle_detach_makes_itself_empty()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2(real_obj);

	obj2.detach();
	ASSERT(obj2.empty());
	ASSERT_FALSE(obj1.empty());
	obj1.detach();
	ASSERT(obj1.empty());
}

void
loose_handle_detach_an_already_empty_loose_handle_does_nothing()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	obj1.detach();
	ASSERT(obj1.empty());
	obj1.detach();
	ASSERT(obj1.empty());

	BasicSharedObject::LooseHandle obj2;
	obj2.detach();
	ASSERT(obj2.empty());
}

void
loose_handle_self_assignment_does_not_increase_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj(real_obj);
	obj = obj;
	ASSERT_EQUAL(0, real_obj->count());
}

void
loose_handle_assignment_does_not_increase_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2 = obj1;

	ASSERT_EQUAL(0, real_obj->count());
	ASSERT_FALSE(obj2.empty());
}

void
loose_handle_assignment_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2 = obj1;

	ASSERT(obj2.get() == real_obj);
}

void
loose_handle_assignment_an_object_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2 = real_obj;

	ASSERT(obj2.get() == real_obj);
	ASSERT_EQUAL(0, real_obj->count());
}

void
loose_handle_assignment_from_empty_loose_handle_discards_previous_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	BasicSharedObject::LooseHandle obj1(real_obj);

	obj1 = BasicSharedObject::LooseHandle();
	ASSERT(obj1.empty());
	ASSERT_EQUAL(0, obj1.count());
}

void
loose_handle_assignment_from_empty_loose_handle_does_not_decrease_previous_object_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	real_obj->ref();

	BasicSharedObject::LooseHandle obj1(real_obj);

	ASSERT_EQUAL(1, real_obj->count());

	obj1 = BasicSharedObject::LooseHandle();
	ASSERT_EQUAL(1, real_obj->count());
}

void
loose_handle_assignment_from_other_loose_handle_discards_previous_object()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard2(real_obj2);

	real_obj1->ref();

	BasicSharedObject::LooseHandle obj1(real_obj1);

	BasicSharedObject::LooseHandle obj2(real_obj2);

	obj1 = obj2;

	ASSERT(obj1 == obj2);
	ASSERT_FALSE(obj1.get() == real_obj1);
}

void
loose_handle_assignment_from_other_loose_handle_does_not_decrease_previous_object_refcount()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard2(real_obj2);

	real_obj1->ref();

	BasicSharedObject::LooseHandle obj1(real_obj1);

	BasicSharedObject::LooseHandle obj2(real_obj2);

	obj1 = obj2;

	ASSERT_EQUAL(1, real_obj1->count());
}

void
loose_handle_assignment_from_other_loose_handle_with_same_object_does_not_decrease_object_refcount()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj1);

	real_obj1->ref();

	BasicSharedObject::LooseHandle obj1(real_obj1);

	BasicSharedObject::LooseHandle obj2(real_obj1);

	ASSERT_EQUAL(1, obj1->count());
	ASSERT_EQUAL(1, obj2->count());

	obj1 = obj2;

	ASSERT_EQUAL(1, real_obj1->count());
}

void
loose_handle_comparing_to_itself_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);
	BasicSharedObject::LooseHandle obj1(real_obj);
	ASSERT(obj1 == obj1);

	BasicSharedObject::LooseHandle obj2;
	ASSERT(obj2 == obj2);
}

void
loose_handle_comparing_to_real_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj(real_obj);
	ASSERT(obj == real_obj);
	ASSERT(real_obj == obj);
}

void
loose_handle_comparing_to_other_loose_handle_with_same_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj);

	BasicSharedObject::LooseHandle obj1(real_obj);
	BasicSharedObject::LooseHandle obj2(real_obj);
	ASSERT(obj1 == obj2);
	ASSERT(obj2 == obj1);
}

void
loose_handle_comparing_to_other_loose_handle_with_different_object_means_always_false()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard1(real_obj1);
	DeleteGuard<BasicSharedObject> guard2(real_obj2);
	BasicSharedObject::LooseHandle obj1(real_obj1);
	BasicSharedObject::LooseHandle obj2(real_obj2);
	ASSERT_FALSE(obj1 == obj2);
	ASSERT(obj1 != obj2);
	ASSERT_FALSE(obj2 == obj1);
	ASSERT(obj2 != obj1);

	BasicSharedObject::LooseHandle obj3;
	ASSERT_FALSE(obj1 == obj3);
	ASSERT(obj1 != obj3);
	ASSERT_FALSE(obj3 == obj1);
	ASSERT(obj3 != obj1);
	ASSERT_FALSE(obj2 == obj3);
	ASSERT(obj2 != obj3);
	ASSERT_FALSE(obj3 == obj2);
	ASSERT(obj3 != obj2);
}

void
loose_handle_comparing_to_a_different_object_means_always_false()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard1(real_obj1);
	BasicSharedObject::LooseHandle obj1(real_obj1);
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard(real_obj2);

	ASSERT_FALSE(obj1 == real_obj2);
	ASSERT(obj1 != real_obj2);
	ASSERT_FALSE(real_obj2 == obj1);
	ASSERT(real_obj2 != obj1);
}

void
loose_handle_swap_does_its_job()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard1(real_obj1);
	DeleteGuard<BasicSharedObject> guard2(real_obj2);
	BasicSharedObject::LooseHandle obj1(real_obj1);
	BasicSharedObject::LooseHandle obj2(real_obj2);

	obj1.swap(obj2);
	ASSERT(obj1.get() == real_obj2);
	ASSERT(obj2.get() == real_obj1);
}

void
loose_handle_swap_does_not_change_refcounts()
{
	BasicSharedObject* real_obj1 = new BasicSharedObject();
	BasicSharedObject* real_obj2 = new BasicSharedObject();
	DeleteGuard<BasicSharedObject> guard1(real_obj1);
	DeleteGuard<BasicSharedObject> guard2(real_obj2);
	BasicSharedObject::LooseHandle obj1(real_obj1);
	BasicSharedObject::LooseHandle obj2(real_obj2);

	obj1.swap(obj2);
	ASSERT_EQUAL(0, real_obj1->count());
	ASSERT_EQUAL(0, real_obj2->count());
}

void
loose_handle_comparing_to_handle_to_same_real_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::LooseHandle loose_obj(real_obj);
	BasicSharedObject::Handle obj(real_obj);

	ASSERT(loose_obj == obj);
}

void
handle_comparing_to_loose_handle_to_same_real_object_means_always_true()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::LooseHandle loose_obj(real_obj);
	BasicSharedObject::Handle obj(real_obj);

	ASSERT(obj == loose_obj);
}

void
loose_handle_assignment_from_handle_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj(real_obj);
	BasicSharedObject::LooseHandle loose_obj = obj;

	ASSERT(obj == loose_obj);
	ASSERT(obj.get() == loose_obj.get());
}

void
loose_handle_assignment_from_handle_does_not_increase_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::Handle obj(real_obj);

	ASSERT_EQUAL(1, real_obj->count());

	BasicSharedObject::LooseHandle loose_obj = obj;

	ASSERT_EQUAL(1, real_obj->count());
	ASSERT_EQUAL(1, loose_obj.count());
}

void
handle_assignment_from_loose_handle_stores_the_same_object()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::LooseHandle loose_obj(real_obj);
	BasicSharedObject::Handle obj = loose_obj;

	ASSERT(obj == loose_obj);
	ASSERT(obj.get() == loose_obj.get());
}

void
handle_assignment_from_loose_handle_increases_refcount()
{
	BasicSharedObject* real_obj = new BasicSharedObject();

	BasicSharedObject::LooseHandle loose_obj(real_obj);

	ASSERT_EQUAL(0, real_obj->count());

	BasicSharedObject::Handle obj = loose_obj;

	ASSERT_EQUAL(1, real_obj->count());
}


void
handle_basic_test()
{
	MyTestObj::instance_count=0;

	{
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
	}

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	{
		std::map<std::string, etl::handle<MyTestObj> > my_map;
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
		my_map["bleh"]=obj_handle;
	}

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));

	ASSERT(obj_handle == obj_handle.constant());
}

void
handle_general_use_test()
{
	MyTestObj::instance_count=0;

	ObjList my_list, my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( ObjHandle(new MyTestObj(rand())) );

	my_other_list=my_list;
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	{
		ObjHandle a(new MyTestObj(27)), b(new MyTestObj(42));
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);
	}

	my_other_list.clear();
	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

struct ListItem
{
	RObjHandle obj;
	int bleh;
	int blah;
	explicit ListItem(RObjHandle obj,int bleh=1, int blah=2):
		obj(obj),bleh(bleh),blah(blah) { }
};

void
rhandle_general_use_test()
{
	MyTestObj::instance_count = 0;

	RObjList my_list;
	int i;

	RObjHandle obj = new MyTestObj(rand());
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back(obj);

	ObjList my_other_list(my_list.begin(),my_list.end());



	ASSERT_EQUAL(NUMBER_OF_OBJECTS*2+1, obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	{RObjHandle bleh(obj);}

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_other_list.clear();

	ASSERT(obj.rcount() == obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, obj.rcount());

	RObjHandle new_obj = new MyTestObj(rand());

	int replacements = obj.replace(new_obj);

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, replacements);

	ASSERT(obj == new_obj);

	{
		RObjHandle bleh(obj);
		RObjHandle blah(obj.get());
	}


	my_list.clear();
	obj.detach();
	new_obj.detach();

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	std::vector<ListItem> my_item_list;
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_item_list.push_back(ListItem(new MyTestObj(rand()),3,4));


	for(i=0;i<100;i++)
	{
		int src,dest;
		src=rand()%NUMBER_OF_OBJECTS;
		dest=rand()%NUMBER_OF_OBJECTS;
		ListItem tmp(my_item_list[src]);
		assert(tmp.obj.rcount()>=2);
		my_item_list.erase(my_item_list.begin()+src);
		assert(tmp.obj.rcount()>=1);
		my_item_list.insert(my_item_list.begin()+dest,tmp);
		assert(tmp.obj.rcount()>=2);
	}

	my_item_list.clear();

	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

void
handle_inheritance_test()
{
	MyTestObj::instance_count = 0;
	MyOtherTestObj::instance_count = 0;

	OtherObjList my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_other_list.push_back( OtherObjHandle(new MyOtherTestObj(rand())) );

	ObjList my_list(my_other_list.begin(),my_other_list.end());
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( OtherObjHandle(new MyOtherTestObj(rand())) );
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, MyOtherTestObj::instance_count);
	ASSERT(MyTestObj::instance_count == MyOtherTestObj::instance_count);

	my_list.sort();
	my_other_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, MyTestObj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_other_list.clear();
	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

int test_func(etl::handle<MyTestObj> handle)
{
	struct Bogus
	{
		int b;

		explicit Bogus(int i)
			: b(i)
		{
			++b;
		}
	};

	if (handle) {
		Bogus b(handle.count());
		return b.b;
	}
	return 5;
}

void
loose_handle_test()
{
	MyTestObj::instance_count=0;

	etl::loose_handle<MyTestObj> obj_handle_loose;
	etl::handle<MyTestObj> obj_handle2;

	{
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
		ASSERT_EQUAL(1, MyTestObj::instance_count);

		obj_handle_loose=obj_handle;
		ASSERT(obj_handle == obj_handle_loose);

		obj_handle2=obj_handle_loose;
		ASSERT_EQUAL(1, MyTestObj::instance_count);

		test_func(obj_handle_loose);
		ASSERT_EQUAL(1, MyTestObj::instance_count);
	}

	{
		auto ra = new MyTestObj(27);
		auto rb = new MyTestObj(42);
		etl::loose_handle<MyTestObj> a(ra), b(rb);
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);

		ASSERT_EQUAL(3, MyTestObj::instance_count);

		delete ra;
		delete rb;
	}

	ASSERT_EQUAL(1, MyTestObj::instance_count);
}

void
handle_cast_test()
{
	etl::handle<MyTestObj> obj;
	etl::handle<MyOtherTestObj> other_obj;
	etl::loose_handle<MyOtherTestObj> loose_obj;

	other_obj.spawn();
	loose_obj = other_obj;

	obj = etl::handle<MyTestObj>::cast_dynamic(loose_obj);

	ASSERT(obj == other_obj);
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	TEST_SUITE_BEGIN()

	TEST_FUNCTION(shared_object_initial_refcount_is_zero);
	TEST_FUNCTION(shared_object_ref_increases_refcount);
	TEST_FUNCTION(shared_object_unref_decreases_refcount);
	TEST_FUNCTION(shared_object_unref_inactive_does_not_change_refcount);
	TEST_FUNCTION(shared_object_auto_deletes_itself_on_zero_refcount);

	TEST_FUNCTION(handle_default_constructor_means_empty);
	TEST_FUNCTION(handle_constructor_increases_refcount);
	TEST_FUNCTION(handle_destructor_decreases_refcount);
	TEST_FUNCTION(handle_constructor_is_not_empty);
	TEST_FUNCTION(handle_constructor_is_unique);
	TEST_FUNCTION(handle_second_constructor_is_not_unique);
	TEST_FUNCTION(handle_constructor_stores_the_same_object);
	TEST_FUNCTION(handle_destructor_deletes_the_object_if_no_more_references);
	TEST_FUNCTION(empty_handle_has_refcount_zero);
	TEST_FUNCTION(empty_handle_is_not_unique);
	TEST_FUNCTION(handle_detach_decreases_refcount);
	TEST_FUNCTION(handle_detach_makes_itself_empty);
	TEST_FUNCTION(handle_detach_an_already_empty_handle_does_nothing);
	TEST_FUNCTION(handle_self_assignment_does_not_increase_refcount);
	TEST_FUNCTION(handle_assignment_increases_refcount);
	TEST_FUNCTION(handle_assignment_stores_the_same_object);
	TEST_FUNCTION(handle_assignment_an_object_stores_the_same_object);
	TEST_FUNCTION(handle_assignment_from_empty_handle_discards_previous_object);
	TEST_FUNCTION(handle_assignment_from_empty_handle_decreases_previous_object_refcount);
	TEST_FUNCTION(handle_assignment_from_empty_deletes_previous_object_if_it_is_time);
	TEST_FUNCTION(handle_assignment_from_other_handle_discards_previous_object);
	TEST_FUNCTION(handle_assignment_from_other_handle_decreases_previous_object_refcount);
	TEST_FUNCTION(handle_assignment_from_other_handle_with_same_object_does_not_decrease_object_refcount);
	TEST_FUNCTION(handle_assignment_from_other_handle_deletes_previous_object_if_it_is_time);
	TEST_FUNCTION(handle_comparing_to_itself_means_always_true);
	TEST_FUNCTION(handle_comparing_to_real_object_means_always_true);
	TEST_FUNCTION(handle_comparing_to_other_handle_with_same_object_means_always_true);
	TEST_FUNCTION(handle_comparing_to_other_handle_with_different_object_means_always_false);
	TEST_FUNCTION(handle_comparing_to_a_different_object_means_always_false);
	TEST_FUNCTION(handle_swap_does_its_job);
	TEST_FUNCTION(handle_swap_does_not_change_refcounts);

	TEST_FUNCTION(loose_handle_default_constructor_means_empty);
	TEST_FUNCTION(loose_handle_constructor_does_not_increase_refcount);
	TEST_FUNCTION(loose_handle_destructor_does_not_decrease_refcount);
	TEST_FUNCTION(loose_handle_constructor_is_not_empty);
	TEST_FUNCTION(loose_handle_constructor_stores_the_same_object);
	TEST_FUNCTION(loose_handle_destructor_does_not_delete_the_object_if_no_more_references);
	TEST_FUNCTION(empty_loose_handle_has_refcount_zero);
	TEST_FUNCTION(loose_handle_detach_does_not_decrease_refcount);
	TEST_FUNCTION(loose_handle_detach_makes_itself_empty);
	TEST_FUNCTION(loose_handle_detach_an_already_empty_loose_handle_does_nothing);
	TEST_FUNCTION(loose_handle_self_assignment_does_not_increase_refcount);
	TEST_FUNCTION(loose_handle_assignment_does_not_increase_refcount);
	TEST_FUNCTION(loose_handle_assignment_stores_the_same_object);
	TEST_FUNCTION(loose_handle_assignment_an_object_stores_the_same_object);
	TEST_FUNCTION(loose_handle_assignment_from_empty_loose_handle_discards_previous_object);
	TEST_FUNCTION(loose_handle_assignment_from_empty_loose_handle_does_not_decrease_previous_object_refcount);
	TEST_FUNCTION(loose_handle_assignment_from_other_loose_handle_discards_previous_object);
	TEST_FUNCTION(loose_handle_assignment_from_other_loose_handle_does_not_decrease_previous_object_refcount);
	TEST_FUNCTION(loose_handle_assignment_from_other_loose_handle_with_same_object_does_not_decrease_object_refcount);
	TEST_FUNCTION(loose_handle_comparing_to_itself_means_always_true);
	TEST_FUNCTION(loose_handle_comparing_to_real_object_means_always_true);
	TEST_FUNCTION(loose_handle_comparing_to_other_loose_handle_with_same_object_means_always_true);
	TEST_FUNCTION(loose_handle_comparing_to_other_loose_handle_with_different_object_means_always_false);
	TEST_FUNCTION(loose_handle_comparing_to_a_different_object_means_always_false);
	TEST_FUNCTION(loose_handle_swap_does_its_job);
	TEST_FUNCTION(loose_handle_swap_does_not_change_refcounts);

	TEST_FUNCTION(loose_handle_comparing_to_handle_to_same_real_object_means_always_true);
	TEST_FUNCTION(handle_comparing_to_loose_handle_to_same_real_object_means_always_true);
	TEST_FUNCTION(loose_handle_assignment_from_handle_stores_the_same_object);
	TEST_FUNCTION(loose_handle_assignment_from_handle_does_not_increase_refcount);
	TEST_FUNCTION(handle_assignment_from_loose_handle_stores_the_same_object);
	TEST_FUNCTION(handle_assignment_from_loose_handle_increases_refcount);

	// Original tests from older ETL/test folder

	TEST_FUNCTION(handle_basic_test);
	TEST_FUNCTION(handle_cast_test);
	TEST_FUNCTION(handle_general_use_test);
	TEST_FUNCTION(handle_inheritance_test);

	TEST_FUNCTION(loose_handle_test);

	TEST_FUNCTION(rhandle_general_use_test);

	TEST_SUITE_END()

	return tst_exit_status;
}
