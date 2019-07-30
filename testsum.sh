#!/bin/bash
nclients=$(grep "Test:" testout.log | wc -l)

echo CLIENTS	$nclients

echo ----

nstore=$(grep "STORE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE DO	$nstore

nstore=$(grep "STORE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE OK	$nstore

nstore=$(grep "STORE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE KO	$nstore

echo ----

nstore=$(grep "RETRIEVE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE DO	$nstore

nstore=$(grep "RETRIEVE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE OK	$nstore

nstore=$(grep "RETRIEVE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE KO	$nstore

echo ----

nstore=$(grep "REMOVE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE DO	$nstore

nstore=$(grep "REMOVE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE OK	$nstore

nstore=$(grep "REMOVE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE KO	$nstore

echo ----
nstore=$(grep "Errore" testout.log | cut -d':' -f 2 | wc -l)
echo Lista degli $nstore errori
echo ""
grep "Errore" testout.log

echo ----
srvpid=$(pidof ./server)
kill -10 $srvpid
