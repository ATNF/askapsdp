.. askap.parset documentation master file, created by sphinx-quickstart on Thu Mar 19 11:46:41 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

=================================================
:mod:`askap.parset` --- LOFAR ParameterSet parser
=================================================

API
===

.. automodule:: askap.parset
    :members:
    :undoc-members:

.. autoclass:: askap.parset.ParameterSet
    :members:

.. autofunction:: askap.parset.decode

.. autofunction:: askap.parset.extract


Sphinx extension
================

The following example parset can be documented using the sphinx extension

.. include::  example.parset
   :literal:

by using the directive:

.. code-block:: rest
   
   .. parameterset:: example.parset

It generates the following:

.. parameterset:: example.parset

Extra options can be specified to control the display

Include a title:

.. code-block:: rest
   
   .. parameterset:: example.parset
      :show-title:

.. parameterset:: example.parset
    :show-title:

Show only a specific key:

.. code-block:: rest
   
   .. parameterset:: example.parset
      :show-title:
      :key: x

.. parameterset:: example.parset
    :show-title:
    :key: x


Show the full key name:

.. code-block:: rest
   
   .. parameterset:: example.parset
      :show-title:
      :key: x

.. parameterset:: example.parset
    :show-title:
    :key: x
    :show-key:

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
