/*
 * Bind variable format: @<variable name>_<variable type>
 * Example: @CONST_VAR_int.
 *     Be aware that @CONST_VAR_INT_int is just for testing,
 *     other types of variables are the same.
 */

/******** STATIC_SQL(SimpleHeartbeat) begin ********/

select 1
from dual
where @CONST_VAR_INT_int = @CONST_VAR_INT_int;

/******** STATIC_SQL(SimpleHeartbeat) end ********/



/******** STATIC_SQL(ComplexHeartbeat) begin ********/

with common_result as
(
     select @VariableString_Char_128_char_128 vstr,
       @CONST_VAR_INT_int aa,
       @CONST_VAR_BIGINT_bigint bb,
       @CONST_VAR_LONG_long cc,
       @CONST_VAR_FLOAT_float dd,
       @CONST_VAR_DOUBLE_double ee,
       @CONST_VAR_CLOB_clob ff,
       @CONST_VAR_BLOB_blob gg,
       @CONST_VAR_TIMESTAMP_timestamp hh,
       @CONST_VAR_VARCHAR_LONG_varchar_long ii,
       @CONST_VAR_RAW_LONG_raw_long jj,
       @CONST_VAR_LONG_DOUBLE_long_double kk,
       @CONST_VAR_UNSIGNED_LONG_INT_unsigned_long_int ll,
       @CONST_VAR_UNSIGNED_SHORT_INT_unsigned_short_int mm
     from dual
     where @CONST_VAR_INT_int = @CONST_VAR_INT_int
)
select bb
from common_result
where @VariableString_Char_128_char_128 = @VariableString_Char_128_char_128
  and @CONST_VAR_INT_int = @CONST_VAR_INT_int
  and @CONST_VAR_BIGINT_bigint = @CONST_VAR_BIGINT_bigint
  and @CONST_VAR_LONG_long = @CONST_VAR_LONG_long
  and @CONST_VAR_FLOAT_float = @CONST_VAR_FLOAT_float
  and @CONST_VAR_DOUBLE_double = @CONST_VAR_DOUBLE_double
  and @CONST_VAR_CLOB_clob = @CONST_VAR_CLOB_clob
  and @CONST_VAR_BLOB_blob = @CONST_VAR_BLOB_blob
  and @CONST_VAR_TIMESTAMP_timestamp = @CONST_VAR_TIMESTAMP_timestamp
  and @CONST_VAR_VARCHAR_LONG_varchar_long = @CONST_VAR_VARCHAR_LONG_varchar_long
  and @CONST_VAR_RAW_LONG_raw_long = @CONST_VAR_RAW_LONG_raw_long
  and @CONST_VAR_LONG_DOUBLE_long_double = @CONST_VAR_LONG_DOUBLE_long_double
  and @CONST_VAR_UNSIGNED_LONG_INT_unsigned_long_int = @CONST_VAR_UNSIGNED_LONG_INT_unsigned_long_int
  and @CONST_VAR_UNSIGNED_SHORT_INT_unsigned_short_int = @CONST_VAR_UNSIGNED_SHORT_INT_unsigned_short_int;

/******** STATIC_SQL(ComplexHeartbeat) end ********/
