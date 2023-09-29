# Minimal Docker image for ViralConsensus using Alpine base
FROM alpine:3.13.5
MAINTAINER Niema Moshiri <niemamoshiri@gmail.com>

# install samtools
RUN apk update && \
    apk add autoconf bash bzip2-dev curl-dev g++ git make xz-dev zlib-dev && \
    wget -qO- "https://github.com/samtools/htslib/releases/download/1.18/htslib-1.18.tar.bz2" | tar -xj && \
    cd htslib-* && autoreconf -i && ./configure && make && make install && cd .. && rm -rf htslib-* && \
    git clone https://github.com/niemasd/ViralConsensus.git && \
    cd ViralConsensus* && \
    make && \
    mv viral_consensus /usr/local/bin/viral_consensus && \
    cd .. && \
    rm -rf ViralConsensus*
