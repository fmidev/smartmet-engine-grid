
PI = 3.1415926;
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
--  FUNCTION : SSI / SummerSimmerIndex
-- ***********************************************************************
-- ***********************************************************************

function SSI(columns,rows,rhArray,tempArray)

  local result = {};
  local simmer_limit = 14.5;


  for index, rh in pairs(rhArray) do

    local t = tempArray[index]-273.15;

    if (t <= simmer_limit) then
      result[index] = t;
    end

    if (rh == ParamValueMissing) then 
      result[index] = ParamValueMissing;
    end

    if (t == ParamValueMissing) then 
      result[index] = ParamValueMissing;
    end

    local rh_ref = 50.0 / 100.0;
    local r = rh / 100.0;

    result[index] =  (1.8 * t - 0.55 * (1 - r) * (1.8 * t - 26) - 0.55 * (1 - rh_ref) * 26) / (1.8 * (1 - 0.55 * (1 - rh_ref)));
  end
           
  return result;

end




-- ***********************************************************************
--  FUNCTION : SSI_COUNT / SummerSimmerIndex
-- ***********************************************************************
--  This is an internal function used by FEELS_LIKE function.
-- ***********************************************************************

function SSI_COUNT(rh,t)

  local simmer_limit = 14.5;

  if (t <= simmer_limit) then 
    return t;
  end

  if (rh == ParamValueMissing) then 
    return ParamValueMissing;
  end

  if (t == ParamValueMissing) then 
    return ParamValueMissing;
  end

  local rh_ref = 50.0 / 100.0;
  local r = rh / 100.0;

  local value =  (1.8 * t - 0.55 * (1 - r) * (1.8 * t - 26) - 0.55 * (1 - rh_ref) * 26) / (1.8 * (1 - 0.55 * (1 - rh_ref)));         
  return value;

end



-- ***********************************************************************
--  FUNCTION : FEELS_LIKE
-- ***********************************************************************

function FEELS_LIKE(columns,rows,tempArray,windArray,rhArray,radArray)

  local result = {};

  --if (numOfParams ~= 4) then
  --  result.message = 'Invalid number of parameters!';
  --  result.value = 0;  
  --  return result.value,result.message;
  --end

  for index, wind in pairs(windArray) do

    local temp = tempArray[index]-273.15;
    local rh = rhArray[index];      
    local rad = radArray[index];      
            
    -- Calculate adjusted wind chill portion. Note that even though
    -- the Canadien formula uses km/h, we use m/s and have fitted
    -- the coefficients accordingly. Note that (a*w)^0.16 = c*w^16,
    -- i.e. just get another coefficient c for the wind reduced to 1.5 meters.

    local a = 15.0;   -- using this the two wind chills are good match at T=0
    local t0 = 37.0;  -- wind chill is horizontal at this T

    local chill = a + (1 - a / t0) * temp + a / t0 * math.pow(wind + 1, 0.16) * (temp - t0);

    -- Heat index

    local heat = SSI_COUNT(rh, temp);

    -- Add the two corrections together

    local feels = temp + (chill - temp) + (heat - temp);

    -- Radiation correction done only when radiation is available
    -- Based on the Steadman formula for Apparent temperature,
    -- we just inore the water vapour pressure adjustment

    if (rad ~= ParamValueMissing) then
  
      -- Chosen so that at wind=0 and rad=800 the effect is 4 degrees
      -- At rad=50 the effect is then zero degrees
    
      local absorption = 0.07;
      feels = feels + 0.7 * absorption * rad / (wind + 10) - 0.25;

    end
    
    result[index] = feels;
    
  end

  return result;
  
end




-- ***********************************************************************
--  FUNCTION : FEELS_LIKE_HL2
-- ***********************************************************************

