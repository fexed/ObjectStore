#!/bin/bash
nclients=$(grep "Test:" testout.log | wc -l)

echo CLIENTS	$nclients

echo ----

nstoredo=$(grep "STORE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE DO	$nstoredo

nstoreok=$(grep "STORE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE OK	$nstoreok

nstoreko=$(grep "STORE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo STORE KO	$nstoreko
echo ""
nstoreokperc=$(echo "scale=2; ( " $nstoreok "/" $nstoredo " ) * 100" | bc -l)
nstorekoperc=$(echo "scale=2; ( " $nstoreko "/" $nstoredo " ) * 100" | bc -l)

echo "Successi:	" $nstoreokperc "%"
echo "Fallimenti:	" $nstorekoperc "%"

echo ----

nretrievedo=$(grep "RETRIEVE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE DO	$nretrievedo

nretrieveok=$(grep "RETRIEVE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE OK	$nretrieveok

nretrieveko=$(grep "RETRIEVE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo RETRIEVE KO	$nretrieveko
echo ""
nretrieveokperc=$(echo "scale=2; ( " $nretrieveok "/" $nretrievedo " ) * 100" | bc -l)
nretrievekoperc=$(echo "scale=2; ( " $nretrieveko "/" $nretrievedo " ) * 100" | bc -l)

echo "Successi:	" $nretrieveokperc "%"
echo "Fallimenti:	" $nretrievekoperc "%"

echo ----

nremovedo=$(grep "REMOVE DO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE DO	$nremovedo

nremoveok=$(grep "REMOVE OK" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE OK	$nremoveok

nremoveko=$(grep "REMOVE KO" testout.log | cut -d':' -f 2 | paste -sd+ | bc)
echo REMOVE KO	$nremoveko
echo ""
nremoveokperc=$(echo "scale=2; ( " $nremoveok "/" $nremovedo " ) * 100" | bc -l)
nremovekoperc=$(echo "scale=2; ( " $nremoveko "/" $nremovedo " ) * 100" | bc -l)

echo "Successi:	" $nremoveokperc "%"
echo "Fallimenti:	" $nremovekoperc "%"

echo ----
nerrorioutput=$(grep "Errore" testout.log | cut -d':' -f 2 | wc -l)
echo "Errori rilevati dall'output:" $nerrorioutput
grep "Errore" testout.log
echo ----

nsuccessi=$(echo "scale=2; (" $nstoreokperc "+" $nretrieveokperc "+" $nremoveokperc ")" / 3 | bc -l)
nfallimenti=$(echo "scale=2; (" $nstorekoperc "+" $nretrievekoperc "+" $nremovekoperc ")" / 3 | bc -l)
echo "	Risultato del test"
echo "		Successi:	" $nsuccessi "%"
echo "		Fallimenti:	" $nfallimenti "%"

echo ----
srvpid=$(pidof ./server)
echo "SIGUSR1 al server con pid" $srvpid

kill -10 $srvpid
