#!/usr/bin/python

# Copyright (C) 2017 Belledonne Communications SARL
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


import pystache
import re
import argparse
import os
import os.path
import sys
import errno
import logging

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
import genapixml as CApi
import abstractapi as AbsApi
import metadoc
import metaname


class CppTranslator(object):
	sharedPtrTypeExtractor = re.compile('^(const )?std::shared_ptr<(.+)>( &)?$')
	
	def __init__(self, rootNs=None):
		self.nameTranslator = metaname.Translator.get('Cpp')
		self.langTranslator = AbsApi.Translator.get('Cpp')
		self.langTranslator.ambigousTypes.append('LinphonePayloadType')
		self.docTranslator = metadoc.DoxygenTranslator('Cpp')
		self.rootNs = rootNs
	
	def translate_enum(self, enum):
		enumDict = {}
		enumDict['name'] = enum.name.translate(self.nameTranslator)
		enumDict['doc'] = enum.briefDescription.translate(self.docTranslator)
		enumDict['enumerators'] = []
		for enumerator in enum.enumerators:
			enumeratorDict = self.translate_enumerator(enumerator)
			enumeratorDict['notLast'] = (enumerator is not enum.enumerators[-1])
			enumDict['enumerators'].append(enumeratorDict)
		return enumDict
	
	def translate_enumerator(self, enumerator):
		enumeratorDict = {
			'name'  : enumerator.name.translate(self.nameTranslator),
			'value' : enumerator.translate_value(self.langTranslator)
		}
		try:
			enumeratorDict['doc'] = enumerator.briefDescription.translate(self.docTranslator)
		except metadoc.TranslationError as e:
			logging.error(e.msg())
		return enumeratorDict
	
	def translate_class(self, _class):
		islistenable = _class.listenerInterface is not None
		ismonolistenable = (islistenable and not _class.multilistener)
		ismultilistenable = (islistenable and _class.multilistener)
		
		classDict = {
			'islistenable'        : islistenable,
			'isnotlistenable'     : not islistenable,
			'ismonolistenable'    : ismonolistenable,
			'ismultilistenable'   : ismultilistenable,
			'isrefcountable'      : _class.refcountable,
			'isnotrefcountable'   : not _class.refcountable,
			'isNotListener'       : True,
			'isListener'          : False,
			'isVcard'             : (_class.name.to_c() == 'LinphoneVcard'),
			'className'           : _class.name.translate(self.nameTranslator),
			'cClassName'          : '::' + _class.name.to_c(),
			'privCClassName'      : '_' + _class.name.to_c(),
			'parentClassName'     : 'Object' if _class.refcountable else None,
			'enums'               : [],
			'methods'             : [],
			'staticMethods'       : [],
			'wrapperCbs'          : [],
			'friendClasses'       : []
		}
		
		if _class.name.to_c() == 'LinphoneCore':
			classDict['friendClasses'].append({'name': 'Factory'});
		
		try:
			classDict['briefDoc'] = _class.briefDescription.translate(self.docTranslator, tagAsBrief=True)
			classDict['detailedDoc'] = _class.detailedDescription.translate(self.docTranslator)
		except metadoc.TranslationError as e:
			logging.error(e.msg())
		
		if islistenable:
			classDict['listenerClassName'] = _class.listenerInterface.name.translate(self.nameTranslator)
			classDict['cListenerName'] = _class.listenerInterface.name.to_c()
			classDict['cppListenerName'] = _class.listenerInterface.name.translate(self.nameTranslator)
			for method in _class.listenerInterface.instanceMethods:
				classDict['wrapperCbs'].append(self._generate_wrapper_callback(_class, method))
		
		if ismonolistenable:
			classDict['cCbsGetter'] = _class.name.to_snake_case(fullName=True) + '_get_callbacks'
			classDict['parentClassName'] = 'ListenableObject'
		elif ismultilistenable:
			classDict['parentClassName'] = 'MultiListenableObject'
			classDict['listenerCreator'] = 'linphone_factory_create_' + _class.listenerInterface.name.to_snake_case()[:-len('_listener')] + '_cbs'
			classDict['callbacksAdder'] = _class.name.to_snake_case(fullName=True)+ '_add_callbacks'
			classDict['callbacksRemover'] = _class.name.to_snake_case(fullName=True)+ '_remove_callbacks'
			classDict['userDataSetter'] = _class.listenerInterface.name.to_snake_case(fullName=True)[:-len('_listener')] + '_cbs_set_user_data'
			classDict['userDataGetter'] = _class.listenerInterface.name.to_snake_case(fullName=True)[:-len('_listener')] + '_cbs_get_user_data'
			classDict['currentCallbacksGetter'] = _class.name.to_snake_case(fullName=True) + '_get_current_callbacks'

		for enum in _class.enums:
			classDict['enums'].append(self.translate_enum(enum))
		
		for _property in _class.properties:
			classDict['methods'] += self.translate_property(_property)
		
		for method in _class.instanceMethods:
			methodDict = self.translate_method(method)
			classDict['methods'].append(methodDict)
		
		for method in _class.classMethods:
			methodDict = self.translate_method(method)
			classDict['staticMethods'].append(methodDict)
		
		return classDict
	
	def _generate_wrapper_callback(self, listenedClass, method):
		namespace = method.find_first_ancestor_by_type(AbsApi.Namespace)
		listenedClass = method.find_first_ancestor_by_type(AbsApi.Interface).listenedClass
		
		params = {}
		params['name'] = method.name.to_snake_case(fullName=True) + '_cb'
		args = []
		wrappedArgs = []
		for arg in method.args:
			args.append(arg.type.cDecl + ' ' + arg.name.to_c())
			wrappedArgs.append(self._wrap_c_expression_to_cpp(arg.name.to_c(), arg.type, usedNamespace=namespace))
		params['params'] = ', '.join(args)
		params['returnType'] = method.returnType.cDecl
		
		wrapperCbDict = {}
		wrapperCbDict['cbName'] = params['name']
		wrapperCbDict['declArgs'] = params['params']
		wrapperCbDict['firstArgName'] = method.args[0].name.to_c()
		wrapperCbDict['returnType'] = params['returnType']
		wrapperCbDict['hasReturnValue'] = (params['returnType'] != 'void')
		wrapperCbDict['hasNotReturnValue'] = not wrapperCbDict['hasReturnValue']
		wrapperCbDict['callbackSetter'] = listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + method.name.to_snake_case()[3:]
		wrapperCbDict['cppMethodCallingLine'] = 'listener->{methodName}({wrappedArgs})'.format(
			methodName=method.name.to_camel_case(lower=True),
			wrappedArgs=', '.join(wrappedArgs))
		wrapperCbDict['cppMethodCallingLine'] = self._wrap_cpp_expression_to_c(wrapperCbDict['cppMethodCallingLine'], method.returnType)
		return wrapperCbDict
	
	def translate_interface(self, interface):
		intDict = {
			'inheritFrom'     : {'name': 'Listener'},
			'className'       : interface.name.translate(self.nameTranslator),
			'constructor'     : None,
			'parentClassName' : 'Listener',
			'isNotListener'   : False,
			'isListener'      : True,
			'methods'         : []
		}
		for method in interface.instanceMethods:
			methodDict = self.translate_method(method, genImpl=False)
			intDict['methods'].append(methodDict)
		
		return intDict
	
	def translate_property(self, _property):
		res = []
		if _property.getter is not None:
			res.append(self.translate_method(_property.getter))
		if _property.setter is not None:
			res.append(self.translate_method(_property.setter))
		return res
	
	def translate_method(self, method, genImpl=True):
		namespace = method.find_first_ancestor_by_type(AbsApi.Class, AbsApi.Interface)
		
		methodDict = {
			'declPrototype': method.translate_as_prototype(self.langTranslator, namespace=namespace),
			'implPrototype': method.translate_as_prototype(self.langTranslator, namespace=AbsApi.GlobalNs),
			'deprecated': method.deprecated,
			'suffix': '',
		}
		
		try:
			methodDict['briefDoc'] = method.briefDescription.translate(self.docTranslator, tagAsBrief=True) if method.briefDescription is not None else None
			methodDict['detailedDoc'] = method.detailedDescription.translate(self.docTranslator) if method.detailedDescription is not None else None
		except metadoc.TranslationError as e:
			logging.error(e.msg())
		
		if type(method.parent) is AbsApi.Interface:
			if isinstance(method.returnType, AbsApi.BaseType) and method.returnType.name == 'void':
				methodDict['suffix'] = ' {}'
			else:
				methodDict['suffix'] = ' = 0'
	
		if genImpl:
			methodDict['sourceCode' ] = self._generate_source_code(method, usedNamespace=namespace)
		
		return methodDict
	
	def _generate_source_code(self, method, usedNamespace=None):
		nsName = usedNamespace.name if usedNamespace is not None else None
		params = {
			'functionName': method.name.to_c(),
			'args': self._generate_wrapped_arguments(method, usedNamespace=usedNamespace)
		}
		
		if method.name.to_camel_case(lower=True) != 'setListener':
			cExpr = '{functionName}({args})'.format(**params)
			cppExpr = self._wrap_c_expression_to_cpp(cExpr, method.returnType, usedNamespace=usedNamespace)
		else:
			cppExpr = 'ListenableObject::setListener(std::static_pointer_cast<Listener>({0}))'.format(method.args[0].name.to_snake_case())
		
		if type(method.returnType) is AbsApi.BaseType and method.returnType.name == 'void' and not method.returnType.isref:
			return cppExpr + ';'
		else:
			return 'return {0};'.format(cppExpr)
	
	def _generate_wrapped_arguments(self, method, usedNamespace=None):
		args = []
		if method.type == AbsApi.Method.Type.Instance:
			_class = method.find_first_ancestor_by_type(AbsApi.Class)
			argStr = '(::{0} *)mPrivPtr'.format(_class.name.to_camel_case(fullName=True))
			args.append(argStr)
		
		for arg in method.args:
			paramName = arg.name.to_camel_case(lower=True)
			args.append(self._wrap_cpp_expression_to_c(paramName, arg.type, usedNamespace=usedNamespace))
		
		return ', '.join(args)
	
	def _wrap_cpp_expression_to_c(self, cppExpr, exprtype, usedNamespace=None):
		if isinstance(exprtype, AbsApi.BaseType):
			if exprtype.name == 'string':
				cExpr = 'StringUtilities::cppStringToC({0})'.format(cppExpr);
			else:
				cExpr = cppExpr
		elif isinstance(exprtype, AbsApi.EnumType):
			cExpr = '(::{0}){1}'.format(exprtype.desc.name.to_c(), cppExpr)
		elif isinstance(exprtype, AbsApi.ClassType):
			cPtrType = exprtype.desc.name.to_c()
			if exprtype.desc.refcountable:
				ptrType = exprtype.translate(self.langTranslator, namespace=usedNamespace)
				ptrType = CppTranslator.sharedPtrTypeExtractor.match(ptrType).group(2)
				param = {
					'ptrType' : ptrType,
					'cPtrType': cPtrType,
					'cppExpr' : cppExpr,
					'object'  : 'const Object' if exprtype.isconst else 'Object'
				}
				cExpr = '(::{cPtrType} *)Object::sharedPtrToCPtr(std::static_pointer_cast<{object},{ptrType}>({cppExpr}))'.format(**param)
			else:
				if exprtype.isref:
					cExpr = '(const ::{_type} *)({expr}).c_struct()'.format(_type=cPtrType, expr=cppExpr)
				else:
					cExpr = '*(const ::{_type} *)({expr}).c_struct()'.format(_type=cPtrType, expr=cppExpr)
		elif isinstance(exprtype, AbsApi.ListType):
			if isinstance(exprtype.containedTypeDesc, AbsApi.BaseType) and exprtype.containedTypeDesc.name == 'string':
				cExpr = 'StringBctbxListWrapper({0}).c_list()'.format(cppExpr)
			elif isinstance(exprtype.containedTypeDesc, AbsApi.ClassType):
				ptrType = exprtype.containedTypeDesc.translate(self.langTranslator, namespace=usedNamespace)
				if exprtype.containedTypeDesc.desc.refcountable:
					ptrType = CppTranslator.sharedPtrTypeExtractor.match(ptrType).group(2)
					cExpr = 'ObjectBctbxListWrapper<{0}>({1}).c_list()'.format(ptrType, cppExpr)
				else:
					cType = exprtype.containedTypeDesc.desc.name.to_c()
					if exprtype.isconst:
						cExpr = 'StructBctbxListWrapper<{0},{1}>({2}).c_list()'.format(ptrType, cType, cppExpr)
					else:
						cExpr = 'StructBctbxListWrapper<{0},{1}>::cppListToBctbxList({2})'.format(ptrType, cType, cppExpr)
			else:
				raise AbsApi.Error('translation of bctbx_list_t of enums or basic C types is not supported')
		
		return cExpr
	
	def _wrap_c_expression_to_cpp(self, cExpr, exprtype, usedNamespace=None):
		if isinstance(exprtype, AbsApi.BaseType):
			if exprtype.name == 'string':
				return 'StringUtilities::cStringToCpp({0})'.format(cExpr)
			elif exprtype.name == 'string_array':
				return 'StringUtilities::cStringArrayToCppList({0})'.format(cExpr)
			elif exprtype.name == 'boolean':
				return '({0} != FALSE)'.format(cExpr)
			else:
				return cExpr
		elif isinstance(exprtype, AbsApi.EnumType):
			cppEnumName = exprtype.translate(self.langTranslator, namespace=usedNamespace)
			return '({0}){1}'.format(cppEnumName, cExpr)
		elif isinstance(exprtype, AbsApi.ClassType):
			cppReturnType = exprtype.translate(self.langTranslator, namespace=usedNamespace)
			if exprtype.desc.refcountable:
				cppReturnType = CppTranslator.sharedPtrTypeExtractor.match(cppReturnType).group(2)
				
				if isinstance(exprtype.parent, AbsApi.Method) and len(exprtype.parent.name.words) >=1 and (exprtype.parent.name.words == ['new'] or exprtype.parent.name.words[0] == 'create'):
					return 'Object::cPtrToSharedPtr<{0}>({1}, false)'.format(cppReturnType, cExpr)
				else:
					return 'Object::cPtrToSharedPtr<{0}>({1})'.format(cppReturnType, cExpr)
			else:
				if exprtype.isref:
					return '{0}({1})'.format(exprtype.desc.name.to_camel_case(), cExpr)
				else:
					return '{0}(StructWrapper<::{1}>({2}).ptr())'.format(
						exprtype.desc.name.to_camel_case(),
						exprtype.desc.name.to_c(),
						cExpr)
		elif isinstance(exprtype, AbsApi.ListType):
			if isinstance(exprtype.containedTypeDesc, AbsApi.BaseType) and exprtype.containedTypeDesc.name == 'string':
				return 'StringBctbxListWrapper::bctbxListToCppList({0})'.format(cExpr)
			elif isinstance(exprtype.containedTypeDesc, AbsApi.ClassType):
				cppReturnType = exprtype.containedTypeDesc.translate(self.langTranslator, namespace=usedNamespace)
				takeRef = 'false' if isinstance(exprtype, AbsApi.OnTheFlyListType) else 'true'
				if exprtype.containedTypeDesc.desc.refcountable:
					cppReturnType = CppTranslator.sharedPtrTypeExtractor.match(cppReturnType).group(2)
					return 'ObjectBctbxListWrapper<{0}>::bctbxListToCppList({1}, {2})'.format(cppReturnType, cExpr, takeRef)
				else:
					cType = exprtype.containedTypeDesc.desc.name.to_c()
					return 'StructBctbxListWrapper<{0},{1}>::bctbxListToCppList({2}, {3})'.format(cppReturnType, cType, cExpr, takeRef)
			else:
				raise AbsApi.Error('translation of bctbx_list_t of enums or basic C types is not supported')
		else:
			return cExpr
	
	@staticmethod
	def fail(obj):
		raise AbsApi.Error('Cannot translate {0} type'.format(type(obj)))


