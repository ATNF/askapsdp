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

package askap.datapublisher;

import IceStorm.*;
import Ice.Communicator;

import org.apache.log4j.Logger;

/**
 * Abstract base class for DataPublishers.
 * 
 * @author David Brodrick
 */
public abstract class DataPublisherBase {
  /**
   * Exception for when the IceStorm Topic could not be fully connected.
   */
  public class NotConnectedException extends Exception {
    public NotConnectedException() {
      super("Cannot connect/publish via IceStorm");
    }

    public NotConnectedException(Exception e) {
      super("Cannot connect/publish via IceStorm: " + e.getClass());
    }
  }

  /** The Ice communicator to be used. */
  protected Communicator itsCommunicator = null;

  /** Name of the IceStorm topic to publish data to. */
  protected String itsTopicName = null;

  /** The IceStorm topic to publish metadata to. */
  protected TopicPrx itsTopic = null;
  
  /** Logger. */
  protected Logger itsLogger = Logger.getLogger(getClass().getName());

  /**
   * Specify the Ice Communicator to be used.
   */
  public void setCommunicator(Communicator comm) {
    itsCommunicator = comm;
    disconnect();
  }

  /**
   * Specify the host name and port of the IceGrid Locator service.
   */
  public void setLocator(String host, int port) {
    Ice.Properties props = Ice.Util.createProperties();
    String locator = "IceGrid/Locator:tcp -h " + host + " -p " + port;
    props.setProperty("Ice.Default.Locator", locator);
    Ice.InitializationData id = new Ice.InitializationData();
    id.properties = props;
    setCommunicator(Ice.Util.initialize(id));
  }

  /**
   * Specify the IceStorm Topic name to publish data on.
   */
  public void setTopicName(String topic) {
    itsTopicName = topic;
    disconnect();
  }

  /**
   * Disconnect from the Topic.
   */
  protected abstract void disconnect();

  /**
   * Check if we are currently connected to the Topic and able to publish data.
   * 
   * @return True if connected, False if not connected.
   */
  protected abstract boolean isConnected();

  /**
   * Connect to the IceStorm Topic so that we can start publishing data.
   */
  protected abstract void connect() throws Exception;
}
