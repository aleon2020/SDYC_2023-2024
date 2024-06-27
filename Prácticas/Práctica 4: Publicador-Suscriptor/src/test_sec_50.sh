#!/usr/bin/bash

mkdir -p test_sec_50/

for i in $(seq 1 50); do
    ./subscriber --ip 0.0.0.0 --port 2345 --topic "prueba" > test_sec_50/$i.txt &
done;
