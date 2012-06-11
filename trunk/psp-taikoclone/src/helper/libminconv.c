#include "libminiconv.h"

char* miniConvUTF16LEConv(const unsigned short* utf16le);
char* miniConvUTF16BEConv(const unsigned short* utf16be);
int miniConvGetConvCount();
char* miniConvGetConvCharset(int index);
void miniConvSetFileSystemConv(const char* charset);
int miniConvHaveFileSystemConv();
char* miniConvFileSystemConv(const unsigned char* s);
void miniConvSetDefaultSubtitleConv(const char* charset);
int miniConvHaveDefaultSubtitleConv();
int miniConvHaveSubtitleConv(const char* charset);
char* miniConvDefaultSubtitleConv(const unsigned char* s);
char* miniConvSubtitleConv(const unsigned char* s, const char* charset);
void miniConvFreeMemory(void* mem);
