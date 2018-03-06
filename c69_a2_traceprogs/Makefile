SRCS = simpleloop.c matmul.c blocked.c
PROGS = simpleloop matmul blocked

all : $(PROGS)

$(PROGS) : % : %.c
	gcc -Wall -g -o $@ $<


traces: $(PROGS)
	./runit simpleloop
	./runit matmul 10
	./runit blocked 10 2

.PHONY: clean
clean : 
	rm -f simpleloop matmul blocked tr-*.ref *.marker *~
