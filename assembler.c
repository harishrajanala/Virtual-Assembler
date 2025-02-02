#define _GNU_SOURCE
#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define WORD_LENGTH 16

typedef struct {
	int address;
	char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */ //\n at the end of line
} TableEntry;

TableEntry symbolTable[MAX_SYMBOLS];
int tabCount=0;


enum{ DONE, OK, EMPTY_LINE };
enum{ ADD, AND, BR, HALT, JMP, JSR, JSRR, LDB, LDW, LEA, NOP, NOT, RET, LSHF, RSHFL, RSHFA, RTI, STB, STW, TRAP, XOR };

FILE *infile;
FILE *outfile;

bool isRealLabel(char* ptr){ //checks if label is valid

  if( (strncmp(ptr, "in", 2)==0) || (strncmp(ptr, "out", 3)==0) || (strncmp(ptr, "getc", 4)==0) || (strncmp(ptr, "puts", 4)==0) || (ptr[0]=='x') || strlen(ptr)>MAX_LABEL_LEN || (strncmp(ptr, "r1", 2)==0) || (strncmp(ptr, "r2", 2)==0) || (strncmp(ptr, "r3", 2)==0) || (strncmp(ptr, "r4", 2)==0) || (strncmp(ptr, "r5", 2)==0) ||(strncmp(ptr, "r6", 2)==0) || (strncmp(ptr, "r7", 2)==0) )
      return false;
  
  for(int j=0; j<strlen(ptr); j++){
    if(isalnum(ptr[j])==0){ //not alphanum
      return false;
    }
  }

  for(int i=0; i<tabCount; i++){ // check if in table already
    if(strcmp(ptr, symbolTable[i].label)==0){
      return false;
    }
  }
  
  return true;
}
int inTable(char* pt){

  for(int i=0; i<tabCount; i++){
    if(strcmp(pt, symbolTable[i].label)==0){ //found
      return i;
    }
  }
  printf("Error code 1: invalid label %s\n", pt);
  exit(1);
}
int isOpcode (char * ptr){ // check is opcode

  if(strcmp(ptr, "add")==0){
    return ADD;
  }
  if(strcmp(ptr, "and")==0){
    return AND;
  }
  if(strncmp(ptr, "br", 2)==0){
    return BR;
  }
  if(strcmp(ptr, "halt")==0){
    return HALT;
  }
  if(strcmp(ptr, "jmp")==0){
    return JMP;
  }
  if(strcmp(ptr, "jsr")==0){
    return JSR;
  }
  if(strcmp(ptr, "jsrr")==0){
    return JSRR;
  }
  if(strcmp(ptr, "ldb")==0){
    return LDB;
  }
  if(strcmp(ptr, "ldw")==0){
    return LDW;
  }
  if(strcmp(ptr, "lea")==0){
    return LEA;
  }
  if(strcmp(ptr, "nop")==0){
    return NOP;
  }
  if(strcmp(ptr, "not")==0){
    return NOT;
  }
  if(strcmp(ptr, "lshf")==0){
    return LSHF;
  }
  if(strcmp(ptr, "rshfl")==0){
    return RSHFL;
  }
  if(strcmp(ptr, "rshfa")==0){
    return RSHFA;
  }
  if(strcmp(ptr, "rti")==0){
    return RTI;
  }
  if(strcmp(ptr, "stb")==0){
    return STB;
  }
  if(strcmp(ptr, "stw")==0){
    return STW;
  }
  if(strcmp(ptr, "trap")==0){
    return TRAP;
  }
  if(strcmp(ptr, "xor")==0){
    return XOR;
  }
  return -1;


}
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4){

          char * lRet, * lPtr;
           int i;
           if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
                return( DONE );

          if(strncmp(pLine, ".end", 4)==0 || strncmp(pLine, ".END",4)==0 ){ //if end
              return DONE;
           }

           for( i = 0; i < strlen( pLine ); i++ )
                pLine[i] = tolower( pLine[i] );
          
          /* convert entire line to lowercase */
           *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

           /* ignore the comments */
           lPtr = pLine;

           while( *lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n' )
                lPtr++;

           *lPtr = '\0';
           if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
                return( EMPTY_LINE );

           if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
           {
                *pLabel = lPtr;
                if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
           }
          
          *pOpcode = lPtr;

           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
          
          *pArg1 = lPtr;
          
          if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

           *pArg2 = lPtr;
           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

           *pArg3 = lPtr;

           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

           *pArg4 = lPtr;

           return( OK );
           
        }

int toNum(char *pStr ){ // string to num

  char * t_ptr;
  char * orig_pStr;
  int t_length,k;
  int lNum, lNeg = 0;
  long int lNumLong;

  orig_pStr = pStr;
  if( *pStr == '#' ) //decimal
  {
    pStr++;
    if( *pStr == '-' )  /* dec is negative */
    {
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    for(k=0;k < t_length;k++)
    {
      if (!isdigit(*t_ptr))
      {
        
         printf("Error: invalid decimal operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNum = atoi(pStr);
    if (lNeg)
      lNum = -lNum;

    return lNum;
  }
  else if( *pStr == 'x' )  /* hex*/
  {
    pStr++;
    if( *pStr == '-' ) /* hex is negative */
    {
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    // printf("%s", t_ptr);

    for(k=0;k < t_length;k++)
    {
      if (!isxdigit(*t_ptr))
      {
         printf("Error: invalid hex operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
    lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
    if( lNeg )
      lNum = -lNum;
    return lNum;
  }
  else
  {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
  }
}
int regToDec(char* regArg){

    if(regArg[0]!='r'){
      printf("Error code 4: invalid argument %s\n", regArg);
      exit(4);
    }
    regArg[0]='#';
    int regi=toNum(regArg);
    if(regi>7){
      printf("Error code 4: R%d, is an invalid register\n", regi);
      exit(4);
    }
    return regi;
}

int main(int argc, char* argv[]) {

    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    if(argc!=3){
        printf("Did not include correct arguments, Try again");
        exit(4);
    }

    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");
    
    if (!infile) {
      printf("Error: Cannot open file %s\n", argv[1]);
      exit(4);
                 }
    if (!outfile) {
      printf("Error: Cannot open file %s\n", argv[2]);
      exit(4);
    }
    

    char asmLine[MAX_LINE_LENGTH+1];
    char *opCopy, *label, *opCode, *Arg1, *Arg2, *Arg3, *Arg4;
    int lRet;
    bool first=true;
    int firstPC; 
    int secPC;
    
    do{ // first pass symbol table
        lRet = readAndParse( infile, asmLine, &label, &opCode, &Arg1, &Arg2, &Arg3, &Arg4 );
        if( lRet != DONE && lRet != EMPTY_LINE ){
          char* opCopy=opCode;
          if(first){ // loop until orig is found
            if(strncmp(asmLine, ".orig", 5)==0){ // found orig
              first=false;
              if(label[0]!='\0'){
                printf("Error code 4: illegal label: %s\n", label);
                exit(4);
              }
              if((toNum(Arg1)%2) == 0 )
                  firstPC = secPC = toNum(Arg1);
              else{
                  printf("Error code 3: Invalid constant: %s\n", opCopy);
                  exit(3);
              }
            }
            continue;
          }
          
          if((*label)!='\0'){
            
            if(!(isRealLabel(label))){

              printf("Error code 4: illegal label: %s\n" , label);
              exit(4);
            }
            if(tabCount>=MAX_SYMBOLS){
              printf("Error code 4: too many labels");
              exit(4);
            }
            symbolTable[tabCount].address=firstPC;
            strncpy(symbolTable[tabCount].label, label, MAX_LABEL_LEN);
            tabCount++;
          }      
        }
          firstPC+=2;
    } while( lRet != DONE );

    if(first){
      printf("Error code 4: could not find .ORIG \n");
      exit(4); 
    }

    rewind(infile);
    first=true;
    int regi;
    int oToFile=0;
    int tIndex=0;
    
    do{ // second pass turn to machine code
        
        oToFile=0;
        lRet = readAndParse( infile, asmLine, &label, &opCode, &Arg1, &Arg2, &Arg3, &Arg4 );
        
        if( lRet != DONE && lRet != EMPTY_LINE ){

          
          if(first){ // loop until orig is found
            if(strncmp(opCode, ".orig", 5)!=0){ // not orig            
              continue;
            }
            first=false;
            fprintf(outfile, "0x%.4X\n", secPC);
            continue;
          }
          secPC+=2;
          if(secPC>=65536){
            printf("Error code 4: PC overflow\n");
            exit(4);
          }
          if(strcmp(Arg4, "")!=0){
            printf("Error code 4: too many arguments\n");
            exit(4);
          }
          switch (isOpcode(opCode))
          {
          case ADD:
            oToFile+=(0x1<<12);
            oToFile+=(regToDec(Arg1)<<9);
            oToFile+=(regToDec(Arg2)<<6);

            if(Arg3[0]=='#' || Arg3[0]=='x'){//immediate value
              oToFile+=32;
              regi=toNum(Arg3);
              if(regi>15 || regi<-31){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x1f);

            }else if(Arg3[0]=='r'){  //or register           
              oToFile+=regToDec(Arg3);
            }else{ //anything else bad
              printf("Error code 4: invalid argument %s \n", Arg3);
              exit(4);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case AND:

            oToFile+=(0x5<<12);
            oToFile+=(regToDec(Arg1)<<9);
            oToFile+=(regToDec(Arg2)<<6);

            if(Arg3[0]=='#'  || Arg3[0]=='x'){//immediate value
              oToFile+=(32);
              regi=toNum(Arg3);
              if(regi>15  || regi<-31){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x1f);

            }else if(Arg3[0]=='r'){  //or register           
              oToFile+=regToDec(Arg3);
            }else{ //anything else bad
              printf("Error code 4: invalid argument %s\n", Arg3);
              exit(4);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case BR:
            if(strcmp(Arg2,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            if(opCode[2]=='z'){ //brz---

              if(opCode[3]=='p'){ //brzp
                oToFile=1536;               
              }else{ //brz
                oToFile=1024;
              }
            }
            else if(opCode[2]=='p'){ //brp
              oToFile=512;
            }
            else if(opCode[2]=='n'){ //brn---
              if(opCode[3]=='p'){ // brnp
              oToFile=2560;
              }
              if(opCode[3]=='z'){ //brnz
                oToFile=3072;
              }
              if(opCode[4]=='p'){ //brnzp
                oToFile+=3584;
              }
              else{ //brn
                oToFile=2048;
              }

            }
            if(Arg1[0]=='#' || Arg1[0]=='x'){//immediate
              regi=(toNum(Arg1));
              if(regi>255 || regi<-511){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x1ff);
            }else{
              tIndex=inTable(Arg1);
              if(tIndex!=-1){ //label found
                
                if((symbolTable[tIndex].address-secPC)>255 || (symbolTable[tIndex].address-secPC)<-511){
                  printf("Error code 3: invalid constant %d\n", regi);
                  exit(3);
                }
                oToFile|=(((symbolTable[tIndex].address-secPC)/2)&0x1ff); 
              }else{
                printf("Error code 4: invalid label %s\n", Arg1);
                exit(4);
              }
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);

          
            break;
          case HALT:
            if(strcmp(Arg1,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile=61477;
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case JMP:
            if(strcmp(Arg2,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=(49152);
            oToFile+=(regToDec(Arg1)<<6);
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case JSR:
            if(strcmp(Arg2,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=(18432);
            if(Arg1[0]=='#' || Arg1[0]=='x'){//immediate
              regi=(toNum(Arg1));
              if(regi>1023 || regi<-2047){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x7ff);
            }else{
              tIndex=inTable(Arg1);
              if(tIndex!=-1){ //label found
                if((symbolTable[tIndex].address-secPC)>255 || (symbolTable[tIndex].address-secPC)<-511){
                  printf("Error code 3: invalid constant %d\n", regi);
                  exit(3);
                }
                oToFile|=(((symbolTable[tIndex].address-secPC)/2)&0x7ff); //check if overflow
              }else{
                printf("Error code 4: invalid label %s\n", Arg1);
                exit(4);
              }
            }
            fprintf(outfile, "0x%.4X\n",oToFile&0xffff);
            break;
          case JSRR:
            if(strcmp(Arg2,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=(1<<14);
            oToFile+=(regToDec(Arg1)<<6);
            fprintf(outfile, "0x%.4X\n", oToFile);
            break;
          case LDB:
            oToFile+=1<<13;
            oToFile+=regToDec(Arg1)<<9;
            oToFile+=regToDec(Arg2)<<6;
            if(Arg3[0]=='#' || Arg3[0]=='x'){
              regi=toNum(Arg3);
              if(regi> 31 || regi<-63){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile+=(regi&0x3f);
            }else{
              printf("Error code 3: invalid constant %s\n", Arg3);
              exit(3);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case LDW:
            oToFile+=(0x6<<12);
            oToFile+=regToDec(Arg1)<<9;
            oToFile+=regToDec(Arg2)<<6;
            if(Arg3[0]=='#' || Arg3[0]=='x'){
              regi=toNum(Arg3);
              if(regi> 31 || regi<-63){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile+=(regi&0x3f);
            }else{
              printf("Error code 3: invalid constant %s\n", Arg3);
              exit(3);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
            break;
          case LEA:
            if(strcmp(Arg3,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=(0xE<<12);
            oToFile+=(regToDec(Arg1)<<9);
            if(Arg2[0]=='#' || Arg2[0]=='x'){//immediate
              regi=(toNum(Arg2));
              if(regi>255 || regi<-511){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x1ff);
            }else{
              tIndex=inTable(Arg2);

              if(tIndex!=-1){ //label found
                if((symbolTable[tIndex].address-secPC)>255 || (symbolTable[tIndex].address-secPC)<-511){
                  printf("Error code 3: invalid constant %d\n", regi);
                  exit(3);
                }
                oToFile|=(((symbolTable[tIndex].address-secPC)/2)&0x1ff); 
              }else{
                printf("Error code 4: invalid label %s\n", Arg2);
                exit(4);
              }
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case NOP:
            if(strcmp(Arg1,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case NOT:
            if(strcmp(Arg3,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=(0x9<<12);
            oToFile+=regToDec(Arg1);
            oToFile+=regToDec(Arg2);
            oToFile+=63;
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case RET:
            if(strcmp(Arg1,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile=49600;
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case RSHFA:
            oToFile+=32;
          case RSHFL:
            oToFile+=16;
          case LSHF:
            oToFile+=(0xD<<12);
            oToFile+=regToDec(Arg1)<<9;
            oToFile+=regToDec(Arg2)<<6;
            if(Arg3[0]!='#' || Arg3[0]=='x'){
              printf("Error code 3: invalid constant %s\n", Arg3);
              exit(3);
            }
            regi=toNum(Arg3);
            if(regi>15 || regi<0){
              printf("Error code 3: invalid constant %d\n", regi);
              exit(3);
            }
            oToFile+=regi;
            fprintf(outfile, "0x%.4X", oToFile);
            break;
          case RTI:
            if(strcmp(Arg1,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile=32768;
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case STB:
            oToFile+=0x3<<12;
            oToFile+=(regToDec(Arg1)<<9);
            oToFile+=(regToDec(Arg2)<<6);

            if(Arg3[0]=='#' || Arg3[0]=='x'){
              regi=toNum(Arg3);
              if(regi> 31 || regi<-63){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile+=(regi&0x3f);
            }else{
              printf("Error code 3: invalid constant %s\n", Arg3);
              exit(3);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case STW:
            oToFile+=(0x7<<12);
            oToFile+=(regToDec(Arg1)<<9);
            oToFile+=(regToDec(Arg2)<<6);

            if(Arg3[0]=='#' || Arg3[0]=='x'){
              regi=toNum(Arg3);
              if(regi> 31 || regi<-63){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile+=(regi&0x3f);
            }else{
              printf("Error code 3: invalid constant %s\n", Arg3);
              exit(3);
            }
            fprintf(outfile, "0x%.4X\n", oToFile&0xffff);
            break;
          case TRAP:
            if(strcmp(Arg2,"")!=0){
              printf("Error code 4: too many arguments\n");
              exit(4);
            }
            oToFile+=0xF0<<8;
            if(Arg1[0]!='x'){
              printf("Error code 3: invalid constant %s\n", Arg1);
              exit(3);
            }
            switch (toNum(Arg1))
            {
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
              oToFile+=toNum(Arg1);
              break;
            default:
              printf("Error code 3: invalid constant %s\n", Arg1);
              exit(3);
              break;
            }
            fprintf(outfile, "0x%.4X\n", oToFile);
            break;
          case XOR:
            oToFile+=(0x9<<12);
            oToFile+=(regToDec(Arg1)<<9);
            oToFile+=(regToDec(Arg2)<<6);

            if(Arg3[0]=='r'){ //register
              oToFile+=(regToDec(Arg3));
            }else if(Arg3[0]=='#' || Arg3[0]=='x'){ // immediate
              oToFile+=32;
              regi=toNum(Arg3);
              if(regi>15 || regi<-31){
                printf("Error code 3: invalid constant %d\n", regi);
                exit(3);
              }
              oToFile|=(regi&0x1f);
            }else{
              printf("Error code 4: invalid argument %s\n", Arg3);
            }
            fprintf(outfile, "0x%.4X\n", oToFile);
            break;
          default:
            if(strncmp(opCode, ".fill", 5) ==0 ){
              fprintf(outfile, "0x%.4X", toNum(Arg1));
            }
            else{
              printf("Error code 2: invalid opcode %s\n", opCode);
              exit(2);
            }
          }
          
        }

      
    }while( lRet != DONE );
    

    
    
    /* Do stuff with files */

    fclose(infile);
    fclose(outfile);

  printf("\nInput ran fully!\n");
  return EXIT_SUCCESS;


}