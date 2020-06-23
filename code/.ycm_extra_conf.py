import os, sys
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LIBCA_INC_ROOT = os.path.join(PROJECT_ROOT, "code", "libcpp_assistant", "include")
LIBCA_SRC_ROOT = os.path.join(PROJECT_ROOT, "code", "libcpp_assistant", "src")
sys.path.append(os.path.join(PROJECT_ROOT, "conf"))
from ycm_basic_conf import *
flags.extend([ "-I", LIBCA_INC_ROOT ])
flags.extend([ "-I", os.path.join(LIBCA_INC_ROOT, "cpp_assistant", "native") ])
flags.extend([ "-I", os.path.join(LIBCA_SRC_ROOT, "native") ])
flags.extend([ "-I", os.path.join(LIBCA_INC_ROOT, "cpp_assistant", "3rdparty") ])
flags.extend([ "-I", os.path.join(LIBCA_INC_ROOT, "cpp_assistant", "3rdparty", "tinyxml") ])

