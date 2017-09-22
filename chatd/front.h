#ifndef FRONT_H
#define FRONT_H

#include "serialization.h"
#include "types.h"

struct front
{
	sptr_cstr	name;
	sptr_cstr	hash;
	sptr_cstr	start;

	bool		ready;

	OID			first_room;

	front();
};

namespace serialization                                                             
{                                                                                   
template <>                                                                         
void s<front>(std::string& buf, const front& obj, int level);               
}   

#endif
