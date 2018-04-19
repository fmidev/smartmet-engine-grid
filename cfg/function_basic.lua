
ParamValueMissing = -16777216;
debug = 0;


-- ***********************************************************************
--  FUNCTION : ABS
-- ***********************************************************************
--  The function returns the absolute value of the given parameter.
-- ***********************************************************************

function ABS(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then
      if (params[1] >= 0) then
        result.value = params[1];
      else
        result.value = -params[1];
      end
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
--  FUNCTION : AVG
-- ***********************************************************************
--  The function returns the average value of the given parameters.
-- ***********************************************************************

function AVG(numOfParams,params)

  local result = {};
  local count = 0;
  
  if (numOfParams > 0) then    
    local sum = 0;
    for index, value in pairs(params) do
      if (value ~= ParamValueMissing) then
	    sum = sum + value;
	    count = count + 1;
	  end
    end
    result.message = 'OK';
    result.value = sum / count;
  else
    result.message = 'No parameters given!';
    result.value = 0;  
  end
    
  return result.value,result.message;

end





-- ***********************************************************************
--  FUNCTION : DIV
-- ***********************************************************************
--  The function divides the first parameter with the second parameter.
-- ***********************************************************************

function DIV(numOfParams,params)

  local result = {};
  
  if (numOfParams == 2) then    
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing and params[2] ~= ParamValueMissing) then
      result.value = params[1] / params[2];
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
--  FUNCTION : MAX
-- ***********************************************************************
--  The function returns the maximum value of the given parameters.
-- ***********************************************************************

function MAX(numOfParams,params)

  local result = {};
  
  if (numOfParams > 0) then    
    local max = params[1];
    for index, value in pairs(params) do
      if (value > max and value ~= ParamValueMissing) then
        max = value;
      end
    end

    result.message = 'OK';
    result.value = max;
  else
    result.message = 'No parameters given!';
    result.value = 0;  
  end
    
  return result.value,result.message;

end





-- ***********************************************************************
--  FUNCTION : MIN
-- ***********************************************************************
--  The function returns the minimum value of the given parameters.
-- ***********************************************************************

function MIN(numOfParams,params)

  local result = {};
  
  if (numOfParams > 0) then    
    local min = params[1];
    for index, value in pairs(params) do
      if (value < min and value ~= ParamValueMissing) then
        min = value;
      end
    end

    result.message = 'OK';
    result.value = min;
  else
    result.message = 'No parameters given!';
    result.value = 0;  
  end
    
  return result.value,result.message;

end
 

 


-- ***********************************************************************
--  FUNCTION : MUL
-- ***********************************************************************
--  The function multiplies the first parameter with the second parameter.
-- ***********************************************************************

function MUL(numOfParams,params)

  local result = {};
  
  if (numOfParams == 2) then    
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing and params[2] ~= ParamValueMissing) then
      result.value = params[1] * params[2];
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
--  FUNCTION : NEG
-- ***********************************************************************
--  The function returns the negative value of the given parameter. Notice
--  that if the given parameter is negative then the result is positive.
-- ***********************************************************************

function NEG(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then    
      result.value = -params[1];
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
--  FUNCTION : SQRT
-- ***********************************************************************
--  The function returns the sqrt value of the given parameter.
-- ***********************************************************************

function SQRT(numOfParams,params)

  local result = {};

  if (numOfParams == 1) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing) then    
      result.value = math.sqrt(params[1]);
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
--  FUNCTION : SUM
-- ***********************************************************************
--  The function returns the sum of the given parameters.
-- ***********************************************************************

function SUM(numOfParams,params)

  local result = {};
  
  if (numOfParams > 0) then   
    local sum = 0;
    for index, value in pairs(params) do
      if (value ~= ParamValueMissing) then
        sum = sum + value;
      else
        result.message = 'OK';
        result.value = ParamValueMissing;
        return result.value,result.message;
      end
    end
    result.message = 'OK';
    result.value = sum;
  else
    result.message = 'No parameters given!';
    result.value = 0;  
  end
    
  return result.value,result.message;

end




-- ***********************************************************************
--  FUNCTION : DIFF
-- ***********************************************************************
--  The function returns the sum of the given parameters.
-- ***********************************************************************

function DIFF(numOfParams,params)

  local result = {};

  if (numOfParams == 2) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing and params[2] ~= ParamValueMissing) then    
      result.value = params[1]-params[2];
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
--  FUNCTION : HYPOT
-- ***********************************************************************
--  The function returns the hypotenuse of the given parameters.
-- ***********************************************************************

function HYPOT(numOfParams,params)

  local result = {};

  if (numOfParams == 2) then
    result.message = 'OK';
    if (params[1] ~= ParamValueMissing and params[2] ~= ParamValueMissing) then    
      result.value = math.sqrt(params[1]*params[1] + params[2]*params[2]);
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
--  FUNCTION : ITEM
-- ***********************************************************************
--  The function returns the item of the given index.
-- ***********************************************************************

function ITEM(numOfParams,params)

  local result = {};
  
  if (numOfParams > 0) then    
    local idx = params[1] + 1;
    for index, value in pairs(params) do
      if (idx == index) then
        result.message = 'OK';
        result.value = value;
        return result.value,result.message;
      end
    end

  end
    
  result.message = 'OK';
  result.value = ParamValueMissing;
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
--    Type 2 : 
--      Function takes three parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params        => Grid values (= Array of float values)
--      Function return one parameter:
--        - result array  => Array of float values (must have the same 
--                           number of values as the input 'params'.               
--
--    Type 3 : 
--      Function takes four parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params1       => Grid 1 values (= Array of float values)
--        - params2       => Grid 2 values (= Array of float values)
--      Function return one parameter:
--        - result array  => Array of float values (must have the same 
--                           number of values as the input 'params1'.               
--  
--    Type 4 : 
--      Function takes five parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params1       => Grid 1 values (= Array of float values)
--        - params2       => Grid 2 values (= Array of float values)
--        - params3       => Grid point angles to latlon-north (= Array of float values)
--      Function return one parameter:
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
--      Function return two parameters:
--        - result value (string)
--        - result string (=> 'OK' or an error message)
--      Can be use for example for translating a numeric value to a string
--      by using the given language.  
--  
--    Type 6 : 
--      Function takes two parameters as input:
--        - numOfParams => defines how many values is in the params array
--        - params      => Array of string values
--      Function return one parameters:
--        - result value (string)
--      This function takes an array of strings and returns a string. It
--      is used for example in order to get additional instructions for
--      complex interpolation operations.  
--  
-- ***********************************************************************


function getFunctionNames(type)

  local functionNames = '';

  if (type == 1) then 
    functionNames = 'ABS,AVG,DIV,ITEM,MAX,MIN,MUL,NEG,SUM,DIFF,HYPOT,SQRT';
  end
  
  return functionNames;

end

