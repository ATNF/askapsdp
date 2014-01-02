Basic Continuum Imaging Tutorial
================================

This tutorial demonstrates basic continuum imaging using ASKAPsoft.

Prerequisites
-------------
You should read the :doc:`../platform/processing` documentation and in particular have
setup your environment per the section entitled "Setting up your account".

You should also have read the :doc:`intro` tutorial and be comfortable with submitting jobs
and monitoring their status.

Setting up a working directory
------------------------------
Your working directory will not be within your home directory, instead it will reside
on the fast Lustre filesystem::

    cd /scratch/$USER
    mkdir continuumtutorial
    cd continuumtutorial
