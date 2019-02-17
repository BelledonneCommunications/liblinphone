#!/usr/bin/python

# Copyright (C) 2019 Belledonne Communications SARL
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import argparse
import logging
import os
import pystache
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
import genapixml as CApi
import abstractapi as AbsApi
import metadoc
import metaname

class PythonTranslator(object):
	def __init__(self):
		self.docTranslator = metadoc.PythonTranslator('Python')
		self.nameTranslator = metaname.Translator.get('Python')
		self.langTranslator = AbsApi.Translator.get('Python')
		self.forbidden_keywords = ['type', 'file', 'list', 'from', 'id', 'filter', 'dir', 'max', 'range', 'min']

	def translate_c_type(self, _ctype):
		if isinstance(_ctype, AbsApi.ClassType):
			return _ctype.name + '*'
		elif isinstance(_ctype, AbsApi.BaseType):
			return self.langTranslator.translate_base_type(_ctype)
		elif isinstance(_ctype, AbsApi.ListType):
			if _ctype.isconst:
				return 'const bctbx_list_t*'
			return 'bctbx_list_t*'
		elif isinstance(_ctype, AbsApi.EnumType):
			return 'int'

	def translate_arg_name(self, _name):
		return self.nameTranslator.translate_arg_name(_name)

###############################################################################

	def create_c_listener_constructor(self, _cclass):
		cMethodDict = {}
		cMethodDict['c_prototype'] = _cclass.name.to_camel_case(fullName=True) + 'Cbs* linphone_factory_create_' + _cclass.name.to_snake_case() + '_cbs('
		cMethodDict['c_prototype'] += 'const LinphoneFactory *factory)'
		return cMethodDict

	def create_c_get_callbacks_for_interface(self, _cclass):
		cMethodDict = {}
		cMethodDict['c_prototype'] = _cclass.name.to_camel_case(fullName=True) + 'Cbs* ' + _cclass.name.to_snake_case(fullName=True) + '_get_callbacks('
		cMethodDict['c_prototype'] += _cclass.name.to_camel_case(fullName=True) + '* ' + self.translate_arg_name(str(_cclass.name.to_camel_case()).lower())
		cMethodDict['c_prototype'] += ')'
		return cMethodDict

	def create_c_add_callback_for_interface(self, _cclass):
		cMethodDict = {}
		cMethodDict['c_prototype'] = 'void ' + _cclass.name.to_snake_case(fullName=True) + '_add_callbacks('
		cMethodDict['c_prototype'] += _cclass.name.to_camel_case(fullName=True) + '* ' + self.translate_arg_name(str(_cclass.name.to_camel_case()).lower())
		cMethodDict['c_prototype'] += ', ' + _cclass.name.to_camel_case(fullName=True) + 'Cbs* cbs)'
		return cMethodDict

	def create_c_remove_callback_for_interface(self, _cclass):
		cMethodDict = {}
		cMethodDict['c_prototype'] = 'void ' + _cclass.name.to_snake_case(fullName=True) + '_remove_callbacks('
		cMethodDict['c_prototype'] += _cclass.name.to_camel_case(fullName=True) + '* ' + self.translate_arg_name(str(_cclass.name.to_camel_case()).lower())
		cMethodDict['c_prototype'] += ', ' + _cclass.name.to_camel_case(fullName=True) + 'Cbs* cbs)'
		return cMethodDict

	def translate_c_class(self, _cclass):
		cClassDict = {}
		cClassDict['c_class_name'] = _cclass.name.to_c()
		return cClassDict

	def translate_c_callback(self, _cclass, _ccallback):
		cCallbackDict = {}

		cCallbackDict['c_name'] = _cclass.name.to_c() + _ccallback.name.to_camel_case() + 'Cb'
		cCallbackDict['c_params'] = ''
		for arg in _ccallback.args:
			if arg is not _ccallback.args[0]:
				cCallbackDict['c_params'] += ', '
			cCallbackDict['c_params'] += self.translate_c_type(arg.type) + ' ' + self.translate_arg_name(arg.name.to_c())

		return cCallbackDict

	def translate_c_callback_method(self, _cclass, _ccallback):
		cCallbackMethodDict = {}

		listenedClass = _ccallback.find_first_ancestor_by_type(AbsApi.Interface).listenedClass
		cCallbackMethodDict['c_prototype'] = 'void '
		cCallbackMethodDict['c_prototype'] += listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + _ccallback.name.to_snake_case()[3:]
		cCallbackMethodDict['c_prototype'] += '(' + _cclass.name.to_c() + '* cbs'
		cCallbackMethodDict['c_prototype'] += ', ' + _cclass.name.to_c() + _ccallback.name.to_camel_case() + 'Cb cb'
		cCallbackMethodDict['c_prototype'] += ')'

		return cCallbackMethodDict

	def translate_c_method(self, _cclass, _cmethod, static = False):
		cMethodDict = {}

		methodElems = {}
		methodElems['return'] = self.translate_c_type(_cmethod.returnType)
		methodElems['name'] = _cmethod.name.to_c()
		methodElems['params'] = '' if static else _cclass.name.to_c() + '* ' + self.translate_arg_name(str(_cclass.name.to_camel_case()).lower())
		for arg in _cmethod.args:
			if arg is not _cmethod.args[0] or not static:
				methodElems['params'] += ', '
			methodElems['params'] += self.translate_c_type(arg.type) + ' ' + self.translate_arg_name(arg.name.to_c())

		cMethodDict['c_prototype'] = "{return} {name}({params})".format(**methodElems)

		return cMethodDict

	def translate_enum(self, _enum):
		enumDict = {}
		enumDict['doc'] = _enum.briefDescription.translate(self.docTranslator)
		enumDict['name'] = _enum.name.to_camel_case()
		enumDict['values'] = []

		i = 0
		lastValue = None
		for enumValue in _enum.enumerators:
			enumValDict = {}
			enumValDict['doc'] = enumValue.briefDescription.translate(self.docTranslator)
			enumValDict['value_name'] = enumValue.name.to_camel_case()

			if isinstance(enumValue.value, int):
				lastValue = enumValue.value
			elif isinstance(enumValue.value, AbsApi.Flag):
				lastValue = 1 << enumValue.value.position
			else:
				if lastValue is not None:
					lastValue += 1
				else:
					lastValue = i

			enumValDict['value'] = str(lastValue)
			enumDict['values'].append(enumValDict)

		return enumDict

