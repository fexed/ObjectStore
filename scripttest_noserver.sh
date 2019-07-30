#!/bin/bash
#./server &

echo ""
echo "**********"
echo "Inizio test"

echo "" > testout.log

echo "STORE"

for i in {1..50}
do
	./client usr_$i 1 >> $i.log &
	pids[$i]=$!
done

for pid in ${pids[*]}; do
	wait $pid
done

echo "RETRIEVE"

for i in {1..29}
do
	./client usr_$i 2 >> $i.log &
	pids[$i]=$!
done

echo "DELETE"

for i in {30..50}
do
	./client usr_$i 3 >> $i.log &
	pids[$i]=$!
done

for pid in ${pids[*]}; do
	wait $pid
done

for i in {1..50}
do
	cat $i.log >> testout.log
	rm $i.log
done
echo "**********"
echo ""