class EnumsHeader(object):
	def __init__(self, translator):
		self.translator = translator
		self.enums = []
	
	def add_enum(self, enum):
		self.enums.append(self.translator.translate_enum(enum))


class ClassHeader(object):
	def __init__(self, _class, translator):
		if type(_class) is AbsApi.Class:
			self._class = translator.translate_class(_class)
		else:
			self._class = translator.translate_interface(_class)
		
		self.rootNs = translator.rootNs
		self.define = '_{0}_HH'.format(_class.name.to_snake_case(upper=True, fullName=True))
		self.filename = '{0}.hh'.format(_class.name.to_snake_case())
		self.priorDeclarations = []
		self.private_type = _class.name.to_camel_case(fullName=True)
		self.includes = {'internal': [], 'external': []}
		self._populate_needed_includes(_class)
	
	def _populate_needed_includes(self, _class):
		if type(_class) is AbsApi.Class:
			for _property in _class.properties:
				if _property.setter is not None:
					self._populate_needed_includes_from_method(_property.setter)
				if _property.getter is not None:
					self._populate_needed_includes_from_method(_property.getter)
		
		if type(_class) is AbsApi.Class:
			methods = _class.classMethods + _class.instanceMethods
		else:
			methods = _class.instanceMethods
		
		for method in methods:
			self._populate_needed_includes_from_type(method.returnType)
			for arg in method.args:
				self._populate_needed_includes_from_type(arg.type)
		
		if isinstance(_class, AbsApi.Class) and _class.listenerInterface is not None:
			decl = 'class ' + _class.listenerInterface.name.translate(metaname.Translator.get('Cpp'))
			self._add_prior_declaration(decl)
		
		currentClassInclude = _class.name.to_snake_case()
		if currentClassInclude in self.includes['internal']:
			self.includes['internal'].remove(currentClassInclude)
	
	def _populate_needed_includes_from_method(self, method):
		self._populate_needed_includes_from_type(method.returnType)
		for arg in method.args:
			self._populate_needed_includes_from_type(arg.type)
	
	def _populate_needed_includes_from_type(self, type_):
		translator = metaname.Translator.get('Cpp')
		if isinstance(type_, AbsApi.ClassType):
			class_ = type_.desc
			if class_.parent == self.rootNs:
				decl = 'class ' + class_.name.translate(translator)
				self._add_prior_declaration(decl)
			else:
				rootClass = class_.find_first_ancestor_by_type(AbsApi.Namespace, priorAncestor=True)
				self._add_include('internal', rootClass.name.to_snake_case())
		elif isinstance(type_, AbsApi.EnumType):
			enum = type_.desc
			if enum.parent == self.rootNs:
				headerFile = 'enums'
			else:
				rootClass = enum.find_first_ancestor_by_type(AbsApi.Namespace, priorAncestor=True)
				headerFile = rootClass.name.to_snake_case()
			self._add_include('internal', headerFile)
		elif isinstance(type_, AbsApi.BaseType):
			if type_.name == 'integer' and isinstance(type_.size, int):
				self._add_include('external', 'cstdint')
			elif type_.name == 'string':
				self._add_include('external', 'string')
		elif isinstance(type_, AbsApi.ListType):
			self._add_include('external', 'list')
			self._populate_needed_includes_from_type(type_.containedTypeDesc)
	
	def _add_include(self, location, name):
		if next((x for x in self.includes[location] if x['name']==name), None) is None:
			self.includes[location].append({'name': name})

	def _add_prior_declaration(self, decl):
		if next((x for x in self.priorDeclarations if x['declaration']==decl), None) is None:
			self.priorDeclarations.append({'declaration': decl})


