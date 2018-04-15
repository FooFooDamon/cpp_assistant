# NOTE:
#   (1) All undefind variables will be specified in the caller makefile
#       or through command line. USER_PROGRAM_ROOT and CPP_ASSISTANT_ROOT must be in absolute path format,
#       while *_DIR must be in relative path format.

CHMOD_SCRIPT = chmod 777 $(CPP_ASSISTANT_ROOT)/scripts/*.sh

CLEAN_CASDK_RESOURCES = $(CPP_ASSISTANT_ROOT)/scripts/revoke_cpp_assistant_resources.sh \
	$(FRAMEWORK) $(USER_PROGRAM_ROOT) "$(CONFIG_DIRS)" "$(SRC_CODE_DIRS)" "$(PROTO_DIRS)"
PREPARE_CASDK_RESOURCES = $(CPP_ASSISTANT_ROOT)/scripts/export_cpp_assistant_resources.sh \
	$(FRAMEWORK) $(USER_PROGRAM_ROOT) "$(CONFIG_DIRS)" "$(SRC_CODE_DIRS)" "$(PROTO_DIRS)"

# It's ok to use $$() to embrace "echo ... | sed ...", but $(shell ) is bad because it runs
# shell commands at the very beginning, not at a proper run-time.
PROTO_TARGETS = `echo $$(for i in $(PROTO_DIRS); do ls $(USER_PROGRAM_ROOT)/$$i/*.proto; done) \
	| sed '/\.proto/s//\.pb.cc/g'`
# This does not work! Why?
#export PROTO_TARGETS
#CLEAN_PROTO = make clean -f protobuf_code_generation.mk
# or:
#define CLEAN_PROTO
#	make clean -f protobuf_code_generation.mk
#endef
#MAKE_PROTO = make -f protobuf_code_generation.mk
# or:
#define MAKE_PROTO
#	make -f protobuf_code_generation.mk
#endef
# This works.
#CLEAN_PROTO = make PROTO_TARGETS="$(PROTO_TARGETS)" clean -f protobuf_code_generation.mk
# or:
define CLEAN_PROTO
	make PROTO_TARGETS="$(PROTO_TARGETS)" clean -f protobuf_code_generation.mk
endef
#MAKE_PROTO = make PROTO_TARGETS="$(PROTO_TARGETS)" -f protobuf_code_generation.mk
# or:
define MAKE_PROTO
	make PROTO_TARGETS="$(PROTO_TARGETS)" -f protobuf_code_generation.mk
endef

VALIDATE = $(USER_PROGRAM_ROOT)/validate_and_prepare.sh

all:
	$(CHMOD_SCRIPT)
	$(PREPARE_CASDK_RESOURCES)
	$(VALIDATE)
	$(MAKE_PROTO)

clean:
	$(CHMOD_SCRIPT)
	$(CLEAN_PROTO)
	$(CLEAN_CASDK_RESOURCES)
