# among tested images only with ubuntu works
FROM ubuntu:16.04

COPY iroha /iroha

ENV IROHA_HOME=/iroha
ENV LD_LIBRARY_PATH=/iroha/lib

WORKDIR /iroha

RUN useradd -ms /bin/bash iroha
USER iroha

CMD /iroha/bin/iroha-main
