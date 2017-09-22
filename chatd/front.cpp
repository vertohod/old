#include "front.h"
#include "util.h"

front::front() : ready(false), first_room(0)
{
    name = sptr_cstr(new std::string(DAEMON));
    hash = sptr_cstr(new std::string(GIT_HASH));
    start = sptr_cstr(new std::string(util::get_local_time()));
}

namespace serialization                                                             
{                                                                                   

template <>
void s<front>(std::string& buf, const front& obj, int level)                
{                                                                                   
    begin(buf); enter(buf);                                                         
    s(buf, "name", *obj.name, level + 1); comma(buf); enter(buf);         
    s(buf, "hash", *obj.hash, level + 1); comma(buf); enter(buf);              
    s(buf, "start", *obj.start, level + 1); comma(buf); enter(buf);                  
    s(buf, "ready", obj.ready, level + 1); enter(buf);                  
    tab(buf, level); end(buf); comma(buf); enter(buf);                              
}   

} // end of namespace
