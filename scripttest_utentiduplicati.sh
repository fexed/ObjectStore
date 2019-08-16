#!/bin/bash
./server &

echo ""
echo "**Inizio test ERRORI**"

echo "" > testout.log

echo "10 Utenti duplicati"

for i in {1..10}
do
	./client usr_duplicated 1 &
	pids[$i]=$!
done

echo "... attendo"

for pid in ${pids[*]}; do
	wait $pid
done

echo "**********************"
echo ""