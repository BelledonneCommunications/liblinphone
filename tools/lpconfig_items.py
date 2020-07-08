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

from collections import defaultdict
items = defaultdict(list)

def get_files_in_coreapi_directory():
	from os import walk
	files = []
	for (dirpath, dirnames, filenames) in walk('../coreapi'):
		files.extend(filenames)
		break
	return files

def parse_file(filename):
	with open('../coreapi/' + filename, 'r') as infile:
		for line in infile:
			if 'linphone_config_get_' in line:
				parse_lpconfig_line(line)

def parse_lpconfig_line(line):
	token = line[line.find('linphone_config_get_') + len('linphone_config_get_'):]
	split = token.split('(', 1)
	item_type = split[0]
	if '_' in item_type:
		return
	
	params_split = split[1].split(',', 3)
	item_section = params_split[1]
	if item_section[0] != '"':
		return
	item_section = item_section.split('"')[1]
	
	item_name = params_split[2]
	if item_name[0] != '"':
		return
	item_name = item_name.split('"')[1]
	
	item_default_value = params_split[3].split(')')[0]
	if item_type == 'string' and item_default_value[0] != '"':
		item_default_value = '<unknown_string>'
	
	item = [item_type, item_name, item_default_value]
	items[item_section].append(item)

for files in get_files_in_coreapi_directory():
	parse_file(files)
for section, items in items.items():
	print('[' + section + ']')
	for item in items:
		print(item[1] + '=' + item[2])
	print('')
