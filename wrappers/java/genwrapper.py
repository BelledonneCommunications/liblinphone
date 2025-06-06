#!/usr/bin/python

# Copyright (c) 2010-2022 Belledonne Communications SARL.
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

CORE_ACCESSOR_LIST = [
    'Call',
    'ChatMessage',
    'ChatRoom',
    'Event',
    'Friend',
    'FriendList',
    'NatPolicy',
    'Player',
    'ProxyConfig'
]


class JNINameTranslator(metaname.Translator):
    _instance = None

    @staticmethod
    def get():
        if JNINameTranslator._instance is None:
            JNINameTranslator._instance = JNINameTranslator()
        return JNINameTranslator._instance

    def translate_class_name(self, name, **params):
        translated_name = name.to_camel_case()
        if name.prev is not None and type(name.prev) is not metaname.NamespaceName:
            return name.prev.translate(self) + self._getseparator(name.prev) + translated_name
        else:
            return translated_name

    def translate_interface_name(self, name, **params):
        return self.translate_class_name(name, **params)

    def translate_enum_name(self, name, **params):
        return self.translate_class_name(name, **params)

    def translate_enumerator_name(self, name, **params):
        raise NotImplemented()

    def translate_method_name(self, name, **params):
        raise NotImplemented()

    def translate_namespace_name(self, name, **params):
        translated_name = name.to_snake_case()
        if name.prev is not None:
            return name.prev.translate(self) + self._getseparator(name.prev) + translated_name
        else:
            return translated_name

    def translate_argument_name(self, name, **params):
        raise NotImplemented()

    def translate_property_name(self, name, **params):
        raise NotImplemented()

    def _getseparator(self, previous_name):
        if isinstance(previous_name, metaname.NamespaceName):
            return '/'
        elif isinstance(previous_name, metaname.ClassName):
            return '$'
        else:
            raise TypeError("no separator for '{0}' type".format(type(previous_name)))


class JNILangTranslator(AbsApi.Translator):
    _instance = None

    @staticmethod
    def get():
        if JNILangTranslator._instance is None:
            JNILangTranslator._instance = JNILangTranslator()
        return JNILangTranslator._instance

    def translate_base_type(self, type_):
        if type_.name == 'string':
            return 'Ljava/lang/String;'
        elif type_.name == 'integer':
            return 'I'
        elif type_.name == 'boolean':
            return 'Z'
        elif type_.name == 'floatant':
            return 'F'
        elif type_.name == 'size':
            return 'I'
        elif type_.name == 'time':
            return 'J'
        elif type_.name == 'long':
            return 'J'
        elif type_.name == 'status':
            return 'I'
        elif type_.name == 'string_array':
            return '[Ljava/lang/String;'
        elif type_.name == 'character':
            return 'C'
        elif type_.name == 'void':
            return 'V'
        return type_.name