class MainHeader(object):
	def __init__(self):
		self.includes = []
		self.define = '_LINPHONE_HH'
	
	def add_include(self, include):
		self.includes.append({'name': include})


class ClassImpl(object):
	def __init__(self):
		self.classes = []
		self.namespace = 'linphone'

class GenWrapper(object):
	def __init__(self, includedir, srcdir, xmldir):
		self.includedir = includedir
		self.srcdir = srcdir

		project = CApi.Project()
		project.initFromDir(xmldir)
		project.check()
		
		self.parser = AbsApi.CParser(project)
		self.parser.parse_all()
		self.translator = CppTranslator(self.parser.namespace)
		self.renderer = pystache.Renderer()
		self.mainHeader = MainHeader()
		self.impl = ClassImpl()

	def render_all(self):
		header = EnumsHeader(self.translator)

		for enum in self.parser.namespace.enums:
			header.add_enum(enum)
		
		self.render(header, self.includedir + '/enums.hh')
		self.mainHeader.add_include('enums.hh')
		
		for _class in self.parser.interfacesIndex.values():
			self.render_header(_class)
		for _class in self.parser.classesIndex.values():
			self.render_header(_class)
		
		self.render(self.mainHeader, self.includedir + '/linphone.hh')
		self.render(self.impl, self.srcdir + '/linphone++.cc')

	def render(self, item, path):
		tmppath = path + '.tmp'
		content = ''
		with open(tmppath, mode='w') as f:
			f.write(self.renderer.render(item))
		with open(tmppath, mode='rU') as f:
			content = f.read()
		with open(path, mode='w') as f:
			f.write(content)
		os.unlink(tmppath)

	def render_header(self, _class):
		if _class is not None:
			header = ClassHeader(_class, self.translator)
			headerName = _class.name.to_snake_case() + '.hh'
			self.mainHeader.add_include(headerName)
			self.render(header, self.includedir + '/' + header.filename)
			
			if type(_class) is not AbsApi.Interface:
				self.impl.classes.append(header._class)


if __name__ == '__main__':
	try:
		argparser = argparse.ArgumentParser(description='Generate source files for the C++ wrapper')
		argparser.add_argument('xmldir', type=str, help='Directory where the XML documentation of the Linphone\'s API generated by Doxygen is placed')
		argparser.add_argument('-o --output', type=str, help='the directory where to generate the source files', dest='outputdir', default='.')
		argparser.add_argument('-v --verbose', help='Show warning and info traces.', action='store_true', default=False, dest='verbose_mode')
		argparser.add_argument('-d --debug', help='Show all traces.', action='store_true', default=False, dest='debug_mode')
		args = argparser.parse_args()

		if args.debug_mode:
			loglevel = logging.DEBUG
		elif args.verbose_mode:
			loglevel = logging.INFO
		else:
			loglevel = logging.ERROR
		logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

		includedir = args.outputdir + '/include/linphone++'
		srcdir = args.outputdir + '/src'
		if not os.path.exists(includedir):
			os.makedirs(includedir)
		if not os.path.exists(srcdir):
			os.makedirs(srcdir)

		genwrapper = GenWrapper(includedir, srcdir, args.xmldir)
		genwrapper.render_all()
	except AbsApi.Error as e:
		logging.critical(e)
