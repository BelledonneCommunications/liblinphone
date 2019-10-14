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
#export EXTRA_ARGS="--workdir $bundle_res"
export LINPHONE_WORKDIR="$bundle_res"
export GIO_EXTRA_MODULES="$bundle_lib/gio/modules"
export PANGO_LIBDIR="$bundle_lib"
export PANGO_SYSCONFDIR="$bundle_etc"
export GDK_PIXBUF_MODULE_FILE="$bundle_lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"

#this is very important not to force a shared library path so that native frameworks can find their dependencies by themselves, 
#and not be forced to use the few libraries we have in the bundle that have the same name as native libs (ex: libiconv)
export DYLD_LIBRARY_PATH=

#the fucking script of the gtk-mac-bundler resets LANG due to obscure bugs. Set it back.
LANG=`defaults read .GlobalPreferences AppleLocale`

case "$LANG" in
	*.UTF-8)
		;;
	*)
		if test -d /usr/share/locale/${LANG}.UTF-8 ; then
			LANG=${LANG}.UTF-8
		fi
		;;
esac

export LANG

echo "LANG is $LANG"
