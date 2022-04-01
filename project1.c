/*
  Filename project1.c
  Daate 03/05/2021
  Author Junsik Seo
  Email jxs161930@utdallas.edu
  Description: This program will simulate a simple computer system consisting of a CPU and Memory
  The CPu and Memory will be simulated by separate processes that communicate
  Memory will contain one program that the CPU will execute and then simulation will end.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char *argv[]){
  
  // get file name and open it
  char *filename=argv[1];
  FILE *file= fopen(filename,"r");
  if (file==NULL){
    printf("The inputfile is not exist. Try again\n");
    return 1;
  }

  int readpipe[2],writepipe[2]; //readpipe[0]=Memory read, readpipe[1]=CPU read, writepipie[0]=CPU write, writepipe[1]=Memory write
  
  //pipes are created -if piep isn't created, it exit the program
  if (pipe(readpipe)<0||pipe(writepipe)<0){
    printf ("There was an error in the piping.\n");
    exit(1);
  }
  
  //start fork
  int forkvalue=fork();
  if (forkvalue==-1){
    printf("There was an error in the fork. No child was created.\n");
    return 1;
  }
  
  //Meomory process
  else if (forkvalue==0){	 
    
    //close parent's pipes
    close(readpipe[1]);
    close(writepipe[0]);
    
    char command;
    int address;
    int memory_array[2000]; //array for memory space
    char buffer[256];
     
    int line=0,data,i; 
    if (file == NULL){
      printf("File does not exist please check!\n");
    }
    //read from input file       
    while (fgets(buffer,256,file)!=NULL){
      if(buffer[0]=='.'){
	if(sscanf(buffer,".%d",&data)==1){
	  line = data;
	}		
      }
      
      else{
	if(sscanf(buffer,"%d",&data)==1){
	  memory_array[line]=data;
	  line++;
	}
      }
    }
    
    fclose(file);
    
    while(1){
      
      read(readpipe[0],&command,sizeof(char));
      //read command, return data at address
      if(command == 'r'){
	read(readpipe[0],&address,sizeof(int));
	data = memory_array[address];		
	write(writepipe[1],&data,sizeof(int));
      }
	  
	  
      //write command, memory store data
      else if(command == 'w'){
	read(readpipe[0],&address,sizeof(int));
	read(readpipe[0],&data,sizeof(int));
	memory_array[address]=data;
      }
      //exit command
      else if(command == 'e'){
	break;
      }
    }
  }
  
  //CPU process
  else{
    //close child's pipes   
    close(readpipe[0]); 
    close(writepipe[1]);
    
    //registers
    int PC,SP,IR,AC,X,Y,user_SP,user_PC; 
    int value,mode,address;
    char read_command='r',write_command='w',exit_command='e';
    //timer
    int timer = atoi(argv[2]);
    
    PC=0,mode=0,SP=1000;
    int count=0;
    
    while(1) {
      //Interrupt processsing
      if(count++>= timer&&(mode==0)){
	mode=1;  //change to kernel mode
	user_SP=SP;
	user_PC=PC;
	SP=1999; //set up SP and PC		
	PC=1000;     
	write(readpipe[1],&write_command,sizeof(char)); //store user SP and PC into the memory
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&user_SP,sizeof(int));	
	SP--;
	write(readpipe[1],&write_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&user_PC,sizeof(int));
	count=0;
      }
      //fetch the instruction from the memory
      write(readpipe[1],&read_command,sizeof(char));
      write(readpipe[1],&PC,sizeof(int));	
      PC++;
      read(writepipe[0],&IR,sizeof(int));
      
      switch(IR){
      case 1 :  //load the value  into the AC
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&value,sizeof(int));
	AC=value;
	break;
      case 2:	//load the value at the address into the AC
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));
	
	if (address >999 && mode==0){
	  printf("Memory violoation: accessing system address %d in user mode.\n",address);
	}
	else {
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&address,sizeof(int));	
	  read(writepipe[0],&value,sizeof(int));
	  AC=value;
	}
	break;	
      case 3:  //load the value from the address found in the given address into the AC
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));

	if (address  >999 && mode==0){
	  printf("Memory violoation: accessing system address %d in user mode.\n",address);
	}
	else {
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&address,sizeof(int));	
	  read(writepipe[0],&value,sizeof(int));
	  AC=value;

	  if (address >999 && mode==0){
	    printf("Memory violoation: accessing system address %d in user mode.\n",address);
	  }
	  else {
	    write(readpipe[1],&read_command,sizeof(char));
	    write(readpipe[1],&address,sizeof(int));	
	    read(writepipe[0],&value,sizeof(int));
	    AC=value;
	  }
	}
	break;
      case 4:  //Load the value at (address+X) into the AC
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));

	if (address+X >999 && mode==0){
	  printf("Memory violoation: accessing system address %d in user mode.\n",address);
	}
	else {
	  address+=X;
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&address,sizeof(int));	
	  read(writepipe[0],&value,sizeof(int));
	  AC=value;
	}
	break;
      case 5:  //load the value at (address+Y) into the AC
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));

	if (address+Y >999 && mode==0){
	  printf("Memory violoation: accessing system address %d in user mode.\n",address);
	}
	else {
	  address+=Y;
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&address,sizeof(int));	
	  read(writepipe[0],&value,sizeof(int));
	  AC=value;
	}
	break;
      case 6:  //load from (SP+X) into the AC
	if (SP+X >999 && mode==0){
	  printf("Memory violoation: accessing system address %d in user mode.\n",address);
	}
	else {
	  address = SP + X;
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&address,sizeof(int));	
	  read(writepipe[0],&value,sizeof(int));
	  AC=value;
	}
	break;
      case 7:  //store the value in the AC into the address
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));
	write(readpipe[1],&write_command,sizeof(char));
	write(readpipe[1],&address,sizeof(int));
	write(readpipe[1],&AC,sizeof(int));
	break;
      case 8:  //Gets a random int from 1 to 100 into the AC
	AC=rand()%100+1;
	break;
      case 9:  //if port=1, writes AC as an int to the screen 
	//if port=2, writes AC as a char to the screen
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&value,sizeof(int));

	if(value==1){
	  printf("%d",AC);
	}

	else if(value==2){
	  printf("%c",AC);
	}
	break;
      case 10:  //add the value in X to the AC
	AC+=X;
	break;
      case 11:  //add the value in Y to the AC
	AC+=Y;
	break;
      case 12:  //subtract the value in X from the AC
	AC-=X;
	break;
      case 13:  //subtract the value in Y from the AC
	AC-=Y;
	break;
      case 14:  //copy the value in the AC to X
	X=AC;
	break;
      case 15:  //copy the value in X to the AC
	AC=X;
	break;
      case 16:  //copy the value in the AC to Y
	Y=AC;
	break;
      case 17:  //copy the value in Y to the AC
	AC=Y;
	break;
      case 18:  //copy the value in AC to the SP
	SP=AC;
	break;
      case 19:  //copy the value in SP to the AC 
	AC=SP;
	break;
      case 20:  //jump to the address
	address=-1;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));
	PC = address;
	break;
      case 21:  //jump to the address only if the value in the AC is zero
	
	address=-1;
	if(AC==0){
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&PC,sizeof(int));
	}					
	PC++;
	
	if(AC==0){
	  read(writepipe[0],&address,sizeof(int));
	  PC=address;
	}
	break;
      case 22:  //jump to the address only if the value in the AC is not zero
	address=-1;
	if(AC!=0){
	  write(readpipe[1],&read_command,sizeof(char));
	  write(readpipe[1],&PC,sizeof(int));	
	}
	PC++;
	
	if(AC!=0){
	  read(writepipe[0],&address,sizeof(int));
	  PC=address;
	}
	break;			
      case 23:  //push return address onto stack, jump to the address
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&PC,sizeof(int));	
	PC++;
	read(writepipe[0],&address,sizeof(int));
	SP--;
	write(readpipe[1],&write_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&PC,sizeof(int));	
	PC=address;
	break;
      case 24:  //pop return address from the stack,jump to the address
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));	
	read(writepipe[0],&address,sizeof(int));
	SP++;
	PC = address;
	break;
      case 25: //increment the value in X
	X++;
	break;
      case 26: //decrement the value in X
	X--;
	break;
      case 27: //push AC onto stack
	SP--;
	write(readpipe[1],&write_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&AC,sizeof(int));		
	break;
      case 28: //pop from stack into AC
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));	
	read(writepipe[0],&AC,sizeof(int));
	SP++;
	break;
      case 29: //perform system call
	mode=1;  //change to kernel mode
	user_SP=SP;
	user_PC=PC;
	SP=1999; //set SP and PC 
	PC=1500;
	write(readpipe[1],&write_command,sizeof(char)); //store user SP and PC into the memory
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&user_SP,sizeof(int));	
	SP--;
	write(readpipe[1],&write_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));
	write(readpipe[1],&user_PC,sizeof(int));
	break;
      case 30: //return from system call
	mode=0; //set user moder
	write(readpipe[1],&read_command,sizeof(char)); //restore registers 
	write(readpipe[1],&SP,sizeof(int));	
	read(writepipe[0],&user_PC,sizeof(int));
	SP++;
	write(readpipe[1],&read_command,sizeof(char));
	write(readpipe[1],&SP,sizeof(int));	
	read(writepipe[0],&user_SP,sizeof(int));
	SP++;
	SP = user_SP;
	PC = user_PC;
	break;
      case 50 : //end execution	
	write(readpipe[1],&exit_command,sizeof(char));
	return 0;
      }
    }
  }
}
