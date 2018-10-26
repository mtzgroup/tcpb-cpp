# To build and upload this:
# docker build .
# docker tag [CONTAINER HASH] sseritan/tcpb-client-docker:latest
# docker push sseritan/tcpb-client-docker

FROM alpine:3.6

MAINTAINER Stefan Seritan <sseritan@gmail.com>

ENV PACKAGES="\
  musl \
  linux-headers \
  build-base \
  bash \
  ca-certificates \
  openssl \
  python2 \
  python2-dev \
  py-setuptools \
  vim"

# Set up base environment
RUN apk update && apk add $PACKAGES

# Set up pip
RUN easy_install-2.7 pip

# Preinstall pip packages to cut down on pipeline time
# NumPy takes forever
RUN pip install google protobuf numpy

# Pull a newer protobuf than available for Alpine APK
RUN wget https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.tar.gz
RUN tar xf protobuf-cpp-3.3.0.tar.gz -C /tmp
WORKDIR /tmp/protobuf-3.3.0
RUN ./configure && make && make install

ENTRYPOINT ["/bin/bash"]
