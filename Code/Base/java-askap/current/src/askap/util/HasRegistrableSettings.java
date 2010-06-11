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

import askap.util.ParameterSet;

/**
 * Interface for objects/resources which can be shared among multiple clients if, and only
 * if, the requirements of the different clients are compatible.
 * 
 * <P>
 * The SimpleSettingsRegister class provides an implementation of this interface which
 * implements the most commonly required functionality, ie, settings are deemed compatible
 * if the values are exactly the same.
 * 
 * @see SettingsRegistrar
 * @see SimpleSettingsRegister
 * @author David Brodrick
 */
public interface HasRegistrableSettings {
  /**
   * Register and apply the specified settings for the given client.
   * 
   * @param client The client who is registering the settings.
   * @param settings The settings to be registered.
   * @return True if the settings were compatible, False if they were incompatible.
   */
  public boolean registerSettings(Object client, ParameterSet settings);

  /**
   * Check if the supplied settings are compatible with any settings which have already
   * been registered.
   * 
   * @param client The client who is checking the settings.
   * @param settings The settings to be checked.
   * @return True if the settings are compatible, False if they are incompatible.
   */
  public boolean checkSettings(Object client, ParameterSet settings);

  /**
   * Deregister any settings which were registered by the given client.
   * 
   * @param client The client who is deregistering their settings.
   */
  public void deregisterSettings(Object client);
}
