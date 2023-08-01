#!/bin/sh
#
#
#       awk.rfc987 - is an awk program that reformats the
#       RFC-987 International tables into PP ones.
#
#
#       syntax :    awk.rfc987  input-table
#
######################################################################


AWK=/usr/bin/awk

FILE=$1

if [ -z "$FILE" ]
then
        echo " "
        echo "Usage:   awk.rfc987 input-table"
        echo " "
        exit 1
fi





#---------------------- Start of the awk routines  --------------------------#


$AWK <$FILE '
BEGIN   {
                FS ="#"
        }
/^#.*/  {
                print $0
		next
        }
        {
		if ((n = split($1, array, ":")) > 1) {
		   for (i = 1; i < n; i++)
		       printf("%s\:",array[i])
		   printf("%s", array[n]);
                } else
		   printf("%s", $1)

		printf(":");

		printf("%s", $2)
		for (i = 3; i < NF; i++)
		    printf("#%s", $i);
		printf("\n");
		next
        }
END     {}
'
