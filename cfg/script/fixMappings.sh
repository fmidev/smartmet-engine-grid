# This script can be used for fixing fields in mapping files. This is sometimes
# needed if the default configuration does not automatically produce correct
# mappings. On the other hand, this script can also be used for fixing or 
# removing old mapppings from the mapping files.

# Usage: fixMappings.sh <inputFile>

# The fixed lines are written to the output (stdout).

awk -F ";" '
{
  for (i=1; i<=NF; i++)
  {
    f[i] = $i;
  }

  #################################################################################
  # Mappings that depend on day/night time:
  #################################################################################
  
  params = "HESSAA-N;SMARTSYMBOL-N";   
  
  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    if (f[4] == p[i]) 
    {
      f[14] = "IFSUM{$dark,$,100}";
    }
  }
  
  #################################################################################
  # Mappings that require Kelvin to Celsius conversion.
  #################################################################################

  params = \
"Temperature,T-K;\
TemperatureF0,F0-T-K;\
TemperatureF10,F10-T-K;\
TemperatureF25,F25-T-K;\
TemperatureF75,F75-T-K;\
TemperatureF90,F90-T-K;\
TemperatureF100,F100-T-K;\
TemperatureSea,TSEA-K;\
TemperatureSeaF0,F0-TSEA-K;\
TemperatureSeaF2.5,F2.5-TSEA-K;\
TemperatureSeaF50,F50-TSEA-K;\
TemperatureSeaF97.5,F97.5-TSEA-K;\
TemperatureSeaF100,F100-TSEA-K;\
TemperatureDeviation,T-STDDEV-K;\
DevPoint,TD-K;\
DewPointF0,F0-TD-K;\
DewPointF50,F50-TD-K;\
DewPointF100,F100-TD-K;\
MaximumTemperature,TMAX-K;\
MaximumTemperature06,TMAX06-K;\
MaximumTemperature12,TMAX12-K;\
MaximumTemperature18,TMAX18-K;\
MaximumTemperature24,TMAX24-K;\
MinimumTemperature,TMIN-K;\
MinimumTemperature06,TMIN06-K;\
MinimumTemperature12,TMIN12-K;\
MinimumTemperature18,TMIN18-K;\
MinimumTemperature24,TMIN24-K;\
AverageTemperature,T-MEAN-K;\
PotentialTemperature,TP-K";

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    a = split(p[i],pp,",");
    if (a == 2  &&  f[2] == pp[1] &&  f[4] == pp[2]) 
    {
      f[14] = "SUM{$,-273.15}";
      f[15] = "SUM{$,273.15}";
    }
  }
  
  
  #################################################################################
  # Mappings that require Pascal to Millibar conversion.
  #################################################################################

  params = \
"PressureF0,F0-P-PA;\
PressureF100,F100-P-PA;\
PressureF10,F10-P-PA;\
PressureF25,F25-P-PA;\
PressureF50,F50-P-PA;\
PressureF75,F75-P-PA;\
PressureF90,F90-P-PA;\
Pressure,P-HPA;\
Pressure,P-PA;\
RaftIceThickness,IRAFTTHK-CM";

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    a = split(p[i],pp,",");
    if (a == 2  &&  f[2] == pp[1] &&  f[4] == pp[2]) 
    {
      f[14] = "MUL{$,0.010000}";
      f[15] = "DIV{$,0.010000}";
    }
  }
  
  #################################################################################
  # Mappings that require Foot to Meter conversion.
  #################################################################################

  params = \
"CloudTop3,CLDTOP-FT";

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    a = split(p[i],pp,",");
    if (a == 2  &&  f[2] == pp[1] &&  f[4] == pp[2]) 
    {
      f[14] = "MUL{$,0.3048}";
      f[15] = "DIV{$,0.3048}";
    }
  }

  #################################################################################
  # Mappings that require multiplication by 100
  #################################################################################

  params = \