class JavaTranslator:
    def __init__(self, packageName, exceptions, platform):
        self.exceptions = exceptions
        self.platform = platform
        package_dirs = packageName.split('.')
        self.jni_package = ''
        self.jni_path = ''
        for directory in package_dirs:
            self.jni_package += directory + '_'
            self.jni_path += directory + '/'

        self.nameTranslator = metaname.Translator.get('Java')
        self.langTranslator = AbsApi.Translator.get('Java')
        self.docTranslator = metadoc.JavaDocTranslator()

        self.jninameTranslator = JNINameTranslator.get()
        self.jnilangTranslator = JNILangTranslator.get()

        self.clangTranslator = AbsApi.Translator.get('C')

    def throws_exception(self, _type):
        if not self.exceptions:
            return False
        if isinstance(_type, AbsApi.BaseType):
            if _type.name == 'status':
                return True
        return False

    def fix_getter_property_name(self, name):
        return "is{0}{1}".format(name[0].title(), name[1:])

    def fix_setter_property_name(self, name):
        return "set{0}Enabled".format(name[6:])

    def translate_property(self, _property, _hasCoreAccessor):
        properties = []
        if _property.getter is not None:
            property = self.translate_method(_property.getter, _hasCoreAccessor)
            name = property["name"]
            if name.endswith("Enabled") and not (name.startswith("is") or name.startswith("get")):
                newName = self.fix_getter_property_name(name)
                property = self.translate_method(_property.getter, _hasCoreAccessor, newName)
                properties.append(property)
            else:
                properties.append(property)
                
        if _property.setter is not None:
            property = self.translate_method(_property.setter, _hasCoreAccessor)
            name = property["name"]
            if name.startswith("enable"):
                newName = self.fix_setter_property_name(name)
                property = self.translate_method(_property.setter, _hasCoreAccessor, newName)
                properties.append(property)
            else:
                properties.append(property)

        return properties

    def translate_jni_property(self, class_, _property):
        properties = []
        if _property.getter is not None:
            property = self.translate_jni_method(class_, _property.getter)
            name = property["methodName"]
            if property["name"].endswith("Enabled") and not (name.startswith("is") or name.startswith("get")):
                newName = self.fix_getter_property_name(name)
                property = self.translate_jni_method(class_, _property.getter, False, newName)
                properties.append(property)
            else:
                properties.append(property)

        if _property.setter is not None:
            property = self.translate_jni_method(class_, _property.setter)
            if property["notEmpty"] == True and property["methodName"].startswith("enable"):
                name = property["methodName"]
                newName = self.fix_setter_property_name(name)
                property = self.translate_jni_method(class_, _property.setter, False, newName)
                properties.append(property)
            else:
                properties.append(property)

        return properties

    def generate_listener(self, name, _class):
        methodDict = {}
        methodDict['return'] = 'void'
        methodDict['return_maybenil'] = False
        methodDict['return_notnil'] = False
        methodDict['return_native'] = 'void'
        methodDict['return_keyword'] = ''
        methodDict['convertInputClassArrayToLongArray'] = False
        methodDict['name'] = name
        methodDict['name_native'] = name
        methodDict['exception'] = False
        methodDict['enumCast'] = False
        methodDict['classCast'] = False

        methodDict['params'] = _class.name.to_camel_case() + 'Listener listener'
        methodDict['native_params'] = 'long nativePtr, ' + _class.name.to_camel_case() + 'Listener listener'
        methodDict['static_native_params'] = ''
        methodDict['native_params_impl'] = 'nativePtr, listener'

        methodDict['deprecated'] = False
        methodDict['briefDoc'] = None
        methodDict['detailedDoc'] = None

        return methodDict

    def generate_add_listener(self, _class):
        return self.generate_listener('addListener', _class)

    def generate_remove_listener(self, _class):
        return self.generate_listener('removeListener', _class)

    def generate_set_listener(self, _class):
        return self.generate_listener('setListener', _class)

    def translate_method_native_arg(self, arg, namespace):
        if type(arg.type) is AbsApi.EnumType:
            return arg.name.translate(self.nameTranslator) + '.toInt()'
        elif type(arg.type) is AbsApi.ListType and isinstance(arg.type.containedTypeDesc, AbsApi.EnumType):
            typename = arg.type.translate(self.langTranslator, isReturn=True, namespace=namespace)
            return typename[:-2] + '.toIntArray(' + arg.name.translate(self.nameTranslator) + ')'
        return arg.name.translate(self.nameTranslator)

    def translate_method(self, _method, _hasCoreAccessor=False, _forceMethodName=""):
        methodDict = {}

        namespace = _method.find_first_ancestor_by_type(AbsApi.Namespace)

        methodDict['return'] = _method.returnType.translate(self.langTranslator, isReturn=True, namespace=namespace, exceptionEnabled=self.exceptions)
        native_type = _method.returnType.translate(self.langTranslator, jni=True, isReturn=True, namespace=namespace)
        isArray = native_type == 'jobjectArray' or native_type == 'jintArray'
        # Wrapper takes care or never returning a null array even if the doc says it can return a null list
        methodDict['return_maybenil'] = _method.maybenil and not isArray
        methodDict['return_notnil'] = _method.notnil or isArray

        methodDict['return_native'] = _method.returnType.translate(self.langTranslator, native=True, isReturn=True, namespace=namespace, exceptionEnabled=self.exceptions)
        methodDict['return_keyword'] = '' if methodDict['return'] == 'void' else 'return '
        methodDict['hasReturn'] = not methodDict['return'] == 'void'
        methodDict['cantBeCalledIfObjectIsConst'] = not _method.isconst

        methodDict['convertInputClassArrayToLongArray'] = False

        methodDict['name'] = _method.name.to_camel_case(lower=True)
        if _forceMethodName != "":
            methodDict['name'] = _forceMethodName
        methodDict['name_native'] = methodDict['name']

        if _method.name.to_c()[-1].isdigit():
            methodDict['name_native'] += _method.name.to_c()[-1]

        methodDict['isNotGetCore'] = not methodDict['name'] == 'getCore'
        methodDict['hasCoreAccessor'] = _hasCoreAccessor
        methodDict['exception'] = self.throws_exception(_method.returnType)

        methodDict['enumCast'] = type(_method.returnType) is AbsApi.EnumType
        methodDict['enumArrayCast'] = type(_method.returnType) is AbsApi.ListType and isinstance(_method.returnType.containedTypeDesc, AbsApi.EnumType)
        methodDict['enumName'] = ""
        if methodDict['enumArrayCast']:
            methodDict['enumName'] = methodDict['return'][:-2]
        methodDict['classCast'] = type(_method.returnType) is AbsApi.ClassType

        if self.platform == "android" or self.platform == "bundled-android":
            methodDict['params'] = ', '.join(['{0}{1}'.format('@Nullable ' if arg.maybenil else '@NonNull ' if arg.notnil else '', arg.translate(self.langTranslator, namespace=namespace)) for arg in _method.args])
        else:
            methodDict['params'] = ', '.join(['{0}'.format(arg.translate(self.langTranslator, namespace=namespace)) for arg in _method.args])
        methodDict['native_params'] = ', '.join(['long nativePtr'] + [arg.translate(self.langTranslator, native=True, namespace=namespace) for arg in _method.args])
        methodDict['static_native_params'] = ', '.join([arg.translate(self.langTranslator, native=True, namespace=namespace) for arg in _method.args])
        methodDict['native_params_impl'] = ', '.join(
            ['nativePtr'] + [self.translate_method_native_arg(arg, namespace) for arg in _method.args])

        methodDict['deprecated'] = _method.deprecated
        methodDict['briefDoc'] = _method.briefDescription.translate(self.docTranslator, tagAsBrief=True) if _method.briefDescription is not None else None
        methodDict['detailedDoc'] = _method.detailedDescription.translate(self.docTranslator) if _method.detailedDescription is not None else None

        return methodDict

    def translate_jni_method(self, class_, _method, static=False, _forceMethodName=""):
        jni_blacklist = ['linphone_call_set_native_video_window_id',
                        'linphone_core_set_native_preview_window_id',
                        'linphone_core_set_native_video_window_id',
                        'linphone_participant_device_set_native_video_window_id']

        namespace = class_.find_first_ancestor_by_type(AbsApi.Namespace)
        className = class_.name.translate(self.nameTranslator)

        methodDict = {'notEmpty': True}
        methodDict['classCName'] = class_.name.to_c()
        methodDict['className'] = className
        methodDict['classImplName'] = className + 'Impl'
        methodDict['isLinphoneFactory'] = (className == 'Factory')
        methodDict['jniPath'] = self.jni_path
        methodDict['isLinphoneCore'] = (className == 'Core')
        methodDict['isNotLinphoneCoreStart'] = (_method.name.to_c() != 'linphone_core_start')
        hasCoreAccessor = class_.name.to_camel_case() in CORE_ACCESSOR_LIST
        methodDict['hasCoreAccessor'] = hasCoreAccessor
        if hasCoreAccessor:
            methodDict['c_core_accessor'] = 'linphone_' + class_.name.to_snake_case() + '_get_core'
        methodDict['needLjb'] = False

        methodDict['return'] = _method.returnType.translate(self.langTranslator, jni=True, isReturn=True, namespace=namespace)
        methodDict['hasListReturn'] = methodDict['return'] == 'jobjectArray' or methodDict['return'] == 'jintArray'
        methodDict['hasByteArrayReturn'] = methodDict['return'] == 'jbyteArray'
        methodDict['hasReturn'] = not methodDict['return'] == 'void' and not methodDict['hasListReturn'] and not methodDict['hasByteArrayReturn']
        methodDict['hasStringReturn'] = methodDict['return'] == 'jstring'
        methodDict['hasNormalReturn'] = not methodDict['hasListReturn'] and not methodDict['hasStringReturn'] and not methodDict['hasByteArrayReturn']
        methodDict['methodName'] = _method.name.translate(self.nameTranslator)
        methodDict['name'] = 'Java_' + self.jni_package + className + 'Impl_' + methodDict['methodName']
        if _forceMethodName != "":
            methodDict['name'] = 'Java_' + self.jni_package + className + 'Impl_' + _forceMethodName

        methodDict['notStatic'] = not static
        methodDict['isConst'] = _method.returnType.isconst
        methodDict['isNotConst'] = not _method.returnType.isconst

        if _method.name.to_c()[-1].isdigit():
            methodDict['name'] += _method.name.to_c()[-1]

        if _method.name.to_c() == 'linphone_factory_create_core':
            methodDict['c_name'] = 'linphone_factory_create_core_3'
        elif _method.name.to_c() == 'linphone_factory_create_core_with_config':
            methodDict['c_name'] = 'linphone_factory_create_core_with_config_3'
        else:
            methodDict['c_name'] = _method.name.to_c()

        methodDict['takeRefOnReturnedObject'] = "FALSE" if _method.returnAllocatedObject else "TRUE"
        methodDict['returnedObjectIsConst'] = "TRUE" if _method.returnType.isconst else "FALSE"

        methodDict['returnObject'] = methodDict['hasReturn'] and type(_method.returnType) is AbsApi.ClassType
        methodDict['returnClassName'] = _method.returnType.translate(self.langTranslator, namespace=namespace)
        methodDict['isRealObjectArray'] = False
        methodDict['isStringObjectArray'] = False
        methodDict['isEnumObjectArray'] = False
        methodDict['c_type_return'] = _method.returnType.translate(self.clangTranslator)

        if methodDict['c_name'] in jni_blacklist:
            return {'notEmpty': False}

        if methodDict['hasListReturn']:
            if isinstance(_method.returnType, AbsApi.BaseType) and _method.returnType.name == 'string_array':
                methodDict['isStringObjectArray'] = True
            elif isinstance(_method.returnType.containedTypeDesc, AbsApi.BaseType):
                methodDict['isStringObjectArray'] = True
            elif isinstance(_method.returnType.containedTypeDesc, AbsApi.EnumType):
                methodDict['isEnumObjectArray'] = True
            elif isinstance(_method.returnType.containedTypeDesc, AbsApi.ClassType):
                methodDict['isRealObjectArray'] = True
                methodDict['needLjb'] = True
                methodDict['isRealObjectConst'] = "TRUE" if _method.returnType.containedTypeDesc.isconst else "FALSE"
                methodDict['objectCPrefix'] = 'linphone_' + _method.returnType.containedTypeDesc.desc.name.to_snake_case()
                methodDict['objectClassCName'] = 'Linphone' + _method.returnType.containedTypeDesc.desc.name.to_camel_case()
                methodDict['objectClassName'] = _method.returnType.containedTypeDesc.desc.name.to_camel_case()
                methodDict['objectClassImplName'] = _method.returnType.containedTypeDesc.desc.name.to_camel_case() + 'Impl'

        methodDict['params'] = 'JNIEnv *env, jobject thiz, jlong ptr'
        methodDict['params_impl'] = ''
        methodDict['strings'] = []
        methodDict['objects'] = []
        methodDict['lists'] = []
        methodDict['array'] = []
        methodDict['bytes'] = []
        methodDict['returnedObjectGetter'] = ''
        for arg in _method.args:
            methodDict['params'] += ', '
            if static:
                if arg is not _method.args[0]:
                    methodDict['params_impl'] += ', '
            else:
                methodDict['params_impl'] += ', '

            methodDict['params'] += arg.translate(self.langTranslator, jni=True, namespace=namespace)
            argname = arg.name.translate(self.nameTranslator)

            if isinstance(arg.type, AbsApi.ClassType):
                classCName = 'Linphone' + arg.type.desc.name.to_camel_case()
                if classCName[-8:] == 'Listener':
                   classCName = 'Linphone' + arg.type.desc.name.to_camel_case()[:-8] + 'Cbs'
                methodDict['objects'].append({'object': argname, 'objectClassCName': classCName})
                methodDict['params_impl'] += 'c_' + argname

            elif isinstance(arg.type, AbsApi.ListType):
                isStringList = type(arg.type.containedTypeDesc) is AbsApi.BaseType and arg.type.containedTypeDesc.name == 'string'
                isObjList = type(arg.type.containedTypeDesc) is AbsApi.ClassType
                isEnumList = type(arg.type.containedTypeDesc) is AbsApi.EnumType
                methodDict['lists'].append({'list': argname, 'isStringList': isStringList, 'isEnumList': isEnumList, 'isObjList': isObjList, 'objectClassCName': arg.type.containedTypeDesc.name})
                methodDict['params_impl'] += 'bctbx_list_' + argname

            elif isinstance(arg.type, AbsApi.EnumType):
                argCType = arg.type.name
                methodDict['params_impl'] += '(' + argCType + ') ' + argname

            elif isinstance(arg.type, AbsApi.BaseType):
                if arg.type.name == 'integer' and arg.type.size is not None and arg.type.isref:
                    methodDict['bytes'].append({'bytesargname': argname, 'bytesargtype' : arg.type.translate(self.clangTranslator)})
                    methodDict['params_impl'] += 'c_' + argname
                elif arg.type.name == 'string':
                    methodDict['strings'].append({'string': argname})
                    methodDict['params_impl'] += 'c_' + argname
                else:
                    methodDict['params_impl'] += '(' + arg.type.translate(self.clangTranslator).replace("const ", "") + ')' + argname
            else:
                methodDict['params_impl'] += argname

        return methodDict

    def translate_class(self, _class):
        classDict = {
            'methods': [],
            'jniMethods': [],
        }
        classDict['toStringFound'] = False
        classDict['isLinphoneFactory'] = _class.name.to_camel_case() == "Factory"
        classDict['isLinphoneCore'] = _class.name.to_camel_case() == "Core"
        hasCoreAccessor = _class.name.to_camel_case() in CORE_ACCESSOR_LIST
        classDict['hasCoreAccessor'] = hasCoreAccessor
        classDict['briefDoc'] = _class.briefDescription.translate(self.docTranslator, tagAsBrief=True) if _class.briefDescription is not None else None
        classDict['detailedDoc'] = _class.detailedDescription.translate(self.docTranslator) if _class.detailedDescription is not None else None
        classDict['refCountable'] = _class.refcountable

        for _property in _class.properties:
            try:
                classDict['methods'] += self.translate_property(_property, hasCoreAccessor)
                classDict['jniMethods'] += self.translate_jni_property(_class, _property)
            except AbsApi.Error as e:
                logging.error('error while translating {0} property: {1}'.format(_property.name.to_snake_case(), e.args[0]))

        for method in _class.instanceMethods:
            try:
                methodDict = self.translate_method(method, hasCoreAccessor)
                jniMethodDict = self.translate_jni_method(_class, method)
                classDict['methods'].append(methodDict)
                classDict['jniMethods'].append(jniMethodDict)
                if (methodDict['name'] == "toString"):
                    classDict['toStringFound'] = True
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(method.name.to_snake_case(fullName=True), e.args[0]))

        for method in _class.classMethods:
            try:
                methodDict = self.translate_method(method, hasCoreAccessor)
                jniMethodDict = self.translate_jni_method(_class, method, True)
                classDict['methods'].append(methodDict)
                classDict['jniMethods'].append(jniMethodDict)
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(method.name.to_snake_case(fullName=True), e.args[0]))

        islistenable = _class.listenerInterface is not None
        if islistenable:
            if _class.multilistener:
                classDict['methods'].append(self.generate_add_listener(_class))
                classDict['methods'].append(self.generate_remove_listener(_class))

        return classDict

    def translate_jni_interface(self, _class, className, _method):
        methodDict = {}
        methodDict['classCName'] = className.to_c()
        methodDict['className'] = className.translate(self.nameTranslator)
        methodDict['classImplName'] = methodDict['className'] + 'Impl'
        methodDict['jniPath'] = self.jni_path
        methodDict['cPrefix'] = _class.name.to_snake_case(fullName=True)
        methodDict['callbackName'] = '_{0}_cb'.format(_method.name.to_snake_case(fullName=True))
        methodDict['jname'] = _method.name.translate(self.nameTranslator)
        methodDict['return'] = _method.returnType.translate(self.clangTranslator)
        methodDict['jniUpcallMethod'] = 'CallVoidMethod'
        methodDict['isJniUpcallBasicType'] = False
        methodDict['isJniUpcallObject'] = False

        if isinstance(_method.returnType, AbsApi.ClassType):
            methodDict['jniUpcallMethod'] = 'CallObjectMethod'
            methodDict['isJniUpcallObject'] = True
            methodDict['jniUpcallType'] = 'jobject'
        elif isinstance(_method.returnType, AbsApi.BaseType):
            if not _method.returnType.name == 'void':
                methodDict['jniUpcallMethod'] = 'CallIntMethod'
                methodDict['jniUpcallType'] = _method.returnType.translate(self.langTranslator, jni=True)
                methodDict['isJniUpcallBasicType'] = True
        methodDict['returnIfFail'] = '' if  methodDict['return'] == 'void' else ' NULL'
        methodDict['hasReturn'] = not methodDict['return'] == 'void'
        methodDict['isMultiListener'] = _class.multilistener

        methodDict['firstParam'] = ''
        methodDict['jobjects'] = []
        methodDict['jenums'] = []
        methodDict['jstrings'] = []
        methodDict['jlists'] = []
        methodDict['params'] = ''
        methodDict['jparams'] = '('
        methodDict['params_impl'] = ''
        for arg in _method.args:
            argname = arg.name.translate(self.nameTranslator)
            if arg is not _method.args[0]:
                methodDict['params'] += ', '
                methodDict['params_impl'] += ', '
            else:
                methodDict['firstParam'] = argname

            methodDict['params'] += '{0} {1}'.format(arg.type.translate(self.clangTranslator), argname)

            if isinstance(arg.type, AbsApi.ClassType):
                methodDict['jparams'] += 'L' + self.jni_path + arg.type.desc.name.to_camel_case() + ';'
                methodDict['params_impl'] += 'j_' + argname
                isConst = 'TRUE' if arg.type.isconst else 'FALSE'
                methodDict['jobjects'].append({'objectName': argname, 'className': arg.type.desc.name.to_camel_case(), 'isObjectConst':isConst})
            elif isinstance(arg.type, AbsApi.BaseType):
                methodDict['jparams'] += arg.type.translate(self.jnilangTranslator)
                if arg.type.name == 'string':
                    methodDict['params_impl'] += 'j_' + argname
                    methodDict['jstrings'].append({'stringName': argname,})
                else:
                    methodDict['params_impl'] += argname
            elif isinstance(arg.type, AbsApi.EnumType):
                methodDict['jparams'] += 'L' + self.jni_path + arg.type.desc.name.translate(self.jninameTranslator) + ';'
                methodDict['params_impl'] += 'j_' + argname
                methodDict['jenums'].append({'enumName': argname, 'cEnumPrefix': arg.type.desc.name.to_snake_case(fullName=True)})
                methodDict['needLjb'] = True
            elif isinstance(arg.type, AbsApi.ListType):
                methodDict['jparams'] += '[L' + self.jni_path + arg.type.containedTypeDesc.desc.name.to_camel_case() + ';'
                methodDict['params_impl'] += 'j_' + argname

                if isinstance( arg.type.containedTypeDesc, AbsApi.BaseType):
                    methodDict['jlists'].append({'list_name': argname, 'isRealObjectArray':False, 'isStringObjectArray':True})
                elif isinstance( arg.type.containedTypeDesc, AbsApi.ClassType):
                    cprefix = 'linphone_' +  arg.type.containedTypeDesc.desc.name.to_snake_case()
                    classcname = 'Linphone' +  arg.type.containedTypeDesc.desc.name.to_camel_case()
                    classname =  arg.type.containedTypeDesc.desc.name.to_camel_case()
                    isConst = 'TRUE' if arg.type.containedTypeDesc.isconst else 'FALSE'
                    methodDict['jlists'].append({'list_name': argname, 'objectCPrefix':cprefix, 'objectClassCName':classcname, 'objectClassName':classname, 'isRealObjectArray':True, 'isStringObjectArray':False, 'isObjectConst':isConst})
                methodDict['needLjb'] = True

        methodDict['jparams'] += ')'
        if (methodDict['return'] == 'void'):
            methodDict['jparams'] += 'V'
        else:
            if isinstance(_method.returnType, AbsApi.ClassType):
                methodDict['jparams'] += 'L' + self.jni_path + _method.returnType.desc.name.to_camel_case() + ';'
            elif isinstance(_method.returnType, AbsApi.BaseType):
                methodDict['jparams'] += self.translate_java_jni_base_type_name(_method.returnType.name)
            else:
                pass #TODO

        return methodDict

    def translate_interface(self, _class):
        interfaceDict = {
            'methods': [],
            'jniMethods': [],
        }

        interfaceDict['briefDoc'] = _class.briefDescription.translate(self.docTranslator, tagAsBrief=True)
        interfaceDict['detailedDoc'] = _class.detailedDescription.translate(self.docTranslator)

        for method in _class.instanceMethods:
            interfaceDict['methods'].append(self.translate_method(method))
            interfaceDict['jniMethods'].append(self.translate_jni_interface(_class.listenedClass, _class.name, method))

        return interfaceDict

    def translate_enum(self, enum):
        enumDict = {
            'jniMethods': [],
        }

        enumDict['name'] = enum.name.to_camel_case()
        enumDict['briefDoc'] = enum.briefDescription.translate(self.docTranslator, tagAsBrief=True)
        enumDict['detailedDoc'] = enum.detailedDescription.translate(self.docTranslator)
        enumDict['values'] = []
        i = 0
        lastValue = None

        enumDict['jniPath'] = self.jni_path

        for enumerator in enum.enumerators:
            enumValDict = {}
            enumValDict['name'] = enumerator.name.to_camel_case()
            enumValDict['briefDoc'] = enumerator.briefDescription.translate(self.docTranslator, tagAsBrief=True)
            enumValDict['detailedDoc'] = enumerator.detailedDescription.translate(self.docTranslator)
            if isinstance(enumerator.value, int):
                lastValue = enumerator.value
                enumValDict['value'] = str(enumerator.value)
            elif isinstance(enumerator.value, AbsApi.Flag):
                enumValDict['value'] = '1<<' + str(enumerator.value.position)
            else:
                if lastValue is not None:
                    enumValDict['value'] = lastValue + 1
                    lastValue += 1
                else:
                    enumValDict['value'] = i
            i += 1
            enumValDict['commarorsemicolon'] = ';' if i == len(enum.enumerators) else ','
            enumDict['values'].append(enumValDict)

        return enumDict

