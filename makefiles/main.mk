# NOTE:
#   (1) All undefined variables will be specified in the caller makefile or through command line.
#       USER_PROGRAM_ROOT and CPP_ASSISTANT_ROOT must be in absolute path format,
#       while *DIRS must be in relative path format.
#       MAKE_TYPE can be install, uninstall, check, clean or dist, or just be unspecified.
#   (2) prerequisites.mk is supposed to be implemented by user, and then be called before any
#       other makefile. Implementation of prerequisites.mk should include but is not limited by
#       the folowing contents:
#       a) making symbol links for makefiles in casdk directory, or copying them to
#          the $(USER_PROGRAM_ROOT) directory.
#       b) making symbol links for configuration files in casdk directory, or copying them to
#          the user configuration directory.
#       c) making symbol links for code files in casdk directory, or copying them to
#          the user source code directory.
#       d) generating source code from protocol buffer file if required.

export USER_PROGRAM_ROOT
export CPP_ASSISTANT_ROOT
export CONFIG_DIRS
export SRC_CODE_DIRS
export PROTO_DIRS
export FRAMEWORK
ifeq ($(IS_SINGLE_TARGET),1)
	export TARGET
	export SRCS
	export CXXFLAGS
	export LDFLAGS
	BUILD_MAKEFILE = $(CPP_ASSISTANT_ROOT)/makefiles/single_executive_template.mk
else
	export SUBDIRS
	BUILD_MAKEFILE = $(CPP_ASSISTANT_ROOT)/makefiles/module_compilation_manager.mk
endif

all:
	echo "MAKEFLAGS[$(MAKEFLAGS)] | MAKEFILES[$(MAKEFILES)] | MAKEFILE[$(MAKEFILE)]"
	make prerequisites
	make -f $(BUILD_MAKEFILE)

prerequisites:
	echo "MAKEFLAGS[$(MAKEFLAGS)] | MAKEFILES[$(MAKEFILES)] | MAKEFILE[$(MAKEFILE)]"
	make $(MAKE_TYPE) -f $(CPP_ASSISTANT_ROOT)/makefiles/prerequisites.mk
	#if [ -z "$(MAKE_TYPE)" ] ; then \
		#cd $(CPP_ASSISTANT_ROOT)/code/libcpp_assistant; \
		#make debug && make install; \
		#cd - ; \
	#fi

install:
	make install -f $(BUILD_MAKEFILE)

uninstall:
	make uninstall -f $(BUILD_MAKEFILE)

check:
	make prerequisites
	make check -f $(BUILD_MAKEFILE)

clean:
	find . -iname "*.plist" | xargs rm -f
	make clean -f $(BUILD_MAKEFILE)
	make prerequisites MAKE_TYPE=clean

dist:
	make dist -f $(BUILD_MAKEFILE)
