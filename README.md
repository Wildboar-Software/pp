# The PP X.400 Message Transfer Agent (MTA)

This is an X.400 Message Transfer Agent (MTA) written in the 1990s, I believe
mostly by Steven Kille. This project was resurrected in 2024 by Jonathan M.
Wilbur / Wildboar Software to be able to build using modern tooling on Linux in
a Docker container. A few bugs were fixed as well.

## Background: X.400 Message Handling Systems

X.400 Message Handling Systems were a messaging service similar in purpose to
email--and, in fact, it competed with email at the time, though it lost--but
devised by the International Telecommunications Union (ITU). It gets its name
from the fact that the ITU's X.400-series of specifications, starting with
[ITU-T Recommendation X.400](https://www.itu.int/rec/T-REC-X.400/en).

X.400 differs significantly by being a lot more technically complicated, but
much more capable than email out-of-the-box, supporting things like binary data,
encryption, authentication, integrity, read receipts, etc. While it has many
benefits, it was designed to only utilize the extremely complicated OSI
networking stack: it was never designed to run over TCP/IP, which was a big
problem for its adoption. In addition to this, it uses attribute-value pairs for
addressing, which makes the parsing, presentation, validation, and routing based
on these addresses much more complicated than email's "user-at-host" addressing,
which some have argued was a major reason for its downfall. It also has poor
support for non-Romance language, but this could be said to be true of email as
well.

Still, the X.400 MHS is
[used in NATO and Western militaries](https://en.wikipedia.org/wiki/Military_Message_Handling_System)
(I believe the U.S. and U.K. use it, at least), and in the
[Air Traffic Services Message Handling Services (AMHS)](https://en.wikipedia.org/wiki/Aeronautical_Message_Handling_System)
which is used by the aviation industry. I have also heard that it is still in
use in the shipping / overseas transport industry for EDI, but I haven't
confirmed this anywhere.

## The PP X.400 MTA

PP is a Message Transfer Agent, intended for high volume message switching,
protocol conversion, and format conversion. It is targeted for use in an
operational environment, but is also be useful for investigating message related
applications. Good management features are a major aspect of this system. PP
supports the 1984 and 1988 versions of the CCITT X.400 / ISO 10021 services and
protocols. Many existing [IETF RFC 822](https://www.rfc-editor.org/info/rfc822)
based protocols are supported, along with RFC 1148bis conversion to X.400. PP is
an appropriate replacement for MMDF or Sendmail. This is the second public
release of PP, and includes substantial changes based on feedback from using PP
on many sites.

- PP is not proprietary and can be used for any purpose. The only restriction is
  that suing of the authors for any damage the code may cause is not allowed.
- PP runs on a range of UNIX and UNIX-like operating systems, including SUNOS,
  Ultrix, and BSD. A full list of platforms on which PP is know to run is
  included in the distribution.
- Current modules include:
  - X.400 (1984) P1 protocol.
  - X.400 (1988) P1 protocol.
  - Simple mail transfer protocol (SMTP), conformant to host requirements.
  - JNT mail (grey book) Protocol.
  - UUCP mail transfer.
  - DECNET Mail-11 transfer
  - Distribution list expansion and maintenance, using either a file based
    mechanism or an X.500 directory.
  - RFC 822-based local delivery.
  - Delivery time processing of messages.
  - Conversion between X.400 and RFC 822 according to the latest revision of RFC
    1148, known as RFC 1148bis.
  - Conversion support for reformatting body parts and headers.
  - X-Window and line-based management console.
  - Message Authorisation checking.
  - Reformatting support for "mail hub" operation.
  - X.500-based distribution list facility using the QUIPU directory.
  - FAX interworking
- No User Agents (UAs) are included with PP. However, procedural access to the
  MTA is documented, to encourage others to write or to port UAs. Several
  existing UAs, such as MH, may be used with PP.
- It is expected that a Message Store to be used in conjunction with PP (PPMS),
  and an associated X-Windows User Agent (XUA) will be released on beta test in
  first quarter 92.
- The core routing of PP 6.0 is table based. DNS is used by the SMTP channel.
  The next version of PP will support Directory Based routing, which may use
  X.500 or DNS.
- PP 6.0 requires ISODE 7.0.
- X-Windows release X11R4 (or greater) is needed by some of the management
  tools. PP can be operated without these tools.
- The primary documentation for this release consists of a three and a half
  volume User's Manual (approx. 300 pages) and a set of UNIX manual pages. The
  sources to the User's Manual are in LaTeX format.

## License

I believe this software was written before the standardized FOSS licenses even
existed, but the authors left this in `COVER-LETTER`, which I interpret to be
something like an MIT License:

> PP is not proprietary and can be used for any purpose. The only restriction is
> that suing of the authors for any damage the code may cause is not allowed.

## Documentation

This documentation describes how to configure, generate, and install the PP
message system. It is intended only as a quick guide. The full details are in
the PP documentation set - _PP Manual: Installation and Operation_. This can be
found in the `doc/manual/volume1` directory

## Description

This is a description of how one can bring up the PP message system. It is
assumed that you have super-user privileges in order to (re-)install the
software. Super-user privileges are not required to configure or generate this
software.

PP is normally installed with most programs and directories owned by a single
user. Conventionally this username is "pp" and is not a normal user id but this
is not required. You should normally do most of the compilation and
configuration as this user.

## Building

First, go to the `config/` directory:

```bash
cd config
```

Select the Makefile and include-file skeletons which most closely match your
system. The makefile skeleton has the extension `.make`, and the include-file
skeleton has the extension `.h`. This will most likely be `linux.make` and
`linux.h` respectively, in your case. Using Linux as an example:

```bash
cp config/linux.h h/config.h
cp config/linux.make ./Make.defs
```

You can configure the build further in these files as well as in
`./OPTIONS.make`.

> [!WARNING]
> This version of PP was designed against ISODE 7.0, and the original
> documentation says that it requires ISODE 7.0. However, it seems to run fine
> on ISODE 8.x and higher. Understand that there may be bugs as a result of this
> version breach. I (Jonathan Wilbur) have created many fixes to ISODE 8.0, so
> I would advise using that instead of ISODE 7.0.

It is assumed that you have ISODE 8.3.11 or higher installed on the machine in
question, including the X.500 libraries `libdsap` `libisode`. If you wish to
build the grey book channels for JANET mail, you should first build and install
the unix-niftp package.

To build this, run this command EXACTLY from the root directory:

```bash
./make
```

This will cause a complete generation of the system. If all goes well, proceed
with the installation. If not, complain, as there "should be no problems" at
this step. It may be worth saving the output of the make stage in a file in case
things do go wrong.

## Installation

You will need to be the super-user to install the software. There are two kinds
of activities: once-only activities that you perform the first time the software
is installed; and each-time activities that you perform every time the software
is installed.

The first once-only activity is to create the necessary basic directories to
install the commands. This is done by issuing the command as super-user:

```bash
su
./make dirs
```

Note that the `pp` user id needs to have been allocated at this point.

The second once-only activity is to make sure that PP will run when when the
machine goes multi-user. On Berkeley UNIX systems, add something along these
lines to the `/etc/rc.local` file:

```bash
if [ -f /usr/lib/pp/pp.start ]; then
     /usr/lib/pp/pp.start & (echo -n ' pp') > /dev/console
fi
```

There are some examples of this start-up script in the `config/` directory.

If you wish to use smtp then it may be appropriate to start the smtp server for
`/etc/inetd` by adding a suitable line to `/etc/inetd.conf`. See _Volume 1_ for
more details, but the basic format is something like:

```
smtp stream tcp nowait  pp  /usr/lib/pp/cmds/chans/smtpd
     smtpd /usr/lib/pp/cmds/chans/smtpsrvr smtp
```

To allow processes to contact the `qmgr` you must add a line to your
`isoentities` file:

```
pickle "pp qmgr" 1.17.6.2.1 #1001/Internet=pickle+18000
```

You will need to replace "pickle" by your local host, and you may need to change
the isomacro "Internet" to something else if you have a local ethernet or
similar.

Alternatively you may add a suitable entry to the directory if this is being
used as a nameserver. Some samples are given below the examples directory.

Then to install the following each-time activity is:

```bash
su
./make install
```

If you plan to run the `MTAconsole` program (and you should!) you will need to
install the application defaults file. This is in the `MTAconsole/` directory
and it should be installed as `MTAconsole` in the app-defaults directory in your
X hierarchy. For instance:

```bash
cd Src/MTAconsole
cp MTAconsole.ad /usr/lib/X11/app-defaults/MTAconsole
```

The same is true of the user utility `xalert`. This has a similar set of
application defaults that should be installed.

That's about it. This will install everything. To clean-up the source tree as
well, then use:

```bash
make clean
```

If this is the first time you have used PP it is worth picking one of the
example configurations in the `examples/` directory. Choose a sample that is
close to your requirements as a starting point. The available samples are

- `LOCALSMTP`: Very simple local SMTP only
- `JANET`: A simple JANET configuration
- `INTERNET`: A simple Internet site, listening for SMTP messages

Change to one of these directories, and read the `README` there. This will tell
you to do various things, depending on what configuration you have chosen.
Remember, this is only an example and will not deal with all your needs.

After all is set up, and at regular intervals, you should check all is ok by
running `ckconfig` (found in the `tools` directory) which will check things are
installed correctly.

<!-- Finally, if you are interested in discussing PP with others running the
software, drop a note to the Janet mailbox "pp-people-request@cs.ucl.ac.uk", and
ask to be added to the "pp-people@cs.ucl.ac.uk" list. -->

## Tailoring

Tailoring is an essential part of running PP. This is somewhat complex to
achieve the correct setup and so is described in _Volume 1_ of the PP manual. It
is important that this is followed carefully.

## Building Documentation

The directory `doc/` contains the documentation set for this release. Consult
the file `doc/README` for a description of each document. The directory
`doc/ps/` contains PostScript versions of each document. These can be used to
generate standard documentation on PostScript printers, but users who want to
preview the documentation should generate the .dvi files from the SLiTeX and
LaTeX sources.

If you received this distribution from the network, then the directory `doc/ps/`
does not contain any PostScript files. There should be a separate compressed
`tar` file, containing only PostScript files, available on the machine where you
retrieved this distribution.

## Support

I didn't write this software, I just resurrected it. If you would like support,
please create an issue on the
[GitHub Issues page](https://github.com/Wildboar-Software/pp/issues). Maybe one
day, you'll be able to reach me over X.400, too! I welcome PRs, too.

<!-- Comments concerning this release should be directed to the authors. Consult the
preface in the _User's Manual_ for a current postal address. Alternately, if you
have access to the Janet network, comments may be sent to the mailbox
"pp-support@cs.ucl.ac.uk". Do not send bug reports to the pp-people discussion
group. -->

## Why "PP"?

I will quote an excerpt from _Implementing X.400 and X.500: The PP and Quipu
Systems_ by Stephen E. Kille:

> PP is not an acronym. There is no truth in the rumour that PP stands for
> "Postman Pat," a famous British postman.

That's all I've got.

## See Also

- [ISODE](https://github.com/Wildboar-Software/isode) - Which I also resurrected
- [Meerkat DSA](https://github.com/Wildboar-Software/directory) - A Free and
  Open-Source X.500 Directory I wrote. It has almost all official X.500 features
  and more. This repo also contains various X.500 libraries and tools, such as
  an X.500 command-line interface and an OSI networking library.
- [Wildboar Software](https://github.com/Wildboar-Software) - A business
  pseudonym for Jonathan M. Wilbur, a computer nerd who also happens to sell
  a "Fail2Ban for Kubernetes" called [Kube2Ban](https://kube2ban.com/).
- [M-Switch](https://www.isode.com/products/m-switch-x400.html) - I believe
  this is the closed-source and proprietary successor to PP, and I also believe
  this is maintained by Stephen E. Kille, or those who work for him.
