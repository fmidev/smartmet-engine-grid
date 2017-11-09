
ParamValueMissing = 340282346638528859811704183484516925440;
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
    result.message = 'ABS() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;  
  end
  
  return result;
  
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
    result.message = 'AVG(): No parameters given!';
    result.value = 0;  
  end
    
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
      result.value = params[1] + 273.16;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'C2K() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;
  end
  
  return result;
  
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
    result.message = 'DIV() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;  
  end
    
  return result;

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
      result.value = params[1] - 273.16;
    else
      result.value = ParamValueMissing;
    end    
  else
    result.message = 'K2C() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;
  end
  
  return result;
  
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
    result.message = 'MAX(): No parameters given!';
    result.value = 0;  
  end
    
  return result;

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
    result.message = 'MIN(): No parameters given!';
    result.value = 0;  
  end
    
  return result;

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
    result.message = 'MUL() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;  
  end
    
  return result;

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
    result.message = 'NEG() : Invalid number of parameters given ('..numOfParameter..')!';
    result.value = 0;  
  end
  
  return result;
  
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
      end
    end
    result.message = 'OK';
    result.value = sum;
  else
    result.message = 'SUM(): No parameters given!';
    result.value = 0;  
  end
    
  return result;

end





function main(functionName,numOfParams,params)

  if (debug > 0) then  
    print('LUA CALL : '..functionName..'('..numOfParams..')');  
    for index, value in pairs(params) do
      print('['..index..']'..value);
    end
  end
  
  local result = {};
  
  if     (functionName == 'ABS') then result = ABS(numOfParams,params);
  elseif (functionName == 'AVG') then result = AVG(numOfParams,params);
  elseif (functionName == 'C2K') then result = C2K(numOfParams,params);
  elseif (functionName == 'DIV') then result = DIV(numOfParams,params);
  elseif (functionName == 'K2C') then result = K2C(numOfParams,params);
  elseif (functionName == 'MAX') then result = MAX(numOfParams,params);
  elseif (functionName == 'MIN') then result = MIN(numOfParams,params);
  elseif (functionName == 'MUL') then result = MUL(numOfParams,params);
  elseif (functionName == 'NEG') then result = NEG(numOfParams,params);
  elseif (functionName == 'SUM') then result = SUM(numOfParams,params);
  else
    result.message = 'Unknown function called ('..functionName..')';
    result.value = ParamValueMissing;
  end
  
  if (debug > 0) then  
    print('RESULT : '..result.value..' ('..result.message..')');
   end
   
  return result.value,result.message;
  
end
