
ParamValueMissing = -16777216;
debug = 0;

-- ***********************************************************************
--  INTERPOLATION METHODS
-- ***********************************************************************
--  0   None
--  1   Linear
--  2   Nearest
--  10  External

--  50  Wind direction





-- ***********************************************************************
--  FUNCTION : interpolate_none
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_none(x,y,val_q11,val_q21,val_q22,val_q12)

  return val_q11;

end





-- ***********************************************************************
--  FUNCTION : interpolate_nearest
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_nearest(x,y,val_q11,val_q21,val_q22,val_q12)

  local dist_x1 = x;
  local dist_y1 = y;
  local dist_x2 = 1- dist_x1;
  local dist_y2 = 1- dist_y1;        

  if (dist_x1 == 0  and  dist_y1 == 0) then
    return val_q11;
  end

  local dist_q11 = (dist_x1)*(dist_x1) + (dist_y1)*(dist_y1);
  local dist_q21 = (dist_x2)*(dist_x2) + (dist_y1)*(dist_y1);
  local dist_q12 = (dist_x1)*(dist_x1) + (dist_y2)*(dist_y2);
  local dist_q22 = (dist_x2)*(dist_x2) + (dist_y2)*(dist_y2);

  if (dist_q11 < dist_q21  and  dist_q11 <= dist_q12 and dist_q11 <= dist_q22) then
    return val_q11;
  end

  if (dist_q21 < dist_q11  and  dist_q21 <= dist_q12 and dist_q21 <= dist_q22) then
    return val_q21;
  end

  if (dist_q12 < dist_q11  and  dist_q12 <= dist_q21  and  dist_q12 <= dist_q22) then
    return val_q12;
  end

  return val_q22;

end





-- ***********************************************************************
--  FUNCTION : interpolate_linear
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_linear(x,y,val_q11,val_q21,val_q22,val_q12)

  local dist_x1 = x;
  local dist_y1 = y;
  local dist_x2 = 1- dist_x1;
  local dist_y2 = 1- dist_y1;        

  -- If the given point is close enough the actual grid point
  -- then there is no need for interpolation.

  local closeDist = 0.1;

  if (dist_x1 <= closeDist  and  dist_y1 <= closeDist) then
    return val_q11;
  end

  if (dist_x1 <= closeDist  and  dist_y2 <= closeDist) then
    return val_q12;
  end

  if (dist_x2 <= closeDist and dist_y1 <= closeDist) then
    return val_q21;
  end

  if (dist_x2 <= closeDist  and  dist_y2 <= closeDist) then
    return val_q22;
  end

  -- If the given point is on the border then we can do simple
  -- linear interpolation.

  if (dist_x1 == 0) then
      
    --  Linear interpolation x1,y1 - x1,y2
    if (val_q11 ~= ParamValueMissing  and  val_q12 ~= ParamValueMissing) then
      return (dist_y1*val_q11 + dist_y2*val_q12);  
    end

    return ParamValueMissing;  
  end

  if (dist_x2 == 0) then
    -- Linear interpolation x2,y1 - x2,y2
    if (val_q21 ~= ParamValueMissing  and  val_q22 ~= ParamValueMissing) then
      return (dist_y1*val_q21 + dist_y2*val_q22);  
    end

    return ParamValueMissing;  
  end

  if (dist_y1 == 0) then
    -- Linear interpolation x1,y1 - x2,y1
    if (val_q11 ~= ParamValueMissing  and  val_q21 ~= ParamValueMissing) then
      return (dist_x1*val_q11 + dist_x2*val_q21);
    end

    return ParamValueMissing;  
  end

  if (dist_y2 == 0) then
    -- Linear interpolation x1,y2 - x2,y2
    if (val_q12 ~= ParamValueMissing  and  val_q22 ~= ParamValueMissing) then
      return (dist_x1*val_q12 + dist_x2*val_q22);
    end

    return ParamValueMissing;  
  end

  -- Bilinear interpolation

  if (val_q11 ~= ParamValueMissing and val_q21 ~= ParamValueMissing  and val_q12 ~= ParamValueMissing and  val_q22 ~= ParamValueMissing) then
    -- All corners have a value.

    local fy1 = dist_x2*val_q11 + dist_x1*val_q21;
    local fy2 = dist_x2*val_q12 + dist_x1*val_q22;
    local f = dist_y2*fy1 + dist_y1*fy2;
    return f;
  end

  -- Three corners have a value (triangular interpolation).

  if (val_q11 == ParamValueMissing and val_q21 ~= ParamValueMissing  and  val_q12 ~= ParamValueMissing and  val_q22 ~= ParamValueMissing) then   
    local wsum = (dist_x2 * dist_y1 + dist_x1 * dist_y1 + dist_x1 * dist_y2);
    local f =  ((dist_x1 * dist_y2 * val_q21 + dist_x2 * dist_y1 * val_q12 + dist_x1 * dist_y1 * val_q22) / wsum);
    return f;          
  end

  if (val_q11 ~= ParamValueMissing and val_q21 == ParamValueMissing  and val_q12 ~= ParamValueMissing and  val_q22 ~= ParamValueMissing) then    
    local wsum = (dist_x2 * dist_y2 + dist_x2 * dist_y1 + dist_x1 * dist_y1);
    local f = ((dist_x2 * dist_y2 * val_q11 + dist_x2 * dist_y1 * val_q12 + dist_x1 * dist_y1 * val_q22) / wsum);
    return f;          
  end

  if (val_q11 ~= ParamValueMissing and val_q21 ~= ParamValueMissing  and val_q12 == ParamValueMissing and  val_q22 ~= ParamValueMissing) then
    local wsum = (dist_x1 * dist_y1 + dist_x2 * dist_y2 + dist_x1 * dist_y2);
    local f = ((dist_x2 * dist_y2 * val_q11 + dist_x1 * dist_y2 * val_q21 + dist_x1 * dist_y1 * val_q22) / wsum);
    return f;          
  end

  if (val_q11 ~= ParamValueMissing and val_q21 ~= ParamValueMissing  and val_q12 ~= ParamValueMissing and  val_q22 == ParamValueMissing) then
    local wsum = (dist_x2 * dist_y1 + dist_x2 * dist_y2 + dist_x1 * dist_y2);
    local f = ((dist_x2 * dist_y2 * val_q11 + dist_x1 * dist_y2 * val_q21 + dist_x2 * dist_y2 * val_q12) / wsum);
    return f;          
  end

  return ParamValueMissing;
  
