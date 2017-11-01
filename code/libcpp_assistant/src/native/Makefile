CA_LIB_ROOT = ../..

include $(CA_LIB_ROOT)/basic_rules.mk
include $(CA_LIB_ROOT)/ca_lib.mk

DIR_LIB_INSTALL = $(CA_LIB_ROOT)/lib

INCLUDES += -I. -I$(CA_LIB_ROOT)/include
INCLUDES += -I$(CA_LIB_ROOT)/include/3rdparty/tinyxml # for base/stl_adapters/string.cpp

SRCS += $(shell find . -name "*.cpp" | grep -v "\.svn")
OBJS += $(addsuffix .o, $(basename $(SRCS)))

LIB_CA_NATIVE.A = libca_native.a
LIB_CA_NATIVE.SO = libca_native.so

LIBS.A = $(LIB_CA_NATIVE.A)
LIBS.SO = $(LIB_CA_NATIVE.SO)

TARGETS = $(LIBS.A) $(LIBS.SO)

all: $(TARGETS)

.PHONY: obj_only

$(LIB_CA_NATIVE.A): obj_only
	$(AR) $(AR_FLAGS) $@ $(OBJS)
	$(RANLIB) $@

static: $(LIB_CA_NATIVE.A)

$(LIB_CA_NATIVE.SO): obj_only
	$(CXX) -shared -o $@.$(CA_MAIN_VER) $(OBJS)
	ln -s $@.$(CA_MAIN_VER) $@

shared: $(LIB_CA_NATIVE.SO)

debug:
	make all DEBUG=1

pure:
	make all CA_USE_STL=0

pure_debug:
	make all DEBUG=1 CA_USE_STL=0

obj_only: $(OBJS)

check:
	cppcheck --enable=all $(INCLUDES) $(DEFINES) . $(CA_LIB_ROOT)/include
	clang --analyze $(SRCS) $(INCLUDES) $(DEFINES)

install:
	for i in $(TARGETS); \
	do \
		cp -a $$i* $(DIR_LIB_INSTALL)/ ;\
		cp -a $$i* $(HOME)/lib/ ; \
	done

uninstall:
	cd $(DIR_LIB_INSTALL)/ && rm -f $(addsuffix *, $(basename $(TARGETS))) && cd -

clean: uninstall
	rm -f $(addsuffix *, $(basename $(TARGETS))) $(OBJS)
