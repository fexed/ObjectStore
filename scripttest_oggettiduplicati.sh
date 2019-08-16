#!/bin/bash
./server &

echo ""
echo "**Inizio test ERRORI**"

echo "" > testout.log

echo "Oggetti duplicati"
echo ""
echo "Memorizzo"

./client usr_same 1 &
pid=$!

echo "... attendo"

wait $pid

echo ""
echo "Rimemorizzo"

./client usr_same 1 &
pid=$!

echo "... attendo"

wait $pid

echo "**********************"
echo ""