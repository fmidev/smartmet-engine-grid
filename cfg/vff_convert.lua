
ParamValueMissing = -16777216;
debug = 0;


function printTable(params)

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      print(index..':'..value);
    end
  end
end


-- ***********************************************************************
--  FUNCTION : C2F
-- ***********************************************************************
--  The function converts given Celcius degrees to Fahrenheit degrees.
-- ***********************************************************************

function C2F(numOfParams,params)

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = value*1.8 + 32;
    else
      result[index] = ParamValueMissing;
    end
  end
  
  -- printTable(result);
  
  return result;
  
end






-- ***********************************************************************
--  FUNCTION : HYPOTENUSE
-- ***********************************************************************
--  Counts the size of the hypotenuse assuming that params1 and params2
--  represents vectors and the angle between them is 90 degrees.
-- ***********************************************************************

function HYPOTENUSE(numOfParams,params1,params2)

  local result = {};

  for index, value in pairs(params1) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = math.sqrt(value*value + params2[index]*params2[index]);
    else
      result[index] = ParamValueMissing;
    end
  end
  
  -- printTable(result);
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : TEST
-- ***********************************************************************
--  
-- ***********************************************************************

function TEST(numOfParams,params1,params2)

  local result = {};

  for index, value in pairs(params1) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = math.sqrt(value*value + params2[index]*params2[index]);
    else
      result[index] = ParamValueMissing;
    end
  end
  
  -- printTable(result);
  
  return result;
  
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
    result.message = 'C2K() : Invalid number of parameters given ('..numOfParams..')!';
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
    result.message = 'F2C() : Invalid number of parameters given ('..numOfParams..')!';
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
    result.message = 'F2K() : Invalid number of parameters given ('..numOfParams..')!';
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

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = value - 273.15;
    else
      result[index] = ParamValueMissing;
    end
  end
  
  -- printTable(result);
  
  return result;
  
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
    result.message = 'K2F() : Invalid number of parameters given ('..numOfParams..')!';
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
    result.message = 'DEG2RAD() : Invalid number of parameters given ('..numOfParams..')!';
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
    result.message = 'RAD2DEG() : Invalid number of parameters given ('..numOfParams..')!';
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
--    Function return two parameters:
--        - result value (function result or ParamValueMissing)
--        - result string (=> 'OK' or an error message)
--
--  So far there are no other types defined.
--  
-- ***********************************************************************


function getFunctionNames(type)

  local functionNames = '';

  if (type == 2) then 
    functionNames = 'C2F,C2K,F2C,F2K,K2C,K2F,DEG2RAD,RAD2DEG';
  end
  if (type == 3) then 
    functionNames = 'HYPOTENUSE';
  end
  
  return functionNames;

end

