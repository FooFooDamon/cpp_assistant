import os, sys
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
INC_ROOT = os.path.join(PROJECT_ROOT, "code", "libcpp_assistant", "include")
sys.path.append(os.path.join(PROJECT_ROOT, "conf"))
from ycm_basic_conf import *
flags.extend([ "-I", INC_ROOT ])
flags.extend([ "-I", os.path.join(INC_ROOT, "cpp_assistant", "native") ])
flags.extend([ "-I", os.path.join(INC_ROOT, "cpp_assistant", "3rdparty") ])
flags.extend([ "-I", os.path.join(INC_ROOT, "cpp_assistant", "3rdparty", "tinyxml") ])

