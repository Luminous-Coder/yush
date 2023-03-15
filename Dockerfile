# Yush – Young's Shell

FROM ubuntu:rolling AS builder

WORKDIR /app

RUN \
    apt update && \
    apt install -y build-essential git cmake

COPY . .
RUN mkdir build && cmake -B build && cmake --build build -j $(nproc)

FROM ubuntu:rolling

COPY --from=builder /app/build/yush /usr/local/bin/yush
ENTRYPOINT [ "yush" ]
