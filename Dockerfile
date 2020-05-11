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

RUN wget \
        https://github.com/n64decomp/qemu-irix/releases/download/v2.11-deb/qemu-irix-2.11.0-2169-g32ab296eef_amd64.deb \
        -O qemu.deb && \
    echo 8170f37cf03a08cc2d7c1c58f10d650ea0d158f711f6916da9364f6d8c85f741 qemu.deb | sha256sum --check && \
    dpkg -i qemu.deb && \
    rm qemu.deb

RUN git clone --depth 1 https://github.com/emscripten-core/emsdk.git && \
    ./emsdk/emsdk install latest && \
    ./emsdk/emsdk activate latest

RUN mkdir /sm64
WORKDIR /sm64
ENV PATH="/sm64/tools:/emsdk:/emsdk/upstream/emscripten:${PATH}"

CMD echo 'usage: docker run --rm --mount type=bind,source="$(pwd)",destination=/sm64 sm64 make VERSION=${VERSION:-us} -j4\n' \
         'see https://github.com/n64decomp/sm64/blob/master/README.md for advanced usage'
