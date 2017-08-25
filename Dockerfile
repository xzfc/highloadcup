FROM ubuntu:zesty

WORKDIR /root

RUN apt-get update
RUN apt-get install -y g++ make libboost-all-dev

RUN mkdir src tmp
COPY *.cpp *.hpp Makefile run.sh src/
COPY rapidjson                   src/rapidjson
COPY simple-web-server           src/simple-web-server

# Компилируем и устанавливаем наш сервер
RUN make -j4 -C src

# Открываем 80-й порт наружу
EXPOSE 80

# Запускаем наш сервер
#CMD ./src/main
CMD bash
