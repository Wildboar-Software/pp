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
