.\" $Header: /xtel/pp/pp-beta/RCS/pp-changes.ms,v 6.0 1991/12/18 20:02:50 jpo Rel $
.\"
.\" $Log: pp-changes.ms,v $
.\" Revision 6.0  1991/12/18  20:02:50  jpo
.\" Release 6.0
.\"
.\"
.\"
.ds Pv 6.0
.ds Lv 5.2
.ds Iv 7.0
.ds Il 6.0
.ds Ii 6.8
.de Bu
.IP \(bu
..
.de Ip
.IP "\fB\\$1\fP\0\0"
..
.TL
Changes from PP version \*(Lv to \*(Pv
.NH
Overview
.LP
This short document is a description of the more user/administrator
visible changes that have taken place in PP between the last interim
version (\*(Lv) and the beta version (\*(Pv). This document is not
exhaustive or complete but is a guide to what to look out for. In
general the following may be assumed.
.Bu
The Queue files are compatible between \*(Lv and \*(Pv.
Note however that the Queue structure has changed. Old channels will
have to be recompiled to recognise the new format, but new channels
will read old queue files.
.Bu
The qmgr protocol is compatible between \*(Lv and \*(Pv
.Bu
The syntax of some of the table files has changed. Where appropriate
conversion scripts are provided.
.Bu
Some procedure calls have been changed or have different parameters.
.Bu
the libdsap.a library is now required for compilation.
.LP
In summary, you should be careful when upgrading to the \*(Pv release.
It will not work by just replacing the \*(Lv binaries with \*(Pv. For
definite the domain table will HAVE to be changed.
.NH
Detailed changes
.LP 
This list is in no particular order of preference or importance.
.Ip "Domain Table"
The domain table syntax has been changed to be more explicit and has
been brought into line with the OR table syntax. We feel this is
clearer and more logical. Tools that process the domain table have
been taught the new format and there are scripts in
Tools/tables/compat to convert between the old and new formats.
There is \fBNO\fP backwards compatibility here. Old format tables will
just plain not work.
.Ip "Aliases Table"
The aliases table syntax has changed to a main choice of
synonym or alias with further qualifiers specifying
the address type of the synonym or alias.
The changes are
.sp
.in +5
.nf
.ta +\w'x:x400-synonym y\0\0'u
x:alias y	x:alias y
x:synonym y	x:synonym y
x:822-synonym y	x:synonym y 822
x:x400-synonym y	x:synonym y x400
x:822 y		x:alias y 822
x:x400 y	x:alias y x400
.fi
.in -5
where any white space in y must be quoted using a backslash
There is also a "external" qualifier.
This disables further lookups of synonyms and aliases and can be used
for a main site mail hub mapping from site_wide_name@localsite to
local_machine_name@localmachine and vice versa.
The use of external stops the looping alias problem which would
otherwise occur. 
.Ip "Return of Contents"
Return of contents is now supported. This is a side effect and part of
the work on routing of delivery reports.
.Ip "Use of pepsy"
Use of the PEPSY ASN.1 compiler rather than PEPY/POSY is now used in
many places. This makes for smaller binaries and much quicker
compilation. 
.Ip "ISODE \*(Iv"
PP \*(Pv is aligned to ISODE \*(Iv. It will not work under ISODE \*(Il.  If
you are using the \*(Pv release you should be using ISODE \*(Iv. It should
however work under ISODE \*(Ii. 
.Ip "Tailor file"
There have been many changes to the tailor file. The introduction of
several new variables and the splitting up of header and body part
types. These are
.sp
.in +5
.nf
.ta +\w'queuestruct\0\0'u
dap_user	Directory username for DS lookups
dap_passwd	Directory Passwd for same
fsync	disable use of fsync(2)
headertype	list of headers c.f. bodypart
queuestruct	use hierarchical queue structure
submit_addr	call submit over network
diskuse		limit the disk space consumed by PP
.fi
.in -5
.sp
Channel tailoring also has a number of new variables
.sp
.in +5
.nf
.\" .ta +\w'bad-sender-policy\0\0'u
drchan	replaces old outchan
hdrin	headers inbound
hdrout	headers outbound
ininfo	inbound channel info
inadr	inbound channel addr type
insubadr	inbound channel sub address type
intable	inbound channel table
key	give alternative names to channel
mtatable	list of known mtas for channel binding
outinfo	outbound channel info
outsubadr	outbound channel sub address
outtable	outbound channel table
solo-proc	channel requires address one at a time
bad-sender-policy	set the policy for unroutable senders
check	set the checking mode of the channel
.fi
.in -5
.Ip "Channel pairing"
Channels may now be explicitly paired together. This allows one
definition to specify both an inbound and an outbound channel.
It also allows several logical channels for one inbound protocol by
selecting a channel per MTA.
.Ip "Timezones"
Many more lettered timezones are recognised. This list is ambiguous at
best and numeric forms should really be used except at user
interfaces.
.Ip "Delivery Reports"
Delivery reports are now routed back to their originator. In \*(Lv they
were returned directly down the channel they arrived on. This allows
for one way links in general makes things easier. It also allows
protocol conversion so that, for instance, a delivery report on an
X.400 message may go back via an RFC-822 if that is the only option.
A side effect of this is the channel variable outchan has been renamed
drchan and has different semantics attached to it (see the manual).
.Ip "Header parts"
Separation of headers and bodyparts for each channel. This is mainly a
syntactic change but will allow more flexibility later. The upshot is
you may need both bptin/bptout and hdrin/hdrout types in channel definitions.
.Ip "Local delivery"
The local channel table (ch.local) has had a change of syntax. The old
format should still work, but the new format (which is key=value)
based allows much more flexibility and setting of options for mail
managers.
.Ip "MTA Console"
The MTA Console has been changed to show more statistics and has a new
heuristics for showing just channel and MTA summary information. This
is useful for very large queues. 
.Ip "Line Console"
Two line based console programs have been provided.
These give some of the functionality of the MTA console for dumb terminals.
.Ip "Directory lists"
The directory list program has been improved more. It now has several
associated tools to convert file based lists into directory based
lists. There is also a checkup tool.
.Ip "Performance"
Some work has been done on general performance. In general the system
should run slightly faster and have less problems with large queues.
.Ip "X.400(88) OIDs"
One of the X.400(1988) object identifiers was incorrect. Originally
the code used the object identifier designated in the Blue Book. It now
uses the one given in the implementors guide.
.Ip "Mail 11"
Our thanks must go to Nigel Titley of British Telecom for the donation
of a Decnet Mail-11 channel. This allows interworking with DECNET and PP.
.Ip "G3Fax channel"
A skeleton outbound G3 Fax channel is included. 
There are currently two drivers written using this skeleton, one for
the Fujitsu dexNet200 modem and one for the Panasonic SystemFax 250 machine.
These channels enable PP to send appropriately encoded messages via fax.
There are a couple of reformatter filters which convert an
hdr.822 to a fax coverpage and an ia5 bodypart to a g3fax bodypart.
These reformatters produce X.400 g3fax bodyparts and so are general
use and are not dependent on any fax machine.
.Ip "Splitter Channel"
A Splitter channel is included.
This channel together with the presence of solo-proc=yes in channel
entries in the tailor file enables exclusive processing for specified
recipients. This was introduced for the fax reformatter channels.
.Ip "Address Parsing"
The address parsing code has been restructured extensively. This
should be more efficient in many cases.
.Ip "X.400 Extensions"
X.400(88) address extensions are now recognised.
.Ip "X.400 addresses"
Address parsing recognises A as ADMD and P as PRMD in RFC1148 encoded
X.400 addresses. These are converted to the form given in RFC1148.
.Ip "Multi level queue"
For very large system (queues in excess of about 4000 messages) a
multi-level queue can be used. if this is in effect messages are
placed at random in subdirectories of the main queue to reduce the
time required by the system to locate files.
.Ip "Authorisation Warnings"
Authorisation warnings are now implemented. If a message fails
authorisation, a warning can be optionally sent to the originator
and/or the recipient.
.Ip "Delayed delivery warning"
Warnings of delays in delivery are now sent to the originator of the message
if the message cannot be delivered in a timely fashion.
.Ip "Submit daemon"
Submit can be run as a daemon in addition to its normal directly
executed form. This may give performance improvements for inbound
messages.
.Ip "administrator assigned alternate recipient"
The system manager can direct bad address to a default
mailbox, where numerous actions may be attempted (such as returning
helpful messages about approximate matchings etc.)
.Ip "X.400(88) downgrading"
A filter to downgrade X.400(88) P2 headers to X.400(84) P2 headers has
been provided.
Similarly downgrading of P1 is done in the appropriate channels.
This new filter may also be used to normalise OR addresses in these headers.
.Ip "Body part support"
Support of a number of different body types, including ISO-6937,
ISO-8859, and T.61 character sets.
.Ip "New queue format"
A new queue format has been introduced. We believe this will give
greater reliability and flexibility. There are now no special
subdirectories of the queue, everything is directly under the queue
directory. There is a utility to convert the old format into the new
format (Tools/tables/compat/que_o2n).
New format for the internal structure of the message queue has also
been added in a backwards compatible way.
.Ip "ckadr"
ckadr no longer guesses which format of addressing an address is (x400
or rfc822).
Explicit choice should be made via the -x and -r flags (-r is default)
It also no longer checks for UK domain ordering by default i.e. will only
identify US domain ordering).
Checking for UK ordering can be done via the -b flag.
.Ip "Disk checking"
Submit can be tailored to check the amount of free space available
on disk and not submit messages if this is below some threshold. This
is not supported on all systems.
.Ip "Control of header type"
The specification of a nondefault header type in the inbound channel will
override the default header types such as hdr.822 and hdr.p2.
This will enable a site to minimise the header
manipulation/normalisation done.
.Ip "Bad senders"
The system manager can control the policy for dealing with bad senders
on an inbound channel basis.
.Ip "X.400(88) Extensions"
X.400(88) O/R Address extensions have been implemented and are useable
for routing.
.Ip "loc_dom_mta, loc_dom_site"
Previously these two values were magically recognised as local when
normalising and routing RFC 822 addresses.
This magic has been removed and now you need explicit entries for
these in the domain table
.Ip "redirection history"
X.400 redirection history is now supported.
This shows up a bug in PP 5.* which may cause PP 5.* MTA to break when
interworking over x400 with a PP 6.0 MTA.
Note that aliases are implemented as redirects.
.Ip "table overrides"
Tables may been overriden by explicit key/value pairs given in the
tailor file. 
.NH
Upgrade path
.PP
This is the basics of how to upgrade from PP \*(Lv to \*(Pv.
.Ip "Compile"
Compile pp-\*(Pv and install.
.Ip "domain table"
Convert the domain table into the new format. There are some tools
under Tools/tables/compat to aid in this, but you may want to rethink
how you generate these a little. This is the major change in this
version.
Also check that the relevant entries for loc_dom_site and loc_dom_mta
are present in the domain table as they are no longer magically recognised.
.Ip "alias table"
This should be altered into the new format. The conversion routine in
Tools/tables/compat may help again.
.Ip "local table"
This should be backwards compatible, but you may want to change to the
new format anyway.
.Ip "Queue"
Any files in the queue should be converted to the new format. This can
be done by the following:
.DS 
.../Tools/tables/compat/que_o2n ..../queue
.DE
.Ip "Tailoring"
Change the tailor file to use the new keys and values.
.Ip "Checkup"
Run ckconfig and see that everything is sensible.
.Ip "restart"
Rebuild the database and restart.

