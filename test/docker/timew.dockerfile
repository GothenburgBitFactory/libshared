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

FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND noninteractive

RUN ( apt-get update && yes | unminimize ) && \
    apt-get install -y \
            asciidoctor \
            cmake \
            g++ \
            git \
            man \
            man-db \
            python3 \
            python3-dateutil \
            tzdata

# Setup language environment
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

# Checkout Timewarrior
WORKDIR /root
RUN git clone https://github.com/GothenburgBitFactory/timewarrior.git code
WORKDIR code
RUN git checkout ${BRANCH}

FROM base AS builder

# Add libshared
ADD . src/libshared

# Build Timewarrior
RUN cmake -DCMAKE_BUILD_TYPE=release . && \
    make -j8

WORKDIR test
RUN make

CMD ["bash", "-c", "./run_all -v ; cat all.log | grep 'not ok' ; ./problems ; FAILED=$? ; python3 --version ; cmake --version ; gcc --version ; exit $FAILED"]
