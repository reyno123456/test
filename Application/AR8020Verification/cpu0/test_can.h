#ifndef TEST_CAN
#define TEST_CAN

void command_TestCanInit(unsigned char *ch, unsigned char *br, unsigned char *acode,unsigned char *amask, unsigned char *rtie, unsigned char *format);
void command_TestCanTx(unsigned char *ch, unsigned char *id, unsigned char *len, unsigned char *tbuf,unsigned char *format, unsigned char *type);


#endif
