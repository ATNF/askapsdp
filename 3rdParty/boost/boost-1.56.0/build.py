import os

from askapdev.rbuild.builders import Autotools as Builder

# Pick up directory structure and boost version from current directory name
boost_version = os.path.basename(os.path.abspath(os.curdir))
boost_dir = boost_version.replace('.','_').replace('-','_')

builder = Builder(pkgname=boost_dir, confcommand='bootstrap.sh',
                  buildcommand='./bjam', installcommand='./bjam install')
builder.remote_archive = "boost_1_56_0.tar.bz2"

builder.add_option('--with-libraries=python,date_time,filesystem,program_options,thread,system,regex --prefix=builder._prefix' )
builder.nowarnings = True

builder.build()
