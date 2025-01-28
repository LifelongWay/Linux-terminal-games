#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

static struct termios old_termios, new_termios;
static int exit_loop = 0;

// GAME BOARD
typedef struct {
	char val;      // value
	char ansi[20]; // effect
} Cell;

int cursorX = -1;
int cursorY = -1;


Cell board[3][3] = {
        {{' ', "\033[m"} , {' ', "\033[m"} , {' ', "\033[m"}},  // Row 1
        {{' ', "\033[m"} , {' ', "\033[m"} , {' ', "\033[m"}},  // Row 2
        {{' ', "\033[m"} , {' ', "\033[m"} , {' ', "\033[m"}}   // Row 3
    };



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
	printf("\x1b[1;1H\x1b[2J");
}



// ENDINGS
void win()
{
	struct timespec sleepTime = {};

        sleepTime.tv_nsec = 0.3 * 1000000000;
        int count = 0;
	while(count < 14)
	{
		if(exit_loop == 1)return; // SIGINT
		printf("\x1b[1;1H\x1b[2J");
		printf("                    ");
		for(int i=0; i < 13 - count; i++)printf(" #");
		printf("\n\n\n");
		printf("\e[32m    		     __        _____ _   _ \n");
		printf("		     \\ \\      / |_ _| \\ | |\n");
		printf("		      \\ \\ /\\ / / | ||  \\| |\n");
		printf("		       \\ V  V /  | || |\\  |\n");
		printf("		        \\_/\\_/  |___|_| \\_|\n\e[m");
		printf("\n\n\n");
                nanosleep(&sleepTime, NULL);
                count++;
        }   
	
}

void nobody()
{
	struct timespec sleepTime = {};

   	sleepTime.tv_nsec = 0.3 * 1000000000;
   	int count = 0;
	while(count < 16)
	{
		if(exit_loop == 1)return;
		printf("\x1b[1;1H\x1b[2J");
		printf("                    ");
            	for(int i=0; i < 15 - count; i++)printf(" #");
		printf("\n\n\n");
		printf(" \e[33m	    	    ____    ____       _     __        __\n");
		printf("		    |  _ \\  |  _ \\     / \\    \\ \\      / /\n");
		printf("		    | | | | | |_) |   / _ \\    \\ \\ /\\ / / \n");
		printf("		    | |_| | |  _ <   / ___ \\    \\ V  V /  \n");
		printf("		    |____/  |_| \\_\\ /_/   \\_\\    \\_/\\_/   \e[m\n");
		printf("\n\n\n");
		nanosleep(&sleepTime, NULL);
         	count++;
	}
}
void lose()
{
	struct timespec sleepTime = {};
	sleepTime.tv_nsec = 0.3 * 1000000000;
        int count = 0;
        while(count < 14)
        {
		if(exit_loop == 1)return;
		printf("\x1b[1;1H\x1b[2J");
                printf("                    ");
                for(int i=0; i < 13 - count; i++)printf(" #");
                printf("\n\n\n");
		printf("\e[31m 		     _     ___  ____  _____ \n");
		printf("		    | |   / _ \\/ ___|| ____|\n");
		printf("		    | |  | | | \\___ \\|  _|  \n");
		printf("		    | |__| |_| |___) | |___ \n");
		printf("		    |_____\\___/|____/|_____|\e[m\n");
		printf("\n\n\n");
                nanosleep(&sleepTime, NULL);
                count++;
	}
}	

