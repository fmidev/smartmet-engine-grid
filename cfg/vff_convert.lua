
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

function C2F(columns,rows,params)

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

function HYPOTENUSE(columns,rows,params1,params2)

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

function C2K(columns,rows,params)

  local result = {};

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = value + 273.15;
    else
      result[index] = ParamValueMissing;
    end
  end  
  
  return result;
  
end





-- ***********************************************************************
--  FUNCTION : F2C
-- ***********************************************************************
--  The function converts given Fahrenheit degrees to Celcius degrees.
-- ***********************************************************************

function F2C(columns,rows,params)

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = 5*(value - 32)/9;
    else
      result[index] = ParamValueMissing;
    end    
  end
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : F2K
-- ***********************************************************************
--  The function converts given Fahrenheit degrees to Kelvin degrees.
-- ***********************************************************************

function F2K(columns,rows,params)

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = 5*(value - 32)/9 + 273.15;
    else
      result[index] = ParamValueMissing;
    end    
  end
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : K2C
-- ***********************************************************************
--  The function converts given Kelvin degrees to Celcius degrees.
-- ***********************************************************************

function K2C(columns,rows,params)

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

function K2F(columns,rows,params)

  local result = {};

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = (value - 273.15)*1.8 + 32;
    else
      result[index] = ParamValueMissing;
    end
  end
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : DEG2RAD
-- ***********************************************************************
--  The function converts given degrees to radians.
-- ***********************************************************************

function DEG2RAD(columns,rows,params)

  local result = {};
  local PI = 3.1415926;

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = 2*PI*value/360;
    else
      result[index] = ParamValueMissing;
    end
  end
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : RAD2DEG
-- ***********************************************************************
--  The function converts given radians to degrees.
-- ***********************************************************************

function RAD2DEG(columns,rows,params)

  local result = {};
  local PI = 3.1415926;

  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      -- print(index..':'..value);
      result[index] = 360*value/(2*PI);
    else
      result[index] = ParamValueMissing;
    end
  end
  
  return result;
  
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
--    Type 2 : 
--      Function takes three parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params        => Grid values (= Array of float values)
--      Function return one parameter:
--        - result array  => Array of float values (must hava the same 
--                           number of values as the input 'params'.               
--
--    Type 3 : 
--      Function takes four parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params1       => Grid 1 values (= Array of float values)
--        - params2       => Grid 2 values (= Array of float values)
--      Function return one parameter:
--        - result array  => Array of float values (must hava the same 
--                           number of values as the input 'params1'.               
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

