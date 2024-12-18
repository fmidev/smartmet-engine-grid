
ParamValueMissing = -16777216;
debug = 0;

local precform = {};
precform[1]  = 1;                   -- Rain
precform[2]  = ParamValueMissing;   -- Thunderstorm
precform[3]  = 5;                   -- Freezing rain
precform[4]  = ParamValueMissing;   -- Mixed/ice
precform[5]  = 3;                   -- Snow
precform[6]  = ParamValueMissing;   -- Wet snow
precform[7]  = 2;                   -- Mixture of rain and snow
precform[8]  = ParamValueMissing;   -- Ice pellets (= small hail)
precform[9]  = ParamValueMissing;   -- Graupel
precform[10] = ParamValueMissing;   -- Hail
precform[11] = 0;                   -- Drizzle
precform[12] = 4;                   -- Freezing drizzle 

local precform_rev = {};
precform_rev[0]  = 11;
precform_rev[1]  = 1;
precform_rev[2]  = 7  
precform_rev[3]  = 5; 
precform_rev[4]  = 12;
precform_rev[5]  = 3;  


-- ***********************************************************************
--  FUNCTION : PRECF
-- ***********************************************************************
--  The function converts given PRECFORM2-N parameter value into Newbase 
--  PrecipitationForm parameter.
-- ***********************************************************************

function PRECF(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] >= 1 and params[1] <= 12) then  
      result.value = precform[params[1]];
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end


-- ***********************************************************************
--  FUNCTION : REV_PRECF
-- ***********************************************************************
--  The function converts given Newbase PrecipitationForm parameter into
--  PRECFORM2-N parameter.  
-- ***********************************************************************

function REV_PRECF(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] >= 0 and params[1] <= 5) then  
      result.value = precform_rev[params[1]];
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end


-- ***********************************************************************
--  FUNCTION : C2F
-- ***********************************************************************
--  The function converts given Celcius degrees to Fahrenheit degrees.
-- ***********************************************************************

function C2F(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = params[1]*1.8 + 32;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end





-- ***********************************************************************
--  FUNCTION : C2K
-- ***********************************************************************
--  The function converts given Celcius degrees to Kelvin degrees.
-- ***********************************************************************

function C2K(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = params[1] + 273.15;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end





-- ***********************************************************************
--  FUNCTION : F2C
-- ***********************************************************************
--  The function converts given Fahrenheit degrees to Celcius degrees.
-- ***********************************************************************

function F2C(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = 5*(params[1] - 32)/9;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : F2K
-- ***********************************************************************
--  The function converts given Fahrenheit degrees to Kelvin degrees.
-- ***********************************************************************

function F2K(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = (5*(params[1] - 32)/9) + 273.15;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : K2C
-- ***********************************************************************
--  The function converts given Kelvin degrees to Celcius degrees.
-- ***********************************************************************

function K2C(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = params[1] - 273.15;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end





-- ***********************************************************************
--  FUNCTION : K2F
-- ***********************************************************************
--  The function converts given Kelvin degrees to Fahrenheit degrees.
-- ***********************************************************************

function K2F(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = (params[1] - 273.15)*1.8 + 32;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : DEG2RAD
-- ***********************************************************************
--  The function converts given degrees to radians.
-- ***********************************************************************

function DEG2RAD(numOfParams,params)

  local result = {};
  local PI = 3.1415926;

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = 2*PI*params[1]/360;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
  end
  
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : RAD2DEG
-- ***********************************************************************
--  The function converts given radians to degrees.
-- ***********************************************************************

function RAD2DEG(numOfParams,params)

  local result = {};
  local PI = 3.1415926;

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then  
      result.value = 360*params[1]/(2*PI);
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;
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
--    Type 4 : 
--      Function takes five parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params1       => Grid 1 values (= Array of float values)
--        - params2       => Grid 2 values (= Array of float values)
--        - params3       => Grid point angles to latlon-north (= Array of float values)
--      Function returns one parameter:
--        - result array  => Array of float values (must have the same 
--                           number of values as the input 'params1'.
--      Can be use for example in order to calculate new Wind U- and V-
--      vectors when the input vectors point to grid-north instead of
--      latlon-north.               
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
--    Type 6 : 
--      Function takes two parameters as input:
--        - numOfParams => defines how many values is in the params array
--        - params      => Array of string values
--      Function returns one parameters:
--        - result value (string)
--      This function takes an array of strings and returns a string. It
--      is used for example in order to get additional instructions for
--      complex interpolation operations.  
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

  if (type == 1) then 
    functionNames = 'PRECF,REV_PRECF,C2F,C2K,F2C,F2K,K2C,K2F,DEG2RAD,RAD2DEG';
  end
  
  return functionNames;

end

