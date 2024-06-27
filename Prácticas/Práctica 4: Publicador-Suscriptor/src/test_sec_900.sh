#!/usr/bin/bash

mkdir -p test_sec_900/

for i in $(seq 1 900); do
    ./subscriber --ip 0.0.0.0 --port 2345 --topic "prueba" > test_sec_900/$i.txt &
done;
