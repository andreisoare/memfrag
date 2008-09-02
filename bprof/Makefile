all: my_gmalloc_hook free_size my_malloc_hook

my_gmalloc_hook: my_gmalloc_hook.c
	gcc --shared -g `pkg-config --cflags --libs glib-2.0` -lrt my_gmalloc_hook.c -omy_gmalloc_hook.so

free_size: free_size.c
	gcc -o free_size free_size.c

my_malloc_hook: my_malloc_hook.c
	gcc -o my_malloc_hook.so -shared my_malloc_hook.c

clean:
	rm -f my_gmalloc_hook.so free_size my_malloc_hook.so
