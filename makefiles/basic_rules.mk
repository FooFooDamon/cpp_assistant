CC = gcc
CXX = g++
AR = ar
AR_FLAGS = -r
RANLIB = ranlib

# CPP_ASSISTANT_ROOT is specified by the parent makefile or through command line.
FRAMEWORK_DIR = $(CPP_ASSISTANT_ROOT)/code/frameworks/framework_templates/$(FRAMEWORK)
FRAMEWORK_CHANGE_LOG = $(FRAMEWORK_DIR)/framework_base_info.h
FRAMEWORK_CHANGE_LOG_ITEM = $(shell grep "[0-9][0-9]/[0-9][0-9], [0-9]." $(FRAMEWORK_CHANGE_LOG) | head -1)

CASDK_NEWEST_MOD_DATE = $(shell echo "$(FRAMEWORK_CHANGE_LOG_ITEM)" | awk -F ", " '{ print $$2 }' | sed 's/\///g')
CASDK_NEWEST_MAIN_VER = $(shell echo "$(FRAMEWORK_CHANGE_LOG_ITEM)" | awk -F ", " '{ print $$3 }' | awk -F ":" '{ print $$1 }')
CASDK_SVN_VER = $(shell LANG=en_US.UTF-8; svn info $(FRAMEWORK_DIR) | grep -e 'Last Changed Rev' -e '最后修改的版本' | sed 's/.* \([0-9]\)/\1/')
ifeq ($(CASDK_SVN_VER),)
	CASDK_SVN_VER := $(shell cd $(FRAMEWORK_DIR) && git log --abbrev-commit --pretty=oneline | head -1 | awk '{ print $$1 }')
endif

REQUIRED_DEFINES = -DCASDK_NEWEST_MOD_DATE=\"$(CASDK_NEWEST_MOD_DATE)\" \
    -DCASDK_NEWEST_MAIN_VER=\"$(CASDK_NEWEST_MAIN_VER)\"
ifneq ($(CASDK_SVN_VER),)
	REQUIRED_DEFINES += -DCASDK_SVN_VER=\"$(CASDK_SVN_VER)\"
endif

MODULE_SVN_VER = $(shell LANG=en_US.UTF-8; svn info | grep 'Last Changed Rev' | sed 's/.* \([0-9]\)/\1/')
ifeq ($(MODULE_SVN_VER),)
	MODULE_SVN_VER := $(shell git log --abbrev-commit --pretty=oneline | head -1 | awk '{ print $$1 }')
endif

ifneq ($(MODULE_SVN_VER),)
	REQUIRED_DEFINES += -DMODULE_VERSION=\"$(MODULE_SVN_VER)\"
endif

REQUIRED_INCLUDES = -I. -I.. -I$(HOME)/include
ifneq ($(CPP_ASSISTANT_ROOT),)
	REQUIRED_INCLUDES += -I$(CPP_ASSISTANT_ROOT)/code/libcpp_assistant/include
endif

REQUIRED_LDFLAGS = -L$(HOME)/lib -lcpp_assistant -ljsoncpp \
	-L/usr/local/lib -lprotobuf -lpthread

ifeq ("$(CC)","gcc")
    CFLAGS += -g -Wall -fstack-protector -fstack-protector-all
endif
ifeq ("$(CXX)","g++")
    CXXFLAGS += -g -Wall -fstack-protector -fstack-protector-all
endif
ifdef GPROF
    CFLAGS += -pg
    CXXFLAGS += -pg
endif
ifeq ($(USE_PURE_FILE_NAME),1)
    CFLAGS += -U__FILE__ -D__FILE__='"$(subst $(dir $^),,$^)"' -Wno-builtin-macro-redefined
    CXXFLAGS += -U__FILE__ -D__FILE__='"$(subst $(dir $^),,$^)"' -Wno-builtin-macro-redefined
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^
