/// @file
/// @brief Common CONRAD utility functions and classes
///
/// @author Ger van Diepen (gvd AT astron DOT nl)
///
/// @copyright

#ifndef CONRAD_UTIL_H
#define CONRAD_UTIL_H

#include <ostream>
#include <string>
#include <vector>
#include <list>

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
