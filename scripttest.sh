#!/bin/bash
./server &
srvpid=$!

echo "Inizio test"

echo "" > testout.log

echo "STORE"

for i in {1..50}
do
	./client $i 1 >> $i.log &
	pids[$i]=$!
done

for pid in ${pids[*]}; do
	wait $pid
done

echo "RETRIEVE"

for i in {1..29}
do
	./client $i 2 >> $i.log &
	pids[$i]=$!
done

echo "DELETE"

for i in {30..50}
do
	./client $i 3 >> $i.log &
	pids[$i]=$!
done

for pid in ${pids[*]}; do
	wait $pid
done

echo "Segnalo SIGUSR1 al server"
kill -10 $srvpid

echo "Chiudo il server"
kill -2 $srvpid

for i in {1..50}
do
	cat $i.log >> testout.log
	rm $i.log
done