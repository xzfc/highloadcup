FROM ubuntu:zesty

WORKDIR /root

RUN apt-get update && apt-get install -y g++ make unzip libboost-all-dev

ADD serv serv.src
RUN make -j`nproc` -C serv.src && mv serv.src/main serv && rm -r serv.src
ADD run.sh run.sh

EXPOSE 80

CMD ./run.sh
