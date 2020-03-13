#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;


// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99


FILE *fp; // file struct for stdin
char **tokens;
char *line;

void fork_fn(int);
void runcmd(int , char **);
int pipes(int);




void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

	// open stdin as a file pointer 
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}




void tokenize (char * string)
{	int token_count=0;
	int size = MAX_TOKENS;
	char *this_token;
tokens = malloc(sizeof(char*)*MAX_TOKENS);
	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

		if (*this_token == '\0') continue;


		tokens[token_count] = this_token;


		//printf("Token %d: %s\n", token_count, tokens[token_count]);

		token_count++;

		// if there are more tokens than space ,reallocate more space
		if(token_count >= size){
			size*=2;

			assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
		}
	}

	fork_fn(token_count);


}

void read_command() 
{

	// getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1); 

//	printf("Shell read this line: %s\n", line);

	tokenize(line);
}

int run_command() {
	
	if (strcmp( tokens[0], EXIT_STR ) == 0)
		return EXIT_CMD;
	
	return UNKNOWN_CMD;
}

int main()
{
	initialize();

	do {	
		printf("mysh> ");
		read_command();
		
	} while( run_command() != EXIT_CMD );
	
	return 0;
}





void fork_fn(int n){

int newfd;
pid_t pid;

for(int i=0;i<n;i++){



if(*tokens[i]=='|'){

pipes(n);
}


}



pid = fork();

if(pid==0){


	for (int i =0;i< n;i++){

		if(*tokens[i]=='>'){


			if ((newfd = open(tokens[i+1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
				
			perror("open failed");	/* open failed */
			exit(1);
			}
			//printf("Token on i+1 gives %s\n",tokens[i+1]);
				//printf("the value of i+1 is : %d\n",(i+1));


		dup2(newfd, 1); 
		tokens[i]=NULL;
		execvp(tokens[0],tokens);
		
		}
	
	}
	
	for(int j=0;j<n;j++){
	if(*tokens[j]=='<'){

                        if ((newfd = open(tokens[j+1],O_RDONLY, 0644)) < 0) {
                        perror("read failed");  /* open failed */
                        exit(1);
                }


                dup2(newfd, 0); 
                tokens[j]=NULL;
                execvp(tokens[0],tokens);
                }
       		}
		
		execvp(tokens[0],tokens);	
		
  
  

        }


else if(pid ==-1){
printf("fork failed\n");

	}

else{
	int wait_status;
    pid_t terminated_child_pid = wait(&wait_status);
    if (terminated_child_pid == -1) {
      perror("wait");
      exit(EXIT_FAILURE);
    }
    else {//printf("Parent: my child %d terminates.\n", terminated_child_pid);
}
	

}
	}






int pipes(int n){

int status;
int pipe1[2];
pid_t cpid1,cpid2;
int p;

char **tokens1;
char **tokens2;


tokens1=malloc(sizeof(char*)*MAX_TOKENS);
tokens2=malloc(sizeof(char*)*MAX_TOKENS);


	for(int i=0;i<n;i++){
		if(*tokens[i]=='|'){
			p=i;
			//printf("p is %d\n",p);
			//printf("found pipe\n");
		}


	}

	for(int i=0;i<p;i++){

	tokens1[i]= tokens[i];
	//printf("tokens is : %s\t token 1 : %s\n",tokens[i],tokens1[i]);
	}
	tokens1[p]=NULL;

	int j=0;
	for(int i=p+1;i<n;i++){

	tokens2[j]=tokens[i];
//	printf("tokens2 is %s\n",tokens2[j]);
	j++;
	}
	tokens2[j]=NULL;

	


	// create a pipe
	if (pipe(pipe1) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	cpid1 = fork();
	// child1 executes
	if (cpid1 == 0) {
		//printf("In CHILD-1 (PID=%d): executing command %s ...\n", getpid(), argv1[0]);
		dup2(pipe1[1], 1);
		close(pipe1[0]);
		close(pipe1[1]);
		execvp(tokens1[0], tokens1);
	} 

	cpid2 = fork();
	// child2 executes
	if (cpid2 == 0) {
		//printf("In CHILD-2 (PID=%d): executing command %s ...\n", getpid(), argv2[0]);
		dup2(pipe1[0], 0);
		close(pipe1[0]);
		close(pipe1[1]);
		execvp(tokens2[0], tokens2);
	} 

	// parent executes
	close(pipe1[0]);
	close(pipe1[1]);
	
	waitpid(cpid1, &status, WUNTRACED);
	//printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), cpid1);
	waitpid(cpid2, &status, WUNTRACED);
	//printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), cpid2);
	exit(0);

	return 0;


}
