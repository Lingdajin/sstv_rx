#include <device/sbdspsoc_sb3500.h>
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

int32_t gDBL = 5;
/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
static char oldcmd[CMDLINE_BUF];
#define in_line oldcmd
int getline(char *lp, uint32_t n, uint8_t dsp)
{
   uint32_t cnt = 0;
   int c;
   c = -1;
   do {
      c = getchar();
      switch (c) {
         case CNTLQ:                       /* ignore Control S/Q             */
         case CNTLS:
            break;;
         case BACKSPACE:
         case DEL:
            if (cnt == 0) {
               break;
            }
            cnt--;                         /* decrement count                */
            lp--;                          /* and line pointer               */
            putchar(0x08);                /* echo backspace                 */
            putchar(' ');
            putchar(0x08);
            //fflush (stdout);
            break;
         case ESC:
            *lp = 0;                       /* ESC - stop editing line        */
            return (FALSE);
         case TAB:
            for(c=0;(oldcmd[c]!=CR)&&(oldcmd[c]!=LF)&&(oldcmd[c]!=0)&&(cnt<60);c++){
                *lp = oldcmd[c];
                lp++;
                cnt++;
                if(dsp)putchar(oldcmd[c]);
                else putchar('*');
            }
            break;
         case CR:                          /* CR - done, stop editing line   */
            *lp = c;
            lp++;                          /* increment line pointer         */
            cnt++;                         /* and count                      */
            c = LF;
         default:
            *lp = c;
            if(dsp)putchar(c);             /* echo and store character       */
            else putchar('*');
            //fflush (stdout);
            lp++;                          /* increment line pointer         */
            cnt++;                         /* and count                      */
            break;
      }
   } while (cnt < n - 2  &&  c != LF);     /* check limit and CR             */
   *lp = 0;                                /* mark end of string             */
   for(c=0;c<=cnt;c++)
      oldcmd[c]= *(lp-cnt+c);
   return (TRUE);
}

/* string -> signed int */
long int mystrtoul(char *s)
{
    int ret;
    int radix = 10;
    int negative = 0;
    int i;
    ret = 0;
    if(NULL==s)return ret;
    if(*s == '-') {
        negative = 1;
        s++;
    }
    else if(*s == '0') {
        s++;
        if((*s == 'x')||(*s == 'X')){
            s++;
            radix = 0x10;
        }
    }
    else if((*s == 'H')||(*s=='h')) {
        s++;
        radix = 0x10;
    }
    while (*s) {
        if (*s >= '0' && *s <= '9')
            i = *s - '0';
        else if (*s >= 'a' && *s <= 'f')
            i = *s - 'a' + 0xa;
        else if (*s >= 'A' && *s <= 'F')
            i = *s - 'A' + 0xa;
        else
            break;
        if(i >= radix) break;
        ret = (ret * radix) + i;
        s++;
    }
    return negative?(-ret):ret;
}

/* signed int --> string */
char* utoa(char *s, unsigned int u)
{
    char temp[10]={0};
    uint8_t i=0;
    if(NULL==s) return NULL;
    if(0==u){
        *s = '0';
        s++;
        *s = 0;
        return s;
    }
    while(0!=u){
        temp[i++]='0'+u%10;
        u /= 10;
    }
    while(i>0){
        *s=temp[--i];
        s++;
    }
    *s = 0;
    return s;
}

const unsigned int auwDbTable[8] = {
 0x00000011,0x000000E0,0x00000DDD,0x0000DBAB
,0x000D9972,0x00D78940,0x0D580472,0x85702C73};
unsigned short ausDb[7][16]={
{0x11C9,0x1664,0x1C30,0x237C,0x2CAC,0x383C,0x46CC,0x5921
,0x7034,0x8D41,0xB1D4,0xDFE0,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x119E,0x162E,0x1BEB,0x2326,0x2C40,0x37B5,0x4621,0x5849
,0x6F25,0x8BEC,0xB027,0xDDC3,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x1173,0x15F8,0x1BA8,0x22D1,0x2BD5,0x372E,0x4577,0x5774
,0x6E18,0x8A9A,0xAE7D,0xDBAB,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x1149,0x15C3,0x1B65,0x227D,0x2B6B,0x36A9,0x44CF,0x56A0
,0x6D0E,0x894B,0xACD7,0xD998,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x111F,0x158E,0x1B23,0x222A,0x2B02,0x3624,0x4429,0x55CF
,0x6C07,0x87FF,0xAB35,0xD78A,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x10F6,0x155A,0x1AE1,0x21D7,0x2A9A,0x35A2,0x4384,0x5500
,0x6B02,0x86B6,0xA998,0xD581,0xFFFF,0xFFFF,0xFFFF,0xFFFF},
{0x10CD,0x1527,0x1AA0,0x2185,0x2A33,0x3520,0x42E1,0x5432
,0x69FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF}};

int LnrToDb(unsigned int uwLnr)
{
    unsigned short *vComp;
    unsigned int auwBase[8];
    unsigned short ausBase[16];
    unsigned short usLnr;
    int i,ind;

    if(uwLnr <17)
    {
        return ((uwLnr>9)?(11):((uwLnr>2)?9:4));
    }
    __sb_rbroad(auwBase, (((unsigned long long)uwLnr)<<32)|(uwLnr), 6);
    i = __sb_rcmpult32(auwBase, auwDbTable);

    if(i==0)
    {
        return ((uwLnr>>29)+90);
    }

    ind = __sb_ctz(i) - 1;
    vComp = ausDb[ind];
    usLnr = ((ind<3)?( uwLnr<<((2-ind)<<2) ):(uwLnr>>((ind-2)<<2)));
    __sb_rbroad(ausBase, (long long)(usLnr<<16), 0);
    i = __sb_rcmpult(vComp, ausBase);

    return ( ind*12 + 44 - __sb_clz(i) );
}
/*
void cmd_rssi(char *par)
{
    printf("cmd_rssi();\n");
}
*/

/*
 * EOF
 */
