from setuptools import setup, Distribution
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

linphone_extension = Extension(
    name="linphone",
    sources=["pylinphone.pyx"],
    libraries=["@LINPHONE_LIBS_FOR_TOOLS@", "@BELLESIP_LIBRARIES@", "@BCTOOLBOX_CORE_LIBRARIES@"],
    library_dirs=["@PROJECT_BINARY_DIR@/src", "@CMAKE_INSTALL_PREFIX@/@CMAKE_INSTALL_LIBDIR@"],
    include_dirs=["@CMAKE_CURRENT_BINARY_DIR@/include", "@PROJECT_BINARY_DIR@/include", "@PROJECT_SOURCE_DIR@/include", "@CMAKE_CURRENT_SOURCE_DIR@", "@BCTOOLBOX_INCLUDE_DIRS@", "@BELLESIP_INCLUDE_DIRS@"],
    extra_link_args=["-Wl,-rpath,$ORIGIN/linphone-sdk/"],
)

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True

    def is_pure(self):
        return False

setup(
    ext_modules = cythonize([linphone_extension], compiler_directives={'language_level' : "3"}),
    name="pylinphone",
    version="@WHEEL_LINPHONESDK_VERSION@",
    author="Belledonne Communications",
    author_email="info@belledonne-communications.com",
    description="A python wrapper for linphone library",
    long_description="A python wrapper for linphone library automatically generated using Cython",
    long_description_content_type="plain/text",
    url="https://linphone.org",
    license="GPLv3",
    packages=['linphone-sdk'],
    package_data={'linphone-sdk': ['*.so*']},
    distclass=BinaryDistribution,
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',
        'Topic :: Communications :: Chat',
        'Topic :: Communications :: Conferencing',
        'Topic :: Communications :: Internet Phone',
        'Topic :: Communications :: Telephony',        
    ],
)
