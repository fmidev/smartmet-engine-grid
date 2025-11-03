----------------------------------------------------------------------
-- Global definitions
----------------------------------------------------------------------

local ParamValueMissing = -16777216;
local debug = 0;



local ICAO_CODE = {};

ICAO_CODE[10] = "BR";
ICAO_CODE[11] = "FG";
ICAO_CODE[12] = "FZFG";
ICAO_CODE[15] = "DRSN";
ICAO_CODE[16] = "BLSN";
ICAO_CODE[20] = "-TSRA";
ICAO_CODE[21] = "TSRA";
ICAO_CODE[22] = "+TSRA";
ICAO_CODE[23] = "-TSGR";
ICAO_CODE[24] = "TSGR";
ICAO_CODE[25] = "+TSGR";
ICAO_CODE[26] = "-TSSN";
ICAO_CODE[27] = "TSSN";
ICAO_CODE[28] = "+TSSN";
ICAO_CODE[29] = "-TSGS";
ICAO_CODE[30] = "TSGS";
ICAO_CODE[31] = "+TSGS";
ICAO_CODE[32] = "TS";
ICAO_CODE[33] = "-TSRASN";
ICAO_CODE[34] = "TSRASN";
ICAO_CODE[35] = "+TSRASN";
ICAO_CODE[36] = "-TSSNRA";
ICAO_CODE[37] = "TSSNRA";
ICAO_CODE[38] = "+TSSNRA";
ICAO_CODE[50] = "-DZ";
ICAO_CODE[51] = "DZ";
ICAO_CODE[52] = "+DZ";
ICAO_CODE[53] = "-FZDZ";
ICAO_CODE[54] = "FZDZ";
ICAO_CODE[55] = "+FZDZ";
ICAO_CODE[60] = "-RA";
ICAO_CODE[61] = "RA";
ICAO_CODE[62] = "+RA";
ICAO_CODE[63] = "-FZRA";
ICAO_CODE[64] = "FZRA";
ICAO_CODE[65] = "+FZRA";
ICAO_CODE[66] = "-RASN";
ICAO_CODE[67] = "RASN";
ICAO_CODE[68] = "+RASN";
ICAO_CODE[69] = "-SNRA";
ICAO_CODE[70] = "SNRA";
ICAO_CODE[71] = "+SNRA";
ICAO_CODE[72] = "-SN";
ICAO_CODE[73] = "SN";
ICAO_CODE[74] = "+SN";
ICAO_CODE[75] = "-SG";
ICAO_CODE[76] = "SG";
ICAO_CODE[77] = "+SG";
ICAO_CODE[78] = "-PL";
ICAO_CODE[79] = "PL";
ICAO_CODE[80] = "+PL";
ICAO_CODE[81] = "-SHRA";
ICAO_CODE[82] = "SHRA";
ICAO_CODE[83] = "+SHRA";
ICAO_CODE[84] = "-SHRASN";
ICAO_CODE[85] = "SHRASN";
ICAO_CODE[86] = "+SHRASN";
ICAO_CODE[87] = "-SHSNRA";
ICAO_CODE[88] = "SHSNRA";
ICAO_CODE[89] = "+SHSNRA";
ICAO_CODE[90] = "-SHSN";
ICAO_CODE[91] = "SHSN";
ICAO_CODE[92] = "+SHSN";
ICAO_CODE[93] = "-SHGS";
ICAO_CODE[94] = "SHGS";
ICAO_CODE[95] = "+SHGS";
ICAO_CODE[96] = "-SHGR";
ICAO_CODE[97] = "SHGR";
ICAO_CODE[98] = "+SHGR";


local ICAO_DESCRIPTION = {};

