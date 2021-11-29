co: main.c coroutine.c hook.c context.S
	gcc -g main.c coroutine.c hook.c context.S -o co  -ldl array.c
clean:
	rm co 
