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
   
   #ifdef _OPENMP

   /// @brief extract zVector from an x,y,z cube for reading
   /// @details it is equivalent to yzPlane(x).row(y)
   /// @param[in] cube input cube
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   template<typename T>
   inline casa::Vector<T> zVector(const casa::Cube<T> &cube, const casa::uInt x, const casa::uInt y) const {
       boost::lock_guard<boost::mutex> lock(itsZVectorMutex);
       casa::Vector<T> result = cube.yzPlane(x).row(y).copy();
       return result;
   }   

   /// @brief generalised copy for array clases
   /// @param[in] in reference to input object
   /// @return a copy
   /// @note the method is templated so it can be used with Array, Vector or Matrix
   template<typename T>
   inline T copy(const T &in) const {
      boost::lock_guard<boost::mutex> lock(itsCopyMutex);
      T result = in.copy();
      return result;
   }

private:
   /// @brief synchronisation mutex for zVector
   mutable boost::mutex itsZVectorMutex;   
   
   /// @brief synchronisation mutex for array copy
   mutable boost::mutex itsCopyMutex;
   
   #else

   /// @brief extract zVector from an x,y,z cube for reading
   /// @details it is equivalent to yzPlane(x).row(y)
   /// @param[in] cube input cube
   /// @param[in] x first coordinate
   /// @param[in] y second coordinate
   template<typename T>
   static inline casa::Vector<T> zVector(const casa::Cube<T> &cube, const casa::uInt x, const casa::uInt y) {
       return cube.yzPlane(x).row(y).copy();
   }


   /// @brief generalised copy for array clases
   /// @param[in] in reference to input object
   /// @return a copy
   /// @note the method is templated so it can be used with Array, Vector or Matrix
   template<typename T>
   static inline T copy(const T &in) {
      return in.copy();
   }

   #endif
};


} // namespace utility

} // namespace askap


#endif // #ifndef ASKAP_CASA_SYNC_HELPER_H

