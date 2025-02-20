#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#define COLS 15
#define ROWS 20

static struct termios old_termios, new_termios;
static int exit_loop = 0;
static int direction = 0; // current direction


// BAIT

typedef struct Bait{
	int x;
	int y;
} Bait;

void respawn(Bait* bait)
{
	bait->x = (rand())%COLS;
	bait->y = (rand())%ROWS;
}

Bait* createBait(){
	Bait* bait = (Bait*) malloc(sizeof(Bait));
	respawn(bait);
	return bait;
}





// SNAKE CODE

typedef struct snakeNode
{
	int x;
	int y;
	struct snakeNode* next;
} snakeNode;

snakeNode* createSnakeNode()
{
	snakeNode* node = (snakeNode*) malloc(sizeof(snakeNode));
	node->next = NULL;
	return node;
}

typedef struct
{
	// we'll keep:
	//
	//	tail, head references
	//	each node has reference to next one, starting from left.
	//
	//	tail	    head
	//	 |	     |
	//	 # # # # # # O
	//
	
	snakeNode* tail;
	snakeNode* head;
} Snake;

Snake* createSnake()
{
	Snake* snake = (Snake*) malloc(sizeof(Snake));
	
	snake->head = createSnakeNode(); 
	snake->tail = snake->head;
	snake->head->next = NULL;

	snake->head->x = (COLS/2 + COLS%2);
	snake->head->y = (ROWS/2 + ROWS%2);

	return snake;
}

int snake_at(Snake* snake,int x,int y)
{
	snakeNode* node = snake->tail;
	while(node != snake->head)
	{
		if(node->x == x && node->y == y)return 1;
		node = node->next;
	}
	return 0;
}
void move(Snake* snake, Bait* bait)
{
	// check if move is wrong 
	// 1. at boundary
	if(snake->head->x == 0 && direction == 3){return;}
	if(snake->head->x == COLS-1 && direction == 4){return;}
	if(snake->head->y == 0 && direction == 1){return;}
	if(snake->head->y == ROWS-1 && direction == 2){return;}
	// 2. snake tries to go in itself
	if(snake_at(snake, snake->head->x-1, snake->head->y) && direction == 3)return;
     	if(snake_at(snake, snake->head->x+1, snake->head->y) && direction == 4)return;
	if(snake_at(snake, snake->head->x, snake->head->y-1) && direction == 1)return;
	if(snake_at(snake, snake->head->x, snake->head->y+1) && direction == 2)return;	
	
	// check if it ate smth on prev move
	if(snake->head->next != NULL)
	{
		// move only our newly created node
		snake->head = snake->head->next;
		
		 // for head mvmt
                switch(direction)
                {   
                        case 1: snake->head->y--; // UP
                                break;
                        case 2: snake->head->y++; // DOWN
                                break;
                        case 3: snake->head->x--; // LEFT
                                break;
                        case 4: snake->head->x++; // RIGHT
                                break;
                }  
	}
	else
	{
		// move all parts except head
		snakeNode* node = snake->tail;
		while(node!=snake->head)
		{
			node->x = node->next->x;
			node->y = node->next->y;	
			node = node->next;
		}

		// for head mvmt
		switch(direction)
		{
			case 1: node->y--; // UP
				break;
			case 2: node->y++; // DOWN
				break;
			case 3: node->x--; // LEFT
				break;
			case 4: node->x++; // RIGHT
				break;
		}
	}	

	// on head at bait
	if(bait->x == snake->head->x && bait->y == snake->head->y)
	{
		// respawn bait in another position
		while(bait->x == snake->head->x && bait->y == snake->head->y)
		{
			respawn(bait);
		}
		// make next for head, but don't move it
		snake->head->next = createSnakeNode();
		snake->head->next->x = snake->head->x;
		snake->head->next->y = snake->head->y;
	}	
}

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

// DRAWING
void draw(Snake* snake, Bait* bait)
{
	// clean terminal
	printf("\x1b[2J");
	printf("\x1b[0;0H");
	printf("\x1b[1;1H\x1b[2J");
	// redraw whole map with objects in it
	for(int i=0; i < ROWS; i++)
	{
		for(int j=0; j < COLS; j++)
		{
			if(bait->x == j && bait->y == i)
			{
				printf("\033[0;31mX\033[m ");
			}
			else if(snake->head->x == j && snake->head->y == i)
			{
				printf("\033[0;32mO\033[m ");
			}
			else if(snake_at(snake, j, i))
			{
				printf("\033[1;32m#\033[m ");
			}
			else printf(". ");
			//else if(snake_at(snake, j, i))
			{
			}
			// else if bait_at(i, j) draw red bait
			// else
		}
		printf("\n");
	}
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
	}
	return 0; // unknown input
}

int readArrowKey(char* str, int cursor)
{
	// since all arrow key consist of 3bytes 
	// first 2 of which are '\x1b' and '['
	if(str[cursor] == '\x1b' && str[cursor+1] == '[')
	{
		switch(str[cursor+2])
		{
			case 'A': return 1; // UP
                        case 'B': return 2; // DOWN
                        case 'D': return 3; // LEFT
                        case 'C': return 4; // RIGHT
		}
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
	
	// debugging
	/*if(read_size > 0)
	{
		printf("Input size: %d\n", read_size);
		printf("----read---\n%s\n-------", buf);
	}*/
	int key = 0; // based on our mappings
	
	for(int i = 0; i < read_size; i++)
	{
		// here prioritise key over arrow (machine as User case)
		// it's easier to prioritise arrow over key
		// because arrow is of 3bytes
		// arrow input check
		if(i*3 <= read_size-3)key = readArrowKey(buf, i);
		
		// 1 byte key inp. check
		int temp = readKey(buf, i);
		key = (key == 0)? temp: key; 

		if (key == 0) continue; // if nothing from mappings found
	}
	return key;
}

void printInput(int key)
{
	switch(key)
	{
		case 1:
			printf("Up\n");
			break;
		case 2:
			printf("Down\n");
			break;
		case 3:
			printf("Left\n");
			break;
		case 4:
			printf("Right\n");
			break;
	}	
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
	Snake* snake = createSnake();
	Bait* bait = createBait();

	timer.tv_nsec = 0.15 * 1000000000; // 0.15s, let process rest :)
	
	while(!exit_loop)
	{
		int lastInput = checkForInput();
		direction = (lastInput == 0)? direction: lastInput;
		printInput(direction);
		move(snake, bait);
		draw(snake, bait);
		nanosleep(&timer, NULL);
	}
	return 0;
}
