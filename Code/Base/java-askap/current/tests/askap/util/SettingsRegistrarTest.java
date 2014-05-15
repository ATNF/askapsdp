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

import org.junit.*;
import static org.junit.Assert.*;

/**
 * Test class for SettingsRegistrar/HasRegistrableSettings/SimpleSettingsRegister
 * 
 * @author David Brodrick
 */
public class SettingsRegistrarTest {
  @Test
  public void test_register_overwrite() {
    // Same client is allowed to overwrite its own settings
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.registerSettings("client1", settings2));
  }

  @Test
  public void test_register_clash() {
    // Different clients cannot have incompatible settings
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    assertFalse(registrar.registerSettings("client2", settings2));
  }

  @Test
  public void test_register_compatible_settings() {
    // Different clients which have compatible settings are ok
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.registerSettings("client2", settings1));
  }

  @Test
  public void test_register_deregistered_settings_clash() {
    // Second client can have different settings after the first has deregistered
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    registrar.deregisterSettings("client1");
    assertTrue(registrar.registerSettings("client2", settings2));
  }

  @Test
  public void test_register_different_keys() {
    // Different client can have different keys no problems
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting2", "bar");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.registerSettings("client2", settings2));
  }

  @Test
  public void test_check_overwrite() {
    // Same client is allowed to overwrite its own settings
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.checkSettings("client1", settings2));
  }

  @Test
  public void test_check_clash() {
    // Different clients cannot have incompatible settings
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    assertFalse(registrar.checkSettings("client2", settings2));
  }

  @Test
  public void test_check_compatible_settings() {
    // Different clients which have compatible settings are ok
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.checkSettings("client2", settings1));
  }

  @Test
  public void test_check_deregistered_settings_clash() {
    // Second client can have different settings after the first has deregistered
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting1", "bar");

    registrar.registerSettings("client1", settings1);
    registrar.deregisterSettings("client1");
    assertTrue(registrar.checkSettings("client2", settings2));
  }

  @Test
  public void test_check_different_keys() {
    // Different client can have different keys no problems
    SettingsRegistrar registrar = new SettingsRegistrar();
    registrar.register(new SimpleSettingsRegister());

    ParameterSet settings1 = new ParameterSet();
    settings1.put("setting1", "foo");
    ParameterSet settings2 = new ParameterSet();
    settings2.put("setting2", "bar");

    registrar.registerSettings("client1", settings1);
    assertTrue(registrar.checkSettings("client2", settings2));
  }
}
