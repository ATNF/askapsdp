from askapdev.rbuild.builders import Autotools as Builder
import  askapdev.rbuild.utils as utils

platform = utils.get_platform()

if platform['system'] == 'Darwin':
    builder = Builder(confcommand='Configure darwin64-x86_64-cc')

    # This works around the problem "Undefined symbols for architecture x86_64:
    # _OPENSSL_ia32cap_P" experienced in using OpenSSL in the "casdaupload"
    #  utility to generate SHA1 checksums. See Jira issue ASKAPSDP-1646
    builder.add_option('no-asm')

elif platform['system'] == 'Linux' and platform['architecture'] == '64bit':
    builder = Builder(confcommand='Configure linux-x86_64')
else:
    builder = Builder(confcommand='config')

if platform['architecture'] == '64bit':
    builder.add_option('-fPIC')

builder.remote_archive = "openssl-1.0.1h.tar.gz"
builder.add_option('no-shared')
builder.parallel = False
builder.nowarnings = True

builder.build()
