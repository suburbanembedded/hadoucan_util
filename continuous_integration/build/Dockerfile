FROM ubuntu:18.04

RUN apt-get update && apt-get install -y \
    build-essential \ 
    can-utils       \
    cmake           \
    git 

RUN apt-get update && apt-get install -y \
    libboost-chrono-dev \
    libboost-date-time-dev \ 
    libboost-thread-dev



RUN groupadd buildbot && useradd --no-log-init --create-home --home-dir /home/buildbot -g buildbot buildbot

USER buildbot
WORKDIR /home/buildbot
RUN git clone https://github.com/suburbanembedded/common_util.git
RUN mkdir /home/buildbot/common_util/build
WORKDIR /home/buildbot/common_util/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN make && make package

USER root
RUN apt-get install ./common_util-0.1.0-Linux.deb

