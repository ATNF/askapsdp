Commissioning Archive Platform
==============================

.. note:: A cache of measurement sets for recently observed scheduling blocks is
          kept on the Galaxy Lustre file system at the following path:
          */scratch2/askap/askapops/askap-scheduling-blocks* - You may wish to check
          there first as access to this location is faster than access to the
          commissioning archive, where the files may reside on tape media.

The archive can be accessed from Galaxy via a command line utility called ashell.py. This can
be loaded manually using::
	
	module load ashell
	
You can add this to .bashrc for automatic loading; see the section "Setting up your account"
in the :doc:`processing` documentation page. Once loaded you can start the interface with::

	ashell.py
	
After starting ashell you should be presented with the the following prompt::

	<ivec.offline>
	
To download the scheduling block 50 data to your current folder use the following commands::

	<ivec.offline>login
	<ivec.online>get /projects/ASKAP Commissioning Data/askap-scheduling-blocks/50.tar
	
Ashell uses the concept of remote and local folders, the remote folder is the directory
where the files are located in the archive, this is set by the 'cf <path>' (Change Folder)
command, you can check the current remote folder with the 'pwf' (Print Working Folder)
command. The local folder is set by the 'cd <path>' command and can be checked with 'pwd'.

Quick help is also available via 'help' and 'help <command>'

Additional Information
----------------------

* `Data@iVEC Help <https://data.ivec.org/help>`_
