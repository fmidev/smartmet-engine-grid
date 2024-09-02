
ParamValueMissing = -16777216;
debug = 0;


-- ***********************************************************************
--  FUNCTION : COUNT
-- ***********************************************************************
--  The function returns the number of the parameter which value is
--  not 'ParamValueMissing'.
-- ***********************************************************************

function COUNT(numOfParams,params)

  local cnt = 0;
  local result = {};
  
  for index, value in pairs(params) do
    if (value ~= ParamValueMissing) then
      cnt = cnt + 1;
    end
  end
  result.message = 'OK';
  result.value = cnt;
    
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : VALID
-- ***********************************************************************
--  The function returns the first valid value in the list.
-- ***********************************************************************

function VALID(numOfParams,params)

  local result = {};
  
  if (numOfParams > 0) then   
    for index, value in pairs(params) do
      if (value ~= ParamValueMissing) then
        result.message = 'OK';
        result.value = value;
		return result.value,result.message;
      end
    end
    result.message = 'No valid value found!';
    result.value = ParamValueMissing;
  else
    result.message = 'No parameters given!';
    result.value = 0;  
  end
    
  return result.value,result.message;

end




function PROB_GT(numOfParams,params)

  local cnt = 0;
  local match = 0;
  local result = {};  
  local val = params[1];
  print("PARAMS "..numOfParams);

  for index, value in pairs(params) do
    print("PROB "..index.." : "..value);
  end
    
  for index, value in pairs(params) do
    if (index > 1) then
      cnt = cnt + 1;    
      if (value > val) then
        match = match + 1;
      end
    end
  end
  print("MATCH "..match.." COUNT "..cnt);
  result.message = 'OK';
  result.value = match / cnt;
    
  return result.value,result.message;
  
end



function XMAX(columns,rows,len,params,extParams)

  print("XMAX");
  local result = {};
  local inIdx = 1;
  local outIdx = 1;
    
  for r=1,rows do
    for c=1,columns do
      local max = ParamValueMissing;
      for i=1,len do
        value = params[inIdx];      
        if (value ~= ParamValueMissing and (max == ParamValueMissing or value > max)) then
          max = value;
        end
        inIdx = inIdx + 1;        
      end
      result[outIdx] = max;
      outIdx = outIdx + 1;
    end
  end
  
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : SnowWaterRatio
-- ***********************************************************************
--  Calculate water to snow conversion factor
-- 
-- The formula is based on the table given on page 12 of the
-- M.Sc thesis by Ivan Dube titled "From mm to cm... Study of
-- snow/liquid water ratios in Quebec".
-- 
-- \param t Temperature in °C
-- \param ff Wind speed in m/s
-- \return Snow/water ratio
-- ***********************************************************************
function SnowWaterRatio(t, ff)

    if (t == ParamValueMissing or ff == ParamValueMissing) then
        return 10.0;
    end

    local knot = 0.51444444444444444444;

    if (ff < 10 * knot) then
        if (t > 0) then
            return 10;
        elseif (t > -5) then
            return 11;
        elseif (t > -10) then
            return 14;
        elseif (t > -20) then
            return 17;
        else
            return 15;
        end
    elseif (ff < 20 * knot) then
        if (t > -5) then
            return 10;
        elseif (t > -10) then
            return 12;
        elseif (t > -20) then
            return 15;
        else
            return 13;
        end
    end

    if (t > -10) then
        return 10;
    elseif (t > -15) then
        return 11;
    elseif (t > -20) then
        return 14;
    end

    return 12;
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
    functionNames = 'COUNT,VALID,PROB_GT';
  end
  
  if (type == 9) then 
    functionNames = 'XMAX';
  end

  return functionNames;

end

