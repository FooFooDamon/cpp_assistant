#
# NOTE: This file defines a rule of how to generate source code with .proto files,
#     and should not be used directly and be included into another Makefile instead.
#     All undefind variables will be defined in that Makefile or through command line.
#

PROTOC = protoc
# Value of PROTO_TARGETS is specified by the caller.
#PROTO_TARGETS := public_protocols.pb.cc operator_protocols.pb.cc

all: $(PROTO_TARGETS)

$(PROTO_TARGETS): %.pb.cc: %.proto
	if [ ! -f $@ ] ; then\
		cd $(dir $@);\
		echo "tranforming $^ to $@ ...";\
		$(PROTOC) --cpp_out=. $(notdir $^) ;\
		cd - > /dev/null;\
	fi;

install:
	@echo "do nothing ..."

uninstall:
	@echo "do nothing ..."

check:
	@echo "do nothing ..."

clean:
	rm -f $(patsubst %.cc, %.o, $(PROTO_TARGETS))
	rm -f $(patsubst %.cc, %.h, $(PROTO_TARGETS))
	rm -f $(PROTO_TARGETS)
