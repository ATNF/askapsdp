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

import java.util.Map;
import java.util.HashMap;
import IceStorm.*;
import Ice.Communicator;
import atnf.atoms.time.AbsTime;
import askap.interfaces.*;
import askap.interfaces.datapublisher.*;

/**
 * Class for publishing data on a TimeTaggedTypedValueMapPublisher IceStorm Topic.
 * 
 * @author David Brodrick
 */
public class TimeTaggedDataPublisher extends DataPublisherBase {
  /** The actual interface for publishing data. */
  protected ITimeTaggedTypedValueMapPublisherPrx itsPublisher = null;

  /**
   * Constructor.
   */
  public TimeTaggedDataPublisher() {
  }

  /**
   * Constructor with topic name.
   * 
   * @param topic The name of the IceStorm topic to publish data to.
   */
  public TimeTaggedDataPublisher(String topic) {
    setTopicName(topic);
  }

  /**
   * Constructor with topic name and communicator.
   * 
   * @param topic The name of the IceStorm topic to publish data to.
   * @param comm The Ice Communicator used for communications.
   */
  public TimeTaggedDataPublisher(String topic, Communicator comm) {
    setTopicName(topic);
    setCommunicator(comm);
  }

  /**
   * Constructor with topic name and host/port locator of the IceGrid locator service.
   * 
   * @param topic The name of the IceStorm topic to publish data to.
   * @param host The name of the host where the locator service is running.
   * @param port The port where the locator service is running.
   */
  public TimeTaggedDataPublisher(String topic, String host, int port) {
    setTopicName(topic);
    setLocator(host, port);
  }

  /**
   * Disconnect from the Topic.
   */
  protected void disconnect() {
    itsTopic = null;
    itsPublisher = null;
  }

  /**
   * Check if we are currently connected to the Topic and able to publish data.
   * 
   * @return True if connected, False if not connected.
   */
  protected boolean isConnected() {
    boolean res = false;
    if (itsTopic != null && itsPublisher != null) {
      return true;
    }
    return res;
  }

  /**
   * Connect to the IceStorm Topic so that we can start publishing data.
   */
  protected void connect() throws Exception {
    try {
      if (itsTopicName == null || itsCommunicator == null) {
        return;
      }
      // Obtain the topic or create
      TopicManagerPrx topicManager;
      Ice.ObjectPrx obj = itsCommunicator.stringToProxy("IceStorm/TopicManager");
      topicManager = IceStorm.TopicManagerPrxHelper.checkedCast(obj);

      try {
        itsTopic = topicManager.retrieve(itsTopicName);
      } catch (NoSuchTopic e) {
        try {
          itsTopic = topicManager.create(itsTopicName);
        } catch (TopicExists e1) {
          itsTopic = topicManager.retrieve(itsTopicName);
        }
      }

      Ice.ObjectPrx pub = itsTopic.getPublisher().ice_oneway();
      itsPublisher = ITimeTaggedTypedValueMapPublisherPrxHelper.uncheckedCast(pub);
    } catch (Exception e) {
      disconnect();
      throw e;
    }
  }

  /**
   * Publish the given data.
   * 
   * @param timestamp The timestamp to be applied to the data.
   * @param data The data to be published.
   * @throws IllegalArgumentException Thrown if the argument contains data which cannot be
   * published.
   * @throws NotConnectedException Thrown if we couldn't connect to the IceStorm Topic.
   */
  public void publish(AbsTime timestamp, Map<String, Object> data) throws IllegalArgumentException, NotConnectedException {
    // Convert the data to typed values
    Map<String, TypedValue> typeddata = TypedValueUtils.objectMap2TypedValueMap(data);

    try {
      if (!isConnected()) {
        // Need to connect to the IceStorm Topic
        connect();
      }

      if (isConnected()) {
        // Publish the data via IceStorm
        TimeTaggedTypedValueMap pubdata = new TimeTaggedTypedValueMap();
        pubdata.timestamp = timestamp.getValue();
        pubdata.data = typeddata;
        itsPublisher.publish(pubdata);
      } else {
        throw new NotConnectedException();
      }
    } catch (Exception e) {
      throw new NotConnectedException(e);
    }
  }

  /**
   * Simple test method.
   */
  public static final void main(String[] args) {
    if (args.length != 3) {
      System.err.println("Needs three arguments: topicname hostname portnum");
      System.exit(1);
    }
    TimeTaggedDataPublisher dp = new TimeTaggedDataPublisher(args[0], args[1], Integer.parseInt(args[2]));

    try {
      dp.publish(new AbsTime(), new HashMap<String, Object>());
    } catch (Exception e) {
      System.err.println("TimeTaggedDataPublisher.main: " + e);
    }
    System.exit(0);
  }
}
