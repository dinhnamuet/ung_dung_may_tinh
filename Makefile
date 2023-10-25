KDIR=/lib/modules/`uname -r`/build
obj-m=dongco.o
all:
	make -C $(KDIR) M=`pwd` modules
	gcc -o app test_app.c #-pthread -lrt
clean:
	make -C $(KDIR) M=`pwd` clean
	rm -rf app
