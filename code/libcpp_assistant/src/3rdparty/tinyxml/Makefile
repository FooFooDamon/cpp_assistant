#****************************************************************************
#
# Makefile for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

#****************************************************************************
#
# Modified by wxc on Sep 17, 2017 for personal use
#
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
#DEBUG          := NO
# DEBUG can be set to 1 to include debugging info, or 0 otherwise.
# Value of DEBUG can be fetched through the command line, execute MAKE command like this:
#     make DEBUG=1

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
# wxc, 2017/09/22: Added a dependency relationship with CA_USE_STL to it.
# TODO: wxc, 2017/09/24: It seems there is something wrong with TinyXML when using std::string,
# thus TINYXML_USE_STL keeps using "NO" until the reason is found out.
#ifeq ($(CA_USE_STL),0)
	TINYXML_USE_STL := NO
#else
#	TINYXML_USE_STL := YES
#endif

#****************************************************************************

TIXML_H_DIR := ../../../include/3rdparty/tinyxml

include ../../../basic_rules.mk

DEBUG_CFLAGS     := -g -DDEBUG
RELEASE_CFLAGS   := -O3
ifeq ("$(CXX)","g++")
    DEBUG_CFLAGS += -Wall -Wno-format -fPIC
    RELEASE_CFLAGS += -Wall -Wno-unknown-pragmas -Wno-format -fPIC
endif

LIBS		 :=

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

#ifeq (YES, ${DEBUG})
ifeq (1, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${TINYXML_USE_STL})
  DEFS := -DTIXML_USE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS := -I${TIXML_H_DIR}


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

#OUTPUT := xmltest
OUTPUT := libtinyxml.a

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

#SRCS := tinyxml.cpp tinyxmlparser.cpp xmltest.cpp tinyxmlerror.cpp tinystr.cpp
SRCS := tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

#${OUTPUT}: ${OBJS}
#	${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
${OUTPUT}: ${OBJS}
	${AR} ${AR_FLAGS} $@ $^
	${RANLIB} $@

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}

depend:
	#makedepend ${INCS} ${SRCS}

# wxc: Generate object files only, which can be packed into other libraries or executives directly
obj_only: ${OBJS}

tinyxml.o: ${TIXML_H_DIR}/tinyxml.h ${TIXML_H_DIR}/tinystr.h
tinyxmlparser.o: ${TIXML_H_DIR}/tinyxml.h ${TIXML_H_DIR}/tinystr.h
#xmltest.o: ${TIXML_H_DIR}/tinyxml.h ${TIXML_H_DIR}/tinystr.h
tinyxmlerror.o: ${TIXML_H_DIR}/tinyxml.h ${TIXML_H_DIR}/tinystr.h
