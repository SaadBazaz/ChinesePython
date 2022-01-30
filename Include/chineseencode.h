/* Chinese encoding conversion */

#ifndef CHINESEENCODE_H
#define CHINESEENCODE_H

#define ishighascii(c)		((signed char)c < -1?1:0)

/* CJ and UNI not implemented yet */
#define BIG5 	1
#define GBK 	2
#define UNI	3
#define CJ	4

#define Source_Encoding BIG5  /* this is the C source code encoding */

#ifdef _cplusplus
extern "C" {
#endif

extern DL_IMPORT(int) Current_Encoding; /* the interpreter's encoding */ 
extern DL_IMPORT(int) File_Encoding; /* the encoding of file 
					   when sees #--XXX-- magic */
extern DL_IMPORT(char *) Chinese_EncodingNames[];
extern DL_IMPORT(char *) gbk_to_big5(const unsigned char *s);
extern DL_IMPORT(char *) big5_to_gbk(const unsigned char *s);

#ifdef _cplusplus
}
#endif

#endif /* !CHINESEENCODE_H */