function FEELS_LIKE_HL2(columns,rows,tempArray,windArray,rhArray,radArray)

  local result = {};

  --if (numOfParams ~= 4) then
  --  result.message = 'Invalid number of parameters!';
  --  result.value = 0;  
  --  return result.value,result.message;
  --end

  for index, wind in pairs(windArray) do

    local temp = tempArray[index]-273.15;
    local rh = rhArray[index] * 100;      
    local rad = radArray[index];      
            
    -- Calculate adjusted wind chill portion. Note that even though
    -- the Canadien formula uses km/h, we use m/s and have fitted
    -- the coefficients accordingly. Note that (a*w)^0.16 = c*w^16,
    -- i.e. just get another coefficient c for the wind reduced to 1.5 meters.

    local a = 15.0;   -- using this the two wind chills are good match at T=0
    local t0 = 37.0;  -- wind chill is horizontal at this T

    local chill = a + (1 - a / t0) * temp + a / t0 * math.pow(wind + 1, 0.16) * (temp - t0);

    -- Heat index

    local heat = SSI_COUNT(rh, temp);

    -- Add the two corrections together

    local feels = temp + (chill - temp) + (heat - temp);

    -- Radiation correction done only when radiation is available
    -- Based on the Steadman formula for Apparent temperature,
    -- we just inore the water vapour pressure adjustment

    if (rad ~= ParamValueMissing) then
  
      -- Chosen so that at wind=0 and rad=800 the effect is 4 degrees
      -- At rad=50 the effect is then zero degrees
    
      local absorption = 0.07;
      feels = feels + 0.7 * absorption * rad / (wind + 10) - 0.25;

    end
    
    result[index] = feels;
    
  end

  return result;
  
end



-- ***********************************************************************
--  FUNCTION : WIND_CHILL
-- ***********************************************************************
-- Return the wind chill, e.g., the equivalent no-wind temperature
-- felt by a human for the given wind speed.
--
-- The formula is the new official one at FMI taken into use in 12.2003.
-- See: http://climate.weather.gc.ca/climate_normals/normals_documentation_e.html
--
-- Note that Canadian formula uses km/h:
--
-- W = 13.12 + 0.6215 × Tair - 11.37 × V10^0.16 + 0.3965 × Tair × V10^0.16
-- W = Tair + [(-1.59 + 0.1345 × Tair)/5] × V10m, when V10m < 5 km/h
--
-- \param wind The observed wind speed in m/s
-- \param temp The observed temperature in degrees Celsius
-- \return Equivalent no-wind temperature
-- ***********************************************************************

function WIND_CHILL(columns,rows,tempArray,windArray)

  local result = {};

  for index, wind in pairs(windArray) do

    local temp = tempArray[index]-273.15;
    if (wind == ParamValueMissing or temp == ParamValueMissing or wind < 0) then
      result[index] = ParamValueMissing;
    else
      local kmh = wind * 3.6;

      if (kmh < 5.0) then
        result[index] = temp + (-1.59 + 0.1345 * temp) / 5.0 * kmh;
      else
        local wpow = math.pow(kmh, 0.16);
        result[index] = 13.12 + 0.6215 * temp - 11.37 * wpow + 0.3965 * temp * wpow;
      end
    end
  end
  
  return result;
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
--  FUNCTION : WIND_SPEED
-- ***********************************************************************
--  Counts the size of the hypotenuse assuming that params1 and params2
--  represents vectors and the angle between them is 90 degrees.
-- ***********************************************************************

function WIND_SPEED(columns,rows,params1,params2)

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
--  FUNCTION : WIND_DIR
-- ***********************************************************************
--  Counts the direction of the wind (wind blows FROM this direction, not
--  TO this direction). 
-- ***********************************************************************

function WIND_DIR(columns,rows,u,v,angles)

  local result = {};
  
  for index, value in pairs(v) do

    a = angles[index];
    b = math.atan(v[index]/u[index]);      
            
    if (u[index] >= 0  and  v[index] >= 0) then
      c = b;
    end
	      
    if (u[index] < 0  and  v[index] >= 0) then
      c = b + PI;
    end
	      
    if (u[index] < 0  and  v[index] < 0) then
      c = b + PI;
    end
	            
    if (u[index] >= 0  and  v[index] < 0) then
      c = b;      
    end
	                
    if (a < (PI/2)) then
      d = c - a
	else
      d = c - (PI-a);
	end   
	
	result[index] = 270-((d*180)/PI);
	
  end
   
  return result;
  
