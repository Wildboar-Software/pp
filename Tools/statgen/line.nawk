#! /bin/sh

# nawk script for producing summary listings of stats
#
#

nawk '
BEGIN { 
	FS = "|"
}
NF > 1 {
	split ($1, parts, ":")
	if (NR == 1) {
		first_month = parts[1]; first_day = parts[2]
		first_hour = parts[3]
		first_min = parts[4] + 0
		first_sec = parts[5] + 0
	}
	last_month = parts[1]; last_day = parts[2]
	last_hour = parts[3]
	last_min = parts[4]
	last_sec = parts[5]
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
#   OR for positive & transit
# cchan|submit-time|queued-time
# 13    14	    15
	if (NF < 13 && NF > 15) {
		print "Bad record line", NR, $0 | "cat 1>&2"
		next
	}
	type = $3
	if (type == "positive")
		good_drs ++
	else if (type == "negative")
		bad_drs ++
	else if (type == "transit")
		trans_drs ++
	else print "Unknown DR type", type, NR, $0 | "cat 1>&2"
}
$2 == "ok" {
# Format
# date-time|ok|msgid|inchan|outchan|p1id|size|sender|inmta|recip|outmta|
# 1	    2  3     4	    5	    6	 7    8	     9     10    11
# msg|submit-time
# 12  13
	if (NF != 13) { 
		print "Bad record line NR", $0 | "cat 1>&2"
		next
	}

	total_addr ++
	msgid = $3
	inchan = $4
	outchan = $5
	size = $7
	inmta = $9
	outmta = $11
	if (im[msgid] == 0) {
		total_msgs ++
		total_size += size
		im[msgid] = 1
		inchan_size[inchan] += size
		if (ih[inchan inmta] == 0) {
			inchan_nhosts[inchan] ++
			ih[inchan inmta] = 1
		}
		if (ih[inmta] == 0) {
			total_inmta ++
			ih[inmta] = 1
		}
		inchan_nmsgs[inchan] ++
	}
	inchan_naddr[inchan] ++

	if (om[msgid outchan] == 0) {
		om[msgid outchan] = 1
		outchan_size[outchan] += size
		if (oh[outchan outmta] == 0) {
			outchan_nhosts[outchan] ++
			oh[outchan outmta] = 1
		}
		if (oh[outmta] == 0) {
			total_outmta ++
			oh[outmta] = 1
		}
		outchan_nmsgs[outchan] ++
	}
	outchan_naddr[outchan] ++;

	
	cp = inchan " -> " outchan
	chpair_naddr[cp] ++
	if (cm[msgid cp] == 0) {
		cm[msgid cp] = 1
		chpair_size[cp] += size
		chpair_nmsgs[cp] ++
	}
}
#BEGIN(nawk)
func bytes(n) {
	if (n < 1000)
		return sprintf ("%d", n)
	if (n < 1000000)
		return sprintf ("%.2fK", n / 1000.0)
	else return sprintf ("%.2fM", n / 1000000.0)
}
#END(nawk)
END {
	printf "SUMMARY for period %d/%d %d:%02d:%02d to %d/%d %d:%02d:%02d\n", \
		first_month, first_day, first_hour, first_min, first_sec, \
		last_month, last_day, last_hour, last_min, last_sec

	printf "%-20s: %s\n", "Messages", bytes(total_msgs)
	printf "%-20s: %s\n", "Addresses", bytes(total_addr)
	printf "%-20s: %s\n", "Bytes", bytes(total_size)
	
	printf "%-20s: %s\n", "Inbound MTAs", bytes(total_inmta)
	printf "%-20s: %s\n", "Outbound MTAs", bytes(total_outmta)
	if (good_drs)
		printf "%-20s: %s\n", "Positive DRs", bytes(good_drs)
	if (bad_drs)
		printf "%-20s: %s\n", "Negative Drs", bytes(bad_drs)
	if (trans_drs)
		printf "%-20s: %s\n", "Transiting Drs", bytes(trans_drs)
	print "\n"

	print "Inbound Channel Totals"
	title =  "Channel"
	maxw = length(title)
	for (i in inchan_naddr)
		if (length(i) > maxw)
			maxw = length(i)
	format = "%-" maxw "s %8d %8d %8s %8s\n"
	printf "%-" maxw "s %8s %8s %8s %8s\n", title, \
		"Msgs", "Addr", "Bytes", "MTAs"
	for (i in inchan_naddr)
		printf format, i, \
			inchan_nmsgs[i], inchan_naddr[i], \
			bytes(inchan_size[i]) ,	inchan_nhosts[i]

	print ""; print "";
	print "Outbound Channel Totals"
	title = "Channel"
	maxw = length(title)
	if (length(i) > maxw)
		maxw = length(i)
	
	for (i in outchan_naddr)
		if (length(i) > maxw)
			maxw = length(i)
	printf "%-" maxw "s %8s %8s %8s %8s\n", title,\
		"Msgs", "Addr", "Bytes", "MTAs"

	for (i in outchan_naddr)
		printf "%-" maxw "s %8d %8d %8s %8s\n", i, \
			outchan_nmsgs[i], outchan_naddr[i], \
			bytes(outchan_size[i]), outchan_nhosts[i]

	print ""; print ""
	print "Channel Pair Totals"
	title = "Channel"
	maxw = length(title)

	for (i in chpair_naddr)
		if (length(i) > maxw)
			maxw = length(i)
	printf "%-" maxw "s %8s %8s %8s\n", title, \
			"Msgs", "Addrs", "Bytes"
	for (i in chpair_naddr)
		printf "%-" maxw "s %8d %8d %8s\n", i, \
			chpair_nmsgs[i], chpair_naddr[i], \
			bytes(chpair_size[i])

}' $*
