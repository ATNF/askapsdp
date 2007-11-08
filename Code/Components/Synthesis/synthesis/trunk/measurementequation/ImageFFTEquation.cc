#include <dataaccess/SharedIter.h>
#include <fitting/Params.h>
#include <measurementequation/ImageFFTEquation.h>
#include <gridding/SphFuncVisGridder.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Axes.h>

#include <gridding/SphFuncVisGridder.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>

#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>

using conrad::scimath::Params;
using conrad::scimath::Axes;
using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
	namespace synthesis
	{

		ImageFFTEquation::ImageFFTEquation(const conrad::scimath::Params& ip,
		    IDataSharedIter& idi) :
			conrad::scimath::Equation(ip), itsIdi(idi)
		{
			itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
			init();
		}
		;

		ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi) :
			conrad::scimath::Equation(), itsIdi(idi)
		{
			itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
			rwParameters()=defaultParameters().clone();
			init();
		}

		ImageFFTEquation::ImageFFTEquation(const conrad::scimath::Params& ip,
		    IDataSharedIter& idi, IVisGridder::ShPtr gridder) :
			conrad::scimath::Equation(ip), itsGridder(gridder), itsIdi(idi)
		{
			init();
		}
		;

		ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi,
		    IVisGridder::ShPtr gridder) :
			conrad::scimath::Equation(), itsGridder(gridder), itsIdi(idi)
		{
			rwParameters()=defaultParameters().clone();
			init();
		}

		ImageFFTEquation::~ImageFFTEquation()
		{
		}

		conrad::scimath::Params ImageFFTEquation::defaultParameters()
		{
			Params ip;
			ip.add("image");
			return ip;
		}

		ImageFFTEquation::ImageFFTEquation(const ImageFFTEquation& other)
		{
			operator=(other);
		}

		ImageFFTEquation& ImageFFTEquation::operator=(const ImageFFTEquation& other)
		{
			if(this!=&other)
			{
				static_cast<conrad::scimath::Equation*>(this)->operator=(other);
				itsIdi=other.itsIdi;
				itsGridder = other.itsGridder;
			}
			return *this;
		}

		void ImageFFTEquation::init()
		{
		}

		void ImageFFTEquation::predict()
		{
			const vector<string> completions(parameters().completions("image.i"));

			// To minimize the number of data passes, we keep copies of the gridders in memory, and
			// switch between these. This optimization may not be sufficient in the long run.

			itsIdi.chooseOriginal();
			std::map<string, IVisGridder::ShPtr> gridders;
			std::cout << "Initialising for model degridding" << std::endl;
			for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
			{
				string imageName("image.i"+(*it));
				const Axes axes(parameters().axes(imageName));
				casa::Array<double> imagePixels(parameters().value(imageName).copy());
				const casa::IPosition imageShape(imagePixels.shape());
				gridders[imageName]=itsGridder->clone();
				gridders[imageName]->initialiseDegrid(axes, imagePixels);
			}
			// Loop through degridding the data
			std::cout << "Starting to degrid model" << std::endl;
			for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
			{
				itsIdi->rwVisibility().set(0.0);
				for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
				{
					string imageName("image.i"+(*it));
					gridders[imageName]->degrid(itsIdi);
				}
			}
			std::cout << "Finished degridding model" << std::endl;
		};

		// Calculate the residual visibility and image. We transform the model on the fly
		// so that we only have to read (and write) the data once. This uses more memory
		// but cuts down on IO
		void ImageFFTEquation::calcEquations(conrad::scimath::NormalEquations& ne)
		{

			// We will need to loop over all completions i.e. all sources
			const vector<string> completions(parameters().completions("image.i"));

			// To minimize the number of data passes, we keep copies of the gridders in memory, and
			// switch between these. This optimization may not be sufficient in the long run.      
			// Set up initial gridders for model and for the residuals. This enables us to 
			// do both at the same time.

			std::map<string, IVisGridder::ShPtr> modelGridders;
			std::map<string, IVisGridder::ShPtr> residualGridders;
			for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
			{
				const string imageName("image.i"+(*it));
				const casa::IPosition imageShape(parameters().value(imageName).shape());
				const Axes axes(parameters().axes(imageName));
				casa::Array<double> imagePixels(parameters().value(imageName).copy());
				modelGridders[imageName]=itsGridder->clone();
				residualGridders[imageName]=itsGridder->clone();
			}
			// Now we initialise appropriately
			std::cout << "Initialising for model degridding and residual gridding" << std::endl;
			for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
			{
				string imageName("image.i"+(*it));
				const Axes axes(parameters().axes(imageName));
				casa::Array<double> imagePixels(parameters().value(imageName).copy());
				const casa::IPosition imageShape(imagePixels.shape());
				/// First the model
				modelGridders[imageName]->initialiseDegrid(axes, imagePixels);
				/// Now the residual images
				residualGridders[imageName]->initialiseGrid(axes, imageShape, true);
			}
			// Now we loop through all the data
			std::cout << "Starting degridding model and gridding residuals" << std::endl;
			for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
			{
				/// Accumulate model visibility for all models
				itsIdi.chooseBuffer("MODEL_DATA");
				itsIdi->rwVisibility().set(0.0);
				for (vector<string>::const_iterator it=completions.begin();it!=completions.end();++it)
				{
					string imageName("image.i"+(*it));
					modelGridders[imageName]->degrid(itsIdi);
				}
				/// Now we can calculate the residual visibility and image
				for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
				{
					const string imageName("image.i"+(*it));
					if(parameters().isFree(imageName))
					{
						itsIdi.chooseOriginal();
						itsIdi.buffer("RESIDUAL_DATA").rwVisibility()=itsIdi->visibility()-itsIdi.buffer("MODEL_DATA").visibility();
						itsIdi.chooseBuffer("RESIDUAL_DATA");
						residualGridders[imageName]->grid(itsIdi);
					}
				}
			}
			std::cout << "Finished degridding model and gridding residuals" << std::endl;

			// We have looped over all the data, so now we have to complete the 
			// transforms and fill in the normal equations with the results from the
			// residual gridders
			std::cout << "Adding residual image, PSF, and weights image to the normal equations" << std::endl;
			for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
			{
				const string imageName("image.i"+(*it));
				const casa::IPosition imageShape(parameters().value(imageName).shape());

				casa::Array<double> imagePSF(imageShape);
				casa::Array<double> imageWeight(imageShape);
				casa::Array<double> imageDeriv(imageShape);

				residualGridders[imageName]->finaliseGrid(imageDeriv);
				residualGridders[imageName]->finalisePSF(imagePSF);
				residualGridders[imageName]->finaliseWeights(imageWeight);
				{
					casa::IPosition reference(4, imageShape(0)/2, imageShape(1)/2, 0, 0);
					casa::IPosition vecShape(1, imagePSF.nelements());
					casa::Vector<double> imagePSFVec(imagePSF.reform(vecShape));
					casa::Vector<double> imageWeightVec(imageWeight.reform(vecShape));
					casa::Vector<double> imageDerivVec(imageDeriv.reform(vecShape));
					ne.addSlice(imageName, imagePSFVec, imageWeightVec, imageDerivVec,
							imageShape, reference);
				}
			}
		};

	}

}
