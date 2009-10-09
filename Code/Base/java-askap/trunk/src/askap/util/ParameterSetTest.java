package askap.util;

import org.junit.* ;
import static org.junit.Assert.* ;

/** Performs JUnit Tests for ParameterSet class. 
 * @author David Brodrick */
public class ParameterSetTest {
  private static ParameterSet theirTestSet;
  
  static {
    //Create the ParameterSet
    theirTestSet = new ParameterSet();
    theirTestSet.put("float", "3.141");
    theirTestSet.put("string", "foo");
    theirTestSet.put("integer", "1234");
    theirTestSet.put("boolean", "true");
  }

  @Test
  public void test_String()
  {
    assertTrue(theirTestSet.getString("string").equals("foo"));   
  }

  @Test
  public void test_StringDefault()
  {
    assertTrue(theirTestSet.getString("string", "bar").equals("foo"));     
  }

  @Test
  public void test_StringDefaultMissing()
  {
    assertTrue(theirTestSet.getString("dummy", "bar").equals("bar"));    
  }
  
  @Test
  public void test_Float()
  {
    assertTrue(theirTestSet.getFloat("float")==3.141f);   
  }

  @Test
  public void test_FloatDefault()
  {
    assertTrue(theirTestSet.getFloat("float", 2.71f)==3.141f);    
  }

  @Test
  public void test_FloatDefaultMissing()
  {
    assertTrue(theirTestSet.getFloat("dummy", 2.71f)==2.71f);    
  }
  
  @Test
  public void test_FloatBad()
  {
    try {
      theirTestSet.getFloat("string");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }

  @Test
  public void test_FloatMissing()
  {
    try {
      theirTestSet.getFloat("dummy");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }  
  
  @Test
  public void test_FloatDefaultBad()
  {
    try {
      theirTestSet.getFloat("string", 2.71f);
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }
    
  @Test
  public void test_Double()
  {
    assertTrue(theirTestSet.getDouble("float")==3.141);   
  }

  @Test
  public void test_DoubleDefault()
  {
    assertTrue(theirTestSet.getDouble("float", 2.71)==3.141);    
  }

  @Test
  public void test_DoubleDefaultMissing()
  {
    assertTrue(theirTestSet.getDouble("dummy", 2.71)==2.71);    
  }

  @Test
  public void test_DoubleBad()
  {
    try {
      theirTestSet.getDouble("string");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }

  @Test
  public void test_DoubleMissing()
  {
    try {
      theirTestSet.getDouble("dummy");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }  

  @Test
  public void test_DoubleDefaultBad()
  {
    try {
      theirTestSet.getDouble("string", 2.71);
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }

  @Test
  public void test_Integer()
  {
    assertTrue(theirTestSet.getInteger("integer")==1234);   
  }

  @Test
  public void test_IntegerDefault()
  {
    assertTrue(theirTestSet.getInteger("integer", 4321)==1234);    
  }

  @Test
  public void test_IntegerDefaultMissing()
  {
    assertTrue(theirTestSet.getInteger("dummy", 4321)==4321);    
  }

  @Test
  public void test_IntegerBad()
  {
    try {
      theirTestSet.getInteger("string");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }

  @Test
  public void test_IntegerMissing()
  {
    try {
      theirTestSet.getInteger("dummy");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }  

  @Test
  public void test_IntegerDefaultBad()
  {
    try {
      theirTestSet.getInteger("string", 4321);
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }
  
  @Test
  public void test_Long()
  {
    assertTrue(theirTestSet.getLong("integer")==1234);   
  }

  @Test
  public void test_LongDefault()
  {
    assertTrue(theirTestSet.getLong("integer", 4321)==1234);    
  }

  @Test
  public void test_LongDefaultMissing()
  {
    assertTrue(theirTestSet.getLong("dummy", 4321)==4321);    
  }

  @Test
  public void test_LongBad()
  {
    try {
      theirTestSet.getLong("string");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }

  @Test
  public void test_LongMissing()
  {
    try {
      theirTestSet.getLong("dummy");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }  

  @Test
  public void test_LongDefaultBad()
  {
    try {
      theirTestSet.getLong("string", 4321);
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }
    
  @Test
  public void test_Boolean()
  {
    assertTrue(theirTestSet.getBoolean("boolean"));   
  }

  @Test
  public void test_BooleanDefault()
  {
    assertTrue(theirTestSet.getBoolean("boolean", false));    
  }

  @Test
  public void test_BooleanDefaultMissing()
  {
    assertTrue(theirTestSet.getBoolean("dummy", true));    
  }  
  
  @Test
  public void test_BooleanMissing()
  {
    try {
      theirTestSet.getBoolean("dummy");
      assertTrue(false);
    } catch (NumberFormatException e) { }    
  }    
}