// Input Logic
int check_end() {
    // Check rows
	
	    // Check rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0].val == board[i][1].val && board[i][1].val == board[i][2].val && 
            board[i][0].val != ' ' && strcmp(board[i][0].ansi, board[i][1].ansi) == 0 && strcmp(board[i][1].ansi, board[i][2].ansi) == 0) {
            // Mark the winning cells purple using strcpy
            strcpy(board[i][0].ansi, "\033[35m");  // Purple color
            strcpy(board[i][1].ansi, "\033[35m");  // Purple color
            strcpy(board[i][2].ansi, "\033[35m");  // Purple color
            return 1;  
        }   
    }   

    // Check columns
    for (int j = 0; j < 3; j++) {
        if (board[0][j].val == board[1][j].val && board[1][j].val == board[2][j].val && 
            board[0][j].val != ' ' && strcmp(board[0][j].ansi, board[1][j].ansi) == 0 && strcmp(board[1][j].ansi, board[2][j].ansi) == 0) {
            // Mark the winning cells purple using strcpy
            strcpy(board[0][j].ansi, "\033[35m");  // Purple color
            strcpy(board[1][j].ansi, "\033[35m");  // Purple color
            strcpy(board[2][j].ansi, "\033[35m");  // Purple color
            return 1;    
        }   
    }   

    // Check diagonals
    if (board[0][0].val == board[1][1].val && board[1][1].val == board[2][2].val && 
        board[0][0].val != ' ' && strcmp(board[0][0].ansi, board[1][1].ansi) == 0 && strcmp(board[1][1].ansi, board[2][2].ansi) == 0) {
        // Mark the winning cells purple using strcpy
        strcpy(board[0][0].ansi, "\033[35m");  // Purple color
        strcpy(board[1][1].ansi, "\033[35m");  // Purple color
        strcpy(board[2][2].ansi, "\033[35m");  // Purple color
        return 1;
    }   

    if (board[0][2].val == board[1][1].val && board[1][1].val == board[2][0].val && 
        board[0][2].val != ' ' && strcmp(board[0][2].ansi, board[1][1].ansi) == 0 && strcmp(board[1][1].ansi, board[2][0].ansi) == 0) {
        // Mark the winning cells purple using strcpy
        strcpy(board[0][2].ansi, "\033[35m");  // Purple color
        strcpy(board[1][1].ansi, "\033[35m");  // Purple color
        strcpy(board[2][0].ansi, "\033[35m");  // Purple color
        return 1;  
    }  

    for(int i=0; i < 3; i++)for(int j=0; j < 3; j++)if(board[i][j].val == ' ')return -2; // game goes on
    return 0;  // DRAW!
}


void changeValue()
{
	switch(board[cursorY][cursorX].val)
	{
		case 'X':
			board[cursorY][cursorX].val = 'O';
			break;
		case 'O':
			board[cursorY][cursorX].val = '-';
			break;
		case '-':
			board[cursorY][cursorX].val = 'X';	
			break;
	}
}

void opponent_move()
{
	int x = rand()%3;
	int y = rand()%3;
	while(board[y][x].val != ' ')
	{
		y = rand()%3;
		x = rand()%3;
	}
	int t = rand()%2;
	if(t == 0)board[y][x].val = 'X';
	else board[y][x].val = 'O';
	strcpy(board[y][x].ansi, "\033[31m");
}

int changeBoard(int command)
{
	if(command >= 1 && command <= 9 && cursorX == -1)
	{
		// get grid coord-s
		int i = (command % 3 == 0)? command/3 - 1: command/3;
		int j = (command % 3 == 0)? 2 : command % 3 - 1;
		
		// if already filled, deny
		if(board[i][j].val != ' ')return -2;
		else {
			// will mean editing 
			cursorX = j;
			cursorY = i;
			strcpy(board[i][j].ansi, "\033[5m\033[48;5;8m");
			board[i][j].val = '-';
		}
	}
	else if(command == 10 && cursorX != -1)
	{
		// value is changed!
		changeValue();
	}	
	else if(command >= 1 && command <= 9 && cursorX != -1)
	{
		// if changing cell while editing another!
		// 1. restore old editing one
		board[cursorY][cursorX].val = ' ';
		strcpy(board[cursorY][cursorX].ansi, "\033[m");
	
		// 2. do as in first branch
		// get grid coord-s
                int i = (command % 3 == 0)? command/3 - 1: command/3;
                int j = (command % 3 == 0)? 2 : command % 3 - 1;

                // if already filled, deny
                if(board[i][j].val != ' ')return -2;
                else {
                        // will mean editing
                        cursorX = j;
                        cursorY = i;
                        strcpy(board[i][j].ansi, "\033[5m\033[48;5;8m");
                        board[i][j].val = '-';
                }
	}
	else if(command == 11 && cursorX != -1)
	{
		// if submitting answer
		if(board[cursorY][cursorX].val == '-')return -2;	
		
		// submit
		strcpy(board[cursorY][cursorX].ansi, "\033[32m");
		cursorX = -1;
		cursorY = -1;

		int t = check_end();
		if(t == -2)
		{
			opponent_move();
			t = check_end();
			if(t == 1)return -1;
		}
		return t;
	}
	return -2;
}



