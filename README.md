[![pipeline status](https://gitlab.linphone.org/BC/public/linphone/badges/master/pipeline.svg)](https://gitlab.linphone.org/BC/public/linphone/commits/master)

liblinphone
===========

This is liblinphone, a free (GPL) video voip library based on the SIP protocol.
This library is used by Linphone. It's source code is available at *linphone-desktop[1]*.


# Building liblinphone


## Required dependencies

* **BcToolbox[2]:** portability layer
* **BelleSIP[3]:** SIP stack
* **Mediastreamer2[4]:** multimedia engine
* **Belcard[5]:** VCard support
* **libxml2**
* **zlib**
* **libsqlite3:** user data storage (disablable)
* **gettext** and **libintl**: internationalization support (disablable)
* **python interpreter** and **pystache**, **six** python module (needed for C++ wrapper and API documentaiton)
* **doxygen** and **dot** (needed for C++ wrapper and API documentation)


## Optional dependencies

* **Bzrtp[6]**: zrtp stack used for Linphone Instant Messaging Encryption.
* For API documentatino generation: **sphinx**, **javasphinx**, **sphinx_csharp** python modules are needed.


## Build instructions

	cmake . -DCMAKE_INSTALL_PREFIX=<prefix> -DCMAKE_PREFIX_PATH=<search_prefixes>

	make
	make install


## Supported build opitons

* **`CMAKE_INSTALL_PREFIX=<string>`** : install prefix
* **`CMAKE_PREFIX_PATH=<string>`**    : column-separated list of prefixes where to search for dependencies
* **`ENABLE_SHARED=NO`**              : do not build the shared library
* **`ENABLE_STATIC=NO`**              : do not build the static library
* **`ENABLE_STRICT=NO`**              : build without strict compilation flags (-Wall -Werror)
* **`ENABLE_DOC=YES`**                : Make the reference documentation of liblinphone to generated
* **`ENABLE_UNIT_TESTS=NO`**          : do not build testing binaries
* **`ENABLE_VCARD=NO`**               : disable VCard support
* **`ENABLE_TOOLS=NO`**               : do not build tool binaries
* **`ENABLE_LIME=YES`**               : disable Linphone Instant Messaging Encryption

## Note for packagers

Our CMake scripts may automatically add some paths into research paths of generated binaries.
To ensure that the installed binaries are striped of any rpath, use `-DCMAKE_SKIP_INSTALL_RPATH=ON`
while you invoke cmake.

Rpm packaging
liblinphone can be generated with cmake3 using the following command:
mkdir WORK
cd WORK
cmake3 ../
make package_source
rpmbuild -ta --clean --rmsource --rmspec liblinphone-<version>-<release>.tar.gz


# Credits

Belledonne Communications SARL, all rights reserved.

# License

This software is distributed under GNU GPLv2. Please read COPYING file for full license text.


------------------------------


- [1] linphone-desktop: https://gitlab.linphone.org/BC/public/linphone-desktop
- [2] bctoolbox: https://gitlab.linphone.org/BC/public/bctoolbox *or* <https://www.linphone.org/releases/sources/bctoolbox>
- [3] belle-sip: https://gitlab.linphone.org/BC/public/belle-sip *or* <https://www.linphone.org/releases/sources/belle-sip>
- [4] mediastreamer2: https://gitlab.linphone.org/BC/public/mediastreamer2 *or* <https://www.linphone.org/releases/sources/mediastreamer>
- [5] belcard: https://gitlab.linphone.org/BC/public/belcard *or* <https://www.linphone.org/releases/sources/belcard>
- [6] bzrtp: https://gitlab.linphone.org/BC/public/bzrtp *or* <https://www.linphone.org/releases/sources/bzrtp>
