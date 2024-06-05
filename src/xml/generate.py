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



from distutils.spawn import find_executable
import os
import sys
from subprocess import Popen, PIPE


def find_xsdcxx():
	xsdcxx = find_executable("xsdcxx")
	if xsdcxx is not None:
		return xsdcxx
	xsdcxx = find_executable("xsd")
	return xsdcxx

def generate(name):
	xsdcxx = find_xsdcxx()
	if xsdcxx is None:
		print("Cannot find xsdcxx (or xsd) program in the PATH")
		return -1
	print("Using " + xsdcxx)
	script_dir = os.path.dirname(os.path.realpath(__file__))
	source_file = name + ".xsd"
	print("Generating code from " + source_file)
	prologue_file =  "prologue.txt"
	epilogue_file =  "epilogue.txt"
	p = Popen([xsdcxx,
		"cxx-tree",
		"--generate-wildcard",
		"--generate-serialization",
		"--generate-ostream",
		"--generate-detach",
		"--generate-polymorphic",
		"--polymorphic-type-all",
		"--std", "c++17",
		"--type-naming", "java",
		"--function-naming", "java",
		"--hxx-suffix", ".h",
		"--ixx-suffix", ".h",
		"--cxx-suffix", ".cpp",
		"--location-regex", "%http://.+/(.+)%$1%",
		"--output-dir", ".",
		"--show-sloc",
		"--prologue-file", prologue_file,
		"--cxx-prologue-file", prologue_file,
		"--cxx-epilogue-file", epilogue_file,
		"--epilogue-file", epilogue_file,
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-([^,-]+)-([^,-]+)-?([^,-]*)%\\u$1\\u$2\\u$3\\u$4\\u$5%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-([^,-]+)-?([^,-]*)%\\u$1\\u$2\\u$3\\u$4%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-?([^,-]*)%\\u$1\\u$2\\u$3%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-?([^,-]*)%\\u$1\\u$2%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-([^,-]+)-?([^,-]*),([^,]+)%\\u$1\\u$2\\u$3\\u$4\\l\\u$5%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-?([^,-]*),([^,]+)%\\u$1\\u$2\\u$3\\l\\u$4%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-?([^,-]*),([^,]+)%\\u$1\\u$2\\l\\u$3%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+)%\\u$1\\u$2\\u$3\\u$4\\l\\u$5\\u$6%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+)%\\u$1\\u$2\\u$3\\l\\u$4\\u$5%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-?([^,-]*),([^,]+),([^,]+)%\\u$1\\u$2\\l\\u$3\\u$4%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+),([^,]+)%\\u$1\\u$2\\u$3\\u$4\\l\\u$5\\u$6\\u$7%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+),([^,]+)%\\u$1\\u$2\\u$3\\l\\u$4\\u$5\\u$6%",
		"--type-regex", "%(?:[^ ]* )?([^,-]+)-?([^,-]*),([^,]+),([^,]+),([^,]+)%\\u$1\\u$2\\l\\u$3\\u$4\\u$5%",
		"--accessor-regex", "%([^,-]+)-([^,-]+)-?([^,-]*)%get\\u$1\\u$2\\u$3%",
		"--accessor-regex", "%([^,-]+)-?([^,-]*)%get\\u$1\\u$2%",
		"--accessor-regex", "%([^,-]+)-([^,-]+)-?([^,-]*),([^,]+)%get\\u$1\\u$2\\u$3\\l\\u$4%",
		"--accessor-regex", "%([^,-]+)-?([^,-]*),([^,]+)%get\\u$1\\u$2\\l\\u$3%",
		"--accessor-regex", "%([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+)%get\\u$1\\u$2\\u$3\\l\\u$4\\u$5%",
		"--accessor-regex", "%([^,-]+)-?([^,-]*),([^,]+),([^,]+)%get\\u$1\\u$2\\l\\u$3\\u$4%",
		"--modifier-regex", "%([^,-]+)-([^,-]+)-?([^,-]*)%set\\u$1\\u$2\\u$3%",
		"--modifier-regex", "%([^,-]+)-?([^,-]*)%set\\u$1\\u$2%",
		"--modifier-regex", "%([^,-]+)-([^,-]+)-?([^,-]*),([^,]+)%set\\u$1\\u$2\\u$3\\l\\u$4%",
		"--modifier-regex", "%([^,-]+)-?([^,-]*),([^,]+)%set\\u$1\\u$2\\l\\u$3%",
		"--modifier-regex", "%([^,-]+)-([^,-]+)-?([^,-]*),([^,]+),([^,]+)%set\\u$1\\u$2\\u$3\\l\\u$4\\u$5%",
		"--modifier-regex", "%([^,-]+)-?([^,-]*),([^,]+),([^,]+)%set\\u$1\\u$2\\l\\u$3\\u$4%",
		"--parser-regex", "%([^-]+)-?([^-]*)%parse\\u$1\\u$2%",
		"--parser-regex", "%([^-]+)-?([^-]*)-?([^-]*)%parse\\u$1\\u$2\\u$3%",
		"--parser-regex", "%([^-]+)-?([^-]*)-?([^-]*)-?([^-]*)%parse\\u$1\\u$2\\u$3\\u$4%",
		"--serializer-regex", "%([^-]+)-?([^-]*)%serialize\\u$1\\u$2%",
		"--serializer-regex", "%([^-]+)-?([^-]*)-?([^-]*)%serialize\\u$1\\u$2\\u$3%",
		"--serializer-regex", "%([^-]+)-?([^-]*)-?([^-]*)-?([^-]*)%serialize\\u$1\\u$2\\u$3\\u$4%",
		"--namespace-map", "http://www.w3.org/2001/XMLSchema=LinphonePrivate::Xsd::XmlSchema",
		"--namespace-map", "urn:ietf:params:xml:ns:conference-info=LinphonePrivate::Xsd::ConferenceInfo",
		"--namespace-map", "linphone:xml:ns:conference-info-linphone-extension=LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension",
		"--namespace-map", "linphone:xml:ns:ekt-linphone-extension=LinphonePrivate::Xsd::PublishLinphoneExtension",
		"--namespace-map", "urn:ietf:params:xml:ns:imdn=LinphonePrivate::Xsd::Imdn",
		"--namespace-map", "urn:ietf:params:xml:ns:im-iscomposing=LinphonePrivate::Xsd::IsComposing",
		"--namespace-map", "http://www.linphone.org/xsds/imdn.xsd=LinphonePrivate::Xsd::LinphoneImdn",
		"--namespace-map", "urn:ietf:params:xml:ns:resource-lists=LinphonePrivate::Xsd::ResourceLists",
		"--namespace-map", "urn:ietf:params:xml:ns:rlmi=LinphonePrivate::Xsd::Rlmi",
		"--namespace-map", "urn:ietf:params:xml:ns:patch-ops=LinphonePrivate::Xsd::PatchOps",
		"--namespace-map", "urn:ietf:params:xml:ns:xcon-conference-info=LinphonePrivate::Xsd::XconConferenceInfo",
		"--namespace-map", "urn:ietf:params:xml:ns:xcon-ccmp=LinphonePrivate::Xsd::XconCcmp",
		source_file
		], shell=False)
	p.communicate()
	os.system("sed -e \'1,32d\' "+os.path.splitext(source_file)[0]+".cpp"+">" + os.path.splitext(source_file)[0]+".cpp.tmp")
	os.system("cat linphone-copyright.txt >"+os.path.splitext(source_file)[0]+".cpp")
	os.system("cat "+os.path.splitext(source_file)[0]+".cpp.tmp >>"+os.path.splitext(source_file)[0]+".cpp")
	os.system("rm "+os.path.splitext(source_file)[0]+".cpp.tmp ")
	os.system("sed -e \'1,32d\' "+os.path.splitext(source_file)[0]+".h"+">" + os.path.splitext(source_file)[0]+".h.tmp")
	os.system("cat linphone-copyright.txt >"+os.path.splitext(source_file)[0]+".h")
	os.system("cat "+os.path.splitext(source_file)[0]+".h.tmp >>"+os.path.splitext(source_file)[0]+".h")
	os.system("rm "+os.path.splitext(source_file)[0]+".h.tmp ")
	return 0

def main(argv = None):
	generate("xml")
	generate("conference-info")
	generate("conference-info-linphone-extension")
	generate("ekt-linphone-extension")
	generate("imdn")
	generate("is-composing")
	generate("linphone-imdn")
	generate("resource-lists")
	generate("rlmi")
	generate("patch-ops")
	generate("xcon-conference-info")
	generate("xcon-ccmp")

if __name__ == "__main__":
	sys.exit(main())
