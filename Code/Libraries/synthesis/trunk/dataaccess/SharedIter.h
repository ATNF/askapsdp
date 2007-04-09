/// @file SharedIter.h
///
/// SharedIter: a helper template to be used with iterators created by
///        factories, which are expected to be handled via a boost's smart
///        pointer. It allows to avoid ugly syntax like *(*it), etc.
///        
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef I_SHARED_ITER_H
#define I_SHARED_ITER_H

#include <boost/shared_ptr.hpp>

namespace conrad {

namespace synthesis {

// a template to handle iterators via a pointer stored in the shared_ptr,
// provides a basic iterator interface
template<typename T> class SharedIter : public boost::shared_ptr<T> 
{
public:
    /// constructors just call the same type of constructor from 
    /// boost::shared_ptr<T>
    /// an empty constructor has a special meaning for SharedIter - it is
    /// the end of iteration recognized by comparison operators
    SharedIter() : boost::shared_ptr<T>() {}; // never throws
    template<typename T> explicit SharedIter(Y *p) : 
	           boost::shared_ptr<T>(p) {};
    template<typename Y, typename D> SharedIter(Y *p, D d) :
	           boost::shared_ptr<T>(p,d) {};

    SharedIter(boost::shared_ptr<T> const &r) :
	     boost::shared_ptr<T>(r) {}; // never throws

    // default copy constructor is OK
    
};

} // end of namespace synthesis

} // end of namespace conrad

#endif // #ifndef I_SHARED_ITER_H
