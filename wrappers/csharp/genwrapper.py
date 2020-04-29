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
import logging
import os
import pystache
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
import genapixml as CApi
import abstractapi as AbsApi
import metadoc
import metaname

class CsharpTranslator(object):
	def __init__(self):
		self.ignore = []
		self.docTranslator = metadoc.SandCastleTranslator('CSharp')
		self.nameTranslator = metaname.Translator.get('CSharp')
		self.langTranslator = AbsApi.Translator.get('CSharp')

	def init_method_dict(self):
		methodDict = {}
		methodDict['has_impl'] = True
		methodDict['impl'] = {}
		methodDict['is_string_list'] = False
		methodDict['is_class_list'] = False
		methodDict['list_type'] = None
		methodDict['is_string'] = False
		methodDict['is_bool'] = False
		methodDict['is_class'] = False
		methodDict['is_enum'] = False
		methodDict['is_generic'] = False
		methodDict['takeRef'] = 'true'
		return methodDict

	def get_class_array_type(self, name):
		length = len(name)
		if length > 11 and name[:11] == 'IEnumerable':
			return name[12:length-1]
		return None
	
	def is_generic(self, methodDict):
		return not methodDict['is_string'] and not methodDict['is_bool'] and not methodDict['is_class'] and not methodDict['is_enum'] and methodDict['list_type'] == None

	def is_linphone_type(self, _type, isArg, dllImport=True):
		if isinstance(_type, AbsApi.ClassType):
			return False if dllImport else True
		elif isinstance(_type, AbsApi.EnumType):
			return False if dllImport else True

	def throws_exception(self, return_type):
		if isinstance(return_type, AbsApi.BaseType):
			if return_type.name == 'status':
				return True
		return False

	def translate_method(self, method, static=False, genImpl=True):
		if method.name.to_c() in self.ignore:
			raise AbsApi.Error('{0} has been escaped'.format(method.name.to_c()))

		methodElems = {}
		methodElems['return'] = method.returnType.translate(self.langTranslator)
		methodElems['name'] = method.name.to_c()
		methodElems['params'] = '' if static else 'IntPtr thiz'
		for arg in method.args:
			if arg is not method.args[0] or not static:
				methodElems['params'] += ', '
			methodElems['params'] += arg.translate(self.langTranslator)

		methodDict = {}
		methodDict['prototype'] = "static extern {return} {name}({params});".format(**methodElems)

		methodDict['doc'] = method.briefDescription.translate(self.docTranslator, tagAsBrief=True)

		methodDict['has_impl'] = genImpl
		if genImpl:
			methodDict['impl'] = {}
			methodDict['impl']['static'] = 'static ' if static else ''
			methodDict['impl']['exception'] = self.throws_exception(method.returnType)
			methodDict['impl']['type'] = method.returnType.translate(self.langTranslator, dllImport=False)
			methodDict['impl']['name'] = method.name.translate(self.nameTranslator)
			methodDict['impl']['override'] = 'override ' if method.name.translate(self.nameTranslator) == 'ToString' else ''
			methodDict['impl']['hasReturn'] = False if methodDict['impl']['type'] == "void" else True
			methodDict['impl']['c_name'] = method.name.to_c()
			methodDict['impl']['nativePtr'] = '' if static else ('nativePtr, ' if len(method.args) > 0 else 'nativePtr')

			methodDict['list_type'] = self.get_class_array_type(methodDict['impl']['type'])
			methodDict['is_string_list'] = methodDict['list_type'] == 'string'
			methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'string'

			methodDict['is_string'] = methodDict['impl']['type'] == "string"
			methodDict['is_bool'] = methodDict['impl']['type'] == "bool"
			methodDict['is_class'] = self.is_linphone_type(method.returnType, False, False) and type(method.returnType) is AbsApi.ClassType
			methodDict['is_enum'] = self.is_linphone_type(method.returnType, False, False) and type(method.returnType) is AbsApi.EnumType
			methodDict['is_generic'] = self.is_generic(methodDict)
			methodDict['takeRef'] = 'true'
			if isinstance(method.returnType.parent, AbsApi.Method) and len(method.returnType.parent.name.words) >=1:
				if method.returnType.parent.name.words == ['new'] or method.returnType.parent.name.words[0] == 'create':
					methodDict['takeRef'] = 'false'

			methodDict['impl']['args'] = ''
			methodDict['impl']['c_args'] = ''
			methodDict['impl']['clean_string_list_ptrs'] = False
			for arg in method.args:
				if arg is not method.args[0]:
					methodDict['impl']['args'] += ', '
					methodDict['impl']['c_args'] += ', '
				if self.is_linphone_type(arg.type, False, False):
					if isinstance(arg.type, AbsApi.ClassType):
						argname = arg.name.translate(self.nameTranslator)
						methodDict['impl']['c_args'] += argname + " != null ? " + argname + ".nativePtr : IntPtr.Zero"
					else:
						methodDict['impl']['c_args'] += '(int)' + arg.name.translate(self.nameTranslator)
				elif arg.type.translate(self.langTranslator, dllImport=False) == "bool":
					methodDict['impl']['c_args'] += arg.name.translate(self.nameTranslator) + " ? (char)1 : (char)0"
				elif self.get_class_array_type(arg.type.translate(self.langTranslator, dllImport=False)) is not None:
					listtype = self.get_class_array_type(arg.type.translate(self.langTranslator, dllImport=False))
					if listtype == 'string':
						methodDict['impl']['c_args'] += "StringArrayToBctbxList(" + arg.name.translate(self.nameTranslator) + ")"
						methodDict['impl']['clean_string_list_ptrs'] = True
					else:
						methodDict['impl']['c_args'] += "ObjectArrayToBctbxList<" + listtype + ">(" + arg.name.translate(self.nameTranslator) + ")"
				else:
					methodDict['impl']['c_args'] += arg.name.translate(self.nameTranslator)
				methodDict['impl']['args'] += arg.translate(self.langTranslator, dllImport=False)

		return methodDict

