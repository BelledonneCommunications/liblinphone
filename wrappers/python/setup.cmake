from setuptools import setup, Distribution
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

linphone_extension = Extension(
    name="pylinphone",
    sources=["pylinphone.pyx"],
    libraries=["linphone"],
    language="c",
    library_dirs=["@PROJECT_BINARY_DIR@/src"],
    include_dirs=["@PROJECT_SOURCE_DIR@/include", "@BELLESIP_INCLUDE_DIRS@", "@BCTOOLBOX_INCLUDE_DIRS@"]
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
    packages=['linphone'],
    package_data={'linphone': ['*.so']},
    distclass=BinaryDistribution
)
