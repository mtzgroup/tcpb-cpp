# To build and upload this:
# docker build .
# docker tag [CONTAINER HASH] sseritan/tcpb-client-docker:<version>
# docker push sseritan/tcpb-client-docker:<version>

FROM centos:7

MAINTAINER Stefan Seritan <sseritan@stanford.edu>

ENV PACKAGES="@development \
    bzip2 \
    bzip2-devel \
    findutils \
    libffi-devel \
    openssl-devel \
    readline-devel \
    sqlite \
    sqlite-devel \
    vim \
    wget \
    xz \
    xz-devel \
    zlib-devel"

# Set up base environment
RUN yum install -y $PACKAGES

# Pyenv for multiple python runtimes
WORKDIR /home/
RUN git clone https://github.com/pyenv/pyenv.git /home/.pyenv
ENV PYENV_ROOT /home/.pyenv
ENV PATH $PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH

# Install Python
RUN pyenv install 3.7.4
RUN pyenv install 3.6.9
RUN pyenv install 3.5.7
RUN pyenv install 2.7.16
RUN pyenv global 3.7.4
RUN pyenv rehash

# Pull new protobuf
RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.10.0/protobuf-cpp-3.10.0.tar.gz
RUN tar xf protobuf-cpp-3.10.0.tar.gz -C /tmp
WORKDIR /tmp/protobuf-3.10.0
RUN ./configure && make && make install
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Preinstall pip packages to cut down on pipeline time
RUN pip install google protobuf numpy tox tox-pyenv

WORKDIR /home/

CMD ["/bin/bash"]
