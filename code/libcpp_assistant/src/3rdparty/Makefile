CA_LIB_ROOT = ../..

include $(CA_LIB_ROOT)/basic_rules.mk
include $(CA_LIB_ROOT)/ca_lib.mk

DIR_TINY_XML = ./tinyxml

DIR_LIB_INSTALL = $(CA_LIB_ROOT)/lib

INCLUDES += -I$(CA_LIB_ROOT)/include/3rdparty

ifeq ($(NO_TINYXML),1)
	ALL_SRCS += $(shell find . -path ./tinyxml -prune -o -type f -iname "*.c*" -print | grep -v "\.svn")
else
	ALL_SRCS += $(shell find . -iname "*.c*" | grep -v "\.svn")
endif
ALL_OBJS += $(addsuffix .o, $(basename $(ALL_SRCS)))

STANDALONE_SRCS += $(shell ls *.c* 2> /dev/null)
STANDALONE_OBJS += $(addsuffix .o, $(basename $(STANDALONE_SRCS)))

LIB_3RD_PARTY.A = libca_3rdparty.a
LIB_3RD_PARTY.SO = libca_3rdparty.so

LIBS.A = $(LIB_3RD_PARTY.A)
LIBS.SO = $(LIB_3RD_PARTY.SO)

TARGETS = $(LIBS.A) $(LIBS.SO)

all: $(TARGETS)

.PHONY: obj_only

$(LIB_3RD_PARTY.A): obj_only
	$(AR) $(AR_FLAGS) $@ $(ALL_OBJS)
	$(RANLIB) $@

static: $(LIB_3RD_PARTY.A)

$(LIB_3RD_PARTY.SO): obj_only
	$(CXX) -shared -o $@ $(ALL_OBJS)
	#ln -s $@.$(CA_MAIN_VER) $@

shared: $(LIB_3RD_PARTY.SO)

debug:
	make all DEBUG=1

pure:
	make all CA_USE_STL=0

pure_debug:
	make all DEBUG=1 CA_USE_STL=0

obj_only: $(STANDALONE_OBJS)
	if [ -z "$(NO_TINYXML)" ] ; \
	then \
		make obj_only $(DEBUG_FLAG) $(STL_FLAG) -C $(DIR_TINY_XML) ; \
	fi

check:
	@echo "Static checking to source code would not be done here."

install:
	for i in $(TARGETS); \
	do \
		cp -a $$i* $(DIR_LIB_INSTALL)/ ;\
		cp -a $$i* $(HOME)/lib/ ; \
	done

uninstall:
	-cd $(DIR_LIB_INSTALL)/ && rm -f $(addsuffix *, $(basename $(TARGETS))) && cd -

clean: uninstall
	rm -f $(addsuffix *, $(basename $(TARGETS))) $(STANDALONE_OBJS)
	make clean -C $(DIR_TINY_XML)
