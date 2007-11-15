/// @file
/// @brief An interface responsible for serialization to/from a blob stream
/// @details Methods to read from / write to a blob stream must present in
/// all classes compatible with such operation. This abstract class provides
/// interface definitions. 
/// @note It should probably go into blob, as scimath is not the appropriate
/// place for such a general interface. At this stage just declare it outside
/// the scimath namespace.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_SERIALIZABLE_H
#define I_SERIALIZABLE_H

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

namespace conrad {

/// @brief An interface responsible for serialization to/from a blob stream
/// @details Methods to read from / write to a blob stream must present in
/// all classes compatible with such operation. This abstract class provides
/// interface definitions. 
/// @note It should probably go into blob, as scimath is not the appropriate
/// place for such a general interface. At this stage just declare it outside
/// the scimath namespace.
struct ISerializable {

  /// @brief an empty virtual destructor to keep the compiler happy
  virtual ~ISerializable();

  /// @brief write the object to a blob stream
  /// @param[in] os the output stream
  virtual void writeToBlob(LOFAR::BlobOStream& os) const = 0;

  /// @brief read the object from a blob stream
  /// @param[in] is the input stream
  /// @note Not sure whether the parameter should be made const or not 
  virtual void readFromBlob(LOFAR::BlobIStream& is) = 0; 
  
};

/// @brief operator to store the object in a blob stream
/// @param[in] os the output stream
/// @param[in] obj a serializable object
/// @return the output steam to be able to use chain semantics
LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &os, const ISerializable& obj);

/// @brief operator to load an object from a blob stream
/// @param[in] is the input stream
/// @param[in] obj a serializable object
/// @return the input steam to be able to use chain semantics
LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &is, ISerializable& obj);


} // namespace conrad

#endif // #ifndef I_SERIALIZABLE_H
