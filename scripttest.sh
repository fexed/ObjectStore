#!/bin/bash
./server &
srvpid=$!

echo "Inizio test"

echo "" > testout.log

for i in {0..10}
do
	./client $i 1 > $i.log &
done

echo "Segnalo SIGUSR1 al server"
kill -10 $srvpid

echo "Chiudo il server"
kill -2 $srvpid

for i in {0..10}
do
	cat $i.log >> testout.log
	rm $i.log
done