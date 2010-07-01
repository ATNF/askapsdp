/*
 * Copyright (c) 2009 CSIRO Australia Telescope National Facility (ATNF) Commonwealth
 * Scientific and Industrial Research Organisation (CSIRO) PO Box 76, Epping NSW 1710,
 * Australia atnf-enquiries@csiro.au
 * 
 * This file is part of the ASKAP software distribution.
 * 
 * The ASKAP software distribution is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this
 * program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
 * 330, Boston, MA 02111-1307 USA
 */

package askap.util;

import java.util.Iterator;
import java.util.Vector;
import askap.util.ParameterSet;

/**
 * This class enables multiple clients to concurrently use shared resources, so long as
 * the requirements of the different clients are compatible.
 * 
 * <P>
 * Resources/objects which have settings and implement the <i>HasRegistrableSettings</i>
 * interface may register themselves, and when a new client needs access to the shared
 * resource a check is performed to ensure that the settings required by the new client
 * are compatible with the settings currently in use by any existing clients.
 * 
 * <P>
 * An example: the integration period which has a global value across the entire facility.
 * The first client to start an observation can use any legal value for the integration
 * period, but subsequent clients can only proceed if they use the same period as the
 * first client, until such time as the first client finishes and deregisters its
 * settings.
 * 
 * <P>
 * There is one instance of this class for facility wide settings, obtainable via the
 * <i>getGlobalInstance</i> method, but it is possible that other sets of devices may
 * instanciate their own instances for finer grained checking.
 * 
 * @see HasRegistrableSettings
 * @author David Brodrick
 */
public class SettingsRegistrar
{
    /** The instance for registering global settings. */
    private static SettingsRegistrar theirGlobalInstance;

    /** Return the instance for global settings. */
    public synchronized static SettingsRegistrar getGlobalInstance()
    {
        if (theirGlobalInstance == null) {
            theirGlobalInstance = new SettingsRegistrar();
        }
        return theirGlobalInstance;
    }

    /** Destroy the global instance. */
    public synchronized static void destroyGlobalInstance()
    {
        if (theirGlobalInstance != null) {
            getGlobalInstance().destroy();
            theirGlobalInstance = null;
        }
    }

    /** List of all objects which may have settings registered. */
    private Vector<HasRegistrableSettings> itsRegistrations = new Vector<HasRegistrableSettings>();

    /**
     * Remove all resources and references to registered objects.
     */
    public void destroy()
    {
        itsRegistrations.clear();
    }

    /**
     * Register a new object which has settings that may be registered.
     * 
     * @param o The object to be registered.
     */
    public synchronized void register(HasRegistrableSettings o)
    {
        if (!itsRegistrations.contains(o)) {
            itsRegistrations.add(o);
        }
    }

    /**
     * Remove the object, which no longer needs settings to be registered.
     * 
     * @param o The object to be removed.
     */
    public synchronized void deregister(HasRegistrableSettings o)
    {
        itsRegistrations.remove(o);
    }

    /**
     * Check if the specified settings are compatible with all registered objects.
     * 
     * @param client The client who is checking the settings.
     * @param settings The settings to be checked.
     * @return True if the settings are compatible, False if they are incompatible.
     */
    public synchronized boolean checkSettings(Object client, ParameterSet settings)
    {
        Iterator<HasRegistrableSettings> i = itsRegistrations.iterator();
        while (i.hasNext()) {
            if (!i.next().checkSettings(client, settings)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Register the settings requested by the client with all registered objects. If the
     * settings cannot be registered globally, then they will be globally deregistered and
     * an exception will be thrown.
     * 
     * @param client The client who wishes to register these settings.
     * @param settings The settings to be registered.
     * @return True if the settings were compatible, False if they were incompatible.
     */
    public synchronized boolean registerSettings(Object client, ParameterSet settings)
    {
        boolean res = true;
        // Register the new settings globally
        Iterator<HasRegistrableSettings> i = itsRegistrations.iterator();
        while (i.hasNext()) {
            if (!i.next().registerSettings(client, settings)) {
                // Was unable to register settings with all objects, so globally
                // deregister
                Iterator<HasRegistrableSettings> j = itsRegistrations.iterator();
                while (j.hasNext()) {
                    j.next().deregisterSettings(client);
                }
                res = false;
                break;
            }
        }
        return res;
    }

    /**
     * Provides an atomic way to check that settings are compatible, and register them if
     * they are.
     * 
     * @param client The client who wishes to register these settings.
     * @param settings The settings to be registered.
     * @return True if the settings were applied, False if they were incompatible.
     */
    public synchronized boolean checkAndRegisterSettings(Object client, ParameterSet settings)
    {
        boolean res = false;
        if (checkSettings(client, settings)) {
            if (registerSettings(client, settings)) {
                res = true;
            }
        }
        return res;
    }

    /**
     * Globally degregister the client and all setting which they had registered.
     * 
     * @param client The client to be deregistered.
     */
    public synchronized void deregisterSettings(Object client)
    {
        Iterator<HasRegistrableSettings> i = itsRegistrations.iterator();
        while (i.hasNext()) {
            i.next().deregisterSettings(client);
        }
    }
}
