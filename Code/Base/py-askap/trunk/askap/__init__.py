import initenv

__all__ = ['get_config']

import os

def get_config(module, filename):
    """Return package relative configuration directory. This should contain ice 
    configuration and slice files.
    """
    from pkg_resources import resource_filename
    cfg = resource_filename(module, 
                            os.path.join("config", filename))
    return cfg
