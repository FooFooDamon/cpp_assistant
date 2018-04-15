CXXFLAGS = -Wall -g -I. -I..
 
%.o: %.cpp
	g++ -c -o $@ $^ $(CXXFLAGS)

%.cpp: %.otl.sql
	$(OTL2CPP_SCRIPT) $^ $@

%.otl.sql: %.oracle.sql
	$(ORACLE2OTL_SCRIPT) $^ $@

%.otl.sql: %.mysql.sql
	$(MYSQL2OTL_SCRIPT) $^ $@

