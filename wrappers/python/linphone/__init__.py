import os
from ctypes import CDLL
lib_path = os.path.join(os.path.dirname(__file__), 'liblinphone.so')
liblinphone_library = CDLL(lib_path)
from pylinphone import *