end




-- ***********************************************************************
--  FUNCTION : interpolate_windDirection
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_windDirection(x,y,val_q11,val_q21,val_q22,val_q12)

  return interpolate_nearest(x,y,val_q11,val_q21,val_q22,val_q12);  

end




-- ***********************************************************************
--  FUNCTION : interpolate_windDirectionWithSpeed
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_windDirectionWithSpeedX(windDir,windSpeed,weight)

    local count = 0;
    local bestWeight = 0;
    local weightSum = 0;    
    local speedSum = 0;
    local speedSumX = 0;
    local speedSumY = 0;
    
    local PI = 3.1415926;
    
    for i, wd in pairs(windDir) do
	  
	  -- print(windDir[i].." "..windSpeed[i].." "..weight[i]);
	  
	  if (count == 0 or weight[i] > bestWeight) then
        bestDirection = windDir[i];
        bestWeight = weight[i];
      end
      
      weightSum = weightSum + weight[i];
      speedSum = speedSum + (weight[i] * windSpeed[i]); 

      local dir = windDir[i]*PI / 180;
      speedSumX = speedSumX + (weight[i] * math.cos(dir));
      speedSumY = speedSumY + (weight[i] * math.sin(dir));
      
      count = count + 1;

    end


  if (count == 0 or weightSum == 0) then  
    return ParamValueMissing;
  end


  local x = speedSumX / weightSum;
  local y = speedSumY / weightSum;

  -- print("XY "..x.." "..y.." BEST "..bestDirection.." "..bestWeight.." SUM "..speedSumX.." "..speedSumY.." WSUM"..weightSum);
  -- If there is almost exact cancellation, return best
  -- weighted direction instead.

  if (math.sqrt(x * x + y * y) < 0.01) then 
  
    return bestDirection;
  end

  -- Otherwise use the 2D mean

  local dir = 180 * math.atan2(y, x) / PI;
  if (dir < 0) then
    dir = dir + 360;
  end
  
  return dir;

end