###########################################################################################################################################

	def translate_property_getter(self, prop, name, static=False):
		methodDict = self.translate_method(prop, static, False)

		methodDict['property_static'] = 'static ' if static else ''
		methodDict['property_return'] = prop.returnType.translate(self.langTranslator, dllImport=False)
		methodDict['property_name'] = (name[3:] if len(name) > 3 else 'Instance') if name[:3] == "Get" else name

		methodDict['has_property'] = True
		methodDict['has_getter'] = True
		methodDict['has_setter'] = False
		methodDict['return'] = methodDict['property_return']
		methodDict['exception'] = self.throws_exception(prop.returnType)
		methodDict['getter_nativePtr'] = '' if static else 'nativePtr'
		methodDict['getter_c_name'] = prop.name.to_c()

		methodDict['list_type'] = self.get_class_array_type(methodDict['return'])
		methodDict['is_string_list'] = methodDict['list_type'] == 'string'
		methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'string'

		methodDict['is_string'] = methodDict['return'] == "string"
		methodDict['is_bool'] = methodDict['return'] == "bool"
		methodDict['is_class'] = self.is_linphone_type(prop.returnType, False, False) and type(prop.returnType) is AbsApi.ClassType
		methodDict['is_enum'] = self.is_linphone_type(prop.returnType, False, False) and type(prop.returnType) is AbsApi.EnumType
		methodDict['is_generic'] = self.is_generic(methodDict)
		methodDict['takeRef'] = 'true'
		if isinstance(prop.returnType.parent, AbsApi.Method) and len(prop.returnType.parent.name.words) >=1:
			if prop.returnType.parent.name.words == ['new'] or prop.returnType.parent.name.words[0] == 'create':
				methodDict['takeRef'] = 'false'

		return methodDict

	def translate_property_setter(self, prop, name, static=False):
		methodDict = self.translate_method(prop, static, False)

		methodDict['property_static'] = 'static ' if static else ''
		methodDict['property_return'] = prop.args[0].type.translate(self.langTranslator, dllImport=False)
		methodDict['property_name'] = (name[3:] if len(name) > 3 else 'Instance') if name[:3] == "Set" else name

		methodDict['has_property'] = True
		methodDict['has_getter'] = False
		methodDict['has_setter'] = True
		methodDict['return'] = methodDict['property_return']
		methodDict['exception'] = self.throws_exception(prop.returnType)
		methodDict['setter_nativePtr'] = '' if static else 'nativePtr, '
		methodDict['setter_c_name'] = prop.name.to_c()

		methodDict['list_type'] = self.get_class_array_type(methodDict['return'])
		methodDict['is_string_list'] = methodDict['list_type'] == 'string'
		methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'string'

		methodDict['is_string'] = methodDict['return'] == "string"
		methodDict['is_bool'] = methodDict['return'] == "bool"
		methodDict['is_class'] = self.is_linphone_type(prop.args[0].type, True, False) and type(prop.args[0].type) is AbsApi.ClassType
		methodDict['is_enum'] = self.is_linphone_type(prop.args[0].type, True, False) and type(prop.args[0].type) is AbsApi.EnumType
		methodDict['is_generic'] = self.is_generic(methodDict)

		return methodDict

	def translate_property_getter_setter(self, getter, setter, name, static=False):
		methodDict = self.translate_property_getter(getter, name, static=static)
		methodDictSet = self.translate_property_setter(setter, name, static)

		protoElems = {}
		methodDict["prototype"] = methodDict['prototype']
		methodDict["has_second_prototype"] = True
		methodDict["second_prototype"] = methodDictSet['prototype']

		methodDict['has_setter'] = True
		methodDict['exception'] = methodDictSet['exception']
		methodDict['setter_nativePtr'] = methodDictSet['setter_nativePtr']
		methodDict['setter_c_name'] = methodDictSet['setter_c_name']

		return methodDict

	def translate_property(self, prop):
		res = []
		name = prop.name.translate(self.nameTranslator)
		if prop.getter is not None:
			if prop.setter is not None:
				res.append(self.translate_property_getter_setter(prop.getter, prop.setter, name))
			else:
				res.append(self.translate_property_getter(prop.getter, name))
		elif prop.setter is not None:
			res.append(self.translate_property_setter(prop.setter, name))
		return res

