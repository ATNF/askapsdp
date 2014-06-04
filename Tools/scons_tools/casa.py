# Copyright (c) 2009 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

import os
import re
import sys
import platform
import SCons

def generate(env):
    env["LEXFLAGS"] = SCons.Util.CLVar("-P${SOURCE.filebase}")
    env["YACCFLAGS"] = SCons.Util.CLVar("-p ${SOURCE.filebase}")


    def DarwinDevSdk():
        import platform        
        devpath = { "4" : "/Developer/SDKs/MacOSX10.4u.sdk",
                    "5" : "/Developer/SDKs/MacOSX10.5.sdk",
                    "6" : "/Developer/SDKs/MacOSX10.6.sdk" }
        version = platform.mac_ver()[0].split(".")
        if version[0] != '10' or int(version[1]) < 4:
            print "Only Mac OS X >= 10.4 is supported"
            env.Exit(1)
        return devpath[version[1]]
    env.DarwinDevSdk = DarwinDevSdk

    def AddCasaPlatform():
	pd = { "darwin": [],
	       "64bit": [],
	       "linux2": []
	       }
	# -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
	platfdefs = []
	sysplf = sys.platform
	sysarch = platform.architecture()[0]
	if sysarch == '64bit':
	    platfdefs += pd["64bit"]
            platfdefs += ["-fPIC"]
            #don't know why but lib*.a needs to have -fPIC here
            env.AppendUnique(SHFORTRANFLAGS=["-fPIC"])
            env.AppendUnique(FORTRANFLAGS=["-fPIC"])
	else:
	    platfdefs += pd[sysplf]
        env.AppendUnique(CPPFLAGS=platfdefs)
	if env["PLATFORM"] == 'darwin':
            uniarch = env.get("universal", False)
            flags = []
            if uniarch:
                for i in uniarch.split(','):		
                    flags += ['-arch', i]
                ppflags =  flags + ['-isysroot' , env.DarwinDevSdk() ]
                linkflags = flags + ['-Wl,-syslibroot,%s' %  env.DarwinDevSdk()]
                env.Append(CPPFLAGS=ppflags)
                env.Append(FORTRANFLAGS=ppflags)
                env.Append(SHFORTRANFLAGS=ppflags)
                env.Append(SHLINKFLAGS=linkflags)
                env.Append(LINKFLAGS=linkflags)
                # otherwise darwin puts builddir into the name
            env.Append(SHLINKFLAGS=["-install_name", "${TARGET.file}"])
            env.Append(SHLINKFLAGS=["-single_module"])
            
    AddCasaPlatform()

    def CheckCasaLib(context, lib):
        context.Message("Checking casa library '%s'..."%lib)
        context.Result(r)
        return r

def exists(env):
    return true
