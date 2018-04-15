#include <stdio.h>

#include "static_sqls.h"

#define USE_STATIC_SQL(name)				DECLARE_STATIC_SQL(name); \
	printf(STATIC_SQL_MACRO_NAME(name)" | "STATIC_SQL_VAR_NAME(name)" will be used, variable address: %p\n", STATIC_SQL_VAR(name))

extern void print_static_sql_definitions(void);

void report_usage_in_business_function(void)
{
	USE_STATIC_SQL(SimpleHeartbeat);
	USE_STATIC_SQL(ComplexHeartbeat);
	printf("\n\n");
}

int main(int argc, char **argv)
{
	report_usage_in_business_function();
	print_static_sql_definitions();
	return 0;
}