###########################################################################################################################################

	def translate_listener(self, _class, method):
		listenedClass = method.find_first_ancestor_by_type(AbsApi.Interface).listenedClass

		listenerDict = {}
		c_name_setter = listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + method.name.to_snake_case()[3:]
		delegate_name_public = method.name.translate(self.nameTranslator) + "Delegate"
		delegate_name_private = delegate_name_public + "Private"
		listenerDict['cb_setter'] = {}
		listenerDict['cb_setter']['name'] = c_name_setter
		listenerDict['name_private'] = delegate_name_private

		listenerDict['delegate'] = {}
		listenerDict['delegate']['name_public'] = delegate_name_public
		listenerDict['delegate']['name_private'] = delegate_name_private
		var_name_public = method.name.to_snake_case() + '_public'
		var_name_private = method.name.to_snake_case() + '_private'
		listenerDict['delegate']['var_public'] = var_name_public
		listenerDict['delegate']['var_private'] = var_name_private
		listenerDict['delegate']['cb_name'] = method.name.to_snake_case()
		listenerDict['delegate']['name'] = method.name.translate(self.nameTranslator)

		listenerDict['delegate']['interfaceClassName'] = listenedClass.name.translate(self.nameTranslator)
		listenerDict['delegate']['isMultiListener'] = listenedClass.multilistener

		listenerDict['delegate']['params_public'] = ""
		listenerDict['delegate']['params_private'] = ""
		listenerDict['delegate']['params'] = ""
		for arg in method.args:
			dllImportType = arg.type.translate(self.langTranslator, dllImport=True)
			normalType = arg.type.translate(self.langTranslator, dllImport=False)

			argName = arg.name.translate(self.nameTranslator)
			if arg != method.args[0]:
				listenerDict['delegate']['params_public'] += ', '
				listenerDict['delegate']['params_private'] += ', '
				listenerDict['delegate']['params'] += ', '

				if normalType == dllImportType:
					listenerDict['delegate']['params'] += argName
				else:
					if normalType == "bool":
						listenerDict['delegate']['params'] += argName + " == 0"
					elif self.is_linphone_type(arg.type, True, dllImport=False) and type(arg.type) is AbsApi.ClassType:
						listenerDict['delegate']['params'] += "fromNativePtr<" + normalType + ">(" + argName + ")"
					elif self.is_linphone_type(arg.type, True, dllImport=False) and type(arg.type) is AbsApi.EnumType:
						listenerDict['delegate']['params'] += "(" + normalType + ")" + argName + ""
					elif isinstance(arg.type, AbsApi.ListType):
						if normalType == "string":
							listenerDict['delegate']['params'] += "MarshalStringArray(" + argName + ")"
						else:
							listenerDict['delegate']['params'] += "MarshalBctbxList<" + self.get_class_array_type(normalType) + ">(" + argName + ")"
					else:
						print('Not supported yet: ' + delegate_name_public)
						return {}
			else:
				listenerDict['delegate']['first_param'] = argName
				listenerDict['delegate']['params'] = 'thiz'

			listenerDict['delegate']['params_public'] += normalType + " " + argName
			listenerDict['delegate']['params_private'] += dllImportType + " " + argName

		listenerDict['delegate']["c_name_setter"] = c_name_setter
		return listenerDict

