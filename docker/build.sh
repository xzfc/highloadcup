#!/usr/bin/env zsh

set -e
cd "${0%/*}/.."

rm -rf "docker-build" 
mkdir docker-build

cp -a --parents serv/**/*.{hpp,cpp,h} serv/Makefile docker-build/
cp -a docker/{Dockerfile,run.sh} docker-build

cd docker-build
docker build -t dumb .
