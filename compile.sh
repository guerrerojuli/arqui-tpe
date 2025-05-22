#!/bin/bash
docker pull agodio/itba-so:2.0
docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name arqui-tpe agodio/itba-so:2.0
docker start arqui-tpe
docker exec -it arqui-tpe make clean -C /root/Toolchain
docker exec -it arqui-tpe make clean -C /root/
docker exec -it arqui-tpe make -C /root/Toolchain
docker exec -it arqui-tpe make -C /root/
docker stop arqui-tpe