##########################################################################

class JavaEnum:
    def __init__(self, package, _enum, translator, platform):
        javaNameTranslator = metaname.Translator.get('Java')
        self._class = translator.translate_enum(_enum)
        self.packageName = package
        self.className = _enum.name.translate(javaNameTranslator)
        self.cPrefix = _enum.name.to_snake_case(fullName=True)
        self.filename = self.className + ".java"
        self.values = self._class['values']
        self.briefDoc = self._class['briefDoc']
        self.detailedDoc = self._class['detailedDoc']
        self.jniName = _enum.name.translate(JNINameTranslator.get())
        self.isAndroid = (platform == "android") or (platform == "bundled-android")
        self.isBundledAndroid = (platform == "bundled-android")

class JniInterface:
    def __init__(self, javaClass, apiClass):
        self.isMultiListener = (apiClass.multilistener)
        self.className = javaClass.className
        self.classCName = javaClass.cName
        self.cPrefix = javaClass.cPrefix
        self.callbacks = []
        listener = apiClass.listenerInterface
        for method in listener.instanceMethods:
            self.callbacks.append({
                'callbackName': '_{0}_cb'.format(method.name.to_snake_case(fullName=True)),
                'callback': method.name.to_snake_case()[3:], # Remove the on_
            })

class JavaInterface:
    def __init__(self, package, _interface, translator, platform):
        javaNameTranslator = metaname.Translator.get('Java')
        self._class = translator.translate_interface(_interface)
        self.packageName = package
        self.className = _interface.name.translate(javaNameTranslator)
        self.filename = self.className + ".java"
        self.cPrefix = 'linphone_' + _interface.name.to_snake_case()
        self.imports = []
        self.methods = self._class['methods']
        self.briefDoc = self._class['briefDoc']
        self.detailedDoc = self._class['detailedDoc']
        self.jniMethods = self._class['jniMethods']
        self.isAndroid = (platform == "android") or (platform == "bundled-android")
        self.isBundledAndroid = (platform == "bundled-android")

