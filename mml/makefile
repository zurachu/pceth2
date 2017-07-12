%.pmd : %.mml
	mucc -o $< $@

MMLS = ${wildcard *.mml}
PMDS = ${patsubst %.mml,%.pmd,$(MMLS)}

all : $(PMDS)

clean :
	del *.pmd