"FrostProbability,PROB-FROST-1;\
HighCloudCover,NH-0TO1;\
Humidity,RH-0TO1;\
IceThickness,ITHK-M;\
LowCloudCover,NL-0TO1;\
MediumCloudCover,NM-0TO1;\
PoP,POP-0TO1;\
ProbabilityOfCBOrTCU,PROB-CBTCU-1;\
ProbabilityOfCeilingLimit1,PROB-CLFT-1;\
ProbabilityOfCeilingLimit2,PROB-CLFT-2;\
ProbabilityOfCeilingLimit3,PROB-CLFT-3;\
ProbabilityOfCeilingLimit4,PROB-CLFT-4;\
ProbabilityOfColdLimit5,PROB-TC-5;\
ProbabilityOfConvectivePrecipitationLimit1,PROB-CONV-RR3-1;\
ProbabilityOfConvectivePrecipitationLimit2,PROB-CONV-RR3-2;\
ProbabilityOfConvectivePrecipitationLimit3,PROB-CONV-RR3-3;\
ProbabilityOfConvectivePrecipitationLimit4,PROB-CONV-RR3-4;\
ProbabilityOfConvectivePrecipitationLimit5,PROB-CONV-RR3-5;\
ProbabilityOfConvectivePrecipitationLimit6,PROB-CONV-RR3-6;\
ProbabilityOfFreezingPrecForm,PROB-FRPREC;\
ProbabilityOfLVP,PROB-LVP-1;\
ProbabilityOfMUCAPE1040Limit1,PROB-CAPE1040-1;\
ProbabilityOfMUCAPE1040Limit2,PROB-CAPE1040-2;\
ProbabilityOfMUCAPE1040Limit3,PROB-CAPE1040-3;\
ProbabilityOfMUCAPE1040Limit4,PROB-CAPE1040-4;\
ProbabilityOfMUCAPE1040Limit5,PROB-CAPE1040-5;\
ProbabilityOfMUCAPELimit5,PROB-CAPE-5;\
ProbabilityOfPr6Limit1,PROB-RR6-1;\
ProbabilityOfPr6Limit2,PROB-RR6-2;\
ProbabilityOfPr6Limit3,PROB-RR6-3;\
ProbabilityOfPrecipitation3h01mm,PROB-RR3-2;\
ProbabilityOfPrecipitation3h05mm,PROB-RR3-3;\
ProbabilityOfPrecipitation3h0mm,PROB-RR3-1;\
ProbabilityOfPrecipitation3h1mm,PROB-RR3-4;\
ProbabilityOfPrecipitation3h2mm,PROB-RR3-5;\
ProbabilityOfPrecLimit1,PROB-RR-1;\
ProbabilityOfPrecLimit2,PROB-RR-2;\
ProbabilityOfPrecLimit3,PROB-RR-3;\
ProbabilityOfPrecLimit5,PROB-RR-5;\
ProbabilityOfPrecLimit6,PROB-RR-6;\
ProbabilityOfSn3Limit1,PROB-SN3-1;\
ProbabilityOfSn3Limit2,PROB-SN3-2;\
ProbabilityOfSn3Limit3,PROB-SN3-3;\
ProbabilityOfSn3Limit4,PROB-SN3-4;\
ProbabilityOfSn3Limit5,PROB-SN3-5;\
ProbabilityOfSn3Limit6,PROB-SN3-6;\
ProbabilityOfVisibility2Limit1,PROB-VV2-1;\
ProbabilityOfVisibility2Limit2,PROB-VV2-2;\
ProbabilityOfVisibility2Limit3,PROB-VV2-3;\
ProbabilityOfVisibility2Limit4,PROB-VV2-4;\
ProbabilityOfVisibility2Limit5,PROB-VV2-5;\
ProbabilityOfVisibilityLimit1,PROB-VV-1;\
ProbabilityOfVisibilityLimit2,PROB-VV-2;\
ProbabilityOfVisibilityLimit3,PROB-VV-3;\
ProbabilityOfVisibilityLimit4,PROB-VV-4;\
ProbabilityOfVisibilityLimit5,PROB-VV-5;\
SevereFrostProbability,PROB-FROST-2;\
TotalCloudCover,N-0TO1;\
TotalCloudCover,N-PRCNT"

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    a = split(p[i],pp,",");
    if (a == 2  &&  f[2] == pp[1] &&  f[4] == pp[2]) 
    {
      f[14] = "MUL{$,100.0}";
      f[15] = "DIV{$,100.0}";
    }
  }

  #################################################################################
  # Mappings that require multiplication by 1000
  #################################################################################

  params = \
"SpecificHumidity,Q-KGKG;\
VerticalVelocityMMS,VV-MS;\
WaterEquivalentOfSnow,SD-TM2";

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    a = split(p[i],pp,",");
    if (a == 2  &&  f[2] == pp[1] &&  f[4] == pp[2]) 
    {
      f[14] = "MUL{$,1000.0}";
      f[15] = "DIV{$,1000.0}";
    }
  }

  
  #################################################################################
  # Mappings that should not be interpolated.
  #################################################################################

  params = \
"DD-D;\
DW1-D;\
DW2-D;\
DW3-D;\
DW-D;\
BLTURB-N;\
CLDSYM-N;\
CLDTYPE-N;\
ENSMEMB-N;\
FOGSYM-N;\
FORESTTYPE-N;\
HSADE1-N;\
ICETYPE-N;\
ICING-N;\
ICINGWARN-N;\
ILSAA1-N;\
LI-N;\
P-N;\
POTPRECF-N;\
POTPRECT-N;\
PRECFORM2-N;\
PRECFORM3-N;\
PRECFORM4-N;\
PRECFORM-N;\
PRECTYPE-N;\
SNDACC-N;\
SNOWDRIFT-N;\
SOILTY-N;\
SSICING-N;\
TOPL2-N;\
TOPL3-N;\
VEGET-N";   

  n = split(params,p,";");   
  for (i=1; i<=n; i++)
  {
    if (f[4] == p[i]) 
    {
      f[9] = "2";
      f[10] = "2";
      f[11] = "2";
    }
  }

  #################################################################################
  # Old producers that should be removed.
  #################################################################################

  deleted_producers = "HL2;HL2MTA";   

  n = split(deleted_producers,p,";");   
  for (i=1; i<=n; i++)
  {
    if (f[1] == p[i]) 
    {
      f[1] = "";
    }
  }

  #################################################################################
  # Old geometries that should be removed.
  #################################################################################

  deleted_geometries = "";   

  n = split(deleted_geometries,p,";");   
  for (i=1; i<=n; i++)
  {
    if (f[1] == p[i]) 
    {
      f[1] = "";
    }
  }
  
  
  #################################################################################
  # Printing lines
  #################################################################################
 
  if (NF < 10)
  {
    printf("%s\n",$0);
  }
  else
  {  
    if (f[1] > " ")
      printf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",f[1],f[2],f[3],f[4],f[5],f[6],f[7],f[8],f[9],f[10],f[11],f[12],f[13],f[14],f[15],f[16]);
  }
   
}
' $1
