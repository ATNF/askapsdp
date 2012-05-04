/// @file
/// @brief syncronisation helper for some casacore methods
/// @details Some operations provided by casacore types are not thread-safe. The
/// purpose of this class is to mediate this problem. Ideally, every method should be
/// either in a class of its own (to avoid carrying unnecessary locks) or even be 
/// integrated into casacore. For now we use this class to avoid overloading the
/// code by the synchronisation primitives. 
///
/// @details TableConstDataAccessor manages a number of cached fields.
/// This class represents a single such field
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ASKAP_CASA_SYNC_HELPER_H
#define ASKAP_CASA_SYNC_HELPER_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>

#ifdef _OPENMP
#include <boost/thread/mutex.hpp>
#endif

namespace askap {

namespace  utility {

/// @brief syncronisation helper for some casacore methods
/// @details Some operations provided by casacore types are not thread-safe. The
/// purpose of this class is to mediate this problem. Ideally, every method should be
/// either in a class of its own (to avoid carrying unnecessary locks) or even be 
/// integrated into casacore. For now we use this class to avoid overloading the
/// code by the synchronisation primitives.
struct CasaSyncHelper {
   /// @brief extract zVector from an x,y,z cube for reading
   /// @details it is equivalent to yzPlane(x).row(y)
   /// @param[in] cube input cube
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   template<typename T>
   #ifdef _OPENMP
   inline casa::Vector<T> zVector(const casa::Cube<T> &cube, const casa::uInt x, const casa::uInt y) const {
       boost::lock_guard<boost::mutex> lock(itsZVectorMutex);
       casa::Vector<T> result = cube.yzPlane(x).row(y).copy();
       return result;
   }
private:
   /// @brief synchronisation mutex for zVector
   mutable boost::mutex itsZVectorMutex;   
   #else
   static inline casa::Vector<T> zVector(const casa::Cube<T> &cube, const casa::uInt x, const casa::uInt y) {
       return cube.yzPlane(x).row(y).copy();
   }
   #endif
};


} // namespace utility

} // namespace askap


#endif // #ifndef ASKAP_CASA_SYNC_HELPER_H

