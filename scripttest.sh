#!/bin/bash
./server &

echo ""
echo "**Inizio test**"

echo "" > testout.log

echo "50 STORE"

for i in {1..50}
do
	./client usr_$i 1 >> $i.log &
	pids[$i]=$!
done

echo "... attendo"

for pid in ${pids[*]}; do
	wait $pid
done

echo "30 RETRIEVE"

for i in {1..30}
do
	./client usr_$i 2 >> $i.log &
	pids[$i]=$!
done

echo "20 DELETE"

for i in {31..50}
do
	./client usr_$i 3 >> $i.log &
	pids[$i]=$!
done

echo "... attendo"

for pid in ${pids[*]}; do
	wait $pid
done

for i in {1..50}
do
	cat $i.log >> testout.log
	rm $i.log
done
echo "***************"
echo ""