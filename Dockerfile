FROM ubuntu:18.04
LABEL maintainer="chriamue@gmail.com"
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential clang git cmake \
	qt5-default qttools5-dev qttools5-dev-tools \
	libopencv-dev libpoco-dev libdlib-dev
RUN cd / && git clone https://github.com/davisking/dlib && cd dlib && mkdir build && cd build && cmake -DBUILD_SHARED_LIBS=1 .. && cmake --build . && make install
COPY . /aicam
RUN cd /aicam && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
	&& make -j3 && make install

CMD /aicam/build/aicamgui