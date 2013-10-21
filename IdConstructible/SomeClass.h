/*
 * SomeClass.h
 *
 *  Created on: Jan 12, 2013
 *      Author: georgios
 */

#ifndef SOMECLASS_H_
#define SOMECLASS_H_
#include "IdConstructible.h"
#include "TheBaseClass.h"

struct SomeClass :
	public IdConstructible<SomeClass,TheBaseClass> {
	SomeClass();
};

#endif /* SOMECLASS_H_ */
