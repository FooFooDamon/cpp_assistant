CC = gcc
CXX = g++
AR = ar
AR_FLAGS = -r
RANLIB = ranlib
# ******** Begin: Should these variables use ":=" or "="? ********
# FRAMEWORK_CHANGE_LOG -> FRAMEWORK_DIR
#FRAMEWORK_DIR = $(shell dirname `ls $(FRAMEWORK_CHANGE_LOG) -l | awk -F "->" '{ print $$2 }'`)
# or FRAMEWORK -> FRAMEWORK_DIR -> FRAMEWORK_CHANGE_LOG
# CPP_ASSISTANT_ROOT is specified by the parent makefile or through command line.
FRAMEWORK_DIR = $(CPP_ASSISTANT_ROOT)/code/frameworks/framework_templates/$(FRAMEWORK)
FRAMEWORK_CHANGE_LOG = $(FRAMEWORK_DIR)/framework_base_info.h
FRAMEWORK_CHANGE_LOG_ITEM = $(shell grep "[0-9][0-9]/[0-9][0-9], [0-9]." $(FRAMEWORK_CHANGE_LOG) | head -1)
CASDK_NEWEST_MOD_DATE = $(shell echo "$(FRAMEWORK_CHANGE_LOG_ITEM)" | awk -F ", " '{ print $$2 }' | sed 's/\///g')
CASDK_NEWEST_MAIN_VER = $(shell echo "$(FRAMEWORK_CHANGE_LOG_ITEM)" | awk -F ", " '{ print $$3 }' | awk -F ":" '{ print $$1 }')
CASDK_SVN_VER = $(shell svn info $(FRAMEWORK_DIR) | grep -e 'Last Changed Rev' -e '最后修改的版本' | sed 's/.* \([0-9]\)/\1/')
# ******** End *************************************************
REQUIRED_DEFINES = -DCASDK_NEWEST_MOD_DATE=\"$(CASDK_NEWEST_MOD_DATE)\" \
    -DCASDK_NEWEST_MAIN_VER=\"$(CASDK_NEWEST_MAIN_VER)\"
ifneq ($(CASDK_SVN_VER),)
	REQUIRED_DEFINES += -DCASDK_SVN_VER=\"$(CASDK_SVN_VER)\"
endif
MODULE_SVN_VER = $(shell LANG=en_US.UTF-8;svn info|grep 'Last Changed Rev'|sed 's/.* \([0-9]\)/\1/')
ifneq ($(MODULE_SVN_VER),)
	REQUIRED_DEFINES += -DMODULE_VERSION=\"r$(MODULE_SVN_VER)\"
endif
ifeq ("$(findstring HAS_DATABASE, $(MODULE_DEFINES))","HAS_DATABASE")
ifeq ("$(findstring DB_ORACLE, $(MODULE_DEFINES))","DB_ORACLE")
	REQUIRED_DEFINES += -DOTL_ORA11G -DOTL_ORA_UTF8 -DOTL_DEFAULT_NUMERIC_NULL_TO_VAL=0
endif
endif
REQUIRED_INCLUDES = -I. -I.. -I$(HOME)/include
ifneq ($(CPP_ASSISTANT_ROOT),)
	REQUIRED_INCLUDES += -I$(CPP_ASSISTANT_ROOT)/code/libcpp_assistant/include -I$(CPP_ASSISTANT_ROOT)/code/libcpp_assistant/include/native
endif
ifeq ("$(findstring HAS_DATABASE, $(MODULE_DEFINES))","HAS_DATABASE")
ifeq ("$(findstring DB_ORACLE, $(MODULE_DEFINES))","DB_ORACLE")
	REQUIRED_INCLUDES += -I$(ORACLE_HOME)/rdbms/public
else
	REQUIRED_INCLUDES += -I$(MYSQL_HOME)/include
endif
endif
REQUIRED_LDFLAGS = -L$(HOME)/lib -lcpp_assistant -ljsoncpp \
	-L/usr/local/lib -lprotobuf -lpthread
ifeq ("$(findstring HAS_DATABASE, $(MODULE_DEFINES))","HAS_DATABASE")
ifeq ("$(findstring DB_ORACLE, $(MODULE_DEFINES))","DB_ORACLE")
    REQUIRED_LDFLAGS += -L$(ORACLE_HOME)/rdbms/lib -locci -L$(ORACLE_HOME)/lib -lclntsh
else
	REQUIRED_LDFLAGS += -lmyitf -lrt -ldl
endif
endif

ifeq ("$(CXX)","g++")
    CFLAGS += -g -Wall -fstack-protector -fstack-protector-all
    CXXFLAGS += -g -Wall -fstack-protector -fstack-protector-all
endif
ifdef GPROF
    CFLAGS += -pg
    CXXFLAGS += -pg
endif
# put this in tcp_framework*
#ifdef VALIDATES_CONNECTION
#    CFLAGS += -DVALIDATES_CONNECTION
#    CXXFLAGS += -DVALIDATES_CONNECTION
#endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^
