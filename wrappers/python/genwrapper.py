# -*- coding: utf-8 -*-

# genwrapper.pyx
# Copyright (c) 2010-2024 Belledonne Communications SARL.
#
# This file is part of Liblinphone 
# (see https://gitlab.linphone.org/BC/public/liblinphone).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

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
        self.docTranslator = metadoc.PythonTranslator()
        self.nameTranslator = metaname.Translator.get('Python')
        self.langTranslator = AbsApi.Translator.get('Python')
        self.forbidden_keywords = ['type', 'file', 'list', 'from', 'id', 'filter', 'dir', 'max', 'range', 'min']

    def translate_c_type(self, _ctype, ctypedef = False):
        if isinstance(_ctype, AbsApi.ClassType):
            if _ctype.isconst:
                return 'const ' + _ctype.name + '*'
            return _ctype.name + '*'
        elif isinstance(_ctype, AbsApi.BaseType):
            if _ctype.name == 'boolean':
                if ctypedef:
                    return "unsigned char"
                else:
                    return "bint"
            return self.langTranslator.translate_base_type(_ctype)
        elif isinstance(_ctype, AbsApi.ListType):
            if _ctype.isconst:
                return 'const bctbx_list_t*'
            return 'bctbx_list_t*'
        elif isinstance(_ctype, AbsApi.EnumType):
            if (ctypedef):
                return _ctype.name
            else:
                return _ctype.name[8:] # To remove Linphone

    def translate_arg_name(self, _name):
        return self.nameTranslator.translate_arg_name(_name)

    def translate_base_type_for_doc(self, type_):
        if type_.name == 'string':
            return 'str'
        elif type_.name == 'integer':
            return 'int'
        elif type_.name == 'boolean':
            return 'bool'
        elif type_.name == 'floatant':
            return 'float'
        elif type_.name == 'size':
            return 'int'
        elif type_.name == 'time':
            return 'int'
        elif type_.name == 'long':
            return 'int'
        elif type_.name == 'status':
            return 'int'
        elif type_.name == 'string_array':
            return 'list[str]'
        elif type_.name == 'character':
            return 'str'
        elif type_.name == 'void' and type_.isref:
            return 'object'

        if type_.name[:8] == 'Linphone':
            return type_.name[8:]
        return type_.name

