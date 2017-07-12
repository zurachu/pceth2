all:
	pcc33 -c -Wall -gp=0x0 -near -DPIECE -O2 -DNDEBUG -b ufe.c
	lib33 ufe.lib ufe.o

clean:
	-del *.o
	-del *.lib
	-del *.$$
	-del *.ps
	-del *.ms
