#!/usr/bin/bash

mkdir -p test_sec_500/

for i in $(seq 1 500); do
    ./subscriber --ip 0.0.0.0 --port 2345 --topic "prueba" > test_sec_500/$i.txt &
done;
