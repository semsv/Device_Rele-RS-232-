all:
	gcc rs-232.c -o rele
	cp -r rele /bin/rele
	chmod a+rwx /bin/rele	
	rmmod ch341.ko 2>/dev/null
	insmod Drivers/ch341.ko
	sh settch341.sh
build:
	gcc rs-232.c -o rele
	cp -r rele /bin/rele
	chmod a+rwx /bin/rele
	sh settch341.sh
clean:
	rm -r rele
install:
	cp -r rele /bin/rele
	chmod a+rwx /bin/rele
	rm -r rele