

# the target is obtained linking all .o files
all: make1 make2

make1:
	make -f Makefile1

make2:
	make -f Makefile2


purge: clean

clean:
	make -f Makefile1 purge && make -f Makefile2 purge

###############################################################################
################################################################################