-- ***********************************************************************
--  FUNCTION : interpolate_windDirectionWithSpeed
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function interpolate_windDirectionWithSpeed(x,y,wd_q11,wd_q21,wd_q22,wd_q12,ws_q11,ws_q21,ws_q22,ws_q12)
 
  local windDir = {};
  windDir[1] =  wd_q11;
  windDir[2] =  wd_q21;
  windDir[3] =  wd_q22;
  windDir[4] =  wd_q12;
  
  local windSpeed = {};
  windSpeed[1] =  ws_q11;
  windSpeed[2] =  ws_q21;
  windSpeed[3] =  ws_q22;
  windSpeed[4] =  ws_q12;

  local weight = {};
  weight[1] = (1-x) * (1-y);
  weight[2] = x * (1-y);
  weight[3] = x * y;
  weight[4] = (1-x)*y;

  return interpolate_windDirectionWithSpeedX(windDir,windSpeed,weight)

end



-- ***********************************************************************
--  FUNCTION : IPL_NONE
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function IPL_NONE(numOfParams,params)

  local result = {};
  
  if (numOfParams ~= 7) then   
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;  
    return result.value,result.message;
  end
    
  local n = params[1];
  local x = params[2];
  local y = params[3];
  local val_q11 = params[4];
  local val_q21 = params[5];
  local val_q22 = params[6];
  local val_q12 = params[7];
        

  result.message = 'OK';
  result.value = interpolate_none(x,y,val_q11,val_q21,val_q22,val_q12);  
  return result.value,result.message;
  
end





-- ***********************************************************************
--  FUNCTION : IPL_LINEAR
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function IPL_LINEAR(numOfParams,params)

  local result = {};
  
  if (numOfParams ~= 7) then   
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;  
    return result.value,result.message;
  end
    
  local n = params[1];
  local x = params[2];
  local y = params[3];
  local val_q11 = params[4];
  local val_q21 = params[5];
  local val_q22 = params[6];
  local val_q12 = params[7];
        

  result.message = 'OK';
  result.value = interpolate_linear(x,y,val_q11,val_q21,val_q22,val_q12);  
  return result.value,result.message;
  
end





-- ***********************************************************************
--  FUNCTION : IPL_NEAREST
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function IPL_NEAREST(numOfParams,params)

  local result = {};
  
  if (numOfParams ~= 7) then   
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;  
    return result.value,result.message;
  end
    
  local n = params[1];
  local x = params[2];
  local y = params[3];
  local val_q11 = params[4];
  local val_q21 = params[5];
  local val_q22 = params[6];
  local val_q12 = params[7];
        

  result.message = 'OK';
  result.value = interpolate_nearest(x,y,val_q11,val_q21,val_q22,val_q12);  
  return result.value,result.message;
  
end




-- ***********************************************************************
--  FUNCTION : IPL_WIND_DIR
-- ***********************************************************************
--  The function returns the interpolate value.
-- ***********************************************************************

function IPL_WIND_DIR(numOfParams,params)

  -- for index, value in pairs(params) do
  --   print(index.." : "..value);
  -- end

  local result = {};
  
  if (numOfParams ~= 7 and numOfParams ~= 14) then   
    result.message = 'Invalid number of parameters given ('..numOfParams..')!';
    result.value = 0;  
    return result.value,result.message;
  end
    
  result.message = 'OK';
    
  if (numOfParams == 7) then   
    local n = params[1];
    local x = params[2];
    local y = params[3];
    local val_q11 = params[4];
    local val_q21 = params[5];
    local val_q22 = params[6];
    local val_q12 = params[7];        
    result.value = interpolate_windDirection(x,y,val_q11,val_q21,val_q22,val_q12);
  else  
    local n = params[1];
    local x = params[2];
    local y = params[3];
    local a_val_q11 = params[4];
    local a_val_q21 = params[5];
    local a_val_q22 = params[6];
    local a_val_q12 = params[7];       
    local b_val_q11 = params[11];
    local b_val_q21 = params[12];
    local b_val_q22 = params[13];
    local b_val_q12 = params[14];       
    result.value = interpolate_windDirectionWithSpeed(x,y,a_val_q11,a_val_q21,a_val_q22,a_val_q12,b_val_q11,b_val_q21,b_val_q22,b_val_q12);
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
-- ***********************************************************************


function getFunctionNames(type)

  local functionNames = '';

  if (type == 1) then 
    functionNames = 'IPL_NONE,IPL_LINEAR,IPL_NEAREST,IPL_WIND_DIR';
  end
  
  return functionNames;

end