ICAO_DESCRIPTION[10] = "Mist";
ICAO_DESCRIPTION[11] = "Fog";
ICAO_DESCRIPTION[12] = "Freezing fog";
ICAO_DESCRIPTION[15] = "Drifting snow";
ICAO_DESCRIPTION[16] = "Blowing snow";
ICAO_DESCRIPTION[20] = "Thunderstorm and light rain";
ICAO_DESCRIPTION[21] = "Thunderstorm and moderate rain";
ICAO_DESCRIPTION[22] = "Thunderstorm and heavy rain";
ICAO_DESCRIPTION[23] = "Thunderstorm and light hail";
ICAO_DESCRIPTION[24] = "Thunderstorm and moderate hail";
ICAO_DESCRIPTION[25] = "Thunderstorm and heavy hail";
ICAO_DESCRIPTION[26] = "Thunderstorm and light snow";
ICAO_DESCRIPTION[27] = "Thunderstorm and moderate snow";
ICAO_DESCRIPTION[28] = "Thunderstorm and heavy snow";
ICAO_DESCRIPTION[29] = "Thunderstorm and light small/soft hail";
ICAO_DESCRIPTION[30] = "Thunderstorm and moderate small/soft hail";
ICAO_DESCRIPTION[31] = "Thunderstorm and heavy small/soft hail";
ICAO_DESCRIPTION[32] = "Thuderstorm nearby (no precipitation)";
ICAO_DESCRIPTION[33] = "Thunderstorm and light rain and snow";
ICAO_DESCRIPTION[34] = "Thunderstorm and moderate rain and snow";
ICAO_DESCRIPTION[35] = "Thunderstorm and heavy rain and snow";
ICAO_DESCRIPTION[36] = "Thunderstorm and light snow and rain";
ICAO_DESCRIPTION[37] = "Thunderstorm and moderate snow and rain";
ICAO_DESCRIPTION[38] = "Thunderstorm and heavy snow and rain";
ICAO_DESCRIPTION[50] = "Light drizzle";
ICAO_DESCRIPTION[51] = "Moderate drizzle";
ICAO_DESCRIPTION[52] = "Heavy drizzle";
ICAO_DESCRIPTION[53] = "Light freezing drizzle";
ICAO_DESCRIPTION[54] = "Moderate freezing drizzle";
ICAO_DESCRIPTION[55] = "Heavy freezing drizzle";
ICAO_DESCRIPTION[60] = "Light rain";
ICAO_DESCRIPTION[61] = "Moderate rain";
ICAO_DESCRIPTION[62] = "Heavy rain";
ICAO_DESCRIPTION[63] = "Light freezing rain";
ICAO_DESCRIPTION[64] = "Moderate freezing rain";
ICAO_DESCRIPTION[65] = "Heavy freezing rain";
ICAO_DESCRIPTION[66] = "Light rain and snow";
ICAO_DESCRIPTION[67] = "Moderate rain and snow";
ICAO_DESCRIPTION[68] = "Heavy rain and snow";
ICAO_DESCRIPTION[69] = "Light snow and rain";
ICAO_DESCRIPTION[70] = "Moderate snow and rain";
ICAO_DESCRIPTION[71] = "Heavy snow and rain";
ICAO_DESCRIPTION[72] = "Light snow";
ICAO_DESCRIPTION[73] = "Moderate snow";
ICAO_DESCRIPTION[74] = "Heavy snow";
ICAO_DESCRIPTION[75] = "Light snow grains";
ICAO_DESCRIPTION[76] = "Moderate snow grains";
ICAO_DESCRIPTION[77] = "Heavy snow grains";
ICAO_DESCRIPTION[78] = "Light ice pellets";
ICAO_DESCRIPTION[79] = "Moderate ice pellets";
ICAO_DESCRIPTION[80] = "Heavy ice pellets";
ICAO_DESCRIPTION[81] = "Light shower of rain";
ICAO_DESCRIPTION[82] = "Moderate shower of rain";
ICAO_DESCRIPTION[83] = "Heavy shower of rain";
ICAO_DESCRIPTION[84] = "Light shower of rain and snow";
ICAO_DESCRIPTION[85] = "Moderate shower of rain and snow";
ICAO_DESCRIPTION[86] = "Heavy shower of rain and snow";
ICAO_DESCRIPTION[87] = "Light shower of snow and rain";
ICAO_DESCRIPTION[88] = "Moderate shower of snow and rain";
ICAO_DESCRIPTION[89] = "Heavy shower of snow and rain";
ICAO_DESCRIPTION[90] = "Light shower of snow";
ICAO_DESCRIPTION[91] = "Moderate shower of snow";
ICAO_DESCRIPTION[92] = "Heavy shower of snow";
ICAO_DESCRIPTION[93] = "Light shower of small/soft hail";
ICAO_DESCRIPTION[94] = "Moderate shower of small/soft hail";
ICAO_DESCRIPTION[95] = "Heavy shower of small/soft hail";
ICAO_DESCRIPTION[96] = "Light shower of hail";
ICAO_DESCRIPTION[97] = "Moderate shower of hail";
ICAO_DESCRIPTION[98] = "Heavy shower of hail";



