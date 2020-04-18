[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/BelledonneCommunications/liblinphone) 

[![pipeline status](https://gitlab.linphone.org/BC/public/linphone/badges/master/pipeline.svg)](https://gitlab.linphone.org/BC/public/linphone/commits/master)

Liblinphone
===========

Liblinphone is a high-level SIP library integrating all calling and instant messaging features into an unified easy-to-use API.
It is the cross-platform VoIP library on which the *Linphone[1]* application is based on, and that anyone can use to add audio and video calls
or instant messaging capabilities to an application.

# License

Copyright © Belledonne Communications

Liblinphone is dual licensed, and is available either :

 - under a [GNU/GPLv3 license](https://www.gnu.org/licenses/gpl-3.0.en.html), for free (open source). Please make sure that you understand and agree with the terms of this license before using it (see LICENSE.txt file for details).

 - under a proprietary license, for a fee, to be used in closed source applications. Contact [Belledonne Communications](https://www.linphone.org/contact) for any question about costs and services.

# Documentation

-   Supported features and RFCs : https://www.linphone.org/technical-corner/liblinphone/features
-   Multi-language API documentation : https://www.linphone.org/snapshots/docs/liblinphone/multilang/
-   Liblinphone developer guide on Linphone public wiki: https://wiki.linphone.org/xwiki/wiki/public/view/Lib/


# Building liblinphone

The *linphone-sdk[7]* git project is the **recommended** way to build liblinphone, as it bundles all required an optional dependencies to build liblinphone
as git submodules. It has a top-level CMake build script that makes life easier.

Here the main dependencies listed:

* **BcToolbox[2]:** portability layer
* **BelleSIP[3]:** SIP stack
* **Mediastreamer2[4]:** multimedia engine
* **Belcard[5]:** VCard4 support
* **Belr** generic parsing engine for ABNF defined languages.
* **libxml2**
* **zlib**
* **libsqlite3:** user data storage (can be disabled)
* **python interpreter** and **pystache**, **six** python module (needed for C++/C#/Java wrappers and API documentation)
* **doxygen** and **dot** (needed for C++ wrapper and API documentation)
* **Bzrtp[6]:** zrtp stack used to secure calls
* For API documentation generation: **sphinx**, **javasphinx**, **sphinx_csharp** python modules are needed.


## Build instructions (when used standalone, outside of linphone-sdk)

	cmake . -DCMAKE_INSTALL_PREFIX=<prefix> -DCMAKE_PREFIX_PATH=<search_prefixes>

	make
	make install


## Supported build options

* **`CMAKE_INSTALL_PREFIX=<string>`** : install prefix
* **`CMAKE_PREFIX_PATH=<string>`**    : column-separated list of prefixes where to search for dependencies
* **`ENABLE_SHARED=NO`**              : do not build the shared library
* **`ENABLE_STATIC=NO`**              : do not build the static library
* **`ENABLE_STRICT=NO`**              : build without strict compilation flags (-Wall -Werror)
* **`ENABLE_DOC=YES`**                : Make the reference documentation of liblinphone to generated
* **`ENABLE_UNIT_TESTS=NO`**          : do not build testing binaries
* **`ENABLE_VCARD=NO`**               : disable VCard4 support
* **`ENABLE_TOOLS=NO`**               : do not build tool binaries
* **`ENABLE_LIME=NO`**                : disable Linphone Instant Messaging Encryption

## Note for packagers

Our CMake scripts may automatically add some paths into research paths of generated binaries.
To ensure that the installed binaries are striped of any rpath, use `-DCMAKE_SKIP_INSTALL_RPATH=ON`
while you invoke cmake.

Rpm packaging
liblinphone can be generated with cmake3 using the following commands:
```
mkdir WORK
cd WORK
cmake3 ../
make package_source
rpmbuild -ta --clean --rmsource --rmspec liblinphone-<version>-<release>.tar.gz
```

# Credits

Belledonne Communications SARL, all rights reserved.

# License

This software is distributed under GNU GPLv3. Please read COPYING file for full license text.


------------------------------


- [1] Linphone: https://linphone.org/technical-corner/linphone
- [2] bctoolbox: https://gitlab.linphone.org/BC/public/bctoolbox *or* <https://www.linphone.org/releases/sources/bctoolbox>
- [3] belle-sip: https://gitlab.linphone.org/BC/public/belle-sip *or* <https://www.linphone.org/releases/sources/belle-sip>
- [4] mediastreamer2: https://gitlab.linphone.org/BC/public/mediastreamer2 *or* <https://www.linphone.org/releases/sources/mediastreamer>
- [5] belcard: https://gitlab.linphone.org/BC/public/belcard *or* <https://www.linphone.org/releases/sources/belcard>
- [6] bzrtp: https://gitlab.linphone.org/BC/public/bzrtp *or* <https://www.linphone.org/releases/sources/bzrtp>
- [7] linphone-sdk https://gitlab.linphone.org/BC/public/linphone-sdk
