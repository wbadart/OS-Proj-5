##
# Makefile
#
# Build instructions and unit test for proj 5.
#
# Badart, Cat
# Badart, Will
# created: APR 2017
##

CXX		  = /usr/bin/gcc
CXX_FLAGS = -Wall -ggdb

LD       = /usr/bin/gcc
LD_FLAGS =

OUT  = virtmem
OBJS = main.o page_table.o disk.o program.o

all: link

link: $(OBJS)
	$(LD) $(LD_FLAGS) $(OBJS) -o $(OUT)

%.o: src/%.c
	$(CXX) $(CXX_FLAGS) $< -c -o $@

clean:
	rm -f *.o $(OUT) myvirtualdisk