###########################################################################################################################################

	def translate_enum(self, enum):
		enumDict = {}
		enumDict['enumName'] = enum.name.translate(self.nameTranslator)
		enumDict['doc'] = enum.briefDescription.translate(self.docTranslator, tagAsBrief=True)
		enumDict['values'] = []
		enumDict['isFlag'] = False
		i = 0
		lastValue = None
		for enumValue in enum.enumerators:
			enumValDict = {}
			enumValDict['name'] = enumValue.name.translate(self.nameTranslator)
			enumValDict['doc'] = enumValue.briefDescription.translate(self.docTranslator, tagAsBrief=True)
			if isinstance(enumValue.value, int):
				lastValue = enumValue.value
				enumValDict['value'] = str(enumValue.value)
			elif isinstance(enumValue.value, AbsApi.Flag):
				enumValDict['value'] = '1<<' + str(enumValue.value.position)
				enumDict['isFlag'] = True
			else:
				if lastValue is not None:
					enumValDict['value'] = lastValue + 1
					lastValue += 1
				else:
					enumValDict['value'] = i
			enumDict['values'].append(enumValDict)
			i += 1
		return enumDict

	def translate_class(self, _class):
		if _class.name.to_c() in self.ignore:
			raise AbsApi.Error('{0} has been escaped'.format(_class.name.to_c()))

		classDict = {}
		classDict['className'] = _class.name.translate(self.nameTranslator)
		classDict['isLinphoneFactory'] = classDict['className'] == "Factory"
		classDict['isLinphoneCall'] = _class.name.to_camel_case() == "Call"
		classDict['isLinphoneCore'] = _class.name.to_camel_case() == "Core"
		classDict['doc'] = _class.briefDescription.translate(self.docTranslator, tagAsBrief=True)
		classDict['dllImports'] = []

		islistenable = _class.listenerInterface is not None
		if islistenable:
			listenerName = _class.listenerInterface.name.translate(self.nameTranslator)
			classDict['listener'] = {}
			classDict['listener']['listener_constructor'] = _class.name.to_snake_case()
			classDict['listener']['interfaceName'] = listenerName

		for method in _class.classMethods:
			try:
				if 'get' in method.name.to_word_list():
					methodDict = self.translate_property_getter(method, method.name.translate(self.nameTranslator), True)
				#The following doesn't work because there a at least one method that has both getter and setter,
				#and because it doesn't do both of them at once, property is declared twice
				#elif 'set' in method.name.to_word_list():
				#	methodDict = self.translate_property_setter(method, method.name.to_camel_case(), True)
				else:
					methodDict = self.translate_method(method, static=True, genImpl=True)
				classDict['dllImports'].append(methodDict)
			except AbsApi.Error as e:
				logging.error('Could not translate {0}: {1}'.format(method.name.to_c(), e.args[0]))

		for prop in _class.properties:
			try:
				classDict['dllImports'] += self.translate_property(prop)
			except AbsApi.Error as e:
				logging.error('error while translating {0} property: {1}'.format(prop.name.to_c(), e.args[0]))

		for method in _class.instanceMethods:
			try:
				methodDict = self.translate_method(method, static=False, genImpl=True)
				classDict['dllImports'].append(methodDict)
			except AbsApi.Error as e:
				logging.error('Could not translate {0}: {1}'.format(method.name.to_c(), e.args[0]))

		return classDict

	def translate_interface(self, interface):
		if interface.name.to_c() in self.ignore:
			raise AbsApi.Error('{0} has been escaped'.format(interface.name.to_c()))

		interfaceDict = {}
		interfaceDict['interfaceName'] = interface.name.translate(self.nameTranslator)
		interfaceDict['set_user_data_name'] = interface.listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_user_data'
		interfaceDict['get_user_data_name'] = interface.listenedClass.name.to_snake_case(fullName=True) + '_cbs_get_user_data'

		interfaceDict['methods'] = []
		for method in interface.instanceMethods:
			interfaceDict['methods'].append(self.translate_listener(interface, method))

		return interfaceDict