###############################################################################

    def create_c_listener_constructor(self, _cclass):
        cMethodDict = {}
        cMethodDict['c_prototype'] = _cclass.name.to_camel_case(fullName=True) + 'Cbs* linphone_factory_create_' + _cclass.name.to_snake_case() + '_cbs('
        cMethodDict['c_prototype'] += 'const LinphoneFactory *factory)'
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
        cClassDict['unref'] = _cclass.name.to_snake_case(fullName=True) + '_unref'
        cClassDict['ref'] = _cclass.name.to_snake_case(fullName=True) + '_ref'
        cClassDict['is_factory'] = _cclass.name.translate(self.nameTranslator) == 'Factory'
        return cClassDict

    def translate_c_callback(self, _cclass, _ccallback):
        cCallbackDict = {}

        cCallbackDict['c_name'] = _cclass.name.to_c() + _ccallback.name.to_camel_case() + 'Cb'
        cCallbackDict['c_params'] = ''
        cCallbackDict['c_return_type'] = self.translate_c_type(_ccallback.returnType)

        for arg in _ccallback.args:
            if arg is not _ccallback.args[0]:
                cCallbackDict['c_params'] += ', '
            cCallbackDict['c_params'] += self.translate_c_type(arg.type, ctypedef=True) + ' ' + self.translate_arg_name(arg.name.to_c())

        return cCallbackDict

    def translate_c_callback_method(self, _cclass, _ccallback):
        cCallbackMethodDict = {}

        listenedClass = _ccallback.find_first_ancestor_by_type(AbsApi.Interface).listenedClass
        cCallbackMethodDict['c_prototype'] = self.translate_c_type(_ccallback.returnType) + ' '
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

    def translate_c_enum(self, _enum):
        enumDict = {}
        enumDict['c_enum_name'] = _enum.name.to_camel_case(fullName=True)
        enumDict['c_enum_values'] = ""

        values = ""
        for enumValue in _enum.enumerators:
            values += enumValue.name.to_camel_case(fullName=True) + ", "

        enumDict['c_enum_values'] = values[:-2]

        return enumDict

    def translate_enum(self, _enum):
        enumDict = {}
        enumDict['briefDoc'] = _enum.briefDescription.translate(self.docTranslator)
        enumDict['detailedDoc'] = _enum.detailedDescription.translate(self.docTranslator) if _enum.detailedDescription is not None else None
        enumDict['name'] = _enum.name.translate(self.nameTranslator)
        enumDict['values'] = []

        i = 0
        lastValue = None
        for enumValue in _enum.enumerators:
            enumValDict = {}
            enumValDict['briefDoc'] = enumValue.briefDescription.translate(self.docTranslator)
            enumValDict['detailedDoc'] = enumValue.detailedDescription.translate(self.docTranslator) if enumValue.detailedDescription is not None else None
            enumValDict['value_name'] = enumValue.name.translate(self.nameTranslator)

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
        methodDict['is_static'] = False
        methodDict['c_name'] = 'linphone_factory_create_' + _obj.name.to_snake_case() + '_cbs'
        methodDict['python_name'] = 'create_' + _obj.name.to_snake_case() + '_listener'

        methodDict['params'] = []
        methodDict['python_params'] = ''
        methodDict['computed_params'] = ''

        methodDict['has_return'] = True
        methodDict['has_return_obj'] = True
        methodDict['constructor'] = True

        methodDict['return_python_type'] = _obj.name.to_camel_case() + 'Listener'
        methodDict['doc_return_type'] = methodDict['return_python_type']

        methodDict['briefDoc'] = ''
        methodDict['detailedDoc'] = ''
        methodDict['fakeDoc'] = 'Creates a ' + methodDict['return_python_type'] + ' listener to object that can be added later using `' + _obj.name.to_camel_case() + '.add_listener()` method'

        return methodDict

    def create_add_callbacks_method(self, _obj):
        methodDict = {}
        methodDict['is_static'] = False
        methodDict['c_name'] = _obj.name.to_snake_case(fullName=True) + '_add_callbacks'
        methodDict['python_name'] = 'add_listener'

        methodDict['params'] = []
        methodDict['python_params'] = ', listener'
        methodDict['computed_params'] = ', listener_ptr'

        paramDict = {}
        paramDict['python_param_name'] = 'listener'
        paramDict['param_c_type'] = _obj.name.to_camel_case(fullName=True)
        paramDict['is_obj'] = True
        paramDict['python_param_type'] = _obj.name.to_camel_case() + 'Listener'
        
        methodDict['doc_python_params'] = ', ' + paramDict['python_param_name'] + ': ' + paramDict['python_param_type']
        methodDict['params'].append(paramDict)

        methodDict['briefDoc'] = ''
        methodDict['detailedDoc'] = ''
        methodDict['fakeDoc'] = 'Adds a listener to this `' + _obj.name.to_camel_case() + '` object\n:param listener: The `' + paramDict['python_param_type'] + '` object to add'

        return methodDict

    def create_remove_callbacks_method(self, _obj):
        methodDict = {}
        methodDict['is_static'] = False
        methodDict['c_name'] = _obj.name.to_snake_case(fullName=True) + '_remove_callbacks'
        methodDict['python_name'] = 'remove_listener'

        methodDict['params'] = []
        methodDict['python_params'] = ', listener'
        methodDict['computed_params'] = ', listener_ptr'

        paramDict = {}
        paramDict['python_param_name'] = 'listener'
        paramDict['param_c_type'] = _obj.name.to_camel_case(fullName=True)
        paramDict['is_obj'] = True
        paramDict['python_param_type'] = _obj.name.to_camel_case() + 'Listener'

        methodDict['doc_python_params'] = ', ' + paramDict['python_param_name'] + ': ' + paramDict['python_param_type']
        methodDict['params'].append(paramDict)

        methodDict['briefDoc'] = ''
        methodDict['detailedDoc'] = ''
        methodDict['fakeDoc'] = 'Removes a listener from this this `' + _obj.name.to_camel_case() + '` object\n:param listener: The `' + paramDict['python_param_type'] + '` object to remove'

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

        callbackDict['python_name'] = _obj.name.translate(self.nameTranslator)
        callbackDict['callback_var_name'] = _method.name.to_snake_case()
        callbackDict['callback_internal_name'] = _method.name.to_snake_case() + '_cb'
        callbackDict['callback_return_type'] = self.translate_c_type(_method.returnType)
        callbackDict['callback_has_return'] = callbackDict['callback_return_type'] != "void"

        listenedClass = _method.find_first_ancestor_by_type(AbsApi.Interface).listenedClass
        callbackDict['callback_setter'] = listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + _method.name.to_snake_case()[3:]

        callbackDict['is_multi_listener'] = listenedClass.multilistener

        callbackDict['params'] = []
        callbackDict['briefDoc'] = _method.briefDescription.translate(self.docTranslator)
        callbackDict['detailedDoc'] = _method.detailedDescription.translate(self.docTranslator) if _method.detailedDescription is not None else None
        callbackDict['c_params'] = ''
        callbackDict['first_python_param_name'] = ''
        callbackDict['computed_params'] = ''
        callbackDict['doc_python_params'] = 'Callable[['

        for arg in _method.args:
            paramDict = {}
            translated_arg_type = self.translate_c_type(arg.type)
            paramDict['python_param_name'] = self.translate_arg_name(arg.name.to_c())
            if arg is not _method.args[0]:
                callbackDict['c_params'] += ', '
                callbackDict['computed_params'] += ', '
                callbackDict['doc_python_params'] += ', '
            else:
                callbackDict['first_python_param_name'] = paramDict['python_param_name']
            callbackDict['computed_params'] += paramDict['python_param_name']
            callbackDict['c_params'] += self.translate_c_type(arg.type, ctypedef=True) + ' ' + self.translate_arg_name(arg.name.to_c())

            paramDict['param_c_type'] = translated_arg_type
            paramDict['is_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(arg.type.containedTypeDesc, AbsApi.ClassType)
            paramDict['is_string_list'] = 'bctbx_list_t*' in translated_arg_type and arg.type.containedTypeDesc.name == 'string'
            paramDict['is_obj'] = isinstance(arg.type, AbsApi.ClassType)
            paramDict['is_bool'] = translated_arg_type == 'bint'
            paramDict['is_enum'] = isinstance(arg.type, AbsApi.EnumType)
            paramDict['is_const'] = arg.type.isconst

            if paramDict['is_obj_list'] or paramDict['is_string_list']:
                callbackDict['computed_params'] += '_list'
                if paramDict['is_obj_list']:
                    paramDict['param_c_type'] = self.translate_c_type(arg.type.containedTypeDesc)
                    paramDict['python_param_type'] = arg.type.containedTypeDesc.desc.name.translate(self.nameTranslator)
            elif paramDict['is_obj']:
                callbackDict['computed_params'] += '_obj'
                paramDict['python_param_type'] = arg.type.desc.name.translate(self.nameTranslator)
            elif paramDict['is_bool']:
                callbackDict['computed_params'] += '_b'

            callbackDict['params'].append(paramDict)
            
            argType = self.translate_base_type_for_doc(arg.type)
            if paramDict['is_obj_list']:
                argType = "list[" + paramDict['python_param_type'] + "]"
            elif paramDict['is_obj']:
                argType = paramDict['python_param_type']
            elif paramDict['is_enum']:
                argType = arg.type.desc.name.translate(self.nameTranslator)
            callbackDict['doc_python_params'] += argType
            if arg.maybenil:
                callbackDict['doc_python_params'] += ' | None'

        callbackDict['doc_python_params'] += '], None]' # assume callback returns nothing

        return callbackDict

    def translate_object_property(self, _obj, _method, getter, setter):
        propertyDict = {}
        propertyDict['python_name'] = self.nameTranslator.translate_property_name(getter.name if getter is not None else setter.name)

        getterDict = {}
        getterDict['python_name'] = propertyDict['python_name']
        if getter is not None:
            getterDict['c_name'] = getter.name.to_c()
            getterDict['briefDoc'] = getter.briefDescription.translate(self.docTranslator)
            getterDict['detailedDoc'] = getter.detailedDescription.translate(self.docTranslator) if getter.detailedDescription is not None else None

            translated_type = self.translate_c_type(getter.returnType)
            getterDict['is_return_obj_list'] = 'bctbx_list_t*' in translated_type and isinstance(getter.returnType.containedTypeDesc, AbsApi.ClassType)
            getterDict['is_return_string_list'] = 'bctbx_list_t*' in translated_type and getter.returnType.containedTypeDesc.name == 'string'
            getterDict['is_return_obj'] = isinstance(getter.returnType, AbsApi.ClassType)
            getterDict['is_return_bool'] = translated_type == 'bint'
            getterDict['is_return_string'] = getter.returnType.name == 'string'
            getterDict['is_return_void_ptr'] = translated_type == 'void*'
            getterDict['is_return_const'] = getter.returnType.isconst
            getterDict['constructor'] = getter.returnAllocatedObject
            getterDict['is_const'] = getter.returnType.isconst
            getterDict['is_simple_return'] = not getterDict['is_return_obj_list'] and not getterDict['is_return_string_list'] and not getterDict['is_return_obj'] and not getterDict['is_return_bool'] and not getterDict['is_return_string'] and not getterDict['is_return_void_ptr']

            getterDict['doc_return_type'] = self.translate_base_type_for_doc(getter.returnType)
            getterDict['doc_return_maybenil'] = getter.maybenil and not getterDict['is_return_obj_list'] and not getterDict['is_return_string_list']
            if getterDict['is_return_obj']:
                getterDict['python_return_type'] = getter.returnType.desc.name.translate(self.nameTranslator)
                getterDict['doc_return_type'] = getterDict['python_return_type']
            elif isinstance(getter.returnType, AbsApi.EnumType):
                getterDict['doc_return_type'] = getter.returnType.desc.name.translate(self.nameTranslator)
            elif getterDict['is_return_obj_list']:
                getterDict['python_return_type'] = getter.returnType.containedTypeDesc.desc.name.translate(self.nameTranslator)
                getterDict['c_return_type'] = getter.returnType.containedTypeDesc.name
                getterDict['doc_return_type'] = "list[" + getterDict['python_return_type'] + "]"

        propertyDict['getter'] = getterDict

        if setter is not None:
            setterDict = {}
            setterDict['c_name'] = setter.name.to_c()
            setterDict['python_name'] = propertyDict['python_name']
            setterDict['briefDoc'] = setter.briefDescription.translate(self.docTranslator)
            setterDict['detailedDoc'] = setter.detailedDescription.translate(self.docTranslator) if setter.detailedDescription is not None else None

            translated_arg_type = self.translate_c_type(setter.args[0].type)
            setterDict['is_value_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(setter.args[0].type.containedTypeDesc, AbsApi.ClassType)
            setterDict['is_value_string_list'] = 'bctbx_list_t*' in translated_arg_type and setter.args[0].type.containedTypeDesc.name == 'string'
            setterDict['is_value_obj'] = isinstance(setter.args[0].type, AbsApi.ClassType)
            setterDict['is_value_bool'] = translated_arg_type == 'bint'
            setterDict['is_value_void_ptr'] = translated_arg_type == 'void*'
            setterDict['is_value_string'] = setter.args[0].type.name == 'string'
            setterDict['is_simple_value'] = not setterDict['is_value_obj_list'] and not setterDict['is_value_string_list'] and not setterDict['is_value_obj'] and not setterDict['is_value_bool'] and not setterDict['is_value_void_ptr'] and not setterDict['is_value_string']

            setterDict['doc_value_type'] = self.translate_base_type_for_doc(setter.args[0].type)
            setterDict['doc_value_maybenil'] = setter.args[0].maybenil
            if setterDict['is_value_obj']:
                setterDict['python_value_type'] = setter.args[0].type.desc.name.translate(self.nameTranslator)
                setterDict['doc_value_type'] = setterDict['python_value_type']
            elif isinstance(setter.args[0].type, AbsApi.EnumType):
                setterDict['doc_value_type'] = setter.args[0].type.desc.name.translate(self.nameTranslator)
            elif setterDict['is_value_obj_list']:
                setterDict['python_value_type'] = setter.args[0].type.containedTypeDesc.desc.name.translate(self.nameTranslator)
                setterDict['doc_value_type'] = "list[" + setterDict['python_value_type'] + "]"

            propertyDict['setter'] = setterDict

        return propertyDict

    def translate_object_method(self, _obj, _method, is_static=False):
        methodDict = {}

        methodDict['is_static'] = is_static
        methodDict['c_name'] = _method.name.to_c()
        methodDict['python_name'] = _method.name.to_snake_case()
        methodDict['briefDoc'] = _method.briefDescription.translate(self.docTranslator)
        methodDict['detailedDoc'] = _method.detailedDescription.translate(self.docTranslator) if _method.detailedDescription is not None else None

        translated_type = self.translate_c_type(_method.returnType)
        methodDict['has_return'] = translated_type != 'void'
        methodDict['has_return_obj_list'] = 'bctbx_list_t*' in translated_type and isinstance(_method.returnType.containedTypeDesc, AbsApi.ClassType)
        methodDict['has_return_string_list'] = 'bctbx_list_t*' in translated_type and _method.returnType.containedTypeDesc.name == 'string'
        methodDict['has_return_obj'] = isinstance(_method.returnType, AbsApi.ClassType)
        methodDict['has_return_bool'] = translated_type == 'bint'
        methodDict['has_return_string'] = _method.returnType.name == 'string'
        methodDict['is_return_const'] = _method.returnType.isconst
        methodDict['has_return_void_vptr'] = translated_type == 'void*'
        methodDict['has_simple_return'] = methodDict['has_return'] and not methodDict['has_return_obj_list'] and not methodDict['has_return_string_list'] and not methodDict['has_return_obj'] and not methodDict['has_return_bool'] and not methodDict['has_return_string'] and not methodDict['has_return_void_vptr']
        methodDict['constructor'] = _method.returnAllocatedObject
        methodDict['is_const'] = _method.returnType.isconst
        
        methodDict['doc_return_type'] = self.translate_base_type_for_doc(_method.returnType)
        methodDict['doc_return_maybenil'] = _method.maybenil and not methodDict['has_return_obj_list'] and not methodDict['has_return_string_list']
        if methodDict['has_return_obj']:
            methodDict['return_python_type'] = _method.returnType.desc.name.translate(self.nameTranslator)
            methodDict['doc_return_type'] = methodDict['return_python_type']
        elif methodDict['has_return_obj_list']:
            methodDict['return_python_type'] = _method.returnType.containedTypeDesc.desc.name.translate(self.nameTranslator)
            methodDict['return_c_type'] = _method.returnType.containedTypeDesc.name
            methodDict['doc_return_type'] = "list[" + methodDict['return_python_type'] + "]"

        methodDict['params'] = []
        methodDict['python_params'] = ''
        methodDict['doc_python_params'] = ''
        methodDict['computed_params'] = ''

        for arg in _method.args:
            paramDict = {}
            translated_arg_type = self.translate_c_type(arg.type)
            paramDict['python_param_name'] = self.translate_arg_name(arg.name.to_c())
            if arg is not _method.args[0] or not is_static:
                methodDict['python_params'] += ', '
                methodDict['doc_python_params'] += ', '
                methodDict['computed_params'] += ', '

            methodDict['python_params'] += paramDict['python_param_name']
            methodDict['computed_params'] += paramDict['python_param_name']

            paramDict['param_c_type'] = translated_arg_type
            paramDict['is_obj_list'] = 'bctbx_list_t*' in translated_arg_type and isinstance(arg.type.containedTypeDesc, AbsApi.ClassType)
            paramDict['is_string_list'] = 'bctbx_list_t*' in translated_arg_type and arg.type.containedTypeDesc.name == 'string'
            paramDict['is_obj'] = isinstance(arg.type, AbsApi.ClassType)
            paramDict['is_enum'] = isinstance(arg.type, AbsApi.EnumType)
            paramDict['is_bool'] = translated_arg_type == 'bint'
            paramDict['is_void_ptr'] = translated_arg_type == 'void*'
            paramDict['is_string'] = arg.type.name == 'string'

            if paramDict['is_obj_list'] or paramDict['is_string_list']:
                methodDict['computed_params'] += '_list'
                if paramDict['is_obj_list']:
                    paramDict['param_c_type'] = self.translate_c_type(arg.type.containedTypeDesc)
                    paramDict['python_param_type'] = arg.type.containedTypeDesc.desc.name.translate(self.nameTranslator)
            elif paramDict['is_obj']:
                methodDict['computed_params'] += '_ptr'
                paramDict['python_param_type'] = arg.type.desc.name.translate(self.nameTranslator)
            elif paramDict['is_bool']:
                methodDict['computed_params'] += '_b'
            elif paramDict['is_void_ptr']:
                methodDict['computed_params'] += '_vptr'
            elif paramDict['is_string']:
                methodDict['computed_params'] += '_s'

            methodDict['params'].append(paramDict)
            
            argType = self.translate_base_type_for_doc(arg.type)
            if paramDict['is_obj_list']:
                argType = "list[" + paramDict['python_param_type'] + "]"
            elif paramDict['is_obj']:
                argType = paramDict['python_param_type']
            elif paramDict['is_enum']:
                argType = arg.type.desc.name.translate(self.nameTranslator)
            methodDict['doc_python_params'] += paramDict['python_param_name'] + ": " + argType
            if arg.maybenil:
                methodDict['doc_python_params'] += ' | None'

        return methodDict

    def translate_object(self, _obj, is_interface=False):
        objDict = {}

        objDict['briefDoc'] = _obj.briefDescription.translate(self.docTranslator)
        objDict['detailedDoc'] = _obj.detailedDescription.translate(self.docTranslator) if _obj.detailedDescription is not None else None
        objDict['c_name'] = _obj.name.to_c()
        objDict['python_name'] = _obj.name.translate(self.nameTranslator)
        objDict['unref'] = _obj.name.to_snake_case(fullName=True) + '_unref'
        objDict['ref'] = _obj.name.to_snake_case(fullName=True) + '_ref'
        objDict['is_factory'] = objDict['python_name'] == 'Factory'

        objDict['callbacks'] = []
        objDict['properties'] = []
        objDict['methods'] = []

        if not is_interface:
            if _obj.listenerInterface is not None:
                if _obj.multilistener:
                    objDict['methods'].append(self.create_add_callbacks_method(_obj))
                    objDict['methods'].append(self.create_remove_callbacks_method(_obj))

            for _method in _obj.classMethods:
                objDict['methods'].append(self.translate_object_method(_obj, _method, True))
            for _method in _obj.properties:
                if _method.getter is None and _method.setter is not None:
                    objDict['methods'].append(self.translate_object_method(_obj, _method.setter))
                else: # Do not wrap a setter that as no getter as a property
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
        self.c_enums = []
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
            self.c_enums.append(translator.translate_c_enum(_enum))
            self.enums.append(translator.translate_enum(_enum))

        for _class in parser.namespace.classes:
            if _class.listenerInterface is not None:
                if _class.multilistener:
                    self.c_methods.append(translator.create_c_add_callback_for_interface(_class))
                    self.c_methods.append(translator.create_c_remove_callback_for_interface(_class))
                    self.factory_constructors.append(translator.create_listener_constructor(_class))
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

class PylinphoneDoc(Pylinphone):
    def __init__(self, parser):
        Pylinphone.__init__(self, parser)

###############################################################################

def render(renderer, item, path):
    tmppath = path + '.tmp'
    content = ''
    with open(tmppath, mode='w') as f:
        f.write(renderer.render(item))
    with open(tmppath, mode='r') as f:
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
    args = argparser.parse_args()
    
    loglevel = logging.INFO
    logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

    entries = os.listdir(args.outputdir)

    project = CApi.Project()
    project.initFromDir(args.xmldir)
    project.check()

    parser = AbsApi.CParser(project)
	# Python can't overload methods
    parser.allow_method_overload = False
    parser.functionBl = \
        ['linphone_factory_get',\
        'linphone_factory_clean',\
        'linphone_vcard_get_belcard',\
        'linphone_core_get_current_vtable',\
        'linphone_core_get_zrtp_cache_db',\
        # Will be wrapped as from property which is a python keyword. linphone_event_get_from_address and linphone_event_get_to_address does the same thing
        'linphone_event_get_from',\
        'linphone_event_get_to',\
        # The following return or use as parameter a pointer towards a basic type, not yet supported
        'linphone_config_get_range',\
        'linphone_call_zoom_video',\
        'linphone_buffer_set_content',\
        'linphone_buffer_new_from_data',\
        'linphone_content_set_buffer',\
        'linphone_factory_create_buffer_from_data',\
        'linphone_buffer_get_content',\
        'linphone_content_get_buffer',\
        # Returns a bctbx_list_t * of enum, not yet supported
        'linphone_call_params_get_srtp_suites',\
        'linphone_core_get_zrtp_available_key_agreement_list',\
        'linphone_core_get_zrtp_key_agreement_list',\
        # Takes a bctbx_list_t * of enum as parameter, not yet supported
        'linphone_core_set_zrtp_key_agreement_suites',\
        # Don't know why but generates error: 
        # Cannot convert Python object to 'const unsigned int *'
        # linphone_factory_set_vfs_encryption(self.ptr, encryptionModule, secret, secretSize)
        #                                                                ^
        'linphone_factory_set_vfs_encryption']
    parser.classBl += 'LinphoneCoreVTable'
    parser.methodBl.remove('getCurrentCallbacks')
    
    # No nested enums
    parser.enum_relocations = {} 
    parser.enable_enum_relocations = False

    parser.parse_all()
    renderer = pystache.Renderer()

    wrapper = Pylinphone(parser)
    render(renderer, wrapper, args.outputdir + '/pylinphone.pyx')

    wrapper = PylinphoneDoc(parser)
    render(renderer, wrapper, args.outputdir + '/pylinphone.py')
