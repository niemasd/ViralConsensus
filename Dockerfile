# Minimal Docker image for ViralConsensus using Alpine base
FROM alpine:latest

# install ViralConsensus
RUN apk update && \
    apk add --no-cache bash bzip2-dev curl-dev g++ make python3 xz-dev zlib-dev && \
    wget -qO- "https://github.com/samtools/htslib/releases/download/1.24/htslib-1.24.tar.bz2" | tar -xj && \
    cd htslib-* && \
    ./configure && \
    make install && \
    cd .. && \
    wget -qO- "https://github.com/niemasd/ViralConsensus/archive/refs/heads/main.tar.gz" | tar -zx && \
    cd ViralConsensus-* && \
    make && \
    mv scripts/build_vcf.py viral_consensus /usr/local/bin/ && \
    cd .. && \
    rm -rf htslib-* ViralConsensus-*
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
