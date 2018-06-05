#ifndef FRONT_H
#define FRONT_H

#include "types.h"
#include "serialization.h"

struct front
{
	sptr_cstr	name;
	sptr_cstr	hash;
	sptr_cstr	start;

	bool		ready;

	front();
};

namespace serialization                                                             
{                                                                                   
template <>                                                                         
void s<front>(std::string& buf, const front& obj, int level);               
}   

#endif
