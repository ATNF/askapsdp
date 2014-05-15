/// @file CleanRequest.h
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_CLEANREQUEST_H
#define ASKAP_CP_CLEANREQUEST_H

// System includes
#include <string>

// ASKAPsoft includes
#include <messages/IMessage.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <casa/Arrays/Array.h>

namespace askap {
    namespace cp {

        class CleanRequest : public IMessage
        {
            public:
                enum PayloadType {
                    WORK,
                    FINALIZE
                };

                /// @brief Constructor.
                CleanRequest();

                /// @brief Destructor.
                virtual ~CleanRequest();

                /// @brief Messages must be self-identifying and must
                /// their type via this interface.
                ///
                /// @note Messages must be self-identifying and must return
                /// their type via this interface. While they can also be
                /// identified by their class type, this method easily translates
                /// to an int which can be used to tag messags (eg. MPI tags).
                virtual MessageType getMessageType(void) const;

                // Setters
                void set_payloadType(PayloadType type);
                void set_patchId(int patchId);
                void set_dirty(const casa::Array<float>& dirty);
                void set_psf(const casa::Array<float>& psf);
                void set_mask(const casa::Array<float>& mask);
                void set_model(const casa::Array<float>& model);
                void set_threshold(double threshold);
                void set_thresholdUnits(std::string thresholdUnits);
                void set_fractionalThreshold(double fractionalThreshold);
                void set_scales(casa::Vector<float> scales);
                void set_niter(int niter);
                void set_gain(double gain);

                // Getters
                PayloadType get_payloadType(void) const;

                int get_patchId(void) const;

                const casa::Array<float>& get_dirty(void) const;
                casa::Array<float>& get_dirty(void);
                const casa::Array<float>& get_psf(void) const;
                casa::Array<float>& get_psf(void);
                const casa::Array<float>& get_mask(void) const;
                casa::Array<float>& get_mask(void);
                const casa::Array<float>& get_model(void) const;
                casa::Array<float>& get_model(void);

                double get_threshold(void) const;
                std::string get_thresholdUnits(void) const;
                double get_fractionalThreshold(void) const;
                casa::Vector<float> get_scales(void) const;
                int get_niter(void) const;
                double get_gain(void) const;


                // Serializer functions

                /// @brief write the object to a blob stream
                /// @param[in] os the output stream
                virtual void writeToBlob(LOFAR::BlobOStream& os) const;

                /// @brief read the object from a blob stream
                /// @param[in] is the input stream
                virtual void readFromBlob(LOFAR::BlobIStream& is);

            private:
                PayloadType itsPayloadType;

                int itsPatchId;
                casa::Array<float> itsDirty;
                casa::Array<float> itsPsf;
                casa::Array<float> itsMask;
                casa::Array<float> itsModel;
                double itsThreshold;
                std::string itsThresholdUnits;
                double itsFractionalThreshold;
                casa::Vector<float> itsScales;
                int itsNiter;
                double itsGain;
        };

    };
};

#endif
