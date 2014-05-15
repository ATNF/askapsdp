"""
:class:`EpicsDbDirective` implements the ``epicsdb`` -directive.
"""

import os, sys, re
import encodings
from docutils.parsers.rst import Directive, directives
from docutils import nodes, utils
from sphinx import addnodes
from sphinx.locale import _

#TODO handle expand keyword

def normalize_ref(text):
    ret = text.replace('$', '').replace('(', '').replace(')', '').replace(':', '-').replace('_', '-')
    ret = ret.lower()
    return encodings.normalize_encoding(ret).replace('_', '-')

class PVNode(nodes.General, nodes.Element):
    pass

class EpicsDbDirective(Directive):
    """
    This implements the directive.
    Directive allows to create RST tables from the contents
    of the ParameterSet file. The functionality is very similar to
    csv-table (docutils) and xmltable (:mod:`sphinxcontrib.xmltable`).

    Example of the directive:

    .. code-block:: rest

        .. epicsdb:: path/to/epics.db
           :hide-pv: hide_regex
           :hide-tag:  

    where:

    hide_regex
        regular expression of PV to hide, e.g. ``.*_RAW$`` to hide all PVs ending in _RAW
    """
    required_arguments = 1
    has_content = False
    option_spec = {
        'show-pv': directives.unchanged,
        'hide-pv': directives.unchanged,
        'hide-tag' : directives.flag
    }

    def __init__(self, *args, **kwargs):
        Directive.__init__(self, *args, **kwargs)

        # pv name valid chars including macros
        pvNameRegex = '[a-zA-Z0-9_\-:\[\]<>;$(),]+'

        self.reRecord = re.compile('\s*record\(\s*(\w+)\s*,\s*"(%s)"\s*\)' % pvNameRegex)
        self.reField = re.compile('(\s*)field\(\s*(FLNK|LNK.?|INP.?|OUT.?|DOL)\s*,\s*"(%s)(.*)"\s*\)' % pvNameRegex)
        self.reComment = re.compile('#')
        self.reVDCTComment = re.compile('#\!')
        self.reTagComment = re.compile('#\$\ *(\w+)')
        self.reExpand = re.compile('\s*expand')
        self.reEndRecord = re.compile('\s*}$')

    def _get_directive_path(self, path):
        """
        Returns transformed path from the directive
        option/content
        """
        source = self.state_machine.input_lines.source(
            self.lineno - self.state_machine.input_offset - 1)
        source_dir = os.path.dirname(os.path.abspath(source))
        path = os.path.normpath(os.path.join(source_dir, path))

        return utils.relative_path(None, path)

    def run(self):
        """
        Implements the directive
        """
        env = self.state.document.settings.env
        if not hasattr(env, 'epicsPVs'):
            env.epicsPVs = {}

        # Get content and options
        file_path = self.arguments[0]
        show_pv = self.options.get('show-pv', None)
        hide_pv = self.options.get('hide-pv', None)
        hide_tag = 'hide-tag' in self.options

        if hide_pv is not None:
            hide_pv = [re.compile(pv.strip()) for pv in hide_pv.split(',')]

        if not file_path:
            return [self._report('file_path -option missing')]

        # Transform the path suitable for processing
        file_path = self._get_directive_path(file_path)

        dbFile = open(file_path, 'r').readlines()
        file_path = os.path.basename(file_path)
        node = nodes.section()
        node['ids'] = [file_path]
        node +=  nodes.title(text=file_path)

        in_record = False
        hide_record = False
        tags = {}
        comments = []
        for line in dbFile:
            # handle dos files
            line = line.replace('\r\n','\n')

            # collect record comments
            if self.reComment.match(line):
                if self.reVDCTComment.match(line):
                    # igorne VDCT comments
                    continue

                tag = self.reTagComment.match(line)
                if tag is not None:
                    tags[tag.group(1)] = True
                    continue

                comments.append(line)
                continue

            # ignore expand blocks for now
            if self.reExpand.match(line):
                hide_record = True
                print "Ignoring db expand"
                continue

            recordMatch = self.reRecord.match(line)
            if recordMatch:
                pvName = recordMatch.group(2)
                if hide_tag and 'HIDE_PV' in tags: 
                    print "hiding tagged PV", pvName
                    hide_record = True
                    continue

                if hide_pv is not None:
                    for regex in hide_pv:
                        if regex.match(pvName):
                            print "hiding found PV", pvName
                            hide_record = True
                            continue

                in_record = True
                record_text = ''
                # where does :ref: role modify the label?
                label = normalize_ref(pvName)
                env.epicsPVs[label] = env.docname
                section = nodes.section()
                section['ids'] = [label]
                title = nodes.title(text=pvName)
                section += title
                if len(comments) > 0:
                    bullets = nodes.bullet_list()
                    for comment in comments:
                        item = nodes.list_item()
                        item += nodes.paragraph(text=comment.lstrip(' #'))
                        bullets += item
                    section += bullets

            if in_record:

                # parse the field for PV names
                fieldMatch = self.reField.match(line)
                fieldPV = '1'
                if fieldMatch:
                    indent, field, fieldPV, attrib = fieldMatch.groups()

                if not fieldPV.isdigit():
                    # expand PV names (only non-constants)
                    record_text += '%sfield(%s, ":pv:`%s`%s")\n' % (indent, field, fieldPV, attrib)
                else:
                    record_text += line

                if self.reEndRecord.match(line):
                    if not hide_record:
                        # parse record through inline rst parser to resolve PV links
                        text_nodes, messages = self.state.inline_text(record_text, self.lineno)
                        section += nodes.literal_block(record_text, '', *text_nodes, **self.options)
                        node += section
                    in_record = False
                    hide_record = False
                    comments = []
                    tags = {}

                    # add the PV to the index
                    indextext = _('%s (PV)') % pvName
                    inode = addnodes.index(entries=[('single', indextext, normalize_ref(pvName), pvName)])
                    node += inode

        return [node]

def epics_pv_role(typ, rawtext, text, lineno, inliner, options={}, content=[]):
    node = PVNode()
    node['pvname'] = text
    return [node], []

def process_pvnodes(app, doctree, fromdocname):
    env = app.builder.env

    for pvnode in doctree.traverse(PVNode):
        pvname = pvnode['pvname']
        ref = normalize_ref(pvname)

        # match against PV basename
        if ref not in env.epicsPVs:
            for pv in env.epicsPVs.keys():
                if re.search('.*' + ref + '$', pv):
                    ref = pv
                    break

        if ref in env.epicsPVs:
            # TODO will break other renderers
            ref = env.epicsPVs[ref] + '.html#' + ref

        newnode = nodes.reference(pvname, pvname, internal=True, refuri=ref)
        pvnode.replace_self([newnode])

def setup(app):
    """
    Extension setup, called by Sphinx
    """
    app.add_node(PVNode)
    app.add_directive('epicsdb', EpicsDbDirective)
    app.add_role('pv', epics_pv_role)
    app.connect('doctree-resolved', process_pvnodes)
