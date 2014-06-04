EPICS DB Sphinx Extension
=========================

The Sphinx extension :py:mod:`askapdev.epicsdb.sphinxext` allows for the documenting and cross
referencing of an EPICS database.

To include a database in the documention, you only need to add the following directive

.. code-block:: rst

  .. epicsdb:: example.db
     :hide-tag:
     :hide-pv:  pv_regex, pv_regex

where:

hide-tag
    hides PVs which are tagged with the comment::

        #$ HIDE_PV

hide-pv
    hides PVs which match the supplied regular expression, matches are done from begingging of PV name.
    Multiple PV expressions can be specified, separated by commas

for example, the following directive will produce a :doc:`formatted database <records_example>`
(indexed, with PV references but with some PVs hidden) from the :doc:`EPICS DB <records_literal>`

.. literalinclude:: records_example.rst
   :language: rst
   :lines: 4-

PVs can then be cross referenced elswhere in the module documentation with

.. code-block:: rst

    :pv:`pvname`

pvname can be the full PV name or just the base name, eg

.. code-block:: rst

    A PV can be referenced with the full PV name,
    eg :pv:`pwr:interFan` or with just the basename,
    eg :pv:`interFacn`

will produce the following output

A PV can be referenced with the full PV name,
eg :pv:`pwr:interFan` or with just the basename,
eg :pv:`interFan`

.. note:: Currently only first PV found will be referenced

All records that are parsed in this way are added to :ref:`documentation index <genindex>`

Adding Sphinx Documentation To an IOC
-------------------------------------

* run sphinx-quickstart
* add setup.py
* add :py:mod:`askapdev.epicsdb.sphinxext` do doc/conf.py
* latest epics builder will build Sphinx doc

add the following to dependencies.default::

  epicsdb=Code/Components/EPICS/support/epicsdb/current

to prevent PV names from listing in sidebar, add the following to conf.py::

  html_sidebars = {'records_rst_file' : ['globaltoc.html', 'sourcelink.html', 'searchbox.html']}

where 'records_rst_file' refers to the rst file()s containing Epics DBs.  It can contain a wildcard
to match multiple rst files

Creating a Documentation Only EPICS Database 
--------------------------------------------

MSI can be used to generate a DB from templates that is only used
in documentation.  This is useful for devices that contain multiple
instances of a set of records.  By creating a documentation only
database we can eliminate duplicates and strip out common prefixes
for better readability.

Example EPICS Database
----------------------

The :ref:`example database <textdb>` is parsed from the following directive::

    .. epicsdb:: example.db

to produce...

.. epicsdb:: example.db

.. _textdb:

Text Only Database
------------------

.. literalinclude:: example.db
