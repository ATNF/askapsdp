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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * A simple implementation of the HasRegistrableSettings interface, which stores a list of
 * which clients have registered which values and defines settings as compatible if the
 * values are exactly the same.
 * 
 * <P>
 * TODO: Want to extend this class so that sub-classes/container classes can specify a
 * subset of keys that they are interested in, rather than necessarily checking every key
 * in the ParameterSet.
 * 
 * @author David Brodrick
 */
public class SimpleSettingsRegister implements HasRegistrableSettings {
  /** Structure with a value and a list of registered client objects. */
  private class RegisteredSetting {
    public String value;

    public Vector<Object> clients = new Vector<Object>();
  }

  /** Records values and clients for each key/setting name. */
  private HashMap<String, RegisteredSetting> itsRegistrations = new HashMap<String, RegisteredSetting>();

  /**
   * Register and apply the specified settings for the given client.
   * 
   * @param client The client who is registering the settings.
   * @param settings The settings to be registered.
   * @return True if the settings were compatible, False if they were incompatible.
   */
  public synchronized boolean registerSettings(Object client, ParameterSet settings) {
    boolean res = true;
    Iterator i = settings.keySet().iterator();
    while (i.hasNext()) {
      String thiskey = (String) i.next();
      RegisteredSetting rs = itsRegistrations.get(thiskey);
      if (rs == null) {
        // This is a new key name/setting
        rs = new RegisteredSetting();
        rs.value = settings.getProperty(thiskey);
        rs.clients.add(client);
        itsRegistrations.put(thiskey, rs);
      } else if (rs.clients.size() == 1 && rs.clients.get(0) == client) {
        // New value for the only currently registered client
        rs.value = settings.getProperty(thiskey);
      } else if (rs.clients.size() == 0) {
        // New client is the only client
        rs.value = settings.getProperty(thiskey);
        rs.clients.add(client);
      } else if (rs.value == settings.get(thiskey)) {
        // Compatible value for a new client
        rs.clients.add(client);
      } else {
        // Incompatible value for a new client
        res = false;
        break;
      }
    }
    return res;
  }

  /**
   * Check if the supplied settings are compatible with any settings which have already
   * been registered.
   * 
   * @param client The client who is checking the settings.
   * @param settings The settings to be checked.
   * @return True if the settings are compatible, False if they are incompatible.
   */
  public synchronized boolean checkSettings(Object client, ParameterSet settings) {
    boolean res = true;
    Iterator i = settings.keySet().iterator();
    while (i.hasNext()) {
      String thiskey = (String) i.next();
      RegisteredSetting rs = itsRegistrations.get(thiskey);
      if (rs == null) {
        // No other clients using this key
        res = true;
      } else if (rs.clients.size() == 0) {
        // No existing clients are using this key
        res = true;
      } else if (rs.clients.size() == 1 && rs.clients.get(0) == client) {
        // Only this client is using the key
        res = true;
      } else if (rs.value == settings.getProperty(thiskey)) {
        // Value is compatible with other clients
        res = true;
      } else {
        res = false;
        break;
      }
    }
    return res;
  }

  /**
   * Deregister any settings which were registered by the given client.
   * 
   * @param client The client who is deregistering their settings.
   */
  public synchronized void deregisterSettings(Object client) {
    Iterator<String> i = itsRegistrations.keySet().iterator();
    while (i.hasNext()) {
      RegisteredSetting rs = itsRegistrations.get(i.next());
      rs.clients.remove(client);
    }
  }
}
