#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>
#include "rgxtest.h"


#define MAX 1024

char buf[MAX];
char *epat = buf;
char *temp;
char patbuf [MAX];
char *pbuf = patbuf;
int rflag, endc;

enum { BOL=1, EOL, DOT, CHAR, STAR, CLASS, PLUS, MINUS, RANGE, BACKSLASH, SPACE, END, ENDPAT } token;

void pushop(int c){ *epat++ = c; }

void class(char *exp){ 
  while(*exp != ']'){
      if(*exp == '\\'){ 
          pushop(BACKSLASH);
          exp++;
          pushop(*exp++);
          return;
          }
      if(isalpha(*exp) || isdigit(*exp))pushop(*exp);
     exp++;
   }
return;
}

void swapop(int c){
 memset(patbuf, 0, sizeof patbuf);
 epat--;                              
  while(*epat > 13){ epat--;}         /* rewind the pattern */
  if(*epat==BACKSLASH)epat--;
  pbuf = patbuf;                      /* Copy the pattern into the buffer */
  while(*epat >0){ *pbuf++=*epat++; }
  epat--;                             /* rewind the pattern again */
  while(*epat > 13){ epat--; }
  if(*epat == BACKSLASH)epat--;       /* backslash */
  *epat++ = c;                        /* COPY the PLUS into the pattern */
  char *t = patbuf;
  while(*t>0){ *epat++ = *t++; }
}

void compile_pattern(char * exp){
   int c;
   memset(buf, 0, MAX);
   epat = buf;
   while(*exp != '\0'){
           if (*exp == '*' || *exp == '+' || *exp == '-') {
                c=*exp--;
                exp++;
               if(c == BOL || c == EOL || c == PLUS || c== MINUS || c == STAR) 
                { 
                     printf ("Incorrect Pattern\n");
                     pushop(END);
                     exit(0);
                }        
            }
      switch(*exp){
       case '^':
         pushop(BOL);
         break;
       case '.':
         pushop(DOT);
         break;
       case '*':
         swapop(STAR); 
         pushop(ENDPAT);    
         break;
       case '[':
         pushop(CLASS);
         rflag = 1;
         class(exp);
         endc = 0;
         break;
       case '-':
         if(rflag == 0){pushop(CHAR);pushop(*exp);}
         break;
       case ']':
         rflag = 0;
         endc = 1;
         break;
       case '+':
         swapop(PLUS); 
         pushop(ENDPAT);     
         break;
       case '$':
         pushop(EOL);
         break;
    default:
      if(rflag == 0){ 
       pushop(CHAR);
       pushop(*exp );
       }
      break;
      }  // END SWITCH
   exp++;
   }  // END WHILE
         pushop(END);
         pushop(0);               
 return;
}

int cmatch(char *s, char *p){
  if(*p == DOT && (*s != '\0' || *s != '\n')) return 1;        /* handle DOT */ 
  if(*p == CLASS && p[1] == BACKSLASH && p[2] == 's' && *s == ' ') return 1; /* handle space */
  if(*p == CLASS && p[1] == BACKSLASH && p[2] == 'S' && *s != ' ') return 1; /* handle non space */
  if(*p == CLASS && s[0] >= p[1] && s[0] <= p[2]) return 1;   /* handle class (RANGE ONLY) */
  if(*p == CHAR && p[1] == s[0])return 1;                     /* handle CHAR  */
 return 0;
}

char *pmatch(char *s, char *p){
  char *are, *e, *l; 
  int op;
 while((op = *p) != END){     
    switch( op ){
     case BOL:
       if(*l != *temp) return 0;
       break;          
     case CHAR:
      if(cmatch(s,p) == 0) return 0;
      s++;
       break; 
     case EOL:
        if (*s != '\0')return(0);
        break;
     case DOT:
        if(cmatch(s, p) == 0) return 0;
        else s++; 
        break;
     case CLASS:
      if(cmatch(s,p)==0) return 0;
       s++;
       break; 
     case PLUS: case STAR:
       p++;
       l = p; /* make a copy of the pattern */
       if ((cmatch(s, l)) == 0 && *p != STAR)return 0;
        else s++;                                     /* This section based on DEC's code */
         are = s;                                     /* Remember line start */
         while (*s && cmatch(s, l))s++;               /* Get longest match   */
         while (*p++ != ENDPAT && *p!= END);                      /* Skip over pattern   */
         while (s >= are) {                          /* Try to match rest   */
            if ((e = (pmatch(s, p))))return(e);
            --s;                 /* Nope, try earlier   */
         }
         return(0);              /* Nothing else worked */
    }   /* end switch */
  p++;
  } /* end while */
       return (s);      
}

int match(char *exp, char *s){
      memset(buf, 0, sizeof(buf));
      memset(patbuf, 0, sizeof patbuf);
      temp = s;
      compile_pattern(exp);
      char *p = buf;       
       while(*s != '\0'){ 
         if(pmatch(s, p))return(1);          
         s++;
        }                
      return 0;         
 }

void print_encoded(){

     int c = 0;
     printf("ENCODED: ");
     epat = buf;

    while(*epat != END){
       if(*epat > 21){
           printf("%c ", *epat);
         }
        else{
           printf("%s ", tokenmap[*epat]);
        }          
     epat++; c++;
     }
     printf("\n");
   return;
}

int main(int argc, char *argv[]){
 if(argc < 3){printf("Incorrect Arguments. Please use match <regex> <string>\n"); exit(0); } 
 printf("match: %d\n",  match(argv[1], argv[2]));  
 print_encoded(); 
return 0;
}
