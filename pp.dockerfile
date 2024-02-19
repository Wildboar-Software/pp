# FROM ubuntu:latest
FROM ghcr.io/wildboar-software/quipu:v8.3.11

LABEL com.wildboarsoftware.app="pp"
LABEL com.wildboarsoftware.major_version="6"
LABEL com.wildboarsoftware.minor_version="0"

# This provides the ndbm.h headers and such.
RUN apt update && apt install -y libgdbm-compat-dev libgdbm-compat4 libgdbm-dev groff libxt-dev libxaw7-dev

ADD . /pp
WORKDIR /pp
ADD config/linux.h h/config.h
ADD config/linux.make ./Make.defs
ENV PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

RUN adduser pp

RUN ./make
RUN ./make dirs
RUN ./make install

# Installs an example configuration just to try to start it up.
WORKDIR /pp/examples/INTERNET
RUN ./make install

# This is how you map the transport selector to the MTA.
# This example was taken directly from _Implementing X.400 and X.500: The PP and Quipu Systems_ by Steven Kille.
# This is confirmed to be the actual location of the channel.
RUN echo '"tsap/p1" "591" /usr/lib/pp/cmds/chans/x400in88' >> /etc/isoservices

# JSYK:
# The ISODE tailoring file is here: /usr/local/etc/isode/isotailor, but it does
# not seem to apply all the time. It seems like the PP tailoring file overrides
# your ISODE settings, particularly when it comes to logging.
WORKDIR /usr/lib/pp

# ISO Transport over TCP (ITOT) is exposed on this port. You should be able to
# perform X.400 (88) MHS operations by using the transport selector "x400".
EXPOSE 20001
CMD /usr/lib/pp/cmds/pptsapd
