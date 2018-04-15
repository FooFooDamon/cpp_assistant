#
# To make or clean all targets at a time, run "make" or "make clean".
#
# To make or clean one of targets at a time, run "make <target name>" or "make clean_<target name>".
# For example: "make otl_format" or "make clean_otl_format".
#

OTL2CPP_SCRIPT = ../scripts/sql2cpp.otl.sh
ORACLE2OTL_SCRIPT = ../scripts/oracle2otl.sh
MYSQL2OTL_SCRIPT = ../scripts/mysql2otl.sh

TARGETS := otl_format oracle_format mysql_format
CLEAN_TARGETS := $(addprefix clean_,$(TARGETS))

all: $(TARGETS)

$(TARGETS): %: %.o main.o
	g++ -o $@ $^

$(CLEAN_TARGETS): %:
	-rm $(subst clean_,,$@) $(subst clean_,,$@).o

clean:
	for i in $(CLEAN_TARGETS);\
	do \
		make $$i;\
	done;
	rm -f main.o static_sqls.h*

include basic_rules.mk

