#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>
#include <strings.h>
#include <ostream>

#include "property.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/// @cond _internal
namespace _internal {
struct nocase_less{
	bool operator() (const std::string& a, const std::string& b) const;
};
}
/// @endcond
	
class PropMap : public std::map<std::string,PropertyValue,_internal::nocase_less>{
public:
	typedef std::list<key_type> key_list;
	typedef std::map<key_type,std::string,_internal::nocase_less> string_map;
	bool valid()const;
	key_list missing()const;
	string_map diff(const PropMap &second)const;
};

}
/** @} */
}

//make PropMap printable
namespace std {
template<typename charT, typename traits> basic_ostream<charT, traits>& 
operator<<(basic_ostream<charT, traits> &out,const isis::util::PropMap& s){
	isis::util::PropMap::const_iterator i=s.begin();
	if(i!=s.end()){
		out << i->first << ": " << i->second->toString(true);
		for(i++;i!=s.end();i++){
			out << std::endl << i->first << ": " << i->second->toString(true);
		}
	}
	return out;
}
}

#endif
