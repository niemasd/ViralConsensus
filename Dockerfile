# Minimal Docker image for ViralConsensus using Alpine base
FROM alpine:3.13.5
MAINTAINER Niema Moshiri <niemamoshiri@gmail.com>

# install samtools
RUN apk update && \
    apk add bash bzip2-dev curl-dev g++ git make xz-dev zlib-dev && \
    git clone https://github.com/niemasd/ViralConsensus.git && \
    cd ViralConsensus* && \
    make && \
    sudo mv viral_consensus /usr/local/bin/viral_consensus && \
    cd .. && \
    rm -rf ViralConsensus*
