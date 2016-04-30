#
# Dockerfile for sniproxy(https://github.com/XiaoxiaoPu/sniproxy)
#

FROM alpine
MAINTAINER Xiaoxiao <i@pxx.io>

COPY sniproxy /sniproxy

EXPOSE 80/tcp
EXPOSE 443/tcp

ENTRYPOINT ["/sniproxy", "-a", "0.0.0.0", "-w", "2"]
