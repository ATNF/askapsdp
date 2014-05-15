#!/usr/bin/env python
"""
Template for making scripts to run from the command line

Copyright (C) Keith Bannister 2013
"""
import time
import warnings

class ErrorTracker(object):
    def __init__(self, name, tol):
        self.name = name
        self.tol = tol
        self.maxerr = 0.
        self.maxargs = None
    
    def add_error(self, error, args):
        tol_exceeded = False
        if error > self.maxerr:
            self.maxerr = error
            self.maxargs = args
            
            if error > self.tol:
                warnings.warn('{0} Error {1} out of tol range {2} for args {3}'.format(self.name, error, self.tol, args))
                tol_exceeded = True

        self.last_tol_exceeded = tol_exceeded

        return tol_exceeded
    
    @property
    def error_ok(self):
        return self.maxerr < self.tol

    def __str__(self):
        if self.error_ok:
            msg = 'OK'
        else:
            msg = 'NOK'

            
        s = '{0} {1} tol:{2} maxerr: {3} maxargs: {4}'.format(self.name, msg, self.tol, self.maxerr, self.maxargs)
        return s

    __repr__ = __str__
    

class Timer:
    def __init__(self):
        self._start_cputime = None
        self._stop_cputime = None
        self._start_realtime = None
        self._stop_realtime = None
        self._interval = None
        self._real_interval = None

    def __enter__(self):
        self.start()
        return self

    def start(self):
        self._start_cputime = time.clock()
        self._start_realtime = time.time()

    def __exit__(self, *args):
        self.stop()


    @property
    def interval(self):
        return self._interval

    def stop(self):
        self._stop_cputime = time.clock()
        self._stop_realtime= time.time()
        self._interval = self._stop_cputime - self._start_cputime
        self._real_interval = self._stop_realtime - self._start_realtime

    def __str__(self):
        print self._real_interval, self._interval
        if self._real_interval is not None and self._interval is not None:
            s = 'Interval Real={0:0.1f} CPU={1:0.1f}s'.format(self._real_interval, self._interval)
        else:
            s = 'CPU start: %s stop: %s REAL start: %s stop: %s' % (self._start_cputime, self._stop_cputime, self._start_realtime, self._stop_realtime)
        
        return s

    __repr__ = __str__


class ResultTracker(object):
    def __init__(self):
        self.num_minus_infs = 0
        self.num_results = 0
        self.last_minus_inf_args = []
        self.last_minus_inf_kwargs = {}

    def track(self, num, *args, **kwargs):
        self.num_results += 1
        if num == -np.inf:
            self.num_minus_infs += 1
            self.last_minus_inf_args = args
            self.last_minus_inf_kwargs = kwargs
            
            if self.num_minus_infs == 1:
                warnings.warn('getting minus infs for args {}'.format(self._format_minus_inf_args()))

    def _format_minus_inf_args(self):
        import StringIO
        out = StringIO.StringIO()

        print >> out, 'Last -inf args:'
        for a in self.last_minus_inf_args:
            print >>out, repr(a)

        print 'Last -inf kwargs:'
        for k, v in self.last_minus_inf_kwargs.iteritems():
            print >> out, k, '=', repr(v)

        return out.getvalue()

        

    def print_report(self):
        print 'results N={} N-minfs{}'.format(self.num_results, self.num_minus_infs)
        print self._format_minus_inf_args()
            
    def __str__(self):
        return 'Results N=%s N-infs=%s last-minus-inf-args=%s ' % (self.num_results, self.num_minus_infs, self.last_minus_inf_args)

    __repr__ = __str__

def exit_on_exception(func):
    def new_func(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except:
            logging.error('Execption on func %s %s %s' % (func, args, kwargs), exc_info=True)
            sys.exit(1)

    return new_func

def _main():
    from optparse import OptionParser
    parser = OptionParser()
    parser.set_usage('%prog [options] args')
    parser.set_description('Script description')
    parser.add_option('-v', '--verbose', dest='verbose', action='store_true', help='Be verbose [Default %default]')
    parser.set_defaults(verbose=False)
    (values, args) = parser.parse_args()
    if values.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    

if __name__ == '__main__':
    _main()
