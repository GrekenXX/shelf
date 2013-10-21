/*
 * SomeOtherClass.h
 *
 *  Created on: Jan 12, 2013
 *      Author: georgios
 */

#ifndef SOMEOTHERCLASS_H_
#define SOMEOTHERCLASS_H_
#include "IdConstructible.h"
#include "TheBaseClass.h"

struct SomeOtherClass :
	public IdConstructible<SomeOtherClass,TheBaseClass> {
	SomeOtherClass();
};

#endif /* SOMEOTHERCLASS_H_ */
