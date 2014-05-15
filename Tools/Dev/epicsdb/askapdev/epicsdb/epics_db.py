# Epics Database creation from CSV
#
# @copyright (c) 2011 CSIRO
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
#
# @author Craig Haskins <Craig.Haskins@csiro.au>
#
'''
Generate EPICS database from CSV file according to format defined in
http://pm.atnf.csiro.au/askap/projects/cmpt/wiki/DGS2EPICS_register_file
'''

import csv
import re
import os, sys
import time

MAX_RECORD_BASE_NAME = 40

RAW_IN_SUFFIX = '_RAW'
RAW_OUT_SUFFIX = '_CTRL'
IN_SUFFIX = '_I'
OUT_SUFFIX = '_O'

SPECIAL_FIELDS = ['NUM', 'LONGDESC', 'REG_NAME', 'REC_TYPE', 'REC_DIR', 'ADDR', 'SHIFT', 'MASK', 'DESC_FALSE', 'DESC_TRUE', 'LINKPV', 'HIDE']
INFO_FIELDS = ['autosaveFields']

# this should really be built from the dbd's but that is too much work...
EPICS_FIELDS = { 'common'   : ['DESC', 'DTYP', 'FLNK', 'SCAN'],
                 'calc'     : ['CALC'],
                 'input'    : ['INP', 'INPA','INPB','INPC','INPD','INPE',
                               'INPF', 'INPG', 'INPH', 'INPI', 'INPJ', 'INPK', 'INPL', 'INPM',
                               'INPN', 'INPO', 'INPP', 'INPQ', 'INPR', 'INPS', 'INPT', 'INPU', 'INP$(inpIndex)'],
                 'fanout'   : ['LNK1', 'LNK2', 'LNK3', 'LNK4', 'LNK5', 'LNK6'],
                 'misc'     : ['OMSL'],
                 'output'   : ['DOL', 'OUT', 'OUTA', 'OUTB'],
                 'binary'   : ['COSV', 'OSV', 'ZSV', 'SHFT', 'ZRST', 'ONST'],
                 'waveform' : ['NELM', 'FTVL'],
                 'analog'   : [ 'EGU', 'VAL', 'LOPR', 'HOPR'],
                 'alarm'    : ['HIHI', 'LOLO', 'HIGH', 'LOW', 'HHSV', 'LLSV', 'HSV', 'LSV', 'HYST']
                }
ALL_EPICS_FIELDS = sum([EPICS_FIELDS[x] for x in ['common', 'calc', 'input', 'fanout', 'misc', 'output', 'binary', 'waveform', 'analog', 'alarm']], [])
EPICS_PREFIX_FIELDS = EPICS_FIELDS['input'] + EPICS_FIELDS['output'] + ['FLNK', 'SIML', 'SIOL', 'SDIS', 'LINKPV']

EpicsInputTypes = { 'a'         : 'ai',
                    'b'         : 'bi',
                    'string'    : 'stringin',
                    'long'      : 'longin',
                    'longin'    : 'longin',
                    'mbb'       : 'mbbi',
                    'mbbDirect' : 'mbbiDirect',
                    'calc'      : 'calc',
                    'waveform'  : 'waveform',
                    'subArray'  : 'subArray',
                    'aSub'      : 'aSub'
                   }

EpicsOutputTypes = {'a'         : 'ao',
                    'b'         : 'bo',
                    'string'    : 'stringout',
                    'long'      : 'longout',
                    'longin'    : 'longout',
                    'mbb'       : 'mbbo',
                    'mbbDirect' : 'mbboDirect',
                    'calc'      : 'calcout',
                    'waveform'  : 'waveform',
                    'subArray'  : 'subArray',
                    'aSub'      : 'aSub'
                   }