-- ***********************************************************************
--  FUNCTION : ICAOWX_N_TO_CODE
-- ***********************************************************************

function ICAOWX_N_TO_CODE(language,numOfParams,params)

  local result = {};

  if (numOfParams ~= 1) then
    result.message = 'Invalid number of parameters!';
    result.value = "-";  
    return result.value,result.message;
  end

  local icaoNumber = params[1];
  
  result.message = "OK"
  if (ICAO_CODE[icaoNumber] ~= nil) then  
    result.value = ICAO_CODE[icaoNumber];
  else
    result.value = "-";
  end
  
  return result.value,result.message;

end



-- ***********************************************************************
--  FUNCTION : ICAOWX_DESCRIPTION
-- ***********************************************************************

function ICAOWX_DESCRIPTION(language,numOfParams,params)

  local result = {};

  if (numOfParams ~= 1) then
    result.message = 'Invalid number of parameters!';
    result.value = "-";
    return result.value,result.message;
  end

  local icaoNumber = params[1];
  result.message = "OK"
  
  if (ICAO_DESCRIPTION[icaoNumber] ~= nil) then  
    result.value = ICAO_DESCRIPTION[icaoNumber];
  else
    result.value = "-";
  end
  
  return result.value,result.message;

end




-- ***********************************************************************
--  FUNCTION : getFunctionNames
-- ***********************************************************************
--  The function returns the list of available functions in this file.
--  In this way the query server knows which function are available in
--  each LUA file.
-- 
--  Each LUA file should contain this function. The 'type' parameter
--  indicates how the current LUA function is implemented.
--
--    Type 1 : 
--      Function takes two parameters as input:
--        - numOfParams => defines how many values is in the params array
--        - params      => Array of float values
--      Function returns two parameters:
--        - result value (function result or ParamValueMissing)
--        - result string (=> 'OK' or an error message)
--  
--    Type 5 : 
--      Function takes three parameters as input:
--        - language    => defines the used language
--        - numOfParams => defines how many values is in the params array
--        - params      => Array of float values
--      Function returns two parameters:
--        - result value (string)
--        - result string (=> 'OK' or an error message)
--      Can be use for example for translating a numeric value to a string
--      by using the given language.  
--  
--    Type 9: Takes vector<float[len]> as input and returns vector<float> as output
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - len           => Number of the values in the array
--        - params        => Grid values (vector<float[len]>)
--        - extParams     => Additional parameters (= Array of float values)
--      Function returns one parameter:
--        - result array  => Array of float values.               
--  
-- ***********************************************************************
 
function getFunctionNames(type)

  local functionNames = '';

  if (type == 5) then 
    functionNames = 'ICAOWX_N_TO_CODE,ICAOWX_DESCRIPTION';
  end

  return functionNames;

end