class JavaInterfaceStub:
    def __init__(self, _interface):
        self.packageName = _interface.packageName
        self.className = _interface.className
        self.classNameStub =  self.className + "Stub"
        self.filename = self.className + "Stub.java"
        self.methods = _interface.methods
        self.isAndroid = _interface.isAndroid

class JavaClass:
    def __init__(self, package, _class, translator, platform):
        self._class = translator.translate_class(_class)
        self.isAndroid = (platform == "android") or (platform == "bundled-android")
        self.isBundledAndroid = (platform == "bundled-android")
        self.isLinphoneFactory = self._class['isLinphoneFactory']
        self.isLinphoneCore = self._class['isLinphoneCore']
        self.isNotLinphoneFactory = not self.isLinphoneFactory
        self.cName = _class.name.to_c()
        self.hasCoreAccessor = self._class['hasCoreAccessor']
        self.cPrefix = _class.name.to_snake_case(fullName=True)
        self.packageName = package
        self.className = _class.name.to_camel_case()
        self.classImplName = self.className + "Impl"
        self.refCountable = self._class['refCountable']
        self.factoryName = _class.name.to_snake_case()
        self.filename = self.className + ".java"
        self.imports = []
        self.methods = self._class['methods']
        self.jniMethods = self._class['jniMethods']
        self.briefDoc = self._class['briefDoc']
        self.detailedDoc = self._class['detailedDoc']
        self.toStringNotFound = not self._class['toStringFound']
        self.enums = []
        for enum in _class.enums:
            self.enums.append(JavaEnum(package, enum, translator, platform))
        self.jniInterface = None
        if _class.listenerInterface is not None:
            self.jniInterface = JniInterface(self, _class)

    def add_enum(self, enum):
        if enum.className.startswith(self.className):
            enum.className = enum.className[len(self.className):]
        self.enums.append(enum)