// DRAWING
void draw()
{
	
	// clean terminal
	printf("\x1b[2J");
	printf("\x1b[0;0H");
	printf("\x1b[1;1H\x1b[2J");
	
 printf("      	 _______ _        _______           _______ \n");        
 printf("	|__   __(_)      |__   __|         |__   __|        \n");
 printf("	   | |   _  ___     | | __ _  ___     | | ___   ___ \n");
 printf("	   | |  | |/ __|    | |/ _` |/ __|    | |/ _   / _  \n");
 printf("	   | |  | | (__     | | (_| | (__     | | (_) |  __/\n");
 printf("	   |_|  |_|(___|    |_|  _,_|(___|    |_| ___/  ___|\n");
                                                     
               printf("\n\n\n\n");          
	
	// grid
	for(int i=0; i < 3; i ++)
	{
		printf("                            ");
		printf(" ");
		for(int j=0; j < 3; j++)
		{
			if(j!=2)printf("%s%c\033[0m | ", board[i][j].ansi, board[i][j].val);
			else printf("%s%c\033[m\n", board[i][j].ansi, board[i][j].val);
		}
		printf("                            ");
		if(i!=2)printf("-----------\n");
	}

	// rules
	printf("\n\n");
    printf("\e[90m       +--------------------------------------------------+\n");
    printf("       |                    TIC-TAC-TOE                   |\n");
    printf("       |                                                  |\n");
    printf("       |     q w e                                        |\n");
    printf("       |     a s d                                        |\n");
    printf("       |     z x c                                        |\n");
    printf("       |                                                  |\n");
    printf("       |  - These are the keys to pick cells.             |\n");
    printf("       |    (Cell can't be picked if already taken)       |\n");
    printf("       |                                                  |\n");
    printf("       |  Spacebar - To change the value of a cell.       |\n");
    printf("       |  Enter - To submit the chosen cell value.        |\n");
    printf("       |                                                  |\n");
    printf("       +--------------------------------------------------+\e[m\n");
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
		case 'Q': return 1;
		case 'q': return 1;
		case 'W': return 2;
		case 'w': return 2;
		case 'E': return 3;
		case 'e': return 3;

		case 'A': return 4;
		case 'a': return 4;
		case 'S': return 5;
		case 's': return 5;
		case 'D': return 6;
		case 'd': return 6;
		
		case 'Z': return 7;
		case 'z': return 7;
		case 'X': return 8;
		case 'x': return 8;
		case 'C': return 9;
		case 'c': return 9;

		case ' ': return 10; // change
		case '\n': return 11; // enter
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



// MAIN
int main()
{
	configure_terminal();
	
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);	
	// timespec for smth like deltaT of frame 
	// (I'll add it just not to check for input each clk cycle)
	// (similar things are done in game engines like Unity)

	struct timespec timer = {};
	timer.tv_nsec = 0.15 * 1000000000; // 0.1s, let process rest :)
	while(!exit_loop)
	{
		//printf("iteration!\n");
		int t = changeBoard(checkForInput());
		draw();
		if(t == 1){win();break;}
		if(t == 0){nobody();break;}
	       	if(t == -1){lose();break;}

		nanosleep(&timer, NULL);
	}

	printf("\x1b[1;1H\x1b[2J");
	return 0;
}

