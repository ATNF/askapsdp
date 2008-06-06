/// @file
///
/// Provides generic methods for stream algorithms
///
/// (c) 2008 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_CP_ASKAPSTREAM_H_
#define ASKAP_CP_ASKAPSTREAM_H_


namespace askap
{
  namespace cp
  {
    /// @brief Support for stream algorithms 
    ///
    /// @details Support for stream applications in the area.
    /// An application is derived from this abstract base. 
    ///
    /// @ingroup stream
    class AskapStream
    {
  public:

      /// @brief Constructor 
      /// @param argc Number of command line inputs
      /// @param argv Command line inputs
      AskapStream(int argc, const char** argv);

      ~AskapStream();

      /// Is this running in stream?
      bool isStream();

  protected:
    };

  }
}
#endif
