# This script can be used for generating addition mappings. 

# Usage: generateAdditionalMappings.sh <inputFile>

# The new mapping lines are written to the output (stdout).

awk -F ";" '
{
  for (i=1; i<=NF; i++)
  {
    f[i] = $i;
  }
 
  n = split(f[4], p, "-");  
 
 
  if (p[n] == "K")  # Kelvin => Celsius
  {
      p[n] = "C";
      f[14] = "SUM{$,-273.15}";
      f[15] = "SUM{$,273.15}";
  }
  else
  if (p[n] == "0TO1")  # 0TO1 => Percent
  {
      p[n] = "PRCNT";
      f[14] = "MUL{$,100}";
      f[15] = "DIV{$,100}";
  }
  else
  if (p[n] == "PRCNT")  # Percent => 0TO1
  {
      p[n] = "0TO1";
      f[14] = "DIV{$,100}";
      f[15] = "MUL{$,100}";
  }
  else
  if (p[n] == "FT")  # Feet => Meter
  {
      p[n] = "M";
      f[14] = "MUL{$,0.3048}";
      f[15] = "DIV{$,0.3048}";
  }
  else
  if (p[n] == "M")  # Meter => Feet
  {
      p[n] = "FT";
      f[14] = "DIV{$,0.3048}";
      f[15] = "MUL{$,0.3048}";
  }
  else
  if (p[n] == "HPA")  # HehtoPascal => Pascal
  {
      p[n] = "PA";
      f[14] = "MUL{$,100}";
      f[15] = "DIV{$,100}";
  }
  else
  if (p[n] == "PA")  # Pascal => HehtoPascal
  {
      p[n] = "HPA";
      f[14] = "DIV{$,100}";
      f[15] = "MUL{$,100}";
  }
  else
  if (p[n] == "MS")  # Meters/sec => Knots
  {
      p[n] = "KT";
      f[14] = "MUL{$,1.94384449244}";
      f[15] = "DIV{$,1.94384449244}";
  }
  else
  if (p[n] == "KT")  # Knots => Meters/sec
  {
      p[n] = "MS";
      f[14] = "DIV{$,1.94384449244}";
      f[15] = "MUL{$,1.94384449244}";
  }
  else
  if (p[n] == "D")  # Degees => Radians
  {
      p[n] = "RAD";
      f[14] = "MUL{$,0.017453292}";
      f[15] = "DIV{$,0.017453292}";
  }
  else
  if (p[n] == "RAD")  # Radians => Degrees
  {
      p[n] = "D";
      f[14] = "DIV{$,0.017453292}";
      f[15] = "MUL{$,0.017453292}";
  }
  else
  {
#    print p[n];
    n = 0;
  }
 
  
  #################################################################################
  # Printing line
  #################################################################################
 
  if (NF < 10)
  {
    printf("%s\n",$0);
  }
  else
  {  
    if (n > 0 &&  f[1] > " ")
    {
      printf("%s;",f[1]);

      for (i=1; i<=n; i++)
      {
        if (i > 1)
          printf("-");
          
        printf("%s",p[i]);
	  }
      
      printf(";%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",f[3],f[4],f[5],f[6],f[7],f[8],f[9],f[10],f[11],f[12],f[13],f[14],f[15],f[16]);
    }
  }
   
}
' $1
