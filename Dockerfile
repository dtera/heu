FROM gcr.io/bazel-public/centos7-java11-devtoolset10@sha256:f335a5d84d385e95bd736abdfb2115fe368a7bd7993da15f833cdec6541edf5f as heu_env

USER root
RUN yum update -y && yum install cmake3 ninja-build -y
RUN cd /usr/bin && ln -s cmake3 cmake && cd -
RUN cd / && git clone https://github.com/dtera/heu.git && cd /heu
WORKDIR /heu
RUN ./bin/init.sh build
RUN mkdir /root/heu && ln -s lib /root/heu/lib && ln -s include /root/heu/include
VOLUME /root/heu
