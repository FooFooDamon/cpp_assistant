CA_LIB_ROOT = ../..

include $(CA_LIB_ROOT)/basic_rules.mk

CXXFLAGS += -g -I../native -I$(CA_LIB_ROOT)/include
CXXFLAGS += -Wno-nonnull
ifneq ("$(GOOGLE_INC_ROOT)","")
	CXXFLAGS += -I$(GOOGLE_INC_ROOT)
endif
LDFLAGS += -L$(CA_LIB_ROOT) -lcpp_assistant -lgtest -lpthread
ifneq ("$(GOOGLE_LIB_DIR)","")
	LDFLAGS += -L$(GOOGLE_LIB_DIR)
endif

TEST_SRCS = $(shell ls test_*.cpp)
TEST_TARGETS = $(basename $(TEST_SRCS))
TEST_OBJS = $(addsuffix .o, $(TEST_TARGETS))

all: $(TEST_TARGETS)

$(TEST_TARGETS): %: %.o main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)

clean:
	rm -f *.o $(TEST_TARGETS)
	rm -rf test_xml_helper.xml ca_test_nonexistent failed_cases.txt core

