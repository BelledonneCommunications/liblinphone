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
from __future__ import print_function

import argparse
import errno
import logging
import os
import pystache
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
import genapixml as CApi
import abstractapi as AbsApi
import metadoc
import metaname


class SwiftTranslator(object):
    def __init__(self):
        self.ignore = []
        self.nameTranslator = metaname.Translator.get('Swift')
        self.langTranslator = AbsApi.Translator.get('Swift')
        self.docTranslator = metadoc.SwiftDocTranslator()

    def init_method_dict(self):
        methodDict = {}
        methodDict['has_impl'] = True
        methodDict['impl'] = {}
        methodDict['is_string_list'] = False
        methodDict['is_class_list'] = False
        methodDict['list_type'] = None
        methodDict['is_string'] = False
        methodDict['is_bool'] = False
        methodDict['is_int'] = False
        methodDict['is_class'] = False
        methodDict['is_enum'] = False
        methodDict['is_generic'] = False
        methodDict['takeRef'] = 'true'
        methodDict['listener'] = False
        return methodDict

    def get_class_array_type(self, name):
        length = len(name)
        if length > 2 and name.startswith('['):
            return name[1:length-1]
        return None

    def is_generic(self, methodDict):
        return not methodDict['is_string'] and not methodDict['is_bool'] and not methodDict['is_class'] and not methodDict['is_enum'] and not methodDict['is_int'] and methodDict['list_type'] == None

    def throws_exception(self, return_type):
        if isinstance(return_type, AbsApi.BaseType):
            if return_type.name == 'status':
                return True
        return False

    def translate_method(self, method, static=False, genImpl=True):
        if method.name.to_c() in self.ignore:
            raise AbsApi.Error('{0} has been escaped'.format(method.name.to_c()))

        methodDict = {}
        namespace = method.find_first_ancestor_by_type(AbsApi.Namespace)
        try:
            methodDict['doc'] = method.briefDescription.translate(self.docTranslator, tagAsBrief=True) if method.briefDescription is not None else None
            methodDict['detailedDoc'] = method.detailedDescription.translate(self.docTranslator) if method.detailedDescription is not None else None
        except metadoc.TranslationError as e:
            logging.error(e.msg())

        if genImpl:
            methodDict['impl'] = {}

            methodDict['impl']['static'] = 'static ' if static else ''
            methodDict['impl']['exception'] = self.throws_exception(method.returnType)
            methodDict['impl']['type'] = method.returnType.translate(self.langTranslator, namespace=namespace)
            methodDict['impl']['return'] = '' if methodDict['impl']['type'] == "void" else 'return '
            methodDict['impl']['name'] = method.name.translate(self.nameTranslator)
            methodDict['impl']['c_name'] = method.name.to_c()
            methodDict['impl']['cPtr'] = '' if static else ('cPtr, ' if len(method.args) > 0 else 'cPtr')

            methodDict['list_type'] = self.get_class_array_type(methodDict['impl']['type'])
            methodDict['is_string_list'] = methodDict['list_type'] == 'String'
            methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'String'
            methodDict['is_string'] = methodDict['impl']['type'] == "String"
            methodDict['is_bool'] = methodDict['impl']['type'] == "Bool"
            methodDict['is_int'] = methodDict['impl']['type'] == "Int" or methodDict['impl']['type'] == "UInt"
            methodDict['is_class'] = type(method.returnType) is AbsApi.ClassType
            methodDict['is_enum'] = type(method.returnType) is AbsApi.EnumType
            methodDict['is_generic'] = self.is_generic(methodDict)
            methodDict['isNotConst'] = not method.returnType.isconst
            methodDict['isDeprecated'] = method.deprecated

            methodDict['impl']['create_method'] = 'create' in method.name.to_word_list()
            if (methodDict['is_class'] and not methodDict['impl']['create_method']):
                methodDict['return_default'] = "?"
            methodDict['throw_default'] = " throws" if methodDict['impl']['exception'] or methodDict['impl']['create_method'] else ""
            methodDict['impl']['args'] = ''
            methodDict['impl']['c_args'] = ''

            for arg in method.args:
                argType = arg.type.translate(self.langTranslator)
                argName = arg.name.translate(self.nameTranslator)
                if argType.endswith('Delegate'):
                    argName = "delegate"
                if arg is not method.args[0]:
                    methodDict['impl']['args'] += ', '
                    methodDict['impl']['c_args'] += ', '
                if isinstance(arg.type, AbsApi.ClassType):
                    methodDict['impl']['c_args'] += argName + "?.cPtr" if arg.maybenil else argName + ".cPtr"
                elif isinstance(arg.type, AbsApi.EnumType):
                    argType = arg.type.translate(self.langTranslator, namespace=namespace)
                    if methodDict['impl']['type'] == "Int":
                        methodDict['impl']['c_args'] += arg.type.name + "(rawValue: CInt(" + argName + ".rawValue))"
                    else:
                        methodDict['impl']['c_args'] += arg.type.name + "(rawValue: CUnsignedInt(" + argName + ".rawValue))"
                elif arg.type.name == "size" or arg.type.name == "time":
                    methodDict['impl']['c_args'] += argName
                elif argType == "Int":
                    methodDict['impl']['c_args'] += "CInt(" + argName + ")"
                elif argType == "UInt":
                    methodDict['impl']['c_args'] += "CUnsignedInt(" + argName + ")"
                elif argType == "Bool":
                    methodDict['impl']['c_args'] += argName + "==true ? 1:0"
                elif self.get_class_array_type(arg.type.translate(self.langTranslator)) is not None:
                    listtype = self.get_class_array_type(arg.type.translate(self.langTranslator))
                    if listtype == 'String':
                        methodDict['impl']['c_args'] += "StringArrayToBctbxList(list:" + argName + ")"
                    else:
                        methodDict['impl']['c_args'] += "ObjectArrayToBctbxList(list: " + argName + ")"
                else:
                    methodDict['impl']['c_args'] += argName
                if argType == "UnsafePointer<Int>" and not arg.type.isconst:
                    argType = "UnsafeMutablePointer<Int32>"

                methodDict['impl']['args'] += argName + ":" + argType + "?" if arg.maybenil else argName + ":" + argType

        return methodDict


