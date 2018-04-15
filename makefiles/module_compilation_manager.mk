#
# NOTE: This file defines a rule of how to compile sub-modules of this module,
#     and should not be used directly and be included into another Makefile instead.
#     All undefind variables will be defined in that Makefile or through command line.
#

export CPP_ASSISTANT_ROOT
export USER_PROGRAM_ROOT
export FRAMEWORK
export SRC_CODE_DIRS
CORES = $(shell cat /proc/cpuinfo | grep "processor" | wc -l)
PARALLEL_FLAG = -j $(CORES)

recursive:
	echo "MAKEFLAGS[$(MAKEFLAGS)] | MAKEFILES[$(MAKEFILES)] | MAKEFILE[$(MAKEFILE)]"
	for i in $(SUBDIRS);\
	do \
		cd $$i;\
		make $(MAKE_TYPE) $(PARALLEL_FLAG);\
		if [ $$? -ne 0 ] ; then\
			exit 1;\
		fi; \
		cd -;\
	done;

install:
	make recursive MAKE_TYPE=install -f module_compilation_manager.mk

uninstall:
	make recursive MAKE_TYPE=uninstall -f module_compilation_manager.mk

check:
	make recursive MAKE_TYPE=check -f module_compilation_manager.mk

clean:
	make recursive MAKE_TYPE=clean -f module_compilation_manager.mk

dist:
	echo "do nothing ..."
