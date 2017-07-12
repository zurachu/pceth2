#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE	*fp = NULL;
	char	th2dir[_MAX_PATH];

	if((fp = fopen("unpack000.ini", "r")) != NULL) {
		fscanf(fp, "%s", th2dir);
		fprintf(stdout, "th2wav -tse %sTH2SOUND.SFS .\\", th2dir);
		fclose(fp);
	}

	return 0;
}
