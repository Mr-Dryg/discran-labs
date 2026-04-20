FROM gcc:15.2.0

RUN apt-get update && \
    apt-get install -y \
        make \
        cmake \
        clangd &&\
    rm -rf /var/lib/apt/lists/*