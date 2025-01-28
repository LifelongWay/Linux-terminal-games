#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>

#define COLS 15
#define ROWS 20

static struct termios old_termios, new_termios;
static int exit_loop = 0;
int cursor = 0; // simple cursor to navigate over buttons

// TERMINAL 
void reset_terminal() {
    printf("\e[m"); // reset color changes
    printf("\e[?25h"); // show cursor
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void configure_terminal() {
    tcgetattr(STDIN_FILENO, &old_termios);
        new_termios = old_termios; // save it to be able to reset on exit

    new_termios.c_lflag &= ~(ICANON | ECHO); // turn off echo + non-canonical mode
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    printf("\x1b[?25l"); // hide cursor
    atexit(reset_terminal);
}

// SIGINT
void signal_handler(__attribute__((unused)) int signum){
	// handling sigint 
	exit_loop = 1;
}

void signal_block(__attribute__((unused)) int signum)
{
	// nothing
}
// DRAWING
void draw(char* current_game)
{
	printf("\x1b[1;1H\x1b[2J");
    printf("\033[32m");  // Set text color to green
    printf("                   __  __  ____  _  _  __  __ \n");
    printf("                  (  \\/  )( ___)( \\( )(  )(  )\n");
    printf("                   )    (  )__)  )  (  )(__)( \n");
    printf("                  (_/\\/\\_)(____)(_)\\_)(______)\n");

    // BUTTONS
    if(cursor==0)    printf("\033[m\n\n              start       \e[5m[\e[m%s\e[5m]\e[m       exit      ", current_game);
if(cursor==-1)  printf("\033[m\n\n              \e[5m[\e[mstart\e[5m]\e[m       %s       exit      ", current_game); 
if(cursor==1)   printf("\033[m\n\n              start       %s       \e[5m[\e[mexit\e[5m]\e[m      ", current_game); 
printf("\033[0m\n\n");


	// RULES
	printf("\n\n");
	printf("+---------------------------------------------------------------+\n");
	printf("|                          MENU RULES                           |\n");
	printf("|                                                               |\n");
	printf("|  \e[32m'w'\e[m and \e[32m's'\e[m to change the game.                              |\n");
	printf("|  \e[32m'a'\e[m and \e[32m'd'\e[m to change the button.                            |\n");
	printf("|  When the desired game is chosen, choose \e[34m'start'\e[m and          |\n");
	printf("|  press \e[32mEnter\e[m to begin.                                        |\n");
	printf("|  To leave, choose \e[34m'exit'\e[m and press \e[32mEnter\e[m or press \e[32m'q'\e[m to exit.|\n");
	printf("|                                                               |\n");
	printf("+---------------------------------------------------------------+\n");


}


// INPUT HANDLING

/* MAPS:
 * 	[MEANING]    [Corresponding Value]  [Corresp. Keys]
 *
 *	undefined             0 	       other keys
 *	up                    1 	     'W' or '\x1b[A'
 *	down 	 	      2      	     'S' or '\x1b[B'
 *	left 		      3              'A' or '\x1b[C'
 *	right 		      4              'D' or '\x1b[D'
 */


int readKey(char* str, int cursor)
{
	// since I added arrows in inp. mappings 
	// I'll check each 3 bytes
	
	switch(str[cursor]){
		case 'W': return 1;
		case 'w': return 1;

		case 'S': return 2;
		case 's': return 2;

		case 'A': return 3;
		case 'a': return 3;

		case 'D': return 4;
		case 'd': return 4;

		case '\n': return 5;
		case 'q': return 6;
		case 'Q': return 6;
	}
	return 0; // unknown input
}


int checkForInput()
{
/*	note: since I have set time/frame something approx. 0.1s  
 *	      our input may consist of more than 1 char 
 *	      in fact if our user isn't a human, but an another machine
 *	      then we can easily get such input
 *	      that's why I'll get 4096 bytes from stdin 
 *	   though human could type at most 2 chars during 0.1x( i checked)
 */ 
	char buf[4096]; // i think that's max size for stdin buffer :/
	size_t read_size = read(STDIN_FILENO, buf, sizeof(buf));
	int key = 0; // based on our mappings
	
	for(int i = 0; i < read_size; i++)
	{	
		// 1 byte key inp. check
		int temp = readKey(buf, i);
		key = (key == 0)? temp: key; 

		if (key == 0) continue; // if nothing from mappings found
	}
	return key;
}






typedef struct Node
{
	char string[20];
	struct Node* next;
	struct Node* prev;
} Node;

typedef struct List
{
	Node* tail;
	Node* head;
} List;

void addString(List* list, char* str)
{
	Node* node = (Node*) malloc(sizeof(Node));
	strcpy(node->string, str);
	node->prev = NULL;
	node->next = NULL;

	if(list->head == NULL)
	{
		list->head = node;
		list->tail = node;
	}
	else
	{
		node->next = list->tail;
		list->tail->prev = node;
		list->tail = node;
	}	
	
}

List* createList()
{
	List* list = (List*) malloc(sizeof(List));
	list->tail = NULL;
	return list;
}

// MAIN
int main()
{
	configure_terminal();
	
    printf("\x1b[?25l"); // hide cursor
	signal(SIGINT, signal_handler);	
	// timespec for smth like deltaT of frame 
	// (I'll add it just not to check for input each clk cycle)
	// (similar things are done in game engines like Unity)

	struct timespec timer = {};
	timer.tv_nsec = 0.15 * 1000000000; // 0.15s, let process rest :)


	// SCAN DIR
	List* fileList = createList();

	struct dirent *entry;
	DIR *dp;

	    // Open the current directory ("." refers to the current directory)
	    dp = opendir(".");
	    if (dp == NULL) {
		perror("opendir");
		return 1; // Error opening directory
	    }

	    // Print the filenames in the directory that start with "game_"
	    while ((entry = readdir(dp)) != NULL) {
		// Only print filenames that start with "game_"
		if (entry->d_type == DT_REG && strncmp(entry->d_name, "game_", 5) == 0) {
		    addString(fileList, entry->d_name);
		}
	    }

	    // Close the directory
	    closedir(dp);
	
	Node* node = fileList->tail;
	// RESPOND TO INP
	while(!exit_loop)
	{
		//printf("iteration!\n");
		int lastInput = checkForInput();
		if(lastInput == 1 && cursor == 0)
		{
			if(node->next == NULL)node = fileList->tail;
			else node = node->next;
		}	
		else if(lastInput == 2 && cursor == 0)
		{
			if(node->prev == NULL)node = fileList->head;
			else node = node->prev;
		}
		else if(lastInput == 3)
		{
			if(cursor > -1)cursor--;
		}
		else if(lastInput == 4)
		{
			if(cursor < 1)cursor++;
		}
		else if(lastInput == 5)
		{
			if(cursor == -1) // entering "start"
			{
				pid_t pid = fork();
				
				if(pid == -1)
				{
						
				}
				else if(pid == 0)
				{
					// child (game) proc
					// Use execvp to execute the program
					char path[20] = "./";
					strcat(path, node->string);
					
					char *args[]={path, NULL};
					execvp(args[0], args);
					printf("CIHLD\n");
					// If execvp fails, this line will be executed
					perror("exec failed");
				}
				else
				{
					// at pareent
					signal(SIGINT, signal_block);
					wait(NULL);  // Wait for the child process to finish	
    					printf("\x1b[?25l"); // hide cursor
					signal(SIGINT, signal_handler);
				}
			}
			else if(cursor == 0) // entering game
			{
				
			}
			else // entering "exit"
			{	
				break;
			}
		}
		else if(lastInput == 6)
		{
			// terminate
			break;
		}
	
		draw(node->string);
		
		nanosleep(&timer, NULL);
	}
	printf("\x1b[1;1H\x1b[2J");
	return 0;
}

