FROM ubuntu:24.04

ARG SOURCE_DIR=/app/3rd_party

WORKDIR /
RUN mkdir -p $SOURCE_DIR

RUN apt update
RUN apt-get install -y sudo build-essential curl git libssl-dev pkg-config

RUN apt-get install -y g++-14 gcc-14
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 14
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 14

WORKDIR $SOURCE_DIR
RUN git clone -b v3.31.5 https://github.com/Kitware/CMake.git
WORKDIR $SOURCE_DIR/CMake
RUN ./bootstrap && make && make install

WORKDIR $SOURCE_DIR
RUN git clone --recursive https://github.com/boostorg/boost.git
WORKDIR $SOURCE_DIR/boost
RUN git checkout boost-1.88.0 
RUN mkdir __build && cd __build && cmake .. && cmake --build . --target install

WORKDIR $SOURCE_DIR
RUN git clone -b 20250127.0 https://github.com/abseil/abseil-cpp.git
WORKDIR $SOURCE_DIR/abseil-cpp
RUN cmake -S . -B build && cmake --build build  --target all
RUN cd build && make && make install



WORKDIR /app