###########################################################################################################################################
    def translate_listener(self, _class, method):
        listenedClass = method.find_first_ancestor_by_type(AbsApi.Interface).listenedClass

        listenerDict = {}
        c_name_setter = listenedClass.name.to_snake_case(fullName=True) + '_cbs_set_' + method.name.to_snake_case()[3:]
        delegate_name_public = method.name.translate(self.nameTranslator) + "Delegate"

        listenerDict['delegate'] = {}
        try:
            listenerDict['delegate']['doc'] = method.briefDescription.translate(self.docTranslator, tagAsBrief=True) if method.briefDescription is not None else None
            listenerDict['delegate']['detailedDoc'] = method.detailedDescription.translate(self.docTranslator) if method.detailedDescription is not None else None
        except metadoc.TranslationError as e:
            logging.error(e.msg())
        listenerDict['delegate']['cb_name'] = method.name.translate(self.nameTranslator)
        listenerDict['delegate']['interfaceClassName'] = listenedClass.name.translate(self.nameTranslator)

        listenerDict['delegate']['type'] = method.returnType.translate(self.langTranslator)
        listenerDict['delegate']['return_class'] = type(method.returnType) is AbsApi.ClassType
        listenerDict['delegate']['return'] = "OpaquePointer?" if listenerDict['delegate']['return_class'] else "Void"

        listenerDict['delegate']['params_public'] = ""
        listenerDict['delegate']['params_private'] = ""
        listenerDict['delegate']['params'] = ""
        listenerDict['delegate']['classLists'] = {}
        namespace = method.find_first_ancestor_by_type(AbsApi.Namespace)
        for arg in method.args:
            referenceType = arg.type.translate(self.langTranslator, namespace = namespace)
            normalType = arg.type.translate(self.langTranslator)

            argName = arg.name.translate(self.nameTranslator)
            if arg != method.args[0]:
                listenerDict['delegate']['params_public'] += ', '
                listenerDict['delegate']['params_private'] += ', '
                listenerDict['delegate']['params'] += ', '

                if normalType == "Bool":
                    listenerDict['delegate']['params'] += argName + ": " + argName + " != 0"
                elif type(arg.type) is AbsApi.ClassType:
                    listenerDict['delegate']['params'] += argName + ": " + normalType + ".getSwiftObject(cObject: " + argName + "!)"
                elif type(arg.type) is AbsApi.EnumType:
                    ends = "" if arg.type.desc.isFlag else "!"
                    listenerDict['delegate']['params'] += argName + ": " + referenceType + "(rawValue: Int(" + argName + ".rawValue))" + ends
                    normalType = referenceType
                elif isinstance(arg.type, AbsApi.ListType):
                    if normalType == "String":
                        listenerDict['delegate']['params'] += "BctbxListToStringArray(list: " + argName + ")"
                    else:
                        listenerDict['delegate']['classLists']['classType'] = self.get_class_array_type(normalType)
                        listenerDict['delegate']['classLists']['argName'] = argName
                        listenerDict['delegate']['params'] += argName + ": " + argName + "sList"
                elif normalType == "String":
                        listenerDict['delegate']['params'] += argName + ": charArrayToString(charPointer: " + argName +")"
                elif normalType == "Int":
                        listenerDict['delegate']['params'] += argName + ": Int(" + argName + ")"
                else:
                    print('Not supported yet: ' + delegate_name_public)
                    return {}
            else:
                listenerDict['delegate']['first_param'] = argName
                listenerDict['delegate']['params'] = argName + ": sObject"

            listenerDict['delegate']['params_public'] += argName + ": " + normalType
            listenerDict['delegate']['params_private'] += argName

        listenerDict['delegate']["c_name_setter"] = c_name_setter
        return listenerDict


    def generate_add_for_listener_callbacks(self, _class, classname):
        methodDict = self.init_method_dict()
        c_name = _class.name.to_snake_case(fullName=True) + '_add_callbacks'

        methodDict['listener'] = True
        methodDict['name'] = 'addDelegate'

        methodDict['c_name'] = c_name
        methodDict['args'] = 'delegate: ' + classname + 'Delegate'
        methodDict['addListener'] = True
        methodDict['removeListener'] = False

        return methodDict

    def generate_remove_for_listener_callbacks(self, _class, classname):
        methodDict = self.init_method_dict()
        c_name = _class.name.to_snake_case(fullName=True) + '_remove_callbacks'

        methodDict['listener'] = True
        methodDict['name'] = 'removeDelegate'

        methodDict['c_name'] = c_name
        methodDict['args'] = 'delegate: ' + classname + 'Delegate'
        methodDict['removeListener'] = True
        methodDict['addListener'] = False

        return methodDict

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
        classDict['classDelegateName'] = classDict['className'] + "Delegate"
        classDict['isLinphoneFactory'] = classDict['className'] == "Factory"
        classDict['isLinphoneCall'] = _class.name.to_camel_case() == "Call"
        classDict['isLinphoneCore'] = _class.name.to_camel_case() == "Core"
        try:
            classDict['doc'] = _class.briefDescription.translate(self.docTranslator, tagAsBrief=True)
            classDict['detailedDoc'] = _class.detailedDescription.translate(self.docTranslator)
        except metadoc.TranslationError as e:
            logging.error(e.msg())
        classDict['properties'] = []
        classDict['classEnums'] = []

        islistenable = _class.listenerInterface is not None
        if islistenable:
            classDict['hasListener'] = True
            listenerName = _class.listenerInterface.name.translate(self.nameTranslator)
            if _class.multilistener:
                classDict['properties'].append(self.generate_add_for_listener_callbacks(_class, listenerName))
                classDict['properties'].append(self.generate_remove_for_listener_callbacks(_class, listenerName))

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
                classDict['properties'].append(methodDict)
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(method.name.to_c(), e.args[0]))

        for prop in _class.properties:
            try:
                classDict['properties'] += self.translate_property(prop)
            except AbsApi.Error as e:
                logging.error('error while translating {0} property: {1}'.format(prop.name.to_c(), e.args[0]))

        for method in _class.instanceMethods:
            try:
                methodDict = self.translate_method(method, static=False, genImpl=True)
                classDict['properties'].append(methodDict)
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(method.name.to_c(), e.args[0]))

        for enum in _class.enums:
            classDict['classEnums'].append(self.translate_enum(enum))

        return classDict
    def translate_interface(self, interface):
        if interface.name.to_c() in self.ignore:
            raise AbsApi.Error('{0} has been escaped'.format(interface.name.to_c()))

        interfaceDict = {}
        interfaceDict['interfaceName'] = interface.name.translate(self.nameTranslator)
        interfaceDict['create_user_data_name'] = 'linphone_factory_create' + interface.listenedClass.name.to_snake_case(fullName=True).lstrip('linphone') + '_cbs'

        interfaceDict['methods'] = []
        for method in interface.instanceMethods:
            interfaceDict['methods'].append(self.translate_listener(interface, method))

        return interfaceDict
