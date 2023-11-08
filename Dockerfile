FROM ubuntu:22.04 as build

RUN apt-get update && \
    apt-get install -y \
        bsdextrautils \
        build-essential \
        git \
        libglew-dev \
        libsdl2-dev \
        python3

RUN mkdir /sm64ex
WORKDIR /sm64ex
ENV PATH="/sm64ex/tools:${PATH}"

CMD echo 'Usage: docker run --rm -v ${PWD}:/sm64ex sm64ex make BETTERCAMERA=1 EXTERNAL_DATA=1 -j4\n' \
         'See https://github.com/sm64pc/sm64ex/wiki/Compiling-on-Docker for more information'
