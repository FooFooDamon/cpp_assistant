#ifndef __SQL_CONVERSION_STATIC_SQLS_H__
#define __SQL_CONVERSION_STATIC_SQLS_H__

#define STATIC_SQL_VAR(name)				g_kStaticSql_##name

#define STATIC_SQL_VAR_NAME(name)			"g_kStaticSql_"#name

#define STATIC_SQL_MACRO_NAME(name)			"STATIC_SQL("#name")"

#define DECLARE_STATIC_SQL(name)			extern const char* STATIC_SQL_VAR(name)

#endif // __SQL_CONVERSION_STATIC_SQLS_H__

