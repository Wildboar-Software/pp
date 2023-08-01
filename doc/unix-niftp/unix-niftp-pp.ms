.ds Un \s-2\fIUNIX-NIFTP\fP\s+2
.de Eg
.sp
.in +5
.nf
..
.de Ee
.sp
.fi
.in -5
..
.de Ts
.in +5
.sp
.nf
..
.de Te
.sp
.in -5
.fi
..
.TL
Configuring \*(Un for use with PP.
.AU
Julian Onions 
J.Onions@xtel.co.uk
.AI
X-Tel Services Ltd
Nottingham University
Nottingham NG7 2RD
.SH
Introduction
.LP
This short note is intended to be a stop-gap that covers what you need
to know to install \*(Un for operation with PP. It is not
intended to be a tutorial on \*(Un, partly because I don't know enough
and partly because I don't have time. 
.LP
The \*(Un package is powerful and has many options, this note is not
meant to be exhaustive or to even try covering all options. It is
simply to get you started. There are many improvements and different
configurations that you could have, but to discover these you will
have to read the README files, the code and ask on the unix-niftp list
(unix-niftp@cs.nott.ac.uk \- unix-niftp-request@cs.nott.ac.uk to join)..
.SH
Compilation
.LP
Currently, both PP and \*(Un think they are in charge of compilation,
therefore there is a slight bootstrap problem. The best method
currently therefore is the following.
.IP 1.
Compile PP without the greybook channel. This is done by not
specifying \*Qgrey\*U in the OPTIONALCHANS variable in Make.defs.
.IP 2.
Compile \*(Un
.IP 3.
Compile PP with the greybook channel (e.g. add it back to OPTIONALCHANS).
.LP
To configure and compile, \*(Un, you basically run \fIConfigure\fP and
answer the questions it asks. Make sure you request use of PP in the
unspooled mode. Other questions such as the place for binaries etc are
up to you. if you don't understand the question \fIConfigure\fP is asking,
hit return and it will normally \*Qdo the right thing\*U.
.Eg
% sh Configure
... lots of questions ...
.Ee
.LP
Once the configuration process is over, type \fImake\fP and wait.
.Eg
% make
.Ee
There should be no problems at this stage.
.LP
After this completes, you should next type \*Q\fImake fullinstall\fP\*U
which should install everything in the correct place. It will also ask
you for a master password. This is only used for ftp transfers, and
you can type anything you like here if you are going to  using
\*(Un for PP mail only. Otherwise you should choose a good password,
and forget it!
.Eg
% make fullinstall
.Ee
.SH
Configuring the runtime system
.LP
The next step is to configure the run time support system and database
that \*(Un needs.
This consists of first creating a tailor file (usually stored in
/etc/niftptailor) and secondly building a dbm database of hosts.
.SH 2
Tailoring the system.
.LP
A sample tailor file can be found in Appendix A. However, the
necessary lines you will need are the following.
.Eg
QUEUE PP level=7,prog=psun
.Ee
This line defines the PP queue. The name \*QPP\*U is what PP will use as
its queue name. The level is for logging in the niftp logs. Suitable
levels vary from \*Q1\*U, minimal logging, through \*Q7\*U - some logging,
to \*Q-1\*U - all logging.
The prog bit is superfluous, but doesn't hurt.
.Eg
NET janet queue=qj,address="000021001029/%E%X%D/%T"
.Ee
This line identifies the network \*Qjanet\*U for outgoing calls.
When the PP greybook channel makes a call, it consults the info field
of the channel to determine the network. This is used to work out what
address to use in outgoing calls.
The queue key sets the queue, but I don't know why.
The address key/value sets up the calling and called addresses
respectively. These are constructed in a printf like manner with
literal characters copied and %'s marking substitutions.
The following substitutions are legal, 
but the default above is suitable - with the appropriate DTE of
course. 
.Ts
.ta \w'%X\0\0'u
%E	The relevant YBTS string for the calling address (e.g. FTP.MAIL)
%X	Separator between calling and called address
%D	Destination DTE number
%T	Destination YBTS string (e.g. FTP, FTP.MAIL)
%O	The variable set by OURNAME.
%P	pink book context
.Te
So, the above string with DTE of 00000511160005 and a YB string of
UCL-CS.FTP.MAIL we would generate
.Ts
.ta \w'calling address:\0\0'u
calling address:	000021001029/FTP.MAIL
called address:	00000511160005/UCL-CS.FTP.MAIL
.Te
.Eg
LISTEN pj address="FTP.MAIL",level=7,prog=qsun,channel=janet
.Ee
This line registers a listener for incoming mail. The address,
\*QFTP.MAIL\*U is not explicitly used in this situation -0 but is a
useful reminder. The level specifies a logging level of \*Q7\*U, and
claims to be from the janet network. This implies that it will be
channel \*Qgb-janet\*U when passing on to PP.
.LP
This is all that is essential for PP to make use of \*(Un. There are
some external things to set up as explained later however.
However, there are a number of useful variables you can set that help
if you wish to move things around (such as log directories for
example). Some of the more useful ones are given in Appendix B.
.SH 2
Building the database.
.LP
The easiest way of building the database is with the c-nrs program. In
fact, this is so easy that I'm not going to describe other methods!
So, first get hold of the c-nrs package and build that. Once this is
done, you can then generate the \*(Un database by the following
commands.
.Eg
nrs dbm1.cf > dbm1
nrs dbm1.cf 'output reverse' >> dbm1
dbencode < dbm1
.Ee
What could be simpler!
.SH
Testing the system for outgoing message.
.LP
With all this done, the system should now work for outgoing calls.
Make sure you have done a \*Qmake install\*U in PP's Chans/grey
directory to get the greybook channel installed. Now mail a message to
a host that makes use of the grey book channel and see if it works.
If it doesn't you will hopefully get a useful diagnostic. This will
appear usually in either the PP log directory in a file such as
\*Qgreyout\*U or in the \*(Un log directory in \*Qlog.pPP\*U.
.LP
A useful test that things are set up is to run the \*(Un program
\fIlookdbm\fP. This looks up names in the database.
.Eg
% bin/lookdbm uk.ac.ucl.cs
.Ee
This checks that the tailoring is correct and the database is present.
.SH
Setting up the incoming system.
.LP
This is best done by running a daemon such as ybtsd. This daemon is
started at boot time usually and listens for Yellow Book transport
calls. When one is received, it looks at the Yellow Book address and
determines what program to call to process this. This matching up is
done through the file ybts-auth. This is usually in an \*Qetc\*U
directory. It can be /etc/ybts-auth, /usr/local/etc/ybts-auth or
/etc/sunlink/cbs/ybts-auth.
.LP
To configure PP to accept incoming calls you need to do the following.
In the PP chans directory, make a symbolic link from greyin to the
name of the LISTEN name in the \*(Un file. This is \*Qpj\*U in the above
example.
.Eg
% ln -s greyin pj
.Ee
Now arrange for the ybtsd to invoke this process for incoming FTP.MAIL
requests. A suitable line in the ybts-auth file might be
.Eg
/usr/pp/chans/pj:ftp.mail:*
.Ee
The first part being the pathname of the symbolic link you just
created, the second being the YBTS address (ftp.mail) and the third
field saying any calling address is allowed to invoke this.
.LP
This should now be set up to allow incoming and outgoing mail.
.bp
.SH
Appendix A \- sample tailor file
.LP
.Eg
# known about queues 
QUEUE   qj      level=7,prog=psun
QUEUE   PP      level=7,prog=psun


# various configuration details for each network
NET janet       queue=qj,address="YOUR-DTE-HERE/%E%X%D/%T"

# Domains that we know about
DOMAIN  "uk.ac", "uk","uk.co"

# The listener information
LISTEN pj address="FTP.MAIL",level=107,prog=qsun,channel=janet
.Ee
.bp
.SH
Appendix B \- useful tailor variables
.LP
The following is a list of the more useful things that you can tailor.
This list is not exhaustive!
.IP BINDIR 10
The directory where binaries private to the \*(Un are looked for.
.Eg
BINDIR /usr/lib/niftp
.Ee
.IP DOMAIN
A list of domain prefixes. When looking for an NRS name, the software
will try adding these names to what it is looking for should the first
lookup fail.
.Eg
DOMAIN "uk.ac", "uk.co"
.Ee
.IP LOGDIR
The directory in which the log files are created and written.
.Eg
LOGDIR /usr/spool/niftp/logs
.Ee
.IP OURNAME
Sets the %O string.
.Eg
OURNAME 000021001029
.Ee
.IP TABLE
The DBM database that contains the NRS name to address translations.
.Eg
TABLE /usr/spool/niftp/nrsdbm
.Ee
