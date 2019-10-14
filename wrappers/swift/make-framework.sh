#!/bin/sh
##
## Copyright (c) 2010-2019 Belledonne Communications SARL.
##
## This file is part of Liblinphone.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program. If not, see <http://www.gnu.org/licenses/>.
##


usage() {
  echo "Usage: $0 [framework_name] -b <build_path> [-v version] [-o <output directory>] -- Create a framework from a shared library and a directory of header files."
}

exit_abnormal() {
  usage
  exit 1
}

if [ "$1" = "" ]; then
	echo "Error: no framework name given."
	exit_abnormal
fi

framework_name=$1
build_path=
output_directory=.
framework_id=org.linphone.${framework_name}
framework_version=

shift

while getopts ":b:o:v:" options; do              
  case "${options}" in    
  	v)
  	  framework_version=${OPTARG}
  	  ;;
    b)                                         
      build_path=${OPTARG}
      ;;
    o)
      output_directory=${OPTARG}
      ;;
    :)
      echo "Error: -${OPTARG} requires an argument."
      exit_abnormal
      ;;
    *)
      echo "Error: invalid option ${OPTARG}." 
      exit_abnormal
      ;;
  esac
done



if [ "$build_path" = "" ]; then
	echo "Error: no build path given."
	exit_abnormal
fi

dylib_path=$build_path/lib${framework_name}.dylib
framework_dir=${output_directory}/${framework_name}.framework
swiftmodule_dir=${framework_dir}/Modules/${framework_name}.swiftmodule
headers_dir=${framework_dir}/Headers
swiftdoc=$build_path/${framework_name}.swiftdoc
swiftmodule=$build_path/${framework_name}.swiftmodule
header=$build_path/${framework_name}.h
arch=`lipo -info $dylib_path | sed -En -e 's/^(Non-|Architectures in the )fat file: .+( is architecture| are): (.*)$/\3/p'`

framework_dir=${output_directory}/${framework_name}.framework
mkdir -p "$framework_dir"
mkdir -p "${framework_dir}/Modules"
mkdir -p "${swiftmodule_dir}"
mkdir -p "${headers_dir}"
cp "$dylib_path" "${framework_dir}/${framework_name}"
cp "$swiftmodule" "${swiftmodule_dir}/${arch}.swiftmodule"
cp "$swiftdoc" "${swiftmodule_dir}/${arch}.swiftdoc"
cp "$header" "${headers_dir}/."
install_name_tool -id "@rpath/${framework_name}.framework/${framework_name}" ${framework_dir}/${framework_name}


cat >"${framework_dir}/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>CFBundleDevelopmentRegion</key>
        <string>English</string>
        <key>CFBundleExecutable</key>
        <string>${framework_name}</string>
        <key>CFBundleGetInfoString</key>
        <string></string>
        <key>CFBundleIconFile</key>
        <string></string>
        <key>CFBundleIdentifier</key>
        <string>${framework_id}</string>
        <key>LSMinimumSystemVersion</key>
        <string>9.0</string>
        <key>MinimumOSVersion</key>
        <string>9.0</string>
        <key>CFBundleInfoDictionaryVersion</key>
        <string>6.0</string>
        <key>CFBundleLongVersionString</key>
        <string></string>
        <key>CFBundleName</key>
        <string></string>
        <key>CFBundlePackageType</key>
        <string>FMWK</string>
        <key>CFBundleShortVersionString</key>
        <string>${framework_version}</string>
        <key>CFBundleSignature</key>
        <string>????</string>
        <key>CFBundleVersion</key>
        <string>${framework_version}</string>
        <key>CSResourcesFileMapped</key>
        <true/>
        <key>NSHumanReadableCopyright</key>
        <string></string>
        <key>NSPrincipalClass</key>
        <string>NSApplication</string>
        <key>NSHighResolutionCapable</key>
        <string>True</string>
</dict>
</plist>
EOF
