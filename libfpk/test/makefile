# libfpk -- using FPK archive on P/ECE
# 	By Yui N., 2003.
# http://www.autch.net/

test.srf: test.c ../libfpk.h ../libfpk.lib
	pcc33 test.c ..\\libfpk.lib muslib.lib

run: test.srf
	isd r"test.srf";

copy: pex
	isd =w"mypmds.par";
	isd =w"test.pex";

pex: test.srf
	ppack -e -v -n"libfpk ƒeƒXƒg" test.srf -otest.pex
