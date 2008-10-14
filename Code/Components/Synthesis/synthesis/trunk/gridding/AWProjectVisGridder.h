/// @file
///
/// AWProjectVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// @copyright (c) 2007 CSIRO
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
#ifndef AWPROJECTVISGRIDDER_H_
#define AWPROJECTVISGRIDDER_H_

#include <gridding/WProjectVisGridder.h>
#include <gridding/IBasicIllumination.h>
#include <dataaccess/IConstDataAccessor.h>

#include <boost/shared_ptr.hpp>


namespace askap
{
  namespace synthesis
  {
    /// @brief Gridder that is appropriate for mosaicing. 
    ///
    /// @details The visibilities are gridded using a convolution
    /// function derived from the antenna illumination pattern,
    /// appropriately shifted in position for each feed, and
    /// incorporating the Fresnel term needed to correct for the
    /// w-term in the full measurement equation.
    ///
    /// The scaling is slow in data points, slow in w planes 
    /// (since the calculation of the convolution function
    /// usually dominates).
    ///
    /// @ingroup gridding
    class AWProjectVisGridder : public WProjectVisGridder
    {
  public:

      /// @brief Construct antenna illumination pattern/W term gridder
      /// @param illum  Antenna illumination model
      /// @param wmax Maximum baseline (wavelengths)
      /// @param nwplanes Number of w planes
      /// @param cutoff Cutoff in determining support e.g. 10^-3 of the peak
      /// @param overSample Oversampling (currently limited to <=1)
      /// @param maxSupport Maximum support to be allowed
      /// @param limitSupport Upper limit of support
      /// @param maxFeeds Maximum number of feeds allowed
      /// @param maxFields Maximum number of fields allowed
      /// @param pointingTol Pointing tolerance in radians
      /// @param frequencyDependent Frequency dependent gridding?
      /// @param name Name of table to save convolution function into
      AWProjectVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
          const double wmax, const int nwplanes, const double cutoff,
	  const int overSample, const int maxSupport, const int limitSupport,
          const int maxFeeds=1, const int maxFields=1, const double pointingTol=0.0001,
          const bool frequencyDependent=true, 
          const std::string& name=std::string(""));

      /// @brief copy constructor
      /// @details It is required to decouple internal array arrays, otherwise
      /// those arrays are shared between all cloned gridders of this type
      /// @param[in] other input object
      /// @note illumination model is copied as a pointer, so the same model is referenced
      AWProjectVisGridder(const AWProjectVisGridder &other);

      virtual ~AWProjectVisGridder();

      /// Clone a copy of this Gridder
      virtual IVisGridder::ShPtr clone();

      /// Form the sum of the convolution function squared, multiplied by the weights for each
      /// different convolution function. This is used in the evaluation of the second derivative.
      /// @param out Output double precision grid
      virtual void finaliseWeights(casa::Array<double>& out);
      
      /*
      /// Form the final output image or PSF
      /// @param out Output double precision image or PSF
      virtual void finaliseGrid(casa::Array<double>& out);
      */

  protected:
      /// @brief Initialise the indices
      /// @param[in] acc const accessor to work with
      virtual void initIndices(const IConstDataAccessor& acc);

      /// Index into convolution function
      /// @param row Row number
      /// @param pol Polarization id
      /// @param chan Channel number
      virtual int cIndex(int row, int pol, int chan);

      /// Initialize convolution function
      /// @param[in] acc const accessor to work with
      virtual void initConvolutionFunction(const IConstDataAccessor& acc);

      /// Correct for gridding convolution function
      /// @param image image to be corrected
      virtual void correctConvolution(casa::Array<double>& image);
  private:
      /// Reference frequency for illumination pattern. 
      double itsReferenceFrequency;
      /// Antenna illumination model
      boost::shared_ptr<IBasicIllumination const> itsIllumination;
      /// Is the convolution function frequency dependent?
      bool itsFreqDep;
      /// Maximum number of feeds
      int itsMaxFeeds;
      /// Maximum number of fields
      int itsMaxFields;
      /// Pointing tolerance in radians
      double itsPointingTolerance;
      /// Current field processed
      int itsCurrentField;
      /// Last field processed
      int itsLastField;
      /// Pointing for this feed and field
      casa::Matrix<casa::MVDirection> itsPointings;

      /// Cube of slopes
      casa::Cube<double> itsSlopes;
      /// Has this feed and field been filled in?
      casa::Matrix<bool> itsDone;

      /// Pad up in size using FFT
      /// @param in Input Array
      /// @param out Output Array
      void fftPad(const casa::Array<double>& in, casa::Array<double>& out);
    };

  }
}
#endif
