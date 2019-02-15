DEFINES += -D_REENTRANT -DCA_NEWEST_MOD_DATE=\"$(CA_MOD_DATE)\" -DCA_NEWEST_MAIN_VER=\"$(CA_MAIN_VER)\"

ifneq ($(CA_SVN_VER),)
    DEFINES += -DCA_SVN_VER=\"$(CA_SVN_VER)\"
endif

ifeq ($(DEBUG), 1)
    DEBUG_FLAG = "DEBUG=1"
    DEFINES += -DDEBUG
    OPTIMIZE_FLAG = -O0 -g -ggdb
else
    OPTIMIZE_FLAG = -O3
endif

ifneq ($(CA_LIB_NAMESPACE),)
    DEFINES += -DCA_LIB_NAMESPACE=$(CA_LIB_NAMESPACE) -DCA_LIB_NAMESPACE_STR=\"$(CA_LIB_NAMESPACE)\"
endif

CFLAGS += $(DEFINES) $(INCLUDES) $(OPTIMIZE_FLAG) -fPIC -Wno-misleading-indentation -Wno-nonnull-compare
CXXFLAGS = $(CFLAGS) -ftemplate-depth-128
LDFLAGS +=
