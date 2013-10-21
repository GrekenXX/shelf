/*
 * factories.cpp
 *
 *  Created on: May 9, 2013
 *      Author: georgios
 */
#include "ParameterList.h"
#include "SomeClass.h"
#include "SomeOtherClass.h"

ID_CONSTRUCTABLE_FACTORY(SomeClass, "SomeClass", param) {
	return new SomeClass();
}

AUTOSTR_ID_CONSTRUCTABLE_FACTORY(SomeOtherClass, param) {
	return new SomeOtherClass();
}

