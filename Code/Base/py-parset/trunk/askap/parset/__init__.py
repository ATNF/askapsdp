# if used outside askapsoft
try:
    import initenv
except ImportError:
    pass
# if used outside askapsoft
try:
    from askap import logging
except ImportError:
    import logging

# module attributes
logger = logging.getLogger(__name__)                  

from askap.parset.parset import ParameterSet, decode
