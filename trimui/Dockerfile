# Build environment for the Trimui port.
# Base image: https://github.com/dimitrovs/trimui-smart-pro-build-system
#   (cd trimui-smart-pro-build-system && make build)
FROM trimui-smart-pro-build
# gl4es needs CMake >= 3.19; Ubuntu 20.04 ships 3.16
RUN apt-get update && apt-get install -y python3-pip zip && pip3 install "cmake<4" && rm -rf /var/lib/apt/lists/*
