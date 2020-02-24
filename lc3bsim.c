/*
    Name 1: Yash Dhaduti
    Name 2: Sai Harish Rajanala
    UTEID 1: ysd97
    UTEID 2: shr546
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ADD 1
#define AND 5
#define BR 0
#define JMP 12
#define JSR 4
#define LDB 2
#define LDW 6
#define LEA 14
#define RTI 8
#define SHF 13
#define STB 3
#define STW 7
#define TRAP 15
#define XOR 9

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */    

  // FETCH INSTRUCTION
   int fetch = MEMORY[(CURRENT_LATCHES.PC>>1)][0]+(MEMORY[(CURRENT_LATCHES.PC>>1)][1]<<8);
   int opCode = (fetch&0xF000)>>12;
   int sext=0;
   bool setCC=false;
   int loadSext;
   int temp;
   CURRENT_LATCHES.PC+=2;
   NEXT_LATCHES.PC=CURRENT_LATCHES.PC;
   switch (opCode)
   {
    case ADD:

      if((fetch&0x0020)==0)
        NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + CURRENT_LATCHES.REGS[fetch&0x0007];
      else{
        if((fetch&0x0010)>=1)
          sext=(fetch&0x001f) | 0xFFFFFFE0;
        else
          sext=(fetch&0x001f);

        NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + sext;
      }
      setCC=true;
      break;

    case AND:
    
      if((fetch&0x0020)==0)
        NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] & CURRENT_LATCHES.REGS[fetch&0x0007];
      else{
        if((fetch&0x0010)>=1)
          sext=(fetch&0x001f) | 0xFFFFFFE0;
        else
          sext=(fetch&0x001f);

        NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] & sext;
      }
      setCC=true;
      break;

    case BR:

     if((fetch&0x0100)>=1)
       sext=(fetch&0x01FF)|0xFFFFFE00;
     else
       sext=(fetch&0x01FF);

     if((fetch&0x0E00) == 0)
       fetch|=0x0E00;
     
     if (((fetch&0x0800)&&CURRENT_LATCHES.N) || ((fetch&0x0400)&&CURRENT_LATCHES.Z) || ((fetch&0x0200)&&CURRENT_LATCHES.P)) 
       NEXT_LATCHES.PC=CURRENT_LATCHES.PC+(sext<<1);
     
     break;

    case JMP:
   
     NEXT_LATCHES.PC=CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6];
     break;

    case JSR:
     
     temp = CURRENT_LATCHES.PC;

     if((fetch&0x0400)>=1)
       sext=(fetch&0x07FF)|0xFFFFF800;
     else
       sext=(fetch&0x07FF);

     if((fetch&0x0800) == 0) 
      NEXT_LATCHES.PC=CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6];
     else
      NEXT_LATCHES.PC=CURRENT_LATCHES.PC+(sext<<1);

     NEXT_LATCHES.REGS[7]=temp;
     break;

    case LDB:
     
     loadSext;
     if((fetch&0x0020)>=1)
       sext=(fetch&0x003F)|0xFFFFFFC0;
     else
       sext=(fetch&0x003F);

     loadSext = MEMORY[(CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + sext)>>1][(CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + sext)%2];

     if((loadSext&0x0080) >= 1)
       loadSext|=0xFFFFFF00;
    
     NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = loadSext;

     setCC=true;
     break;

    case LDW:
   
     loadSext;
     if((fetch&0x0020)>=1)
       sext=(fetch&0x003F)|0xFFFFFFC0;
     else
       sext=(fetch&0x003F);

      loadSext = MEMORY[(CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + (sext<<1)) >> 1][0] + (MEMORY[(CURRENT_LATCHES.REGS[(fetch&0x01C0)>>6] + (sext<<1))>>1 ][1] << 8);
      
      if((loadSext&0x0080) >= 1)
       loadSext|=0xFFFFFF00;
      
      NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = loadSext;
     setCC=true;
     break;

    case LEA:

     if((fetch&0x0100)>=1)
       sext=(fetch&0x01FF)|0xFFFFFE00;
     else
       sext=(fetch&0x01FF);

     //printf("%d", ((fetch&0x0E00)>>9));
     NEXT_LATCHES.REGS[(fetch&0x0E00)>>9] = CURRENT_LATCHES.PC+(sext<<1);
     break;

    case SHF:
   
      if((fetch&0x0010)==0){ //LSHF

        NEXT_LATCHES.REGS[(fetch&0x0e00)>>9] = CURRENT_LATCHES.REGS[(fetch&0x0e00)>>6] << (fetch&0xf);
        if((fetch&0x0020)==0){ // RSHFL
          unsigned int logic = (unsigned int) (CURRENT_LATCHES.REGS[(fetch&0x0e00)>>6]);
          NEXT_LATCHES.REGS[(fetch&0x0e00)>>9] = logic >> (fetch&0xf);
        }else{ //RSHFA
          NEXT_LATCHES.REGS[(fetch&0x0e00)>>9] = (CURRENT_LATCHES.REGS[(fetch&0x0e00)>>6]) >> (fetch&0xf);
        }
      }

      setCC=true;
     break;

    case STB:
   
     if((fetch&0x0020)>=1)
       sext=(fetch&0x003F)|0xFFFFFFC0;
     else
       sext=(fetch&0x003F);

     MEMORY[(CURRENT_LATCHES.REGS[((fetch&0x01c0)>>6)]+sext)>>1][(CURRENT_LATCHES.REGS[((fetch&0x01c0)>>6)]+sext)%2] = CURRENT_LATCHES.REGS[(fetch&0x0e00>>9)]&0x00ff;
     break;
    
    case STW:
   
     if((fetch&0x0020)>=1)
       sext=(fetch&0x003F)|0xFFFFFFC0;
     else
       sext=(fetch&0x003F);
     
     MEMORY[(CURRENT_LATCHES.REGS[(fetch&0x01c0)>>6] + (sext<<1)) >> 1][0] = (CURRENT_LATCHES.REGS[(fetch&0x01c0)>>9] & 0xff);
     MEMORY[(CURRENT_LATCHES.REGS[(fetch&0x01c0)>>6] + (sext<<1)) >> 1][1] = ((CURRENT_LATCHES.REGS[(fetch&0x01c0)>>9] >> 8) & 0xff);
     break;

    case TRAP:
     NEXT_LATCHES.PC = 0;
     break;

    case XOR:
     
     if((fetch&0x0010)>=1)
          sext=(fetch&0x001f) | 0xFFFFFFE0;
        else
          sext=(fetch&0x001f);

     if((fetch&0x0020)==0)
       NEXT_LATCHES.REGS[(fetch&0x0e00)>>9] = ( CURRENT_LATCHES.REGS[(fetch&0x01c0)>>6] ^ CURRENT_LATCHES.REGS[(fetch&0x0007)] );
     else
       NEXT_LATCHES.REGS[(fetch&0x0e00)>>9] = (CURRENT_LATCHES.REGS[(fetch&0x01c0)>>6] ^ sext);
       
     setCC=true;
     break;

   default:
     break;
   }

   if(setCC){

     if(NEXT_LATCHES.REGS[(fetch&0x0E00)>>9]>0){
       NEXT_LATCHES.N=0;
       NEXT_LATCHES.Z=0;
       NEXT_LATCHES.P=1;
     }
     else if(NEXT_LATCHES.REGS[(fetch&0x0E00)>>9]==0){
       NEXT_LATCHES.N=0;
       NEXT_LATCHES.Z=1;
       NEXT_LATCHES.P=0;
     }
     else if(NEXT_LATCHES.REGS[(fetch&0x0E00)>>9]<0){
       NEXT_LATCHES.N=1;
       NEXT_LATCHES.Z=0;
       NEXT_LATCHES.P=0;
     }
     setCC=false;
   }


}