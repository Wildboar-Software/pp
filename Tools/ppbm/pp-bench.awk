#! /bin/sh

nawk '
BEGIN { FS = "@" }
NF > 1 {
	if ($NF	== "T{") {
		for (i = 1; i < NF; i++)
			printf "%s@", $i
		
		while (getline > 0) {
			if ($1 == "T}") {
				for (i = 2; i <= NF; i++)
					printf "@%s", $i
				break
			}
			else printf "%s ", $0
		}
		printf "\n"
	}
	else
		print $0
}
' $* |
nawk '
BEGIN { FS = "@"; OFS = "@"
}
NF == 1 { next }
/tab\(/ { next }
$1 == "Model" { next }
	{
		if ($1 == "\\^") $1 = last1
		if ($2 == "\\^") $2 = last2
		print $1, $2, $6, $7
		last1 = $1
		last2 = $2
	}
' | sort -t@ +2nr |
nawk '
BEGIN {
	print ".TS H"
	print "tab(@);"
	print "c s s s"
	print "cfB cfB cfB cfB"
	print "a a n l."
	print "PP Benchmark Summary (sorted)"
	print ".sp .5"
	print "Model@Disc@M/s@Notes"
	print ".TH"
	print "="
}
	{ print }
END { print ".TE" }
