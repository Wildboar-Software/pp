#! /bin/sh

#
# Find who is the slowest of the slow...
#
#
nawk '
BEGIN { FS = "|" 
	days[0] = 0
	days[1] = 31
	days[2] = days[1] + 28
	days[3] = days[2] + 31
	days[4] = days[3] + 30
	days[5] = days[4] + 31
	days[6] = days[5] + 30
	days[7] = days[6] + 31
	days[8] = days[7] + 31
	days[9] = days[8] + 30
	days[10] = days[9] + 31
	days[11] = days[10] + 30
	days[12] = days[11] + 31
}
	{ ctime = $1
	  if(!starttime)
		starttime = ctime
	}
$2 == "Deliv" {
# Format
# date-time|Deliv|msgid|inchan|outchan|p1id|size|sender|inmta|recip|outmta|
# 1         2     3     4      5       6    7    8      9     10    11
# sub-time|queue-time
# 12       13
	next
}
$2 == "DR" {
# Format
# date-time|DR|type|msgid|p1id|size|inchan|dest|inhost|outchan|src|host|
# 1         2  3    4     5    6    7      8    9      10      11  12
# cchan|reason|diag|mesg
# 13    14     15   16
# cchan|submit-time|queued-time
# 13    14	    15

	next
}
$2 == "ok" {
# Format
# date-time|ok|msgid|inchan|outchan|p1id|size|sender|inmta|recip|outmta|
# 1	    2  3     4	    5	    6	 7    8	     9     10    11
# msg|submit-time
# 12  13
	inmta = $9
	msgid= $3
	subtime = $13
	if (inmtamsg[inmta msgid] == 0) {
		inmtamsg[inmta msg] = 1
		t1 = tdiff(subtime, ctime)
		if (! (inmta in mintime)) {
			mintime[inmta] = 999999999
			if (length(inmta) > maxmtalen)
				maxmtalen = length(inmta)
		}
		if (t1 < mintime[inmta])
			mintime[inmta] = t1
		if (t1 > maxtime[inmta])
			maxtime[inmta] = t1
		countmta[inmta] ++
		avrgtime[inmta] = \
		    (avrgtime[inmta] * (countmta[inmta]-1) / countmta[inmta])\
		    + t1/countmta[inmta]
	}
	next
}
	{ error( "ignore line " NR $0) }
func error(mesg) {
	print mesg | "cat 1>&2"
}

func line(c, n,		i, str) {
	for (i = 0; i < n; i++)
		str = str c
	print str
}
func tdiff(t1,t2,	time1,time2) {
	split(t1, t1pts, ":")
	split(t2, t2pts, ":")
	time1 = days[t1pts[1]-1] * 24 * 3600
	time1 += t1pts[2] * 24 * 3600
	time1 += t1pts[3] * 3600
	time1 += t1pts[4] * 60
	time1 += t1pts[5]
	time2 = days[t2pts[1]-1] * 24 * 3600
	time2 += t2pts[2] * 24 * 3600
	time2 += t2pts[3] * 3600
	time2 += t2pts[4] * 60
	time2 += t2pts[5]
	return time2 - time1
}
func totime(secs,	strtime) {
	if (secs < 0) {
		strtime = "-"
		secs = -secs
	}
	else strtime = ""
	if (secs > 60*60*24) {
		strtime = strtime sprintf("%2d ", secs/(60*60*24))
		secs %= 60*60*24
	}
	if (secs > 60*60) {
		strtime = strtime sprintf("%2d:", secs/(60*60))
		secs %= 60*60
	}
	if (secs > 60) {
		strtime = strtime sprintf(strtime ? "%02d:" : "%2d:", secs/60)
		secs %= 60
	}
	strtime = strtime sprintf(strtime ? "%02d" : "%2d", secs)
	return strtime
}	
END {
	line("=", 80)
	split(starttime, part1, ":")
	split(ctime, part2, ":")
	msg = sprintf(\
		"Statistics from %s/%s %d:%02d:%02d to %s/%s %d:%02d:%02d",\
		part1[1], part1[2], part1[3], part1[4], part1[5],\
		part2[1], part2[2], part2[3], part2[4], part2[5])
	printf "%" (length(msg) - 80) / 2 "s%s\n", "", msg
	line("=",80)

	printf "\n\t\tDelay time in receiving messages"

	printf "\n\n%-" maxmtalen "s %5s %12s %12s %12s\n",\
		"MTA", "Msgs", "Min", "Max", "Avrg"
	line("-", 80)
	for (i in countmta)
		printf "%-" maxmtalen "s %5d %12s %12s %12s\n", \
			i, countmta[i], \
			totime(mintime[i]), \
			totime(maxtime[i]), \
			totime(avrgtime[i])
}
' $*