class Jni:
    def __init__(self, package):
        self.enums = []
        self.interfaces = []
        self.callbacks = []
        self.objects = []
        self.methods = []
        self.jni_package = ''
        self.jni_path = ''
        self.coreListener = []
        package_dirs = package.split('.')
        for directory in package_dirs:
            self.jni_package += directory + '_'
            self.jni_path += directory + '/'

    def add_enum(self, javaEnum):
        obj = {
            'jniPrefix': self.jni_package,
            'jniPath': self.jni_path,
            'jniName': javaEnum.jniName,
            'cPrefix': javaEnum.cPrefix,
            'className': javaEnum.className,
        }
        self.enums.append(obj)

    def add_object(self, javaClass):
        if javaClass.className == 'Factory':
            return
        obj = {
            'jniPrefix': self.jni_package,
            'jniPath': self.jni_path,
            'cPrefix': javaClass.cPrefix,
            'className': javaClass.className,
            'classCName': javaClass.cName,
            'classImplName': javaClass.classImplName,
            'refCountable': javaClass.refCountable,
            'notRefCountable': not javaClass.refCountable
        }
        self.objects.append(obj)
        
        jniInterface = javaClass.jniInterface
        if jniInterface is not None:
            interface = {
                'isMultiListener': jniInterface.isMultiListener,
                'classCName': jniInterface.classCName,
                'className': jniInterface.className,
                'cPrefix': jniInterface.cPrefix,
                'jniPackage': self.jni_package,
                'factoryName': javaClass.factoryName,
                'isCore': jniInterface.cPrefix == 'linphone_core',
                'isNotCore': not jniInterface.cPrefix == 'linphone_core',
                'callbacksList': []
            }
            for callback in jniInterface.callbacks:
                interface['callbacksList'].append(callback)
                if obj['className'] == 'Core':
                    self.coreListener.append(callback)
            self.interfaces.append(interface)

    def add_callbacks(self, name, callbacks):
        for callback in callbacks:
            self.callbacks.append(callback)

    def add_methods(self, name, methods):
        for method in methods:
            self.methods.append(method)

