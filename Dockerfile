FROM ubuntu:22.04 as build

RUN apt-get update && \
    apt-get install -y \
        bsdextrautils \
        build-essential \
        git \
        libglew-dev \
        libsdl2-dev \
        python3 \
        python-is-python3


RUN mkdir /sm64ex
WORKDIR /sm64ex
ENV PATH="/sm64ex/tools:${PATH}"

CMD echo 'Usage: docker run --rm -v ${PWD}:/sm64ex sm64ex make BETTERCAMERA=1 EXTERNAL_DATA=1 -j4\n' \
         'See https://github.com/sm64pc/sm64ex/wiki/Compiling-on-Docker for more information'

FROM build AS web

WORKDIR /

RUN git clone https://github.com/emscripten-core/emsdk.git

RUN /emsdk/emsdk install 1.39.5
RUN /emsdk/emsdk activate 1.39.5

# normally you'd run 'source $EMSDK/emsdk_env.sh' to load the emsdk environment
# both Dockerfile and Makefile run each process in a separate subshell so I don't think something like
#  RUN . /emsdk/emsdk_env.sh
# will work, nor would doing the same within the Makefile
# wouldn't be an issue on a bare metal build as the user would have already sourced the file per emsdk docs
# also, the env script only works in bash, not sh, and I'm not sure about changing shells in Docker
# versions are pinned, so we can live with it, I guess. node version will change if emsdk is updated
# run
#   docker run --rm sm64ex bash -c '. /emsdk/emsdk_env.sh'
# to print the emsdk environment after a version change
ENV EMSDK="/emsdk"
ENV EM_CACHE="/emsdk/upstream/emscripten/cache"
ENV EM_CONFIG="/emsdk/.emscripten"
ENV EMSDK_NODE="/emsdk/node/16.20.0_64bit/bin/node"
ENV PATH="${EMSDK}:${EMSDK_NODE}:${EMSDK}/upstream/emscripten:${PATH}"

WORKDIR /sm64ex

CMD echo 'Usage: docker run --rm -v ${PWD}:/sm64ex sm64ex make BETTERCAMERA=1 EXTERNAL_DATA=1 -j4\n' \
         'See https://github.com/sm64pc/sm64ex/wiki/Compiling-on-Docker for more information'
