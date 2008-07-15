/// @file
/// 
/// @brief Generic template to iterate inner contour of a 2D image
/// @details Several applications across the synthesis require estimation of
/// some statistics along the inner contour around the peak in a 2D image.
/// This generic template acts as an iterator over the points of the given contour
/// enclosing the peak. Each value point referenced by this iterator is an IPosition of
/// a contour point. The points may appear at an arbitrary order, so some kind of sorting
/// is necessary if one wants to join the nearest neighbours. The contour is defined by
/// a predicate (template argument). It is a locus of points where the predicate becomes true,
/// which is closest to the maximum. The predicate can be an arbitrary type with the operator()
/// defined, which receives the value of the array and returns true or false. This class
/// is generic enough to be moved to an upper level (e.g. at least to Base, but may be even
/// to casacore). If it is ever going to become a part of casacore, it would be good to
/// generalize the class to be able to get several contours at once. In this case
/// iterator can return IPosition and the contour index.

#ifndef CONTOUR_FINDER_TCC
#define CONTOUR_FINDER_TCC

/// @brief initialize the finder to work with the given array
/// @details This is a basic constructor, which stores a reference to the 
/// working array inside and rewinds the iterator to the first point.
/// The array can have any number of dimensions, but only two first will be used in the
/// search (i.e. the contour is a curve, rather than a surface). 
/// It is also possible to give a central position for the contour search (default is to 
/// search for a peak). If peak position is given, it should have the same dimensionality as
/// the array.
/// @param[in] array an array to work with
/// @param[in] pred predicate to define the contour
/// @param[in] peak peak position around which the search is performed (allows to work with
///            local optima). The predicate should give false for this point to get a
///            sensible result out. Default is IPosition(1,-1), which means to search for
///            a maximum and use its position.
/// @param[in] clip if true, a contour will always be closed by returning edge pixels if
///            contour goes beyond the array 
template<typename T, typename P> 
ContourFinder::ContourFinder(const casa::Array<T> &array, const P &pred,
                const casa::IPosition &peak, bool clip) : itsArray(array),
                itsPredicate(pred), itsPeak(peak), itsDoClip(clip),
                itsIsEndMark(false) 
{
  init();
}

/// @brief default constructor, serves as an end-mark
/// @details This constructor makes an iterator, which is equivalent to the end method 
/// for stl containers.
template<typename T, typename P> 
ContourFinder::ContourFinder() : itsEndMark(true) {}

/// @brief Comparison operator
/// @details It checks whether the iterator reached an end. Only comparison with an end-mark
/// is allowed.
/// @param[in] other another iterator (e.g. an end mark)
/// @return true, if iterators are equal
template<typename T, typename P> 
bool ContourFinder::operator==(const ContourFinder<T,P> &other) const
{
  if (itsIsEndMark) {
      return other.itsEndMark;
  } 
  return !other.itsEndMark;
}

/// @brief Comparison operator
/// @details It checks whether the iterator has more points to iterate over. Only comparison 
/// with an end-mark is allowed.
/// @param[in] other another iterator (e.g. an end mark)
/// @return true, if iterators are not equal
template<typename T, typename P> 
bool ContourFinder::operator!=(const ContourFinder<T,P> &other) const
{
  if (itsIsEndMark) {
      return !other.itsEndMark;
  } 
  return other.itsEndMark;  
}

/// @brief access operator
/// @details It returns IPosition for the current point of the contour.
/// @return const reference to the currnt point of the contour.
template<typename T, typename P> 
const IPosition& ContourFinder::operator*() const
{
  return itsTestedPosition;
}
   
/// @brief access operator
/// @details It returns IPosition for the current point of the contour by pointer.
/// @return const pointer to the currnt point of the contour.
template<typename T, typename P> 
const IPosition* ContourFinder::operator->() const
{
  return &itsTestedPosition;
}
  
/// @brief rewind the iterator
/// @details This method rewinds the iterator to its initial state. It should not be called
/// for an end-mark iterator.
/// @return a reference to itself (to be able to call stl algorithms in a more concise way)
template<typename T, typename P> 
ContourFinder<T,P>& ContourFinder::init()
{
  return *this;
}
  
/// @brief increment operator
/// @details It makes a step to the next contour point.
/// @return a reference to itself (to allow chaining)
template<typename T, typename P> 
ContourFinder<T,P>& ContourFinder::operator++()
{
  return *this;
}

#endif // #ifndef CONTOUR_FINDER_TCC
