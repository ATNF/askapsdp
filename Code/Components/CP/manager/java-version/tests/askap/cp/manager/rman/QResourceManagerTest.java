package askap.cp.manager.rman;

// Junit imports
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

// Support classes
import askap.cp.manager.rman.JobTemplate;

public class QResourceManagerTest {

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testBuildDependencyArg() {
		JobTemplate template = new JobTemplate("testjob");
		QResourceManager rm = new QResourceManager();

		// Test with no dependencies
		assertEquals("", rm.buildDependencyArg(template));

		// Add 1234 as an afterok dependency
		QJob j1 = new QJob("1234");
		template.addDependency(j1, JobTemplate.DependType.AFTEROK);

		assertEquals("-W depend=afterok:1234", rm.buildDependencyArg(template));
		
		// Add 5678 as an after dependency
		QJob j2 = new QJob("5678");
		template.addDependency(j2, JobTemplate.DependType.AFTERSTART);
		assertEquals("-W depend=afterok:1234,after:5678", rm.buildDependencyArg(template));
	}
}
