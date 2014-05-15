__all__ = ["BuildError"]

import sys

class BuildError(Exception):
    """An exception to be raise for any known build process error.
    This is treated with a special :func:`sys.excepthook` as to not
    print any traceback on fail just the message prefixed by 'error: '.
    """
    pass

def except_hook(etype, value, tb):
    if isinstance(value, BuildError):
        print >>sys.stderr, "error:", value
    else:
        sys.__excepthook__(etype, value, tb)
    sys.exit(1)

sys.excepthook = except_hook
