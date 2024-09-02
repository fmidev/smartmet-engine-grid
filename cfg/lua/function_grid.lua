-- This file enables simple caclulation of new grids. The idea is that GRID of GRIDX functions 
-- call a sub-function for each grid point and this sub-function calculates a new value for 
-- the grid point and returns it to the calling function. 

-- The difference between GRID and GRIDX function is that the first one can call a function that 
-- returns the new value as value-message -object. The GRIDX assumes that the function returns only 
-- the new value. 

-- On the other hand, GRIDX support delivery of extra parameters needed for the calculation, 
-- but GRID does not. For example, if we want to calculate probability that a value is in range A..B
-- then these range parameters are delivered in 'extParams' vector. 

-- These functions are called from WMS/Timeseries so, that the sub-function name is added after 
-- the GRID or GRIDX function name: 

--     GRID:MAX(Temperature:MEPS:1093:6:2:3:1-14}
--     GRIDX:MIN(Temperature:MEPS:1093:6:2:3:1-14}


-- If you want to define you own sub-functions just create a new file and add the "require" command
-- below. Notice that LUA cannot find the file if its location is not defined in the LUA_PATH enviroment
-- varible. This should be defined before the server is started:

--         export LUA_PATH=/smartmet/cnf/smartmetd/clients/engines/grid-engine/?.lua


require "function_basic"



ParamValueMissing = -16777216;


-- An example of a function that GRIDX might be calling. Notice that this
-- function is a little bit simpler 

function MIN(numOfParams,params,extParams)
 
  if (numOfParams > 0) then    
    local min = params[1];
    for index, value in pairs(params) do
      if (min == ParamValueMissing or (value < min and value ~= ParamValueMissing)) then
        min = value;
      end
    end

    return min;
  end
  return ParamValueMissing;

end



function GRID(subfunction, columns, rows, len, params, extParams)

  local result = {};

  -- print("GRID "..subfunction.." len "..len);
  -- for index, value in pairs(extParams) do
  --  print(index.." : "..value);
  -- end

  local inIdx = 1;
  local outIdx = 1;    
  local p = {};

  for r = 1, rows do
    for c = 1, columns do
      for t = 1, len do            
        p[t] = params[inIdx+t-1];
      end
      local res,msg = _G[subfunction](len,p);
      result[outIdx] = res;
      inIdx = inIdx + len;
      outIdx = outIdx + 1;
    end
  end
  return result;
end





function GRIDX(subfunction, columns, rows, len, params, extParams)

  local result = {};

  -- print("GRIDX "..subfunction.." len "..len);
  -- for index, value in pairs(extParams) do
  --  print(index.." : "..value);
  -- end

  local inIdx = 1;
  local outIdx = 1;    
  local p = {};

  for r = 1, rows do
    for c = 1, columns do
      for t = 1, len do            
        p[t] = params[inIdx+t-1];
      end
      result[outIdx] = _G[subfunction](len,p,extParams);
      inIdx = inIdx + len;
      outIdx = outIdx + 1;
    end
  end
  return result;
end




function getFunctionNames(type)

    local functionNames = '';

    if (type == 8) then
        functionNames = "GRID,GRIDX";
    end

    return functionNames;

end
