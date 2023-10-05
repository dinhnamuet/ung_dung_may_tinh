KDIR=/lib/modules/`uname -r`/build
obj-m=dongco.o
all:
	make -C $(KDIR) M=`pwd` modules
	gcc -o app motor_app.c
clean:
	make -C $(KDIR) M=`pwd` clean
	rm -rf app
