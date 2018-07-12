#
# NOTE: This file defines a rule of how to produce a single executive,
#     and should not be used directly and be included into another Makefile instead.
#     All undefind variables will be defined in that Makefile or through command line.
#

SRCS_BASE = $(basename $(SRCS))
OBJS += $(addsuffix .o, $(SRCS_BASE))

all: $(TARGET)

$(TARGET): $(OBJS)
	echo "MAKEFLAGS[$(MAKEFLAGS)] | MAKEFILES[$(MAKEFILES)] | MAKEFILE[$(MAKEFILE)]"
	$(CXX) -o $@ -fPIE -Wl,--start-group $^ $(LDFLAGS) -Wl,--end-group
	@#$(CXX) -o $@ -fPIE -Xlinker "-(" $^ $(LDFLAGS) -Xlinker "-)"
	-ln -snf `pwd`/$(TARGET) $(USER_PROGRAM_ROOT)/$(TARGET)

install:
	mkdir -p $(USER_PROGRAM_ROOT)/bin
	cp $(TARGET) $(USER_PROGRAM_ROOT)/bin

uninstall:
	rm -f $(USER_PROGRAM_ROOT)/bin/$(TARGET)

check:
	cppcheck --enable=all $(INCLUDES) $(DEFINES) .
	clang --analyze $(SRCS) $(INCLUDES) $(DEFINES)

clean:
	rm -f $(TARGET) $(USER_PROGRAM_ROOT)/$(TARGET) $(OBJS)