###############################################################################

	def create_listener_constructor(self, _obj):
		methodDict = {}
		methodDict['is_not_static'] = True
		methodDict['c_name'] = 'linphone_factory_create_' + _obj.name.to_snake_case() + '_cbs'
		methodDict['python_name'] = 'create_' + _obj.name.to_snake_case() + '_listener'
		methodDict['params'] = []
		methodDict['python_params'] = ''
		methodDict['computed_params'] = ''
		methodDict['has_return'] = True
		methodDict['has_return_obj'] = True
		methodDict['constructor'] = True
		methodDict['return_python_type'] = _obj.name.to_camel_case() + 'Listener'
		return methodDict

	def create_add_callbacks_method(self, _obj):
		methodDict = {}
		methodDict['is_not_static'] = True
		methodDict['c_name'] = _obj.name.to_snake_case(fullName=True) + '_add_callbacks'
		methodDict['python_name'] = 'add_listener'
		methodDict['is_add_listener'] = True

		methodDict['params'] = []
		methodDict['python_params'] = ', listener'
		methodDict['computed_params'] = ', listener_ptr'

		paramDict = {}
		paramDict['python_param_name'] = 'listener'
		paramDict['param_c_type'] = _obj.name.to_camel_case(fullName=True)
		paramDict['is_obj'] = True
		paramDict['python_param_type'] = _obj.name.to_camel_case() + 'Listener'

		methodDict['params'].append(paramDict)
		return methodDict

	def create_remove_callbacks_method(self, _obj):
		methodDict = {}
		methodDict['is_not_static'] = True
		methodDict['c_name'] = _obj.name.to_snake_case(fullName=True) + '_remove_callbacks'
		methodDict['python_name'] = 'remove_listener'
		methodDict['is_remove_listener'] = True

		methodDict['params'] = []
		methodDict['python_params'] = ', listener'
		methodDict['computed_params'] = ', listener_ptr'

		paramDict = {}
		paramDict['python_param_name'] = 'listener'
		paramDict['param_c_type'] = _obj.name.to_camel_case(fullName=True)
		paramDict['is_obj'] = True
		paramDict['python_param_type'] = _obj.name.to_camel_case() + 'Listener'
		
		methodDict['params'].append(paramDict)
		return methodDict

	def create_get_callbacks_method(self, _obj):
		propertyDict = {}
		propertyDict['python_name'] = 'listener'

		getterDict = {}
		getterDict['python_name'] = propertyDict['python_name']
		getterDict['c_name'] = _obj.name.to_snake_case(fullName=True) + '_get_callbacks'
		getterDict['is_return_obj'] = True
		getterDict['python_return_type'] = _obj.name.to_camel_case() + 'Listener'

		propertyDict['getter'] = getterDict
		return propertyDict

	def translate_callback_method(self, _obj, _method):
		callbackDict = {}

		callbackDict['python_name'] = _obj.name.to_camel_case()
		callbackDict['callback_var_name'] = _method.name.to_snake_case()
		callbackDict['callback_internal_name'] = _method.name.to_snake_case() + '_cb'

		listenedClass = _method.find_first_ancestor_by_type(AbsApi.Interface).listenedClass
		callbackDict['callback_setter'] = listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + _method.name.to_snake_case()[3:]

		callbackDict['is_single_listener'] = listenedClass.singlelistener
		callbackDict['is_multi_listener'] = listenedClass.multilistener

		callbackDict['params'] = []
		callbackDict['doc'] = _method.briefDescription.translate(self.docTranslator)
		callbackDict['c_params'] = ''
		callbackDict['first_python_param_name'] = ''
		callbackDict['computed_params'] = ''
		for arg in _method.args:
			paramDict = {}
			translated_arg_type = self.translate_c_type(arg.type)
			paramDict['python_param_name'] = self.translate_arg_name(arg.name.to_c())
			if arg is not _method.args[0]:
				callbackDict['c_params'] += ', '
				callbackDict['computed_params'] += ', '
			else:
				callbackDict['first_python_param_name'] = paramDict['python_param_name']
			callbackDict['computed_params'] += paramDict['python_param_name']
			callbackDict['c_params'] += self.translate_c_type(arg.type) + ' ' + self.translate_arg_name(arg.name.to_c())

			paramDict['param_c_type'] = translated_arg_type
			paramDict['is_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(arg.type.containedTypeDesc, AbsApi.ClassType)
			paramDict['is_string_list'] = 'bctbx_list_t*' in translated_arg_type and arg.type.containedTypeDesc.name == 'string'
			paramDict['is_obj'] = isinstance(arg.type, AbsApi.ClassType)
			paramDict['is_bool'] = translated_arg_type == 'bint'

			if paramDict['is_obj_list'] or paramDict['is_string_list']:
				callbackDict['computed_params'] += '_list'
				if paramDict['is_obj_list']:
					paramDict['dont_ref_list_elem'] = isinstance(arg.type, AbsApi.OnTheFlyListType)
					paramDict['param_c_type'] = self.translate_c_type(arg.type.containedTypeDesc)
					paramDict['python_param_type'] = arg.type.containedTypeDesc.desc.name.to_camel_case()
			elif paramDict['is_obj']:
				callbackDict['computed_params'] += '_obj'
				paramDict['python_param_type'] = arg.type.desc.name.to_camel_case()
			elif paramDict['is_bool']:
				callbackDict['computed_params'] += '_b'

			callbackDict['params'].append(paramDict)

		return callbackDict

	def translate_object_property(self, _obj, _method, getter, setter):
		propertyDict = {}
		propertyDict['python_name'] = getter.name.to_snake_case() if getter is not None else setter.name.to_snake_case()
		if propertyDict['python_name'][:3] == 'get' or propertyDict['python_name'][:3] == 'set':
			propertyDict['python_name'] = propertyDict['python_name'][4:]
		elif propertyDict['python_name'][:6] == 'enable':
			propertyDict['python_name'] = propertyDict['python_name'][:7] + '_enabled'

		getterDict = {}
		getterDict['python_name'] = propertyDict['python_name']
		if getter is not None:
			getterDict['c_name'] = getter.name.to_c()
			getterDict['doc'] = getter.briefDescription.translate(self.docTranslator)

			translated_type = self.translate_c_type(getter.returnType)
			getterDict['is_return_obj_list'] = 'bctbx_list_t*' in translated_type and isinstance(getter.returnType.containedTypeDesc, AbsApi.ClassType)
			getterDict['is_return_string_list'] = 'bctbx_list_t*' in translated_type and getter.returnType.containedTypeDesc.name == 'string'
			getterDict['is_return_obj'] = isinstance(getter.returnType, AbsApi.ClassType)
			getterDict['is_return_bool'] = translated_type == 'bint'
			getterDict['is_simple_return'] = not getterDict['is_return_obj_list'] and not getterDict['is_return_string_list'] and not getterDict['is_return_obj'] and not getterDict['is_return_bool']

			if getterDict['is_return_obj']:
				getterDict['python_return_type'] = getter.returnType.desc.name.to_camel_case()
			if getterDict['is_return_obj_list']:
				getterDict['dont_ref_list_elem'] = isinstance(getter.returnType, AbsApi.OnTheFlyListType)
				getterDict['python_return_type'] = getter.returnType.containedTypeDesc.desc.name.to_camel_case()
				getterDict['c_return_type'] = getter.returnType.containedTypeDesc.name
		else:
			getterDict['write_only'] = True

		propertyDict['getter'] = getterDict

		if setter is not None:
			setterDict = {}
			setterDict['c_name'] = setter.name.to_c()
			setterDict['python_name'] = propertyDict['python_name']
			setterDict['doc'] = setter.briefDescription.translate(self.docTranslator)

			translated_arg_type = self.translate_c_type(setter.args[0].type)
			setterDict['is_value_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(setter.args[0].type.containedTypeDesc, AbsApi.ClassType)
			setterDict['is_value_string_list'] = 'bctbx_list_t*' in translated_arg_type and setter.args[0].type.containedTypeDesc.name == 'string'
			setterDict['is_value_obj'] = isinstance(setter.args[0].type, AbsApi.ClassType)
			setterDict['is_value_bool'] = translated_arg_type == 'bint'
			setterDict['is_simple_value'] = not setterDict['is_value_obj_list'] and not setterDict['is_value_string_list'] and not setterDict['is_value_obj'] and not setterDict['is_value_bool']

			if setterDict['is_value_obj']:
				setterDict['python_value_type'] = setter.args[0].type.desc.name.to_camel_case()
			if setterDict['is_value_obj_list']:
				setterDict['python_value_type'] = setter.args[0].type.containedTypeDesc.desc.name.to_camel_case()

			propertyDict['setter'] = setterDict

		return propertyDict

	def translate_object_method(self, _obj, _method, is_static=False):
		methodDict = {}

		methodDict['is_static'] = is_static
		methodDict['is_not_static'] = not is_static

		methodDict['c_name'] = _method.name.to_c()
		methodDict['python_name'] = _method.name.to_snake_case()
		methodDict['doc'] = _method.briefDescription.translate(self.docTranslator)

		translated_type = self.translate_c_type(_method.returnType)
		methodDict['has_return'] = translated_type is not 'void'
		methodDict['has_return_obj_list'] = 'bctbx_list_t*' in translated_type and isinstance(_method.returnType.containedTypeDesc, AbsApi.ClassType)
		methodDict['has_return_string_list'] = 'bctbx_list_t*' in translated_type and _method.returnType.containedTypeDesc.name == 'string'
		methodDict['has_return_obj'] = isinstance(_method.returnType, AbsApi.ClassType)
		methodDict['has_return_bool'] = translated_type == 'bint'
		methodDict['has_simple_return'] = methodDict['has_return'] and not methodDict['has_return_obj_list'] and not methodDict['has_return_string_list'] and not methodDict['has_return_obj'] and not methodDict['has_return_bool']
		methodDict['constructor'] = False
		
		if methodDict['has_return_obj']:
			methodDict['return_python_type'] = _method.returnType.desc.name.to_camel_case()
		elif methodDict['has_return_obj_list']:
			methodDict['dont_ref_list_elem'] = isinstance(_method.returnType, AbsApi.OnTheFlyListType)
			methodDict['return_python_type'] = _method.returnType.containedTypeDesc.desc.name.to_camel_case()
			methodDict['return_c_type'] = _method.returnType.containedTypeDesc.name

		if isinstance(_method.returnType.parent, AbsApi.Method) and len(_method.returnType.parent.name.words) >= 1:
			if _method.returnType.parent.name.words == ['new'] or _method.returnType.parent.name.words[0] == 'create':
				methodDict['constructor'] = True

		methodDict['params'] = []
		methodDict['python_params'] = ''
		methodDict['computed_params'] = ''

		for arg in _method.args:
			paramDict = {}
			translated_arg_type = self.translate_c_type(arg.type)
			paramDict['python_param_name'] = self.translate_arg_name(arg.name.to_c())
			if arg is not _method.args[0] or not is_static:
				methodDict['python_params'] += ', '
				methodDict['computed_params'] += ', '
			methodDict['python_params'] += paramDict['python_param_name']
			methodDict['computed_params'] += paramDict['python_param_name']

			paramDict['param_c_type'] = translated_arg_type
			paramDict['is_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(arg.type.containedTypeDesc, AbsApi.ClassType)
			paramDict['is_string_list'] = 'bctbx_list_t*' in translated_arg_type and arg.type.containedTypeDesc.name == 'string'
			paramDict['is_obj'] = isinstance(arg.type, AbsApi.ClassType)
			paramDict['is_bool'] = translated_arg_type == 'bint'

			if paramDict['is_obj_list'] or paramDict['is_string_list']:
				methodDict['computed_params'] += '_list'
				if paramDict['is_obj_list']:
					paramDict['param_c_type'] = self.translate_c_type(arg.type.containedTypeDesc)
					paramDict['python_param_type'] = arg.type.containedTypeDesc.desc.name.to_camel_case()
			elif paramDict['is_obj']:
				methodDict['computed_params'] += '_ptr'
				paramDict['python_param_type'] = arg.type.desc.name.to_camel_case()
			elif paramDict['is_bool']:
				methodDict['computed_params'] += '_b'

			methodDict['params'].append(paramDict)

		return methodDict

	def translate_object(self, _obj, is_interface=False):
		objDict = {}

		objDict['doc'] = _obj.briefDescription.translate(self.docTranslator)
		objDict['c_name'] = _obj.name.to_c()
		objDict['python_name'] = _obj.name.to_camel_case()
		objDict['unref'] = _obj.name.to_snake_case(fullName=True) + '_unref'
		objDict['ref'] = _obj.name.to_snake_case(fullName=True) + '_ref'
		objDict['is_factory'] = objDict['python_name'] == 'Factory'
		objDict['is_not_factory'] = not objDict['is_factory']

		objDict['callbacks'] = []
		objDict['properties'] = []
		objDict['methods'] = []

		if not is_interface:
			if _obj.listenerInterface is not None:
				if _obj.multilistener:
					objDict['methods'].append(self.create_add_callbacks_method(_obj))
					objDict['methods'].append(self.create_remove_callbacks_method(_obj))
				if _obj.singlelistener:
					objDict['properties'].append(self.create_get_callbacks_method(_obj))

			for _method in _obj.classMethods:
				objDict['methods'].append(self.translate_object_method(_obj, _method, True))
			for _method in _obj.properties:
				objDict['properties'].append(self.translate_object_property(_obj, _method, _method.getter, _method.setter))
			for _method in _obj.instanceMethods:
				objDict['methods'].append(self.translate_object_method(_obj, _method))
		else:
			for _method in _obj.instanceMethods:
				objDict['callbacks'].append(self.translate_callback_method(_obj, _method))

		return objDict

###############################################################################

class Pylinphone(object):
	def __init__(self, parser):
		self.c_classes = []
		self.c_callbacks = []
		self.c_methods = []
		self.enums = []
		self.objects = []
		self.factory_constructors = []

		translator = PythonTranslator()
		for _interface in parser.namespace.interfaces:
			self.c_classes.append(translator.translate_c_class(_interface))
			self.objects.append(translator.translate_object(_interface, True))

			for method in _interface.instanceMethods:
				self.c_callbacks.append(translator.translate_c_callback(_interface, method))
				self.c_methods.append(translator.translate_c_callback_method(_interface, method))

		for _enum in parser.namespace.enums:
			self.enums.append(translator.translate_enum(_enum))

		for _class in parser.namespace.classes:
			if _class.listenerInterface is not None:
				if _class.multilistener:
					self.c_methods.append(translator.create_c_add_callback_for_interface(_class))
					self.c_methods.append(translator.create_c_remove_callback_for_interface(_class))
					self.factory_constructors.append(translator.create_listener_constructor(_class))
				if _class.singlelistener:
					self.c_methods.append(translator.create_c_get_callbacks_for_interface(_class))
				self.c_methods.append(translator.create_c_listener_constructor(_class))

			self.c_classes.append(translator.translate_c_class(_class))
			self.objects.append(translator.translate_object(_class))

			for method in _class.classMethods:
				self.c_methods.append(translator.translate_c_method(_class, method, True))
			for method in _class.properties:
				if method.getter is not None:
					self.c_methods.append(translator.translate_c_method(_class, method.getter))
				if method.setter is not None:
					self.c_methods.append(translator.translate_c_method(_class, method.setter))
			for method in _class.instanceMethods:
				self.c_methods.append(translator.translate_c_method(_class, method))
			
		for _obj in self.objects:
			if _obj['python_name'] == 'Factory':
				_obj['methods'].extend(self.factory_constructors)
				break

class Setup(object):
	def __init__(self, version, lib, include):
		self.version = version
		self.lib_dir = lib
		self.include_dir = include

###############################################################################

def render(renderer, item, path):
	tmppath = path + '.tmp'
	content = ''
	with open(tmppath, mode='w') as f:
		f.write(renderer.render(item))
	with open(tmppath, mode='rU') as f:
		content = f.read()
	with open(path, mode='w') as f:
		f.write(content)
	os.unlink(tmppath)

if __name__ == '__main__':
	import subprocess
	git_version = subprocess.check_output(["git", "describe"]).strip()

	argparser = argparse.ArgumentParser(description='Generate source files for the Python wrapper')
	argparser.add_argument('xmldir', type=str, help='Directory where the XML documentation of the Linphone\'s API generated by Doxygen is placed')
	argparser.add_argument('-o --output', type=str, help='the directory where to generate the source files', dest='outputdir', default='.')
	argparser.add_argument('-l --libdir', type=str, help='the directory where the liblinphone.so is installed', dest='lib_dir', default='.')
	argparser.add_argument('-i --includedir', type=str, help='the directory where the headers are installed', dest='include_dir', default='.')
	args = argparser.parse_args()
	
	loglevel = logging.INFO
	logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

	entries = os.listdir(args.outputdir)

	project = CApi.Project()
	project.initFromDir(args.xmldir)
	project.check()

	parser = AbsApi.CParser(project)
	# Some of the following take as param or return void * or unsigned int * etc...
	parser.functionBl = \
		['linphone_factory_clean',\
		'linphone_factory_get',\
		'linphone_event_get_from',\
		'linphone_vcard_get_belcard',\
		'linphone_core_get_current_vtable',\
		'linphone_config_get_range',\
		'linphone_call_zoom_video',\
		'linphone_buffer_set_content',\
		'linphone_buffer_new_from_data',\
		'linphone_content_set_buffer',\
		'linphone_factory_create_buffer_from_data',\
		'linphone_factory_create_core_2',\
		'linphone_factory_create_core_3',\
		'linphone_factory_create_core_with_config_2',\
		'linphone_factory_create_core_with_config_3',\
		'linphone_core_create_local_player',\
		'linphone_buffer_get_content',\
		'linphone_call_get_native_video_window_id',\
		'linphone_call_set_native_video_window_id',\
		'linphone_core_get_native_video_window_id',\
		'linphone_core_set_native_video_window_id',\
		'linphone_content_get_buffer',\
		'linphone_core_get_native_preview_window_id',\
		'linphone_core_set_native_preview_window_id',\
		'linphone_core_get_zrtp_cache_db',]
	parser.classBl += 'LinphoneCoreVTable'
	parser.methodBl.remove('getCurrentCallbacks')
	parser.enum_relocations = {} # No nested enums
	parser.parse_all()
	renderer = pystache.Renderer()

	wrapper = Pylinphone(parser)
	setup = Setup(git_version, args.lib_dir, args.include_dir)

	render(renderer, wrapper, args.outputdir + '/pylinphone.pyx')
	render(renderer, setup, args.outputdir + '/setup.py')
