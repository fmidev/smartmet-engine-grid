
ParamValueMissing = -16777216;
debug = 0;


-- ***********************************************************************
--  FUNCTION : E_AVG
-- ***********************************************************************
--  The function returns the average values for each ensemle members. These
--  values are counted over an area (= each member has a list of values).
--  Let's assume that we have N members, then the 'param' array contains
--  the following values:
--    
--    N,M[1][1],M2[2][1],..,M[N][1],N,M[1][2],M[2][2],..,M[N][2],N,M[1][3],
--    M[2][3],..,M[N][3] ...
--
--  As we can see, in the beginning we have the first values of each member, 
--  then the second values of each member, then the third values, an so on.
--  Notice that the number of ensemble members 'N' starts each value sequence.
-- ***********************************************************************

function E_AVG(language,numOfParams,params)

  -- for index, value in pairs(params) do
  --   print(" -- "..index.." : "..value);
  -- end

  local n = params[1] + 1;

  -- Initializing sum and count values for each member (=> arrays):
  
  local sum = {};
  local count = {};
  for a=1,n do
    sum[a] = 0;
    count[a] = 0;
  end
  
  -- Counting values:
  
  for index, value in pairs(params) do
    local idx = ((index-1) % n) + 1;
    if (value ~= ParamValueMissing) then
      sum[idx] = sum[idx] + value;
      count[idx] = count[idx] + 1;
    end
  end
  
  -- Building the result string:
  
  local str = "";  
  for i=2,n do  
    if (count[i] > 0) then      
      local value = sum[i] / count[i];
      str = str..value;      
    else
      str = str..ParamValueMissing
    end
    if (i < n) then
      str = str..";"
    end  
  end
    
  local result = {};
  result.message = 'OK';
  result.value = str;
  return result.value,result.message;

end



-- ***********************************************************************
--  FUNCTION : E_AVG_DIFF
-- ***********************************************************************
--  The function returns the average difference values for each ensemle members. 
--  These values are counted over an area (= each member has a list of values).
--  Let's assume that we have N members, then the 'param' array contains
--  the following values:
--    
--    N,M[1][1],M2[2][1],..,M[N][1],N,M[1][2],M[2][2],..,M[N][2],N,M[1][3],
--    M[2][3],..,M[N][3] ...
--
--  As we can see, in the beginning we have the first values of each member, 
--  then the second values of each member, then the third values, an so on.
--  Notice that the number of ensemble members 'N' starts each value sequence.
-- ***********************************************************************

function E_AVG_DIFF(language,numOfParams,params)

  -- for index, value in pairs(params) do
  --   print(" -- "..index.." : "..value);
  -- end

  local n = params[1] + 1;

  -- Initializing sum and count values for each member (=> arrays):
  
  local totalSum = 0;
  local totalCount = 0;
  local sum = {};
  local count = {};
  for a=1,n do
    sum[a] = 0;
    count[a] = 0;
  end
  
  -- Counting values:
  
  for index, value in pairs(params) do
    local idx = ((index-1) % n) + 1;
    if (value ~= ParamValueMissing) then
      sum[idx] = sum[idx] + value;
      count[idx] = count[idx] + 1;
      if (idx > 1) then      
        totalSum = totalSum + value;
        totalCount = totalCount + 1;
      end
    end
  end
  
  -- Building the result string:
  
  local total = totalSum / totalCount;
  
  local str = "";  
  for i=2,n do  
    if (count[i] > 0) then      
      local value = (sum[i] / count[i]) - total;
      str = str..value;      
    else
      str = str..ParamValueMissing
    end
    if (i < n) then
      str = str..";"
    end  
  end
    
  local result = {};
  result.message = 'OK';
  result.value = str;
  return result.value,result.message;

end


-- ***********************************************************************
--  FUNCTION : E_MAX
-- ***********************************************************************
--  The function returns the maximum values for each ensemle members. These
--  values are counted over an area (= each member has a list of values).
--  Let's assume that we have N members, then the 'param' array contains
--  the following values:
--    
--    N,M[1][1],M2[2][1],..,M[N][1],N,M[1][2],M[2][2],..,M[N][2],N,M[1][3],
--    M[2][3],..,M[N][3] ...
--
--  As we can see, in the beginning we have the first values of each member, 
--  then the second values of each member, then the third values, an so on.
--  Notice that the number of ensemble members 'N' starts each value sequence.
-- ***********************************************************************

function E_MAX(language,numOfParams,params)

  -- for index, value in pairs(params) do
  --   print(" -- "..index.." : "..value);
  -- end

  local n = params[1] + 1;

  -- Initializing sum and count values for each member (=> arrays):
  
  local val = {};
  for a=1,n do
    val[a] = ParamValueMissing;
  end
  
  -- Counting values:
  
  for index, value in pairs(params) do
    local idx = ((index-1) % n) + 1;
    if (value ~= ParamValueMissing) then    
      if (val[idx] == ParamValueMissing or val[idx] < value) then
        val[idx] = value;
      end;
    end
  end
  
  -- Building the result string:
  
  local str = "";  
  for i=2,n do  
    if (val[i] ~= ParamValueMissing) then      
      str = str..val[i];      
    else
      str = str..ParamValueMissing
    end
    if (i < n) then
      str = str..";"
    end  
  end
    
  local result = {};
  result.message = 'OK';
  result.value = str;
  return result.value,result.message;

end
 



-- ***********************************************************************
--  FUNCTION : E_MIN
-- ***********************************************************************
--  The function returns the minimum values for each ensemle members. These
--  values are counted over an area (= each member has a list of values).
--  Let's assume that we have N members, then the 'param' array contains
--  the following values:
--    
--    N,M[1][1],M2[2][1],..,M[N][1],N,M[1][2],M[2][2],..,M[N][2],N,M[1][3],
--    M[2][3],..,M[N][3] ...
--
--  As we can see, in the beginning we have the first values of each member, 
--  then the second values of each member, then the third values, an so on.
--  Notice that the number of ensemble members 'N' starts each value sequence.
-- ***********************************************************************

function E_MIN(language,numOfParams,params)

  -- for index, value in pairs(params) do
  --   print(" -- "..index.." : "..value);
  -- end

  local n = params[1] + 1;

  -- Initializing sum and count values for each member (=> arrays):
  
  local val = {};
  for a=1,n do
    val[a] = ParamValueMissing;
  end
  
  -- Counting values:
  
  for index, value in pairs(params) do
    local idx = ((index-1) % n) + 1;
    if (value ~= ParamValueMissing) then    
      if (val[idx] == ParamValueMissing or val[idx] > value) then
        val[idx] = value;
      end;
    end
  end
  
  -- Building the result string:
  
  local str = "";  
  for i=2,n do  
    if (val[i] ~= ParamValueMissing) then      
      str = str..val[i];      
    else
      str = str..ParamValueMissing
    end
    if (i < n) then
      str = str..";"
    end  
  end
    
  local result = {};
  result.message = 'OK';
  result.value = str;
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
--  -
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
-- ***********************************************************************


function getFunctionNames(type)

  local functionNames = '';

  if (type == 5) then 
    functionNames = 'E_AVG,E_AVG_DIFF,E_MAX,E_MIN';
  end
 
  
  
  return functionNames;

end

