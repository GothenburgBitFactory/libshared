###############################################################################
#
# Copyright 2023, Gothenburg Bit Factory.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# https://www.opensource.org/licenses/mit-license.php
#
###############################################################################

ARG BRANCH

FROM ubuntu:24.04 AS base

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install -y \
            build-essential \
            cmake \
            curl \
            faketime \
            git \
            libgnutls28-dev \
            locales \
            python3 \
            uuid-dev

# Setup language environment
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

# Setup Rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs > rustup.sh && \
    sh rustup.sh -y --profile minimal --default-toolchain stable --component rust-docs

# Checkout Taskwarrior
WORKDIR /root
# clone repo with submodels as we need corrosion
RUN git clone --recurse-submodules https://github.com/GothenburgBitFactory/taskwarrior.git code
WORKDIR code
RUN git checkout ${BRANCH}

# remove libshared folder which is the submodel
RUN rm -rf src/libshared

FROM base AS builder

# Add libshared
ADD . src/libshared

# Build Taskwarrior
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j 8 --target test_runner --target task_executable

# Running tests
CMD ctest --test-dir build -j 8 --output-on-failure --rerun-failed
