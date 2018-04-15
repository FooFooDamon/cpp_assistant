/*
 * Bind variable format: :<variable name>\<<variable type>\>
 * Example: :CONST_VAR<int>.
 *     Be aware that :CONST_VAR_INT<int> is just for testing,
 *     other types of variables are the same.
 */

/******** STATIC_SQL(SimpleHeartbeat) begin ********/

select 1
from dual
where :CONST_VAR_INT<int> = :CONST_VAR_INT<int>;

/******** STATIC_SQL(SimpleHeartbeat) end ********/



/******** STATIC_SQL(ComplexHeartbeat) begin ********/

with common_result as
(
     select :VariableString_Char_128<char[128]> vstr,
       :CONST_VAR_INT<int> aa,
       :CONST_VAR_BIGINT<bigint> bb,
       :CONST_VAR_LONG<long> cc,
       :CONST_VAR_FLOAT<float> dd,
       :CONST_VAR_DOUBLE<double> ee,
       :CONST_VAR_CLOB<clob> ff,
       :CONST_VAR_BLOB<blob> gg,
       :CONST_VAR_TIMESTAMP<timestamp> hh,
       :CONST_VAR_VARCHAR_LONG<varchar_long> ii,
       :CONST_VAR_RAW_LONG<raw_long> jj,
       :CONST_VAR_LONG_DOUBLE<long double> kk,
       :CONST_VAR_UNSIGNED_LONG_INT<unsigned long int> ll,
       :CONST_VAR_UNSIGNED_SHORT_INT<unsigned short int> mm
     from dual
     where :CONST_VAR_INT<int> = :CONST_VAR_INT<int>
)
select bb
from common_result
where :VariableString_Char_128<char[128]> = :VariableString_Char_128<char[128]>
  and :CONST_VAR_INT<int> = :CONST_VAR_INT<int>
  and :CONST_VAR_BIGINT<bigint> = :CONST_VAR_BIGINT<bigint>
  and :CONST_VAR_LONG<long> = :CONST_VAR_LONG<long>
  and :CONST_VAR_FLOAT<float> = :CONST_VAR_FLOAT<float>
  and :CONST_VAR_DOUBLE<double> = :CONST_VAR_DOUBLE<double>
  and :CONST_VAR_CLOB<clob> = :CONST_VAR_CLOB<clob>
  and :CONST_VAR_BLOB<blob> = :CONST_VAR_BLOB<blob>
  and :CONST_VAR_TIMESTAMP<timestamp> = :CONST_VAR_TIMESTAMP<timestamp>
  and :CONST_VAR_VARCHAR_LONG<varchar_long> = :CONST_VAR_VARCHAR_LONG<varchar_long>
  and :CONST_VAR_RAW_LONG<raw_long> = :CONST_VAR_RAW_LONG<raw_long>
  and :CONST_VAR_LONG_DOUBLE<long double> = :CONST_VAR_LONG_DOUBLE<long double>
  and :CONST_VAR_UNSIGNED_LONG_INT<unsigned long int> = :CONST_VAR_UNSIGNED_LONG_INT<unsigned long int>
  and :CONST_VAR_UNSIGNED_SHORT_INT<unsigned short int> = :CONST_VAR_UNSIGNED_SHORT_INT<unsigned short int>;

/******** STATIC_SQL(ComplexHeartbeat) end ********/
