cat mapping_newbase.csv | awk -F ";" '{for (i=1; i<NF; i++) {f=$i; if (i==6) f="1"; if (NF > 10) if (i == 8 &&  ($7 == "3" || $7=="2")) printf("*/xxx/",$i);else {if (i==8 && length($8) == 5) printf("0");printf("%s/xxx/",f)}};printf("\n");}' | sort | uniq | awk -F "/" '{for (i=1; i<NF; i++) if ($i != "xxx") printf("%s;",$i);printf("\n");}'
