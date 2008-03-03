/// @file
/// @brief CONRAD global types
///
/// @author Malte Marquarding <Malte.Marquarding@csiro.au>
/// @copyright 2007 CONRAD, All Rights Reserved.

#ifndef CONRAD_CONRADTYPES_H
#define CONRAD_CONRADTYPES_H

#include <sys/types.h>

namespace conrad {
  // Use sys/types.h where available
  /// @name Sized types
  /// typedefs and imports for sized types
  ///
  /// \b ulong
  ///
  /// \b uint
  ///
  /// \b ushort
  //@{
  using ::ushort;
  using ::uint;

  // Apple doesn't define uLong
#if defined(__APPLE__)
  typedef unsigned long        ulong;
#else
  using ::ulong;
#endif
  typedef unsigned char        uchar;
  typedef long long            longlong;
  typedef unsigned long long   ulonglong;
  typedef long double          ldouble;
  typedef char                 int8;
  typedef unsigned char        uint8;
  typedef short                int16;
  typedef int                  int32;
  typedef long long            int64;
  typedef unsigned short       uint16;
  typedef unsigned int         uint32;
  typedef unsigned long long   uint64;
  //@}

}

#endif
