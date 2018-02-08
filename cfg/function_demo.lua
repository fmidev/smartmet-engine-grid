
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

  if (type == 1) then 
    functionNames = 'COUNT,VALID';
  end
  
  return functionNames;

end

