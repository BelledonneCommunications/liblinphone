#!/usr/bin/python
#
# Copyright (c) 2010-2019 Belledonne Communications SARL.
#
# This file is part of Liblinphone.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import types
import sys

def main(argv = None):
    if argv is None:
        argv = sys.argv
    argparser = argparse.ArgumentParser(description="Generate enums documentation of the Linphone API.")
    argparser.add_argument('-o', '--outputfile', metavar='outputfile', type=argparse.FileType('w'), help="Output .rst file describing the Linphone API enums.")
    args = argparser.parse_args()
    if args.outputfile == None:
        args.outputfile = open('enums.rst', 'w')

    module = __import__('linphone', globals(), locals())

    for name in dir(module):
        if name == 'testing' or name == 'linphone':
            continue
        if type(getattr(module, name)) == types.ModuleType:
            args.outputfile.write('linphone.' + name + '\n')
            args.outputfile.write('^' * len('linphone.' + name) + '\n\n')
            args.outputfile.write(getattr(module, name).__doc__)
            args.outputfile.write('\n')

if __name__ == "__main__":
    sys.exit(main())