end



-- ***********************************************************************
--  FUNCTION : WIND_TO_DIR
-- ***********************************************************************
--  Counts the direction of the wind (wind blows TO this direction, not
--  FROM this direction). 

-- ***********************************************************************

function WIND_TO_DIR(columns,rows,u,v,angles)

  local result = {};
  
  for index, value in pairs(v) do

    a = angles[index];
    b = math.atan(v[index]/u[index]);      
            
    if (u[index] >= 0  and  v[index] >= 0) then
      c = b;
    end
	      
    if (u[index] < 0  and  v[index] >= 0) then
      c = b + PI;
    end
	      
    if (u[index] < 0  and  v[index] < 0) then
      c = b + PI;
    end
	            
    if (u[index] >= 0  and  v[index] < 0) then
      c = b;      
    end
	                
    if (a < (PI/2)) then
      d = c - a
	else
      d = c - (PI-a);
	end   
	
	result[index] = ((d*180)/PI);
	
  end
   
  return result;
  
end



-- ***********************************************************************
--  FUNCTION : WIND_V
-- ***********************************************************************
-- ***********************************************************************

function WIND_V(columns,rows,u,v,angles)

  local result = {};
  
  for index, value in pairs(v) do

    a = angles[index];
    b = math.atan(v[index]/u[index]);      
    hh = math.sqrt(u[index]*u[index] + v[index]*v[index]);     
            
    if (u[index] >= 0  and  v[index] >= 0) then
      c = b;
    end
	      
    if (u[index] < 0  and  v[index] >= 0) then
      c = b + PI;
    end
	      
    if (u[index] < 0  and  v[index] < 0) then
      c = b + PI;
    end
	            
    if (u[index] >= 0  and  v[index] < 0) then
      c = b;      
    end
	                
    if (a < (PI/2)) then
      d = c - a
	else
      d = c - (PI-a);
	end   
	
  val = hh * math.sin(d);         
	result[index] = val;
	
  end
    
  return result;
  
end




-- ***********************************************************************
--  FUNCTION : WIND_U
-- ***********************************************************************

function WIND_U(columns,rows,u,v,angles)

  local result = {};
  
  for index, value in pairs(v) do

    a = angles[index];
    b = math.atan(v[index]/u[index]);      
    hh = math.sqrt(u[index]*u[index] + v[index]*v[index]);     
            
    if (u[index] >= 0  and  v[index] >= 0) then
      c = b;
    end
	      
    if (u[index] < 0  and  v[index] >= 0) then
      c = b + PI;
    end
	      
    if (u[index] < 0  and  v[index] < 0) then
      c = b + PI;
    end
	            
    if (u[index] >= 0  and  v[index] < 0) then
      c = b;      
    end
	                
    if (a < (PI/2)) then
      d = c - a
	else
      d = c - (PI-a);
	end   
	
    val = hh * math.cos(d);         
	result[index] = val;
	
  end
    
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
--    Type 7 : 
--      Function takes four parameters as input:
--        - columns       => Number of the columns in the grid
--        - rows          => Number of the rows in the grid
--        - params1       => Grid 1 values (= Array of float values)
--        - params2       => Grid 2 values (= Array of float values)
--        - params3       => Grid 3 values (= Array of float values)
--      Function return one parameter:
--        - result array  => Array of float values (must have the same 
--                           number of values as the input 'params1'.               
--  
--  
-- ***********************************************************************
 

function getFunctionNames(type)

  local functionNames = '';

  if (type == 2) then 
    functionNames = 'C2F,C2K,F2C,F2K,K2C,K2F,DEG2RAD,RAD2DEG';
  end
  if (type == 3) then 
    functionNames = 'SSI,WIND_CHILL,WIND_SPEED';
  end
  if (type == 4) then 
    functionNames = 'WIND_V,WIND_U,WIND_DIR,WIND_TO_DIR';
  end
  if (type == 8) then 
    functionNames = 'FEELS_LIKE,FEELS_LIKE_HL2';
  end
  
  return functionNames;

end

