all: memtest_sys memtest_dev

memtest_sys: memtest.c
	gcc -o memtest_sys memtest.c -DSYSTEM_MALLOC

memtest_dev: memtest.c memory_manager.c memory_manager.h
	gcc -o memtest_dev memtest.c memory_manager.c

clean:
	rm -f memtest_sys memtest_dev