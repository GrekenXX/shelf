/*
 * main.cpp
 *
 *  Created on: Jan 12, 2013
 *      Author: georgios
 */
#include <iostream>
#include "IdConstructible.h"
#include "ParameterList.h"
#include "TheBaseClass.h"
#include <memory>

using namespace std;
using namespace id_constructable;

typedef Creator<TheBaseClass> creator;

int main(int ac, char* av[]) {
	cout << "enter main()" << endl;
	creator create;

	// With creator object, like this...
	unique_ptr<TheBaseClass> someClassPtr1(create("SomeClass"));

	// Or without creator object, like this...
	unique_ptr<TheBaseClass> someClassPtr2(creator::create("SomeClass"));
	if(creator::can_create("SomeOtherClass"))
		unique_ptr<TheBaseClass> someOtherClassPtr(creator::create("SomeOtherClass"));

	// And, of course, exceptions at runtime errors that the compiler cannot catch.
	try { unique_ptr<TheBaseClass> yourMamaPtr(creator::create("YourMama")); }
	catch(const std::exception& e) { cout << "Caught exception:\n" << e.what() << endl; }
}
