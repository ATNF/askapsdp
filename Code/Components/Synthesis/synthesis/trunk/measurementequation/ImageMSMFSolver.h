/// @file
///
/// ImageMSMFSSolver: This solver does Multi Scale Multi Frequency deconvolution
/// for all parameters called image*
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGEMSMFSSOLVER_H_
#define SYNIMAGEMSMFSSOLVER_H_

#include <measurementequation/ImageSolver.h>

#include <boost/shared_ptr.hpp>
#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/MultiTermLatticeCleaner.h>

#include <map>

namespace askap
{
  namespace synthesis
  {
    /// @brief Multiscale solver for images.
    ///
    /// @details This solver performs multi-scale clean using the 
    /// casa::LatticeCleaner classes
    ///
    /// @ingroup measurementequation
    class ImageMSMFSolver : public ImageSolver
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// The default scales are 0, 10, 30 pixels
        /// @param ip Parameters i.e. the images
        ImageMSMFSolver(const askap::scimath::Params& ip);

        /// @brief Constructor from parameters and scales and ntaylorterms
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// @param[in] ip Parameters i.e. the images
        /// @param[in] scales Scales to be solved in pixels
        /// @param[in] nterms number of terms
        ImageMSMFSolver(const askap::scimath::Params& ip,
          const casa::Vector<float>& scales, const int& nterms);
          
        /// @brief Initialize this solver
        virtual void init();

	/// @brief Precondition the normal equations prior to solving them
	/// TODO Send in a parset to specify params for diff kinds of preconditioning.
	///virtual void preconditionNormalEquations();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(askap::scimath::Quality& q);
        
/// @brief Clone this object
        virtual askap::scimath::Solver::ShPtr clone() const;
        
        /// Set the scales
        void setScales(const casa::Vector<float>& scales);
               
      protected:

	/// Parse the image parameter name to get out the Stokes information
        inline std::string getStokes(const std::string& paramstring){return paramstring.substr(6,1);};
	/// Parse the image parameter name to get out the Order information
	/// Order == enumeration index (e.x. second-order Taylor coefficient)
        inline int getOrder(const std::string& paramstring){return atoi(paramstring.substr(8,1).data());};
        /// Create an image parameter string, from stokes and order parameters
	inline std::string makeImageString(const std::string& samplestring, const std::string& stokes, const int& order){std::string newstring(samplestring);newstring.replace(6,1,stokes);newstring.replace(8,1,(casa::String::toString(order)).data());return newstring;};
	

        /// Scales in pixels
        casa::Vector<float> itsScales;

	    /// @brief Number of terms in the Taylor expansion
        int itsNTaylor;
        /// @brief number of terms in the Taylor expansion for PSF
        int itsNPsfTaylor;
        
	/// Map of Cleaners - one for each "image type : i,q,u,v.."
        std::map<string, boost::shared_ptr<casa::MultiTermLatticeCleaner<float> > > itsCleaners;

        /// @brief for debugging (MV - ?)
	    bool dbg;
    };

  }
}
#endif
