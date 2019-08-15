#!/bin/bash
./server &

echo ""
echo "**Inizio BIG test**"

echo "" > testout.log

echo "STORE"

for i in {1..1000}
do
	./client usr_$i 1 >> $i.log &
	pids="$pids $!"
done

echo "... attendo"

wait $pids

pids=""

echo "RETRIEVE"

for i in {1..500}
do
	./client usr_$i 2 >> $i.log &
	pids="$pids $!"
done

echo "DELETE"

for i in {501..1000}
do
	./client usr_$i 3 >> $i.log &
	pids="$pids $!"
done

echo "... attendo"

wait $pids

for i in {1..1000}
do
	cat $i.log >> testout.log
	rm $i.log
done
echo "*******************"
echo ""

./testsum.sh