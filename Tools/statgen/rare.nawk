#! /bin/sh

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
	mtaout = $11
	size = $7
	msg = $3
	queuetime = $13
	storeit(mtaout, msg, queuetime, size)
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

	msg = $4
	mtain = $8
	mtaout = $12
	type = $3
	queuetime = $15
	size = $6
	if (type == "negative") {
	#	storeit(mtaout, msg, queuetime,size)
		drsnegative ++
	} else if (type == positive) {
		storeit(mtaout, msg, queuetime, size)
		drspositive ++
	} else if (type == "transit") {
		drsin ++;
		drsmta[mtain] ++
	}
	next
}
$2 == "ok" {
# Format
# date-time|ok|msgid|inchan|outchan|p1id|size|sender|inmta|recip|outmta|
# 1	    2  3     4	    5	    6	 7    8	     9     10    11
# msg|submit-time
# 12  13
	mtain = $9
	msg = $3
	size = $7
	mtaout = $11
	if(mtamsgin[mtain msg] == 0) {
		mtamsgin[mtain msg] = 1
		if (mtainbytes[mtain] == 0) {
			n = split(mtain, parts, ".")
			if (n > 1)
				domcom[mtain] = parts[n]
			else	domcom[mtain] = "local"
		}
		mtainbytes[mtain] += size
		mtainnumb[mtain] ++
	}
	next
}
	{ error( "ignore line " NR $0) }
func error(mesg) {
	print mesg | "cat 1>&2"
}

func storeit(mtaout, msg, queuetime,size) {
	if (mtamsgout[mtaout msg] == 0){
		mtamsgout[mtaout msg] = 1
		if (mtaoutbytes[mtaout] == 0) {
			n = split(mtaout, parts, ".")
			if (n > 1)
				domcom[mtaout] = parts[n]
			else	domcom[mtaout] = "local"
		}
		mtaoutbytes[mtaout] += size
		mtaoutnumb[mtaout] ++

		n = tdiff(queuetime, ctime)
		for (i = n; i <= 5; i++)
			elapsed[mtaout i] ++
	}
}
func line(c, n,		i, str) {
	for (i = 0; i < n; i++)
		str = str c
	print str
}
func percent(a,b) {
	if (b == 0) return 0;
	return a * 100 / b
}
func tdiff(t1,t2,	diff,time1,time2) {
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
	diff = time2 - time1
	if (diff < 60)
		return 1
	else if (diff < 5 * 60)
		return 2
	else if (diff < 3600)
		return 3
	else if (diff  < 3600*24)
		return 4
	else return 5
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
	msg = "Inbound statistics"
	print "\n\n"
	print msg
	line("-", length(msg))
	print "\n\nSource"
	partner = "Partner"
	MTA = "MTA"
	wid = length(MTA)
	for (i in mtainnumb)
		if (length(i) > wid)
			wid = length(i)
	wid2 = length(partner)
	for (i in domcom)
		if (length(domcom[i]) > wid2)
			wid2 = length(domcom[i])
	printf "%-" wid2 "s %-" wid "s %8s %10s\n", partner,\
			MTA, "#messg", "#kByte"
	line("-", 80)
	for (i in mtainnumb) {
		printf "%-" wid2 "s %-" wid "s %8d %10.2f\n", domcom[i], i,\
			mtainnumb[i], mtainbytes[i]/1000
		totbytes += mtainbytes[i]/1000.0
		totmsgs += mtainnumb[i]
	}
	line("-", 80)
	printf "%-" wid + wid2 "s  %8d %10.2f\n", "Total", totmsgs, totbytes

	msg = "Outbound statistics"
	print "\n\n"
	print msg
	line("-", length(msg))
	wid = length(MTA)
	for (i in mtaoutnumb)
		if (length(i) > wid)
			wid = length(i)

	printf "\n\n%-" wid + wid2 + 28 "s %s\n", "", \
			"% of messages"
	printf "%-" wid + wid2 + 27 "s %s\n", "Destination", "relayed within"
	printf "%-" wid2 "s %-" wid "s %8s %10s  %4s %4s %4s %4s\n", \
		partner, MTA, "#messg", "#kByte", "1m", "5m", "1h", "1d"
	totbytes = totmsgs = 0
	line("-", 80)
	for (i in mtaoutnumb) {
		printf "%-" wid2 "s %-" wid "s %8d %10.2f  %4d %4d %4d %4d\n",\
			domcom[i], i, mtaoutnumb[i], mtaoutbytes[i]/1000,\
			percent(elapsed[i 1], mtaoutnumb[i]), \
			percent(elapsed[i 2], mtaoutnumb[i]), \
			percent(elapsed[i 3], mtaoutnumb[i]), \
			percent(elapsed[i 4], mtaoutnumb[i])
		for (j = 1; j < 5; j++)
			telapsed[j] += elapsed[i j]
		totbytes += mtaoutbytes[i]/1000
		totmsgs += mtaoutnumb[i]
	}
	line("-", 80)
	printf "%-" wid + wid2 "s  %8d %10.2f  %4d %4d %4d %4d\n\n", \
			"Total", totmsgs, totbytes, \
			percent(telapsed[1], totmsgs), \
			percent(telapsed[2], totmsgs), \
			percent(telapsed[3], totmsgs), \
			percent(telapsed[4], totmsgs)

	if (drspositive || drsnegative)
		printf "Delivery reports generated: %d positive, %d negative\n", \
		drspositive, drsnegative
	if (drsin) 
		printf "Delivery reports relayed: %d\n", drsin
}
' $*
