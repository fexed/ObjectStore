#!/bin/bash
./server &
srvpid=$!

echo "Inizio test"
echo "" > testout.log

for i in {0..50}
do
	./client $i 1 1>> testout.log &
done

echo "Segnalo SIGUSR1 al server"
kill -10 $srvpid

echo "Chiudo il server"
kill -2 $srvpid