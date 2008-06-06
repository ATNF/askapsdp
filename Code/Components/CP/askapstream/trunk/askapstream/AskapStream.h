/// @file
///
/// Provides generic methods for stream algorithms
///
/// @copyright (c) 2008 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
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
