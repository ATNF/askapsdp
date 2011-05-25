/// @file ComponentResultSet.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_SKYMODELSERVICE_COMPONENTRESULTSET_H
#define ASKAP_CP_SKYMODELSERVICE_COMPONENTRESULTSET_H

// System includes
#include <deque>

// ASKAPsoft includes
#include "casa/aips.h"
#include "SkyModelService.h" // Ice generated interface
#include "boost/shared_ptr.hpp"

// Local package includes
#include "skymodelclient/Component.h"

namespace askap {
namespace cp {
namespace skymodelservice {

/// Encapsulates a result set from the sky model service cone search.
/// Contains an iterator which may be used to access the individual components.
/// Upon creation of the iterator it points to the first element in the 
/// result set.
///
/// The below example illustrates how to use the iterator:
/// @code
///   ComponentResultSet rs = itsService.coneSearch(.....);
///    ComponentResultSet::Iterator it = rs.createIterator();
///    while (it.hasNext()) {
///      const Component& c = *it;
///      it.next();
///    }
/// @endcode
class ComponentResultSet {

    public:

        // Constructor
        // @note This is used internally to the package and typically should not be
        // called directly.
        ComponentResultSet(
                const askap::interfaces::skymodelservice::ComponentIdSeq& componentList,
                askap::interfaces::skymodelservice::ISkyModelServicePrx& service);

        /// Returns the number of components in the result set.
        /// @return the number of components in the result set.
        size_t size() const;

        // Constant iterator for ComponentResultSet
        class Iterator
        {
            public:

                /// The class ComponentResultSet is a friend so it can call the
                /// private init method.
                friend class ComponentResultSet;

                /// Constructor.
                /// @note This is intended to be used by ComponentResultSet::createIterator()
                /// only.
                Iterator();

                /// Move the cursor forward one position
                /// @throw AskapError In the case the cursor is already pointing
                /// to the last element. If hasNext() returns true, this
                /// exception will not be thrown.
                void next();

                /// Returns true if the cursor is still not at the end of
                /// the iterator.
                bool hasNext() const;

                /// Access the element the cursor currently refers to.
                const Component& operator*();

                /// Access the element the cursor currently refers to.
                const Component* operator->();

            private:

                /// @note This is intended to be used by ComponentResultSet::createIterator() only.
                void init(const askap::interfaces::skymodelservice::ComponentIdSeq* componentList,
                        askap::interfaces::skymodelservice::ISkyModelServicePrx* service);

                // Fills the buffer (itsComponentBuffer) with components
                void fillBuffer(void);

                // Index/iterator position
                size_t itsIndex;

                // List of component ids returned from search. Note: The object this
                // pointer points to is owned by the ComponentResultSet
                const askap::interfaces::skymodelservice::ComponentIdSeq* itsComponentList;

                // Proxy object for remote service: Note: The object this
                // pointer points to is owned by the ComponentResultSet
                askap::interfaces::skymodelservice::ISkyModelServicePrx* itsService;

                // Buffer - The component the iterator currently points to.
                std::deque< boost::shared_ptr<Component> > itsComponentBuffer;
        };

        /// Create an iterator.
        /// @return an iterator for the result set.
        Iterator createIterator();

    private:

        // List of component ids returned from search
        const askap::interfaces::skymodelservice::ComponentIdSeq itsComponentList;

        // Proxy object for remote service
        askap::interfaces::skymodelservice::ISkyModelServicePrx itsService;
};

};
};
};

#endif
