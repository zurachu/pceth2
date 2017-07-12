#ifndef __PCETH2_MSG_H_
#define __PCETH2_MSG_H_

#define NAMEBUF_LEN	9

#define FONT_W	10
#define FONT_H	10

BOOL pceth2_isLineTop();
BOOL pceth2_isPageTop();
void pceth2_setPageTop();

void pceth2_clearMessage(void);
void pceth2_putKanji(const char *str);
void pceth2_putCR(void);

int pceth2_procControl(SCRIPT_DATA *s);
int pceth2_procEscape(SCRIPT_DATA *s);
int pceth2_jpnHyphenation(const char *str);
int pceth2_lineFeed(const char *str);

#endif
