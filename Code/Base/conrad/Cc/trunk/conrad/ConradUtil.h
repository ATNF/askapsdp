/// @file
/// @brief Common CONRAD utility functions and classes
///
/// @author Ger van Diepen (gvd AT astron DOT nl)
///
/// @copyright

#ifndef CONRAD_UTIL_H
#define CONRAD_UTIL_H

#include <conrad/ConradError.h>


#include <ostream>
#include <string>
#include <vector>
#include <list>
#include <sstream>


namespace conrad {


  /// Convert a string to uppercase.
  std::string toUpper(std::string s);

  /// Convert a string to lowercase.
  std::string toLower(std::string s);

	/// Round to nearest integer
	/// @param x Value to be rounded
	int nint(double x);

	/// Round to nearest integer
	/// @param x Value to be rounded
	int nint(float x);

  /// Write a vector to an ostream with a given separator, prefix and postfix.
  /// \note operator<<() must be defined for the container elements.
  template<typename Container>
  void printContainer(std::ostream& os, const Container& ctr,
		      const char* separator=",",
		      const char* prefix="[", const char* postfix="]")
    {
    os << prefix;
    for (typename Container::const_iterator it = ctr.begin();
	 it!=ctr.end();
	 ++it) {
      if (it != ctr.begin()) {
	os << separator;
      }
      os << *it;
    }
    os << postfix;
  }

/// a number of helper functions are gathered in this namespace
namespace utility {

// comments from Max Voronkov:
// probably something like this exists somewhere in the Blob support.
// I grabbed this code from one of my old programs to speed up the development.
// It sat for a while inside GainCalibrationEquation, but now I need it in
// other unrelated classes, so the code has been moved here.
// If the appropriate code existing somewhere else in conrad,
// we can switch to use that code instead. 

/// @brief helper method to interpret string
/// @details any type supported by the input stream can be converted
/// using this method (e.g. string to numbers)
/// @param[in] str input string
/// @return result of the conversion
/// @exception ConradError is thrown if the conversion failed
template<class T> T fromString(const std::string &str) throw(ConradError) {
         std::istringstream is(str);
         T buf;
         is>>buf;
         if (!is) {
             CONRADTHROW(ConradError, "Unable to convert "<<str);
         } 
         return buf;
}

/// @brief helper method to convert any type (e.g. numbers) to a string
/// @details any type supported by the input stream can be converted
/// using this method.
/// @param[in] in a const reference to the value to convert
/// @return resulting string
/// @exception ConradError is thrown if the conversion failed
template<class T> std::string toString(const T &in) throw(ConradError) {
         std::ostringstream os;
         os<<in;
         if (!os) {
             CONRADTHROW(ConradError, "Unable to convert "<<in<<" to string");
         }
         return os.str();
}

} // namespace utility


} // end namespace conrad


namespace std {
  /// @name operator<< Extensions
  /// Print the contents of a vector or list.
  ///
  /// Enclose it in square brackets, using a comma as separator.
  /// \note operator<<() must be defined for type \c T.
  //@{
  template<typename T>
  inline ostream& operator<<(ostream& os, const vector<T>& c)
  {
    conrad::printContainer(os, c, ",", "[", "]");
    return os;
  }
  template<typename T>
  inline ostream& operator<<(ostream& os, const list<T>& c)
  {
    conrad::printContainer(os, c, ",", "[", "]");
    return os;
  }
  //@}
} // end namespace std


#endif
