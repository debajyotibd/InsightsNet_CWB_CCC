FROM ubuntu:20.04

##########################
# INSTALL OS DEPENDENCIES
##########################
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get install --no-install-recommends -y \
    apt-utils \
    autoconf \
    bison \
    flex \
    gcc \
    libc6-dev \
    libglib2.0-0 \
    libglib2.0-dev \
    libncurses5 \
    libncurses5-dev \
    libpcre3-dev \
    libreadline8 \
    libreadline-dev \
    make \
    pkg-config \
    subversion \
    git \
    python3 \
    python3-dev \
    python3-pip \
    python3-setuptools \
    cython3 \
    wget \
    tar \
    gzip \
    less \
    mg

#####################
# INSTALL LATEST CWB
#####################
RUN svn co http://svn.code.sf.net/p/cwb/code/cwb/trunk /cwb
WORKDIR /cwb
RUN  sed -i 's/SITE=beta-install/SITE=standard/' config.mk && \
    ./install-scripts/install-linux && \
    ldconfig


##############################
# INSTALL PYTHON DEPENDENCIES
##############################
RUN python3 -m pip install --upgrade pip && \
    python3 -m pip install -q pipenv

WORKDIR /cwb-ccc
COPY . /cwb-ccc


###############
# BUILD & TEST
###############
RUN make clean
RUN make install
RUN make compile
RUN make build
RUN make test
