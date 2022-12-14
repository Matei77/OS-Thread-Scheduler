CC = gcc
CFLAGS = -g -fPIC -Wall

.PHONY: build
build: libscheduler.so

libscheduler.so: so_scheduler.o threads_handler.o priority_queue.o
	$(CC) -shared -o $@ $^

threads_handler.o: threads_handler.c threads_handler.h
	$(CC) $(CFLAGS) -o $@ -c $<

priority_queue.o: priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) -o $@ -c $<

so_scheduler.o: so_scheduler.c so_scheduler.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f so_scheduler.o threads_handler.o priority_queue.o libscheduler.so