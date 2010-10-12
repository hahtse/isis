//
// C++ Interface: type
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include "type_base.hpp"
#include "string.h"

#include <string>
#include <functional>

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{

template<class TYPE > class Type;

namespace _internal
{
template<typename T, bool isNumber> class type_compare
{
public:
	bool operator()( const Type<T> &first, const TypeBase &second )const {
		LOG( Debug, error ) << "comparison of " << Type<T>::staticName() << " is not supportet";
		return false;
	}
};
template<typename T> class type_compare<T, true>
{
protected:
	virtual bool posOverflow( const Type<T> &first, const Type<T> &second )const {return false;} //default to false
	virtual bool negOverflow( const Type<T> &first, const Type<T> &second )const {return false;} //default to false
	virtual bool inRange( const Type<T> &first, const Type<T> &second )const {return false;} //default to false
public:
	bool operator()( const Type<T> &first, const TypeBase &second )const {
		// ask second for a converter from itself to Type<T>
		const TypeBase::Converter conv = second.getConverterTo( Type<T>::staticID );

		if ( conv ) {
			//try to convert second into T and handle results
			Type<T> buff;

			switch ( conv->convert( second, buff ) ) {
			case boost::numeric::cPosOverflow:
				LOG( Debug, info ) << "Positive overflow when converting " << second.toString( true ) << " to " << Type<T>::staticName() << ".";
				return posOverflow( first, buff );
			case boost::numeric::cNegOverflow:
				LOG( Debug, info ) << "Negative overflow when converting " << second.toString( true ) << " to " << Type<T>::staticName() << ".";
				return negOverflow( first, buff );
			case boost::numeric::cInRange:
				return inRange( first, buff );
			}
		} else {
			LOG( Debug, error ) << "No conversion of " << second.typeName() << " to " << Type<T>::staticName() << " available";
			return false;
		}

		return false;
	}
	virtual ~type_compare() {}
};

template<typename T, bool isNumber> class type_less : public type_compare<T, isNumber> {};// we are going to specialize this for numeric T below
template<typename T, bool isNumber> class type_greater : public type_compare<T, isNumber> {};
template<typename T, bool isNumber> class type_eq : public type_compare<T, isNumber>
{
protected:
	bool inRange( const Type<T> &first, const Type<T> &second )const {
		return ( T )first == ( T )second;
	}
};

template<typename T> class type_less<T, true> : public type_compare<T, true>
{
protected:
	bool posOverflow( const Type<T> &first, const Type<T> &second )const {
		return true; //getting an positive overflow when trying to convert second into T, obviously means first is less
	}
	bool inRange( const Type<T> &first, const Type<T> &second )const {
		return ( T )first < ( T )second;
	}
};
template<typename T> class type_greater<T, true> : public type_compare<T, true>
{
protected:
	bool negOverflow( const Type<T> &first, const Type<T> &second )const {
		return true; //getting an negative overflow when trying to convert second into T, obviously means first is greater
	}
	bool inRange( const Type<T> &first, const Type<T> &second )const {
		return ( T )first > ( T )second;
	}
};

}

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase
{
	TYPE m_val;
	static const char m_typeName[];
protected:
	TypeBase *clone() const {
		return new Type<TYPE>( *this );
	}
public:
	static const unsigned short staticID = _internal::TypeId<TYPE>::value;
	Type() {
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		check_type<TYPE>();
	}
	/**
	 * Create a Type from any type of value-type.
	 * If the type of the parameter is not the same as the content type of the object, the system tries to do a type conversion.
	 * If that fails, boost::bad_lexical_cast is thrown.
	 */
	template<typename T> Type( const T &value ) {
		m_val = __cast_to( this, value );
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		check_type<TYPE>();
	}
	std::string toString( bool labeled = false )const {
		std::string ret;
		Reference ref = copyToNewById( Type<std::string>::staticID );

		if ( ref.empty() ) {
			LOG( Debug, error ) << "Automatic conversion of " << *this << " to string failed. Falling back to boost::lexical_cast<std::string>";
			ret = boost::lexical_cast<std::string>( m_val );
		} else {
			ret = ref->cast_to<std::string>();
		}

		if ( labeled )ret += "(" + staticName() + ")";

		return ret;
	}
	virtual std::string typeName()const {
		return staticName();
	}
	virtual unsigned short typeID()const {
		return staticID;
	}

	/// \returns true if this and second contain the same value of the same type
	virtual bool operator==( const TypeBase &second )const {
		if ( second.is<TYPE>() ) {
			return m_val == second.cast_to<TYPE>();
		} else
			return  false;
	}

	/// \returns the name of the type
	static std::string staticName() {return m_typeName;}

	/**
	 * Implicit conversion of Type to its value type.
	 * Only the actual type is allowed.
	 * However, the following is valid:
	 * \code
	 * Type<int> i(5);
	 * float f=i;
	 * \endcode
	 * In this case the function returns int which is then also implicitely converted to float.
	 * \return the stored value
	 */
	operator const TYPE&()const {return m_val;}
	operator TYPE&() {return m_val;}

	bool gt( const _internal::TypeBase &ref )const {
		return _internal::type_greater<TYPE, boost::is_arithmetic<TYPE>::value >()( *this, ref );
	}
	bool lt( const _internal::TypeBase &ref )const {
		return _internal::type_less<TYPE, boost::is_arithmetic<TYPE>::value >()( *this, ref );
	}
	bool eq( const _internal::TypeBase &ref )const {
		return _internal::type_eq<TYPE, boost::is_arithmetic<TYPE>::value >()( *this, ref );
	}

	virtual ~Type() {}
};

template<typename T> const Type<T>& _internal::TypeBase::cast_to_Type() const
{
	check_type<T>();
	return m_cast_to<Type<T> >();
}
template<typename T> const T &_internal::TypeBase::cast_to() const
{
	const Type<T> &ret = cast_to_Type<T>();
	return ret.operator const T & ();
}
template<typename T> Type<T>& _internal::TypeBase::cast_to_Type()
{
	check_type<T>();
	return m_cast_to<Type<T> >();
}
template<typename T> T &_internal::TypeBase::cast_to()
{
	Type<T> &ret = cast_to_Type<T>();
	return ret.operator T & ();
}

template<typename T> bool _internal::TypeBase::is()const
{
	check_type<T>();
	return typeID() == Type<T>::staticID;
}

}
/// @}
}

#endif //DATATYPE_INC