///
/// VisWeightsMultiFrequency: 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Urvashi Rau <rurvashi@aoc.nrao.edu>
///
#ifndef VISWIEGHTSMULTIFREQUENCY_H_
#define VISWEIGHTSMULTIFREQUENCY_H_

#include <gridding/IVisWeights.h>

#include <string>

#include <casa/BasicSL/Complex.h>


namespace askap
{
  namespace synthesis
  {

    /// @brief Class to calculate visibility weights for Multi-Frequency Synthesis
    ///
    /// Blah blah...
    ///
    /// @ingroup gridding
    class VisWeightsMultiFrequency : public IVisWeights
    {
  public:

      /// @brief aaa
      VisWeightsMultiFrequency();
      VisWeightsMultiFrequency(casa::Double & reffreq);

      /// @brief copy constructor
      /// @param[in] other input object
      VisWeightsMultiFrequency(const VisWeightsMultiFrequency &other);

      
      ~VisWeightsMultiFrequency();

      virtual IVisWeights::ShPtr clone();
     
      /// @brief Set the context
      /// @param order The order of the Taylor term
      void setParameters(int order);

      /// @brief Calculate the visibility weight.
      /// @param i Sample Index
      /// @param chan Channel index
      /// @param pol Polarization index
      float getWeight(int i, double freq, int pol);
      
  protected:


  private:
      // reference frequency.
      casa::Double itsRefFreq;
      // Taylor term 'order'
      casa::Int itsOrder;


    };
  }
}
#endif
