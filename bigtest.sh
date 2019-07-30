for i in {0..10}
do
	make -i clean > /dev/null
	make all > /dev/null 2>&1
	make test
done