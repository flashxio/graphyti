FROM ubuntu:16.04
MAINTAINER Disa Mhembere

RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get -o Dpkg::Options::="--force-confold" --force-yes -y upgrade

RUN apt-get -y install \
        build-essential \
        git \
        libmpich-dev \
        libnuma-dbg libnuma-dev libnuma1 \
        python-all-dev python-pip \
        vim \
        libaio-dev \
        libatlas-base-dev \
        libgoogle-perftools-dev \
        libaio-dev libatlas-base-dev zlib1g-dev \
        libnuma-dev libhwloc-dev \
        libboost-all-dev

WORKDIR /home/ubuntu/
#RUN git clone --recursive https://github.com/flashxio/knor.git
#WORKDIR knor
#RUN make -j8

# To ingest example
RUN pip install --upgrade pip
RUN pip install --upgrade setuptools pybind11 numpy
RUN apt-get -y install python3
RUN apt-get -y install python3-pip
RUN pip3 install --upgrade pip
RUN pip3 install setuptools pybind11 numpy

# Enter bash shell
ENTRYPOINT ["bash"]