class OrderedDict(dict):
    '''
    Dictionary which preserves insert order
    '''
    def __init__(self, *args, **kwargs):
        dict.__init__(self, *args, **kwargs)
        self._order = self.keys()

    def __setitem__(self, key, value):
        dict.__setitem__(self, key, value)
        if key in self._order:
            self._order.remove(key)
        self._order.append(key)

        #useful trap for debugging
        #if 'REG_NAME' == key and 'CTRL_SYS_GOODBEEF_RAW' == value:
        #    raise RuntimeError('Trapped')

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self._order.remove(key)

    def copy(self):
        newDict = OrderedDict()
        for (key, value) in self.ordered_items():
            newDict[key] = value
        return newDict

    def order(self):
        return self._order[:]

    def ordered_items(self):
        '''returns list of (key, val) tuples in inserted order'''
        return [(key, self[key]) for key in self._order]

class CommentedFile:
    def __init__(self, f, commentstring="#"):
        self.f = f
        self.commentstring = commentstring

    def next(self):
        line = self.f.next()
        while line.startswith(self.commentstring) or 0 == len(line.strip()):
            line = self.f.next()
        return line

    def __iter__(self):
        return self

class EpicsRecord(OrderedDict):
    pass

class EpicsDB(OrderedDict):
    def __init__(self, debug=False):
        self._fanoutNum = 1
        self.debug = debug
        OrderedDict.__init__(self)

    def load_from_csv(self, inputFile, update=False):
        ''' convert csv row into a dictionary of dictionaries
            point[pvname] = recordDictionary
        '''
        
        try:
            reader = csv.DictReader(CommentedFile(inputFile),skipinitialspace=True)
            for row in reader:
                updateOnly = False
                if len(row['REG_NAME']) >  MAX_RECORD_BASE_NAME:
                    raise RuntimeError('%s too long' % row['REG_NAME'])

                recName = row['REG_NAME']
                if update:
                    if recName not in self:
                        self._warn("%s doesn't exist in primary file so cannot patch" % recName)
                        continue
                    rec = self[recName]
                else:
                    rec = EpicsRecord()

                # populate record from row
                for field in reader.fieldnames:
                    if 0 == len(field):
                        continue

                    if None != row[field] and len(row[field]) > 0:
                        if update and 'REG_NAME' == field:
                            # don't modify reg name on patching
                            continue
                        elif 'HIDE' == field:
                            self._info('HIDING %s' % recName)
                            rec['HIDE'] = row['HIDE'].strip().lower() in ('yes', 'true', '1')
                        else:
                            rec[field] = row[field]
                    elif field not in rec:
                        rec[field] = ''

                if 'HIDE' not in rec:
                    rec['HIDE'] = False

                # truncate DESC field
                if 'LONGDESC' not in rec or 0 == len(rec['LONGDESC']):
                    rec['LONGDESC'] = rec['DESC']
                if 'DESC' in rec and len(rec['DESC']) > 40:
                    rec['DESC'] = rec['DESC'][:40]
                    self._info('truncated DESC field for %s' % recName)

                if update:
                    continue

                # remove non-waveform fields
                if rec['REC_TYPE'] not in ('waveform', 'subArray'):
                    for field in EPICS_FIELDS['waveform']:
                        if field in rec:
                            del rec[field]
                else:
                    rec['FTVL'] = rec['FTVL'].upper()

                # remove alarm fields for types without alarms
                if rec['REC_TYPE'] in ('b', 'mbb', 'waveform', 'subArray', 'string', 'aSub'):
                    for field in EPICS_FIELDS['alarm']:
                        if field in rec:
                            del rec[field]
                else:
                    # add Alarm Severitie
                    AlarmToSevMap = {'HIHI' : 'HHSV', 'LOLO' : 'LLSV', 'HIGH' : 'HSV', 'LOW' : 'LSV'}
                    SevToLevelMap = {'HHSV' : 'MAJOR', 'LLSV' : 'MAJOR', 'HSV' : 'MINOR', 'LSV' : 'MINOR'}
                    autoSaveFields = []
                    for alarm, sev in AlarmToSevMap.items():
                        if alarm in rec and len(rec[alarm]) > 0:
                            autoSaveFields.append(alarm)
                            if len(rec[sev]) == 0:
                                rec[sev] = SevToLevelMap[sev]
                    if len(autoSaveFields) > 0:
                        rec['autosaveFields'] = ' '.join(autoSaveFields)

                # add DTYP, asyn stuff
                if 'mbb' == rec['REC_TYPE']:
                    rec['DTYP'] = 'Raw Soft Channel'
                    if 'INP' not in rec:
                        self._warn('Empty input for %s %s' % (rec['REC_TYPE'], rec['REG_NAME']))
                elif 'a' == rec['REC_TYPE']:
                    rec['DTYP'] = 'asynFloat64'
                elif 'b' == rec['REC_TYPE']:
                    rec['DTYP'] = 'Raw Soft Channel'
                    rec['ONAM'] = 'On'
                    rec['ZNAM'] = 'Off'
                    if 'DESC_TRUE' in rec:
                        rec['ONAM'] = rec['DESC_TRUE']
                    if 'DESC_FALSE' in rec:
                        rec['ZNAM'] = rec['DESC_FALSE']
                    if 'INP' not in rec:
                        self._warn('Empty input for %s %s' % (rec['REC_TYPE'], rec['REG_NAME']))
                elif 'long' == rec['REC_TYPE']:
                    rec['DTYP'] = 'asynInt32'
                elif 'mbbDirect' == rec['REC_TYPE']:
                    rec['DTYP'] = 'asynUInt32Digital'
                elif 'string' == rec['REC_TYPE']:
                    rec['DTYP'] = 'asynOctetRead'
                elif 'waveform' == rec['REC_TYPE']:
                    if rec['FTVL'] in ('CHAR', 'UCHAR'):
                        if 'ro' in rec['REC_DIR']:
                            rec['DTYP'] = 'asynOctetRead'
                        else:
                            rec['DTYP'] = 'asynOctetWrite'
                    elif rec['FTVL'] in ('SHORT', 'USHORT'):
                        rec['DTYP'] = 'asynInt16ArrayIn'
                    elif rec['FTVL'] in ('LONG', 'ULONG'):
                        rec['DTYP'] = 'asynInt32ArrayIn'
                    elif rec['FTVL'] in 'FLOAT':
                        rec['DTYP'] = 'asynFloat32ArrayIn'
                    elif rec['FTVL'] in 'DOUBLE':
                        rec['DTYP'] = 'asynFloat64ArrayIn'
                    else:
                        raise RuntimeError('Unknown FTVL %s in waveform/subArray %s', rec['FTVL'], rec['REG_NAME'])

                # add asyn register addressing
                if 'mbbDirect' == rec['REC_TYPE']: 
                    rec['INP'] = '@asynMask($(PORT),$(ADDR),%s)%s' % (rec['MASK'], rec['REG_NAME'].lstrip('%'))
                elif 'DTYP' in rec and rec['DTYP'].startswith('asyn'):
                    rec['INP'] = '@asyn($(PORT),$(ADDR))%s' % rec['REG_NAME'].lstrip('%')

                if rec['REC_TYPE'] in ('calc', 'aSub', 'subArray'):
                    # fix up names
                    for field in EPICS_PREFIX_FIELDS:
                        if field in rec and len(rec[field]) > 0:
                            if rec[field].startswith('%'):
                                rec[field] = rec[field].lstrip('%')
                            elif not rec[field].startswith('$'):
                                rec[field] += RAW_IN_SUFFIX

                    # force maximise severity for calc inputs
                    for field in EPICS_FIELDS['input']:
                        if field in rec and len(rec[field]) > 0 and ' MS' not in rec[field]:
                            rec[field] = rec[field] + ' MS'

                    if 'ro' == rec['REC_DIR']:
                        rec['REC_TYPE'] = EpicsInputTypes[rec['REC_TYPE']]
                    elif 'wo' == rec['REC_DIR']:
                        rec['REC_TYPE'] = EpicsOutputTypes[rec['REC_TYPE']]
                    elif 'calc' == rec['REC_TYPE']:
                        raise RuntimeError('Unexpected record direction on record %s' % rec['REG_NAME'])
                    self._info('added record %s %s' % (recName, rec['HIDE']))
                    self[recName] = rec
                elif 'asyn' in rec['REC_TYPE']:
                    # asyn parameters not records so hide them from epics
                    rec['HIDE'] = True
                    rec['DTYP'] = rec['REC_TYPE']
                    self[recName] = rec
                    self._info('added asyn param %s' % recName)
                else:
                    # generate records based on I/O direction(s)
                    outputRec = rec.copy()
                    if rec['REC_DIR'] in ('rw', 'ro'):
                        if rec['REC_TYPE'] not in EpicsInputTypes:
                            raise RuntimeError('unknown type %s' % rec['REC_TYPE'])
                        rec['REC_TYPE'] = EpicsInputTypes[rec['REC_TYPE']]
                        
                        if recName.startswith('%'):
                            recName = recName.lstrip('%')
                            rec['REG_NAME'] = rec['REG_NAME'].lstrip('%')
                            suffix = ''
                        elif 'DTYP' in rec and rec['DTYP'].startswith('asyn'):
                            suffix = RAW_IN_SUFFIX
                        else:
                            suffix = IN_SUFFIX

                        self[recName + suffix] = rec
                        self._info('added record %s %s' % (recName + suffix, rec['HIDE']))

                        # fix up input record (add suffix and maximise serverity)
                        if 'INP' in rec and len(rec['INP']) > 0 and '@asyn' not in rec['INP']:
                            rec['INP'] += RAW_IN_SUFFIX  + ' MS'
                        elif '@asyn' in rec['INP'] and ('SCAN' not in rec or 0 == len(rec['SCAN'])):
                            rec['SCAN'] = 'I/O Intr'

                    if rec['REC_DIR'] in ('rw', 'wo'):
                        if outputRec['REC_TYPE'] not in EpicsOutputTypes:
                            raise RuntimeError('unknown type %s' % outputRec['REC_TYPE'])
                        outputRec['REC_TYPE'] = EpicsOutputTypes[outputRec['REC_TYPE']]
                        # fix up output records
                        if rec['REC_TYPE'] in ('waveform', 'subArray'):
                            outputRec['DTYP'] = outputRec['DTYP'].replace('ArrayIn', 'ArrayOut')
                        elif 'INP' in outputRec:
                            outputRec['OUT'] = outputRec['INP']
                            del outputRec['INP']
                            if 'asynOctetRead' in outputRec['DTYP']:
                                outputRec['DTYP'] = 'asynOctetWrite'
                            if '@asyn' not in outputRec['OUT'] and outputRec['REC_TYPE'] not in ('bo', 'mbbo'):
                                outputRec['OUT'] += RAW_OUT_SUFFIX
                            if 'OMSL' not in outputRec or 0 == len(outputRec['OMSL']):
                                outputRec['OMSL'] = 'supervisory'

                        if recName.startswith('%'):
                            recName = recName.lstrip('%')
                            outputRec['REG_NAME'] = outputRec['REG_NAME'].lstrip('%')
                            suffix = ''
                        elif 'DTYP' in rec and rec['DTYP'].startswith('asyn'):
                            suffix = RAW_OUT_SUFFIX
                        else:
                            suffix = OUT_SUFFIX
                        
                        self[recName + suffix] = outputRec
                        self._info('added record %s %s' % (recName + suffix, rec['HIDE']))

        except csv.Error, e:
            sys.exit('input line %d: %s' % (reader.line_num, e))

    def load_from_db(self, inputFile):
        currRecord = None
        recordCount = 1
        for line in inputFile:
            # handle dos files
            line = line.replace('\r\n','\n')

            # ignore comments
            if re.search('^#', line):
                continue

            recordMatch = re.search('record\((\w+),\ *"(.*)"\)', line)
            if not recordMatch:
                recordMatch = re.search('record\((\w+),\ *(.*)\)', line)

            if recordMatch:
                currentRecord = {'REC_TYPE' : recordMatch.group(1)}
                self[recordMatch.group(2)] = currentRecord
                continue

            rowMatch = re.search('field\((.*), "(.*)"\ *\)', line)
            if rowMatch:
                currentRecord[rowMatch.group(1)] = rowMatch.group(2)
                continue

            if re.search('^}$', line):
                recordCount += 1
                currentRecord['NUM'] = recordCount

    def add_fanout(self, srcRecName):
        ''' add fanout record to given source record
        '''
        srcRec = self[srcRecName]
        # create a fanout record
        fanoutName = 'fanout%02d' % self._fanoutNum
        self._fanoutNum += 1
        fanoutRec = EpicsRecord()
        fanoutRec['FLNK'] = srcRec['FLNK']
        fanoutRec['REC_TYPE'] = 'fanout'
        fanoutRec['REG_NAME'] = fanoutName
        self[fanoutName] = fanoutRec

        # point srcRec to new fanout
        srcRec['FLNK'] = fanoutName
        self._info('creating fanout %s' % fanoutName)
        self._info('forward linking %s.FLNK -> %s' % (srcRecName, fanoutName))
        self._info('forward linking %s.FLNK -> %s' % (fanoutName, fanoutRec['FLNK']))
        return fanoutRec

    def mask_to_shift(self, maskStr):
        '''
        return shift given mask
        '''
        mask = int(maskStr, 16)
        shift = 0
        while not (mask&0x1):
            mask >>= 1
            shift += 1

        return shift

    def num_bits(self, maskStr):
        '''
        count number of bits set in mask
        '''
        mask = int(maskStr, 16)
        count = 0
        while (mask):
            mask &= mask - 1
            count += 1

        shift = self.mask_to_shift(maskStr)

        # check that mask bits are contiguous
        assert( (int(maskStr, 16) >> shift) == ((1 << count) - 1))

        return count

    def fixup_binary_records(self, chain=False):
        '''
        for binary inputs:

            set state descriptions
            convert bi to single bit mbbi (for Raw Soft Channel Support with SHIFT)
            set SHFT and NOBT from mask
            add forward links from source

        for binary outputs:

            set bo's to asynUint32Digitals
        '''
        for (recName, rec) in self.items():
            if rec['REC_TYPE'] in ('bi', 'bo', 'mbbi', 'mbbo'):
                ioDir = {'bi' : 'INP', 'bo' : 'OUT', 'mbbi' : 'INP', 'mbbo' : 'OUT'}[rec['REC_TYPE']]
                self._info('binary %s %s <-> %s' % (rec['REC_TYPE'], recName, rec[ioDir]))
                if ioDir not in rec:
                    continue
                ioRecName = rec[ioDir].rstrip(' MS')

                if ioDir == 'INP':
                    nobt = self.num_bits(rec['MASK'])
                    rec['SHFT'] = '%d' % self.mask_to_shift(rec['MASK'])
                    rec['NOBT'] = '%d' % nobt
                    if 1 == nobt:
                        # convert bi to 1 bit mbbi records
                        rec['REC_TYPE'] = 'mbbi'

                        rec['ZRVL'] = '0'
                        rec['ONVL'] = '1'
                        if 'ZNAM' in rec:
                            rec['ZRST'] = rec['ZNAM']
                            del rec['ZNAM']

                        if 'ONAM' in rec:
                            rec['ONST'] = rec['ONAM']
                            del rec['ONAM']
                    else:
                        # convert multi bit binary to mbbiDirect
                        rec['REC_TYPE'] = 'mbbiDirect'

                    del rec['ZSV']
                    del rec['OSV']

                    # add forward link from longin
                    self.add_link(ioRecName, recName, chain=chain)
                else:
                    # for binary outputs use asynUnit32Ditial
                    rec['DTYP'] = 'asynUInt32Digital'
                    rec['OUT'] = '@asynMask($(PORT),$(ADDR), %s)%s' % (rec['MASK'], rec['OUT'])
    

    def add_link(self, srcRecName, dstRecName, chain=False):
        ''' adds a forward link from src record to dst record
            will chain or fanout links as necessary
        '''
        srcRec = self[srcRecName]

        self._info('adding forward link from %s to %s' % (srcRecName, dstRecName))
        # traverse forward link chain
        while True:
            if 'FLNK' not in srcRec or 0 == len(srcRec['FLNK']) or srcRec['FLNK'] not in self:
                # end of chain
                self._info('end of chain')
                break

            if srcRec['FLNK'] == dstRecName:
                self._info('already linked, aborting %s -> %s' %(srcRecName, dstRecName))
                return

            if chain:
                self._info('chaining flink %s -> %s' % (srcRecName, srcRec['FLNK']))
                srcRecName = srcRec['FLNK']
                srcRec = self[srcRecName]
            else:
                if 'fanout' not in srcRec['FLNK']:
                    fanoutRec = self.add_fanout(srcRecName)
                else:
                    fanoutRec = self[srcRec['FLNK']]

                # find an empty link
                for i in range(1,6):
                    lnkNum = 'LNK%d' % i
                    if lnkNum not in fanoutRec:
                        break

                if i <= 6:
                    fanoutRec[lnkNum] = dstRecName
                    self._info('forward linking %s.%s -> %s' % (fanoutRec['REG_NAME'], lnkNum, fanoutRec[lnkNum]))
                else:
                    # TODO chain fanouts in this case
                    raise RuntimeError('too many forward links for fanout, need to chain fanouts (not yet implemented)')
                break

        self._info('forward linking %s -> %s' % (srcRecName, dstRecName))
        if 'FLNK' in srcRec and len(srcRec['FLNK']):
            self[dstRecName]['FLNK'] = srcRec['FLNK']

        srcRec['FLNK'] = dstRecName
                
    def add_forward_links(self, chain=False):
        ''' scan DB and fixup forward links for calc & mbbi records
        '''
        for (recName, rec) in self.ordered_items():
            if not (rec['REC_TYPE'] in ('calc', 'aSub', 'mbbi')):
                continue

            self._info('adding links for %s' % recName)
            # extract record name from PV name (which may include field name
            # and modifiers (PP, NMS etc))
            if rec['REC_TYPE'] in ('calc', 'aSub'):
                inputRecName = rec['INPA'].split(' ')[0].split('.')[0]
            else:
                inputRecName = rec['INP'].split(' ')[0].split('.')[0]

            if len(inputRecName) == 0:
                self._warn('input is missing for %s' % recName)
                continue
            elif inputRecName not in self:
                self._warn('input %s not in this database for %s' % (inputRecName, recName))
                continue

            # link input record to calc record
            self.add_link(inputRecName, recName, chain=chain)

    def add_prefix(self, prefix):
        ''' add a prefix to all record names and references in DB
        '''
        if 0 == len(prefix):
            return

        # apply prefix to any value in any of the defined prefix fields
        for (recName, rec) in self.ordered_items():
            for field in EPICS_PREFIX_FIELDS:
                if field in rec and len(rec[field]) > 0 \
                    and '@asyn' not in rec[field] \
                    and not rec[field].startswith('$'):
                        try:
                            # test for constant
                            int(rec[field])
                        except ValueError:
                            # must be a string so prefix it
                            rec[field] = prefix + rec[field]

        # now update record names
        for (recName, rec) in self.ordered_items():
            if not recName.startswith('$'):
                self[prefix + recName] = rec
                del self[recName]

    def command_line(self):
        cmdLine = os.path.basename(sys.argv[0])
        for arg in sys.argv[1:]:
            cmdLine += ' ' + arg
        return cmdLine 

    def save_to_db(self, outputFile, supressComments=False):
        ''' save DB to EPICS .db file
        '''
        if not supressComments:
            outputFile.write('# ***********\n')
            outputFile.write('# DO NOT EDIT\n')
            outputFile.write('# ***********\n')
            outputFile.write('# DB automatically generated on %s\n' % time.asctime())
            outputFile.write('# %s' % self.command_line())
            outputFile.write('\n#\n\n')
        for (recName, rec) in self.ordered_items():
            if 'HIDE' in rec and rec['HIDE']:
                continue

            fieldList = []
            fieldList += ALL_EPICS_FIELDS
            for field in sorted(rec):
                if field not in (ALL_EPICS_FIELDS + SPECIAL_FIELDS + INFO_FIELDS):
                    fieldList.append(field)

            # write special fields as a comment above record
            # so VDCT will keep track of them
            for field in SPECIAL_FIELDS:
                if field not in rec:
                    continue
                value = str(rec[field])
                if len(value) > 0 and not supressComments:
                    outputFile.write('# %s = %s\n' % (field, value))

            # write out record
            outputFile.write('record(%s, "%s") {\n' % (rec['REC_TYPE'], recName))
            for field in fieldList:
                if field not in rec:
                    continue
                value = rec[field]
                if len(value) > 0:
                    outputFile.write('    field(%s, "%s")\n' % (field, value))

            # write info fields
            for field in INFO_FIELDS:
                if field not in rec:
                    continue
                value = rec[field]
                if len(value) > 0:
                    outputFile.write('    info(%s, "%s")\n' % (field, value))


            outputFile.write('}\n\n')

    def save_to_substitution(self, outputFile):
        raise RuntimeError('Not yet implemented')
        for (recName, rec) in self.ordered_items():
            sub = '{REG_NAME=%s, ' % recName
            for (field, value) in rec.ordered_items():
                if field in ALL_EPICS_FIELDS:
                    sub += '%s = "%s", ' % (field, value)
            outputFile.write(sub.rstrip(', ') + '}')

    def save_to_csv(self, outputFile):
        raise RuntimeError('Not yet implemented')
        for (recName, rec) in self.ordered_items():
            line = ''
            for (field, value) in rec.ordered_items():
                line += '"%s",' % value
            outputFile.write(line.rstrip(',') + '\n')

    def save_to_wiki(self, outputFile):
        raise RuntimeError('Not yet implemented')
        for (recName, rec) in self.ordered_items():
            line = '|'
            for (field, value) in rec.ordered_items():
                line += '%s|' % value
            outputFile.write(line.rstrip('|') + '\n')

    def dtype_to_param_type(self, dtype):
        return  \
            {   'asynInt32'             : 'asynParamInt32',
                'asynUInt32Digital'     : 'asynParamUInt32Digital',
                'asynFloat64'           : 'asynParamFloat64',
                'asynOctetRead'         : 'asynParamOctet',
                'asynOctetWrite'        : 'asynParamOctet',
                'asynInt8ArrayIn'       : 'asynParamInt8Array',
                'asynInt8ArrayOut'      : 'asynParamInt8Array',
                'asynInt16ArrayIn'      : 'asynParamInt16Array',
                'asynInt16ArrayOut'     : 'asynParamInt16Array',
                'asynInt32ArrayIn'      : 'asynParamInt32Array',
                'asynInt32ArrayOut'     : 'asynParamInt32Array',
                'asynFloat32ArrayIn'    : 'asynParamFloat32Array',
                'asynFloat64ArrayIn'    : 'asynParamFloat64Array',
                'asynFloat32ArrayOut'   : 'asynParamFloat32Array',
            }[dtype]
              
    def save_to_seq_header(self, outputFile):
        outputFile.write('''/*
 *  @%s
 *
 *  This file was automatically generated on %s from register
 *  definition files by csv2epics and should NOT BE DIRECTLY EDITED.
 *
 *  @copyright (c) 2011 CSIRO
 *  Australia Telescope National Facility (ATNF)
 *  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *  PO Box 76, Epping NSW 1710, Australia
 *  atnf-enquiries@csiro.au
 *
 *  This file is part of the ASKAP software distribution.
 *
 *  The ASKAP software distribution is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  @author Craig Haskins<Craig.Haskins@csiro.au>
 *
 *  %s
 */

''' % (outputFile.name, time.asctime(), self.command_line()))
        first = True
        for (recName, rec) in self.ordered_items():
            if ('EVNT' in rec and len(rec['EVNT']) > 0) or 'SCAN' not in rec or 'Event' not in rec['SCAN']:
                continue
            if not first:
                outputFile.write(',\n')
                
            outputFile.write('    "%s"' % recName)
            first = False

    def save_to_asyn_header(self, outputFile, externalDec=False):
        paramList = OrderedDict()
        for (recName, rec) in self.ordered_items():
            #if 'ADDR' in rec and 0 != len(rec['ADDR']):
            if 'DTYP' in rec and 'asyn' in rec['DTYP']:
                name = rec['REG_NAME']
                paramList[name] = rec;

        if externalDec:
            decString =  'extern int AsynIndexToAddr[PL_COUNT];\n'
            decString += 'extern AsynParameter ParamList[];\n'
            decString += '#define ASYN_PARAMETER_LIST \\'
            lineCont = '\\'
        else:
            decString =  'static int AsynIndexToAddr[PL_COUNT];\n'
            decString += 'static AsynParameter ParamList[] = '
            lineCont = ''
            
        outputFile.write('''/*
 *  @%s
 *
 *  This file was automatically generated on %s from register
 *  definition files by csv2epics and should NOT BE DIRECTLY EDITED.
 *
 *  @copyright (c) 2012 CSIRO
 *  Australia Telescope National Facility (ATNF)
 *  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *  PO Box 76, Epping NSW 1710, Australia
 *  atnf-enquiries@csiro.au
 *
 *  This file is part of the ASKAP software distribution.
 *
 *  The ASKAP software distribution is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  @author Craig Haskins<Craig.Haskins@csiro.au>
 *
 *  %s
 */

#ifndef ASYN_PARAM_LIST_H
#define ASYN_PARAM_LIST_H
#define ASYN_PARAM(x)           (&ParamList[PL_##x])
#define ASYN_INDEX(x)           (ParamList[PL_##x].index)
#define ASYN_NAME(x)            (ParamList[PL_##x].name)
#define ASYN_HWADDR(x)          (ParamList[PL_##x].addr)

#define ASYN_INDEX_TO_ADDR(x)   (AsynIndexToAddr[x])

typedef struct {
    int             index;      // asyn index
    epicsUInt32     addr;       // hardware address
    asynParamType   type;       // asyn data type
    const char*     name;       // asyn name
} AsynParameter;

typedef enum {
''' % (outputFile.name, time.asctime(), self.command_line()))
        for (reg, rec) in paramList.ordered_items():
                outputFile.write('    PL_%s,\n' % reg)

        outputFile.write('''    PL_COUNT
} ParameterListEnum;

%s
{ %s\n''' % (decString, lineCont))
        for (reg, rec) in paramList.ordered_items():
                if 'ADDR' in rec and 0 != len(rec['ADDR']):
                    addr = rec['ADDR']
                else:
                    addr = '0xFFFFFFFF'
                outputFile.write('    {0, %s, %s, "%s"}, %s\n' % (addr, self.dtype_to_param_type(rec['DTYP']), reg, lineCont))
        outputFile.write('};\n')
        outputFile.write('#endif //define ASYN_PARAM_LIST_H\n')

    def _info(self, msg):
        if self.debug:
          print 'info:', msg

    def _warn(self, msg):
        print >> sys.stderr, 'warn:', msg