###########################################################################################################################################

    def translate_property_getter(self, prop, name, static=False):
        methodDict = self.translate_method(prop, static, False)

        methodDict['property_name'] = "Instance" if name == "get" else name
        methodDict['Instance'] = name == "get"
        methodDict['has_property'] = True
        methodDict['has_getter'] = True
        methodDict['has_setter'] = False
        methodDict['isDeprecated'] = prop.deprecated
        namespace = prop.find_first_ancestor_by_type(AbsApi.Namespace)
        methodDict['return'] = prop.returnType.translate(self.langTranslator, namespace=namespace)
        if methodDict['return'].endswith('Delegate'):
            methodDict['is_callbacks'] = True
        methodDict['exception'] = self.throws_exception(prop.returnType)
        methodDict['getter_c_name'] = prop.name.to_c()

        methodDict['list_type'] = self.get_class_array_type(methodDict['return'])
        methodDict['is_string_list'] = methodDict['list_type'] == 'String'
        methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'String'

        methodDict['is_string'] = methodDict['return'] == "String"
        methodDict['is_bool'] = methodDict['return'] == "Bool"
        methodDict['is_void'] = methodDict['return'] == "UnsafeMutableRawPointer"
        methodDict['is_int'] = methodDict['return'] == "Int" or methodDict['return'] == "UInt"
        methodDict['is_class'] = type(prop.returnType) is AbsApi.ClassType
        methodDict['is_enum'] = type(prop.returnType) is AbsApi.EnumType
        methodDict['is_generic'] = self.is_generic(methodDict)
        methodDict['cPtr'] = '' if static else 'cPtr'
        methodDict['static'] = 'static ' if static else ''
        methodDict['isNotConst'] = not prop.returnType.isconst
        methodDict['isFlag'] = methodDict['is_enum'] and prop.returnType.desc.isFlag

        if (methodDict['is_class'] and name != "get") or methodDict['is_void']:
            methodDict['return_default'] = "?"

        return methodDict

    def translate_property_setter(self, prop, name, static=False):
        methodDict = self.translate_method(prop, static, False)

        methodDict['property_name'] = name
        methodDict['func_name'] = "set" + name.capitalize()
        methodDict['isDeprecated'] = prop.deprecated

        methodDict['has_getter'] = False
        methodDict['has_setter'] = True
        namespace = prop.find_first_ancestor_by_type(AbsApi.Namespace)
        methodDict['return'] = prop.args[0].type.translate(self.langTranslator, namespace=namespace)
        methodDict['returnCType'] = prop.args[0].type.name
        methodDict['exception'] = self.throws_exception(prop.returnType)
        methodDict['has_property'] = not methodDict['exception']
        methodDict['setter_c_name'] = prop.name.to_c()

        methodDict['list_type'] = self.get_class_array_type(methodDict['return'])
        methodDict['is_string_list'] = methodDict['list_type'] == 'String'
        methodDict['is_class_list'] = not methodDict['list_type'] == None and not methodDict['list_type'] == 'String'

        methodDict['is_string'] = methodDict['return'] == "String"
        methodDict['is_bool'] = methodDict['return'] == "Bool"
        methodDict['is_void'] = methodDict['return'] == "UnsafeMutableRawPointer"
        methodDict['is_int'] = methodDict['return'] == "Int" or methodDict['return'] == "UInt"
        methodDict['int_method'] = "" if prop.args[0].type.name in ["size", "long"] else ("CInt" if methodDict['return'] == "Int" else "CUnsignedInt")
        methodDict['is_class'] = type(prop.args[0].type) is AbsApi.ClassType
        methodDict['is_enum'] = type(prop.args[0].type) is AbsApi.EnumType
        methodDict['enum_type'] = "CUnsignedInt" if methodDict['is_enum'] and prop.args[0].type.desc.isUnsigned else "CInt"
        methodDict['is_generic'] = self.is_generic(methodDict)

        if methodDict['is_class'] or methodDict['is_void'] or methodDict['is_bool'] or methodDict['is_enum']:
            methodDict['return_default'] = "?"
        elif methodDict['is_generic'] or methodDict['is_int']:
            methodDict['return_default'] = " = 0"
        elif methodDict['is_string_list']:
            methodDict['return_default'] = " = []"
        elif methodDict['is_string']:
            methodDict['return_default'] = ' = ""'

        return methodDict

    def translate_property_getter_setter(self, getter, setter, name, static=False):
        methodDict = self.translate_property_getter(getter, name, static=static)
        methodDictSet = self.translate_property_setter(setter, name, static)

        protoElems = {}

        methodDict['has_setter'] = True
        methodDict['has_getter'] = True
        methodDict['has_property'] = True
        methodDict['exception'] = methodDictSet['exception']
        methodDict['setter_c_name'] = methodDictSet['setter_c_name']
        methodDict['enum_type'] = methodDictSet['enum_type']
        methodDict['int_method'] = methodDictSet['int_method']
        methodDict['func_name'] = methodDictSet['func_name']
        methodDict['returnCType'] = methodDictSet['returnCType']

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



