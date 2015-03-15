#!/bin/bash

# enum genera la media e il sigma. Oltre che contare il numero di valori
read SIGMA MEAN SAMPLES <<< `./enum`


/usr/bin/gnuplot <<EOF
 reset
set terminal svg
set terminal svg size 1024,768 fname 'Verdana' fsize 10
set o "volt.svg"
set title 'Distribuzione reale e teorica $SAMPLES valori sigma=$SIGMA mean=$MEAN'
set key reverse Left outside
invsqrt2pi = 0.398942280401433
normal(x,mu,sigma)=sigma<=0?1/0:invsqrt2pi/sigma*exp(-0.5*((x-mu)/sigma)**2) #ecco la varianza
set grid

set boxwidth 0.75 absolute #per le barre
set style fill solid 1.0 noborder

bin_width = 1;

bin_number(x) = floor(x/bin_width)

rounded(x) = bin_width * ( bin_number(x) + 0.5 )

#plot 'gaussian.txt' using (rounded($1)):(1) smooth frequency with boxes



set style data linespoints

#plot "volt-gnuplot" using 2:(normal(\$2,$MEAN,$SIGMA)) title "Teorica" axes x1y1 w lines, \
#     "" using 2:(\$1/$SAMPLES) title "Reale" axes x1y2 w lines

plot '<sort -rn volt.log|uniq -c|grep -v " 0\.0"' using 2:(normal(\$2,$MEAN,$SIGMA)) title "Teorica" axes x1y1 w lines, \
     "" using 2:(\$1/$SAMPLES) title "Reale" axes x1y2 w lines

#plot 'volt1.log' using (rounded(\$1)):(1) smooth frequency with boxes title "Reale" axes x1y1, \
#     '<sort -rn volt1.log|uniq -c' using 2:(normal(\$2,$MEAN,$SIGMA)) title "Teorica" axes x1y2 w lines
     


EOF


scp volt.svg reario@insomnia247.nl:/home/reario/volt.svg
