/// @file
///
/// Utility class to easily write out a CASA image, with optional piece-wise writing
///
/// @copyright (c) 2014 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#ifndef ASKAP_ANALYSIS_DISTRIB_IMAGE_WRITER_H_
#define ASKAP_ANALYSIS_DISTRIB_IMAGE_WRITER_H_

#include <outputs/ImageWriter.h>
#include <askapparallel/AskapParallel.h>
#include <string>
#include <duchamp/Cubes/cubes.hh>
#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>


namespace askap {

namespace analysis {

class DistributedImageWriter : public ImageWriter {
    public:
        DistributedImageWriter(askap::askapparallel::AskapParallel& comms,
                               duchamp::Cube *cube,
                               std::string imageName);
        virtual ~DistributedImageWriter() {};

    /// @details Handles the creation of the image, only doing
    /// so when either in serial mode or in the master process
    /// when distributed. The ImageWriter::create function is
    /// called. Worker processes in distributed mode do
    /// nothing here.
        void create();

        /// @details Handles distributed writing of the requested
    /// data. When in parallel mode, the master cycles through
    /// the workers, sending an OK signal for them to write,
    /// and waiting for an OK reply before contacting the
    /// next. Once all have finished, an 'all done' signal is
    /// broadcast. The workers wait for the signal from the
    /// master for them to write, then write the array using
    /// the write function from ImageWriter, then send an OK
    /// signal back to the master.
    /// In serial mode, we directly call ImageWriter::write.
void write(const casa::Array<casa::Float> &data,
                   const casa::IPosition &loc,
                   bool accumulate = false);


    protected:

        askap::askapparallel::AskapParallel *itsComms;

};

}
}

#endif