class Proguard:
    def __init__(self, package):
        self.package = package
        self.classes = []
        self.enums = []
        self.listeners = []

    def add_class(self, javaClass):
        obj = {
            'package': self.package,
            'className': javaClass.className,
            'classImplName': javaClass.classImplName,
        }
        self.classes.append(obj)

        for javaEnum in javaClass.enums:
            enumObj = {
                'package': self.package,
                'className': javaClass.className + "$" + javaEnum.className,
            }
            self.enums.append(enumObj)

    def add_enum(self, javaEnum):
        obj = {
            'package': self.package,
            'className': javaEnum.className,
        }
        self.enums.append(obj)

    def add_interface(self, javaInterface):
        obj = {
            'package': self.package,
            'className': javaInterface.className,
        }
        self.listeners.append(obj)

class Overview:
    def __init__(self, directory, version):
        self.directory = directory
        self.version = version

##########################################################################

class GenWrapper:
    def __init__(self, srcdir, javadir, javadocdir, package, xmldir, exceptions, upload_dir, version, platform):
        self.srcdir = srcdir
        self.javadir = javadir
        self.javadocdir = javadocdir
        self.package = package
        self.exceptions = exceptions
        self.platform = platform

        project = CApi.Project()
        project.initFromDir(xmldir)
        project.check()

        self.parser = AbsApi.CParser(project)
        self.parser.functionBl = [
            'linphone_factory_create_core_with_config',
            'linphone_factory_create_core',
            'linphone_factory_create_core_2',
            'linphone_factory_create_core_with_config_2',
            'linphone_vcard_get_belcard',
            'linphone_core_get_current_vtable',
            'linphone_factory_get',
            'linphone_factory_clean',
            'linphone_call_zoom_video',
            'linphone_core_get_zrtp_cache_db',
            'linphone_config_get_range',
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
            'linphone_core_get_new_chat_room_from_conf_addr',
            'linphone_call_params_is_capability_negotiation_reinvite_enabled',
            'linphone_conference_params_set_audio_enabled',
            'linphone_conference_params_is_audio_enabled',
            'linphone_conference_params_set_video_enabled',
            'linphone_conference_params_is_video_enabled',
            'linphone_conference_params_set_chat_enabled',
            'linphone_conference_params_is_chat_enabled',
            'linphone_conference_params_set_local_participant_enabled',
            'linphone_conference_params_is_local_participant_enabled',
            'linphone_conference_params_set_one_participant_conference_enabled',
            'linphone_conference_params_is_one_participant_conference_enabled',
            'linphone_core_set_native_ringing_enabled',
            'linphone_core_is_native_ringing_enabled',
            'linphone_core_set_session_expires_enabled',
            'linphone_core_get_session_expires_enabled',
            'linphone_core_is_capability_negotiation_reinvite_enabled',
            'linphone_core_is_friend_list_subscription_enabled',
            'linphone_core_set_auto_download_voice_recordings_enabled',
            'linphone_core_is_auto_download_voice_recordings_enabled',
            'linphone_core_set_auto_download_icalendars_enabled',
            'linphone_core_is_auto_download_icalendars_enabled',
            'linphone_core_set_record_aware_enabled',
            'linphone_core_is_record_aware_enabled',
            'linphone_core_set_push_notification_enabled',
            'linphone_core_is_push_notification_enabled',
            'linphone_core_set_auto_iterate_enabled',
            'linphone_core_is_auto_iterate_enabled',
            'linphone_core_set_vibration_on_incoming_call_enabled',
            'linphone_core_is_vibration_on_incoming_call_enabled',
            'linphone_account_params_set_register_enabled',
            'linphone_account_params_set_publish_enabled',
            'linphone_account_params_set_dial_escape_plus_enabled',
            'linphone_account_params_set_quality_reporting_enabled',
            'linphone_account_params_get_quality_reporting_enabled',
            'linphone_account_params_get_publish_enabled',
            'linphone_account_params_get_register_enabled',
            'linphone_account_params_get_dial_escape_plus_enabled',
            'linphone_account_params_set_outbound_proxy_enabled',
            'linphone_account_params_get_outbound_proxy_enabled',
            'linphone_account_is_avpf_enabled',
        ]
        self.parser.parse_all()
        self.translator = JavaTranslator(package, exceptions, platform)
        self.renderer = pystache.Renderer()
        self.jni = Jni(package)
        self.proguard = Proguard(package)
        self.overview = Overview(upload_dir, version)

        self.enums = {}
        self.interfaces = {}
        self.classes = {}

    def render_all(self):
        for _interface in self.parser.namespace.interfaces:
            self.render_java_interface(_interface)
        for _class in self.parser.namespace.classes:
            self.render_java_class(_class)
        for enum in self.parser.namespace.enums:
            self.render_java_enum(enum)

        for name, value in self.enums.items():
            self.render(value, self.javadir + '/' + value.filename)
            self.proguard.add_enum(value)
        for name, value in self.interfaces.items():
            self.render(value, self.javadir + '/' + value.filename)
            self.proguard.add_interface(value)
        for name, value in self.classes.items():
            self.render(value, self.javadir + '/' + value.filename)
            self.jni.add_object(value)
            self.proguard.add_class(value)

        self.render(self.jni, self.srcdir + '/linphone_jni.cc')
        self.render(self.proguard, self.srcdir + '/proguard.txt')
        self.render(self.overview, self.javadocdir + '/overview.html')

    def render(self, item, path):
        tmppath = path + '.tmp'
        content = ''
        with open(tmppath, mode='w') as f:
            f.write(self.renderer.render(item))
        with open(tmppath, mode='r') as f:
            content = f.read()
        with open(path, mode='w') as f:
            f.write(content)
        os.unlink(tmppath)

    def render_java_enum(self, _class):
        if _class is not None:
            try:
                javaenum = JavaEnum(self.package, _class, self.translator, self.platform)
                self.enums[javaenum.className] = javaenum
                self.jni.add_enum(javaenum)
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(_class.name.to_camel_case(fullName=True), e.args[0]))

    def render_java_interface(self, _class):
        if _class is not None:
            try:
                javainterface = JavaInterface(self.package, _class, self.translator, self.platform)
                self.interfaces[javainterface.className] = javainterface
                javaInterfaceStub = JavaInterfaceStub(javainterface)
                self.interfaces[javaInterfaceStub.classNameStub] = javaInterfaceStub
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(_class.name.to_camel_case(fullName=True), e.args[0]))
            self.jni.add_callbacks(javainterface.className, javainterface.jniMethods)

    def render_java_class(self, _class):
        if _class is not None:
            try:
                javaclass = JavaClass(self.package, _class, self.translator, self.platform)
                self.classes[javaclass.className] = javaclass
                for enum in javaclass.enums:
                    self.jni.add_enum(enum)
            except AbsApi.Error as e:
                logging.error('Could not translate {0}: {1}'.format(_class.name.to_camel_case(fullName=True), e.args[0]))
            self.jni.add_methods(javaclass.className, javaclass.jniMethods)