###########################################################################################################################################

class EnumImpl(object):
	def __init__(self, enum, translator):
		namespace = enum.find_first_ancestor_by_type(AbsApi.Namespace)
		self.namespace = namespace.name.concatenate(fullName=True) if namespace is not None else None
		self.enum = translator.translate_enum(enum)

class ClassImpl(object):
	def __init__(self, _class, translator):
		namespace = _class.find_first_ancestor_by_type(AbsApi.Namespace)
		self.namespace = namespace.name.concatenate(fullName=True) if namespace is not None else None
		self._class = translator.translate_class(_class)

class InterfaceImpl(object):
	def __init__(self, interface, translator):
		namespace = interface.find_first_ancestor_by_type(AbsApi.Namespace)
		self.namespace = namespace.name.concatenate(fullName=True) if namespace is not None else None
		self.interface = translator.translate_interface(interface)

class WrapperImpl(object):
	def __init__(self, version, enums, interfaces, classes):
		self.version = version
		self.enums = enums
		self.interfaces = interfaces
		self.classes = classes

###########################################################################################################################################

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

	argparser = argparse.ArgumentParser(description='Generate source files for the C# wrapper')
	argparser.add_argument('xmldir', type=str, help='Directory where the XML documentation of the Linphone\'s API generated by Doxygen is placed')
	argparser.add_argument('-o --output', type=str, help='the directory where to generate the source files', dest='outputdir', default='.')
	argparser.add_argument('-n --name', type=str, help='the name of the genarated source file', dest='outputfile', default='LinphoneWrapper.cs')
	argparser.add_argument('-v --verbose', action='store_true', dest='verbose_mode', default=False, help='Verbose mode.')
	args = argparser.parse_args()
	
	loglevel = logging.INFO if args.verbose_mode else logging.ERROR
	logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

	entries = os.listdir(args.outputdir)

	project = CApi.Project()
	project.initFromDir(args.xmldir)
	project.check()

	parser = AbsApi.CParser(project)
	parser.functionBl = [
		'linphone_vcard_get_belcard',
		'linphone_core_get_current_vtable',
		'linphone_factory_create_shared_core',
		'linphone_factory_create_shared_core_with_config',
		'linphone_config_new_for_shared_core',
		'linphone_push_notification_message_new',
		'linphone_push_notification_message_ref',
		'linphone_push_notification_message_unref',
		'linphone_push_notification_message_is_using_user_defaults',
		'linphone_push_notification_message_get_call_id',
		'linphone_push_notification_message_is_text',
		'linphone_push_notification_message_get_text_content',
		'linphone_push_notification_message_get_subject',
		'linphone_push_notification_message_get_from_addr',
		'linphone_push_notification_message_get_local_addr',
		'linphone_push_notification_message_get_peer_addr',
		'linphone_core_get_new_message_from_callid',
		'linphone_core_get_new_chat_room_from_conf_addr'
	]
	parser.classBl += 'LinphoneCoreVTable'
	parser.methodBl.remove('getCurrentCallbacks')
	parser.enum_relocations = {} # No nested enums in C#, will cause ambiguousness between Call.State (the enum) and call.State (the property)
	parser.parse_all()
	translator = CsharpTranslator()
	renderer = pystache.Renderer()

	enums = []
	interfaces = []
	classes = []
	for _interface in parser.namespace.interfaces:
		impl = InterfaceImpl(_interface, translator)
		interfaces.append(impl)
	for _enum in parser.namespace.enums:
		impl = EnumImpl(_enum, translator)
		enums.append(impl)
	for _class in parser.namespace.classes:
		impl = ClassImpl(_class, translator)
		classes.append(impl)
		for _enum in _class.enums:
			enum_impl = EnumImpl(_enum, translator)
			enums.append(enum_impl)

	wrapper = WrapperImpl(git_version, enums, interfaces, classes)
	render(renderer, wrapper, args.outputdir + "/" + args.outputfile)
