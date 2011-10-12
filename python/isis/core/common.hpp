/*
 * common.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef PYTHON_COMMON_HPP
#define PYTHON_COMMON_HPP

#include "CoreUtils/log.hpp"
#include "CoreUtils/common.hpp"

/*! \addtogroup python
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis
{

struct PythonLog {static const char *name() {return "Python";}; enum {use = _ENABLE_LOG};};
struct PythonDebug {static const char *name() {return "PythonDebug";}; enum {use = _ENABLE_DEBUG};};

namespace python
{
namespace core
{
typedef PythonLog Runtime;
typedef PythonDebug Debug;

template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
namespace _internal
{
enum types { BOOL,
			 INT8_T, UINT8_T, INT16_T, UINT16_T, INT32_T,
			 UINT32_T, INT64_T, UINT64_T, FLOAT, DOUBLE,
			 FVECTOR4, DVECTOR4, IVECTOR4, ILIST, DLIST,
			 SLIST, STDSTRING, SELECTION, COMPLEX_FLOAT,
			 COMPLEX_DOUBLE, BOOST_PTIME, BOOST_DATE
		   };
} // namespace _internal

} // namespace core

} //namespace python

} //namespace isis
/** @} */
#endif /* PYTHON_COMMON_HPP_ */


