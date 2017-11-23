
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









function getFunctionNames(type)

  local functionNames = '';

  if (type == 2) then 
    functionNames = 'C2F,C2K,F2C,F2K,K2C,K2F,DEG2RAD,RAD2DEG';
  end
  if (type == 3) then 
    functionNames = 'TEST';
  end
  
  return functionNames;

end

