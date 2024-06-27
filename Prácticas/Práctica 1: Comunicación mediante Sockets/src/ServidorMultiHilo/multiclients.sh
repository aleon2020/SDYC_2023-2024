#!/bin/sh

for i in `seq 1 30`; do
	WAIT=`printf '0.%06d\n' $RANDOM`;
	(sleep $WAIT; echo "Lanzando cliente $i ..."; 
	./client $i 127.0.0.1 8000) &
done


