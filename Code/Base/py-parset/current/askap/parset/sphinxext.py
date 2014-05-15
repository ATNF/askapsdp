# Copyright (c) 2012-2013 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
"""
:class:`ParameterSetDirective` implements the ``parameterset`` -directive.
"""
import os

# Import required docutils modules
from docutils.parsers.rst import Directive, directives, Parser
from docutils.parsers.rst.directives.tables import ListTable
from docutils import nodes, utils

from askap.parset import ParameterSet, encode


class DirectiveTemplate(Directive):
    """
    Template intended for directive development, providing
    few handy functions
    """

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



class ParameterSetDirective(ListTable, DirectiveTemplate):
    """
    This implements the directive.
    Directive allows to create RST tables from the contents
    of the ParameterSet file. The functionality is very similar to
    csv-table (docutils) and xmltable (:mod:`sphinxcontrib.xmltable`).

    Example of the directive:

    .. code-block:: rest

        .. parameterset:: path/to/parset_file.parset


    """
    required_arguments = 1
    has_content = False
    option_spec = {
        'key': directives.unchanged,
        'class': directives.class_option,
        'show-title': directives.flag,
        'show-key': directives.flag,
        'show-header': directives.flag,
    }

    def run(self):
        """
        Implements the directive
        """
        # Get content and options
        file_path = self.arguments[0]
        use_title = 'show-title' in self.options
        use_header = 'show-header' in self.options
        main_key = self.options.get('key', None)
        show_key = 'show-key' in self.options
        if not file_path:
            return [self._report('file_path -option missing')]

        # Transform the path suitable for processing
        file_path = self._get_directive_path(file_path)

        parset = ParameterSet(file_path)
        if main_key:
            parset = parset[main_key]

        title, messages = self.make_title()

        if not parset:
            return [nodes.paragraph(text='')]

        table_data = []
        docparser = Parser()
        # Iterates rows: put the given data in rst elements
        for key in parset.keys():
            the_val = encode(parset[key])
            the_doc = parset.get_doc(key) or ''
            if main_key and show_key:
                key = ".".join([main_key.split(".")[-1],key])
            node1 = nodes.strong(text=key)
            node2 = nodes.literal(text=the_val)
            subdoc = utils.new_document('<>', self.state.document.settings)
            docparser.parse(the_doc, subdoc)
            node3 = subdoc.children
            table_data.append([node1, node2, node3])


        col_widths = self.get_column_widths(3)
        self.check_table_dimensions(table_data, 0, 0)
        header_rows = 0
        if use_header:
            header_rows = 1
            table_data.insert(0, [nodes.strong(text="Key"),
                                  nodes.strong(text="Default"),
                                  nodes.strong(text="Description"),
                                  ])

        # Generate the table node from the given list of elements
        table_node = self.build_table_from_list(table_data, col_widths, 
                                                header_rows, 0)

        # Optional class parameter
        table_node['classes'] += self.options.get('class', [])

        if use_title and title:
            if main_key:
                ttxt = title.astext()
                title = nodes.title(text="".join([ttxt,' (',main_key,')']))
            table_node.insert(0, title)

        return [table_node] + messages


def setup(app):
    """
    Extension setup, called by Sphinx
    """
    app.add_directive('parameterset', ParameterSetDirective)
