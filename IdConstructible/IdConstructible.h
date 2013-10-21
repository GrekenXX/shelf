/*
 * IdConstructible.hpp
 *
 *  Created on: Jan 12, 2013
 *      Author: Georgios Dimitriadis
 *
 * Copyright (C) 2013 Georgios Dimitriadis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef IdConstructible_HPP_
#define IdConstructible_HPP_
#include <map>
#include <string>
#include <sstream>

#define ID_CONSTRUCTABLE_FACTORY(Class, IdValue, ParameterName) \
	template<> \
	Class::idc_traits::id_type Class::idc_traits::id() { return IdValue; } \
	template<> \
	Class::idc_traits::base_type* Class::idc_traits::create( \
			const Class::idc_traits::parameter_type& ParameterName)

#define AUTOSTR_ID_CONSTRUCTABLE_FACTORY(Class, ParameterName) \
	ID_CONSTRUCTABLE_FACTORY(Class, #Class, ParameterName)

namespace id_constructable {

struct NoFactoryException : public std::exception {
	template<typename ID>
	NoFactoryException(const ID& id) throw() {
		std::stringstream ss;
		ss <<
				"\n  No factory method associated with type id '" << id << "' found."
				"\n  Stuff to check: "
				"\n    - Does the base class of '" << id << "' match the one required by the used creator?"
				"\n    - Does the parameter type of '" << id << "' match the one required by the used creator?"
				"\n    - Is '" << id << "' really IdConstructible?";
		what_ = ss.str();
	}

	virtual ~NoFactoryException() throw() {}

	virtual const char* what() const throw() {
		 return what_.c_str();
	}
private:
	std::string what_;
};

template<typename T, typename ID, typename Params>
struct IdConstructibleRegistry {
	typedef  T* (*IdConstructibleFactory)(const Params&);
	typedef std::map<ID, IdConstructibleFactory> factory_map_t;

	void add(const ID& id, const IdConstructibleFactory& factory) {
		factory_map[id] = factory;
	}

	bool contains(const ID& id) const {
		return (factory_map.find(id) != factory_map.end());
	}

	T* create(const ID& id, const Params& parameters) const {
		typename factory_map_t::const_iterator factory = factory_map.find(id);
		if ( factory==factory_map.end() )
			throw NoFactoryException(id);

		return factory->second(parameters);
	}

private:
	factory_map_t factory_map;
};

// This function creates and delivers the registry of factories
// for all IdConstructible classes derived from T.
template<typename T, typename ID, typename Params>
IdConstructibleRegistry<T, ID, Params>& registry() {
	static IdConstructibleRegistry<T, ID, Params> registry;
	return registry;
}

// Convenience class. Use this instead of calling registry yourself.
template<
	typename T,
	typename Params=std::map<std::string,std::string>,
	typename ID=std::string
	>
struct Creator {
	static IdConstructibleRegistry<T,ID,Params> registry() {
		return id_constructable::registry<T,ID,Params>();
	}

	static T* create(const ID& id, const Params& params=Params()) {
		return registry().create(id, params);
	}

	T* operator()(const ID& id, const Params& params=Params()) const {
		return create(id, params);
	}

	static bool can_create(const ID& id) {
		return registry().contains(id);
	}
};

template<typename T>
struct Registrar {
	// This constructor need only be called once for every Derived class.
	Registrar() {
		registry<
			typename T::idc_traits::base_type,
			typename T::idc_traits::id_type,
			typename T::idc_traits::parameter_type
			>().add(T::idc_traits::id(), &T::idc_traits::create);
	}
};

} // namespace id_constructable

template<
	typename Derived,
	typename Parent,
	typename Parameter=std::map<std::string,std::string>,
	typename Id=std::string
	>
struct IdConstructible : Parent {
	struct IdConstructibleTraits {
		typedef IdConstructible type;
		typedef Parent base_type;
		typedef Id id_type;
		typedef Parameter parameter_type;
		static Id id();
		static Parent * create(const Parameter &);
	};
	typedef IdConstructibleTraits idc_traits;

	IdConstructible () {
		/**
		 * Here is where creation of registrar is ensured. We just
		 * tell the compiler that we actually need the static
		 * registrar to be created. Since construction_guarnatee is
		 * empty any competent compiler is likely to remove the call
		 * when building an optimized build. Cool huh?
		 */
		construction_guarantee(registrar);
	}

protected:
	// Creation of registrator object ONCE BEFORE MAIN ensures
	// one registration of factory function for Derived class.
	static const id_constructable::Registrar<IdConstructible> registrar;

	static void construction_guarantee(
			const id_constructable::Registrar<IdConstructible>& registrar_) { };
};
template<typename Derived, typename Parent, typename Parameter, typename Id>
const id_constructable::Registrar<IdConstructible<Derived, Parent, Parameter, Id> >
	IdConstructible<Derived, Parent, Parameter, Id>::registrar;

#endif /* IdConstructible_HPP_ */
