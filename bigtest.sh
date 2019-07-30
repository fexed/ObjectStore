for i in {1..10}
do
	make -i clean > /dev/null
	make all > /dev/null 2>&1
	make test
	#read -p "Premere invio per continuare"
done