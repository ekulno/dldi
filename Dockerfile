# syntax=docker/dockerfile:experimental

FROM debian:bookworm-slim as base

RUN apt-get update && apt-get install -y cmake build-essential ninja-build libserd-dev zlib1g-dev valgrind git

# not sure what version the apt catch2 is, but it doesn't have the same headers 
# as the latest version. So doing a manual build then. 
RUN	git clone https://github.com/catchorg/Catch2 dep && \
	cd dep && \
	git checkout v3.3.2 && \
	mkdir build && cd build && \
 	cmake -G Ninja .. && \
	cmake --build . && \
	cmake --install . && \
	cd ../../ && \
	rm -rf dep

FROM base as ci 

RUN mkdir /dldi 
COPY . /dldi
WORKDIR /dldi
RUN ./bin/build
RUN ./bin/test

RUN cd Release && ctest -j $(nproc) \
	--overwrite MemoryCheckSuppressionFile=/dldi/valgrind.suppressions.Release \
	--overwrite MemoryCheckCommandOptions="--error-exitcode=1 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --track-origins=yes --verbose --dsymutil=yes" \
	-T memcheck --output-on-failure --short-progress --error-exitcode=1 \
	--timeout 500 

