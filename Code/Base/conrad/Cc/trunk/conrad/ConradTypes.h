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

  /// @typedef ushort
  /// typedef for sized type  
  using ::ushort;
  /// @typedef uint
  /// typedef for sized type
  using ::uint;
  // Apple doesn't define uLong
  /// @typedef ulong
  /// typedef for sized type
#if defined(__APPLE__)
  typedef unsigned long        ulong;
#else
  using ::ulong;
#endif
  /// @typedef longlong
  /// typedef for sized type
  typedef long long            longlong;
  /// @typedef ulonglong
  /// typedef for sized type
  typedef unsigned long long   ulonglong;
  /// @typedef ldouble
  /// typedef for sized type
  typedef long double          ldouble;
  /// @typedef int8
  /// typedef for sized type
  typedef char                 int8;
  /// @typedef uint8
  /// typedef for sized type
  typedef unsigned char        uint8;
  /// @typedef int16
  /// typedef for sized type
  typedef short                int16;
  /// @typedef int32
  /// typedef for sized type
  typedef int                  int32;
  /// @typedef int64
  /// typedef for sized type
  typedef long long            int64;
  /// @typedef uint16
  /// typedef for sized type
  typedef unsigned short       uint16;
  /// @typedef uint32
  /// typedef for sized type
  typedef unsigned int         uint32;
  /// @typedef uint64
  /// typedef for sized type
  typedef unsigned long long   uint64;
  //@}

}

#endif
