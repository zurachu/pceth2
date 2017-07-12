int isKanji(const unsigned char c);
int getNumber(const BYTE *ptr, int *num);
int getRegIndex(const BYTE *ptr, int *num);
int getOperator(const BYTE *ptr, char *ope);
int calcRevPolish(const BYTE *ptr, int *num);
int getRevPolish(const BYTE *ptr, char *str);
int getString(const BYTE *ptr, char *str);
int getLabel(const BYTE *ptr, char *str);
int skipScript(const BYTE *ptr);
int convertBGNum(int num);
void replaceName(char *str);

extern int manaka_count;