##########################################################################

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='Generate source files for the Java wrapper')
    argparser.add_argument('xmldir', type=str, help='Directory where the XML documentation of the Linphone\'s API generated by Doxygen is placed')
    argparser.add_argument('-o --output', type=str, help='the directory where to generate the source files', dest='outputdir', default='.')
    argparser.add_argument('-p --package', type=str, help='the package name for the wrapper', dest='package', default='org.linphone.core')
    argparser.add_argument('-n --name', type=str, help='the name of the genarated source file', dest='name', default='linphone_jni.cc')
    argparser.add_argument('-v --version', type=str, help='the version of the SDK', dest='version', default='4.5.0')
    argparser.add_argument('-d --directory', type=str, help='the directory where doc will be upload', dest='directory', default='snapshots')
    argparser.add_argument('-e --exceptions', type=bool, help='enable the wrapping of LinphoneStatus into CoreException', dest='exceptions', default=False)
    argparser.add_argument('-P --platform', type=str, help='the platform targeted by the wrapper, either "desktop" or "android"', dest='platform', default='android')
    argparser.add_argument('-V --verbose', action='store_true', dest='verbose_mode', default=False, help='Verbose mode.')
    args = argparser.parse_args()

    loglevel = logging.INFO if args.verbose_mode else logging.ERROR
    logging.basicConfig(format='%(levelname)s[%(name)s]: %(message)s', level=loglevel)

    srcdir = args.outputdir + '/src'
    javadir = args.outputdir + '/java'
    javadocdir = javadir + '/src/main/javadoc/'
    package_dirs = args.package.split('.')
    for directory in package_dirs:
        javadir += '/' + directory

    try:
        os.makedirs(srcdir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            logging.critical("Cannot create '{0}' dircetory: {1}".format(srcdir, e.strerror))
            sys.exit(1)

    try:
        os.makedirs(javadir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            logging.critical("Cannot create '{0}' dircetory: {1}".format(javadir, e.strerror))
            sys.exit(1)

    try:
        os.makedirs(javadocdir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            logging.critical("Cannot create '{0}' dircetory: {1}".format(
                javadocdir, e.strerror))
            sys.exit(1)

    genwrapper = GenWrapper(srcdir, javadir, javadocdir, args.package,
                            args.xmldir, args.exceptions, args.directory, args.version, args.platform)
    genwrapper.render_all()
