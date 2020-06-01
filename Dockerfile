FROM ubuntu:18.04 as build

RUN apt-get update && \
    apt-get install -y \
        binutils-mips-linux-gnu \
        bsdmainutils \
        build-essential \
        git \
        libaudiofile-dev \
        libsdl2-dev \
        pkg-config \
        python3 \
        wget \
        zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 https://github.com/emscripten-core/emsdk.git && \
    ./emsdk/emsdk install latest && \
    ./emsdk/emsdk activate latest

RUN mkdir /sm64
WORKDIR /sm64
ENV PATH="/sm64/tools:/emsdk:/emsdk/upstream/emscripten:${PATH}"

CMD echo 'usage: docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 sm64 make VERSION=${VERSION:-us} -j4\n' \
         'see https://github.com/n64decomp/sm64/blob/master/README.md for advanced usage'