##########################################################################

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
##########################################################################

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

##########################################################################

if __name__ == '__main__':
    import subprocess
    git_version = subprocess.check_output(["git", "describe"]).strip()
    argparser = argparse.ArgumentParser(description='Generate source files for the Swift wrapper')
    argparser.add_argument('xmldir', type=str, help='Directory where the XML documentation of the Linphone\'s API generated by Doxygen is placed')
    argparser.add_argument('-o --output', type=str, help='the directory where to generate the source files', dest='outputdir', default='.')
    argparser.add_argument('-p --package', type=str, help='the package name for the wrapper', dest='package', default='org.linphone.core')
    argparser.add_argument('-n --name', type=str, help='the name of the genarated source file', dest='outputfile', default='LinphoneWrapper.swift')
    argparser.add_argument('-e --exceptions', type=bool, help='enable the wrapping of LinphoneStatus into CoreException', dest='exceptions', default=False)
    argparser.add_argument('-v --verbose', action='store_true', dest='verbose_mode', default=False, help='Verbose mode.')
    args = argparser.parse_args()

    loglevel = logging.INFO if args.verbose_mode else logging.ERROR
    logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

    project = CApi.Project()
    project.initFromDir(args.xmldir)
    project.check()

    parser = AbsApi.CParser(project)

    parser.functionBl += [
    'linphone_account_creator_get_route',
    'linphone_account_creator_set_route',
    'linphone_account_creator_configure',
    'linphone_core_get_log_level_mask',
    'linphone_core_set_log_level_mask',
    'linphone_core_get_chat_database_path',
    'linphone_core_set_chat_database_path',
    'linphone_core_get_linphone_specs',
    'linphone_core_set_linphone_specs',
    'linphone_core_accept_call',
    'linphone_core_accept_call_update',
    'linphone_core_accept_call_with_params',
    'linphone_core_accept_early_media',
    'linphone_core_accept_early_media_with_params',
    'linphone_core_create_client_group_chat_room',
    'linphone_core_create_client_group_chat_room_2',
    'linphone_core_decline_call',
    'linphone_core_defer_call_update',
    'linphone_core_enable_lime',
    'linphone_core_lime_available',
    'linphone_core_lime_enabled',
    'linphone_core_pause_call',
    'linphone_core_redirect_call',
    'linphone_core_resume_call',
    'linphone_core_terminate_call',
    'linphone_core_transfer_call',
    'linphone_core_transfer_call_to_another',
    'linphone_core_update_call',
    'linphone_factory_create_core',
    'linphone_factory_create_core_2',
    'linphone_factory_create_core_with_config_2',
    'linphone_factory_create_core_with_config',
    'linphone_core_get_zrtp_cache_db'
    ]



    parser.classBl += 'LinphoneCoreVTable'
    parser.methodBl = ['ref', 'unref', 'new', 'destroy']
    parser.enum_relocations = {
        'LinphoneAccountCreatorActivationCodeStatus' : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorDomainStatus'         : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorEmailStatus'          : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorLanguageStatus'       : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorPasswordStatus'       : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorPhoneNumberStatus'    : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorStatus'               : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorTransportStatus'      : 'LinphoneAccountCreator',
        'LinphoneAccountCreatorUsernameStatus'       : 'LinphoneAccountCreator',
        'LinphoneCallDir'                            : 'LinphoneCall',
        'LinphoneCallState'                          : 'LinphoneCall',
        'LinphoneCallStatus'                         : 'LinphoneCall',
        'LinphoneConferenceState'                    : 'LinphoneConference',
        'LinphoneChatRoomState'                      : 'LinphoneChatRoom',
        'LinphoneChatMessageDirection'               : 'LinphoneChatMessage',
        'LinphoneChatMessageState'                   : 'LinphoneChatMessage',
        'LinphoneCoreLogCollectionUploadState'       : 'LinphoneCore',
        'LinphoneFriendListStatus'                   : 'LinphoneFriendList',
        'LinphoneFriendListSyncStatus'               : 'LinphoneFriendList',
        'LinphonePlayerState'                        : 'LinphonePlayer',
        'LinphoneTunnelMode'                         : 'LinphoneTunnel'
    }
    parser.parse_all()
    translator = SwiftTranslator()
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


    wrapper = WrapperImpl(git_version, enums, interfaces, classes)
    render(renderer, wrapper, args.outputdir + "/" + args.outputfile)
