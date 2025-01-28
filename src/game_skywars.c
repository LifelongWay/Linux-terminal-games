#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#define WIDTH 39
#define HEIGHT 50


static struct termios old_termios, new_termios;
static int exit_loop = 0;
int score = 0;
int level = 0;

char art[10][40] = {
        "  ___ _                             ",
        " / __| |___  _  __ __ ____ _ _ _ ___",
        " \\__ \\ / / || | \\ V  V / _` | '_(_-<",
        " |___/_\\_\\\\_, |  \\_/\\_/\\__,_|_| /__/",
        "          |__/                        ",
        "+----------------------+              ",
        "| 'a' / 'A' - Move R    |              ",
        "| 'b' / 'B' - Move L    |              ",
        "| Spacebar   - Fire     |              ",
        "+----------------------+"
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



// PLANE

typedef struct Plane
{
	char chars[13][40];

	int width;
	int height;
	int x; // leftmost x
	
	// to get whether it's plane
	// use is_plane(Plane* plane, int x, int y)
       
} Plane;


Plane* createPlane()
{
	// Allocate memory for the Plane struct
	Plane* plane = (Plane*) malloc(sizeof(Plane));

// Initialize the chars array with the new plane art
    strcpy(plane->chars[0], "       /\\       ");
    strcpy(plane->chars[1], "      |ee|       ");
    strcpy(plane->chars[2], "      |{}|       ");
    strcpy(plane->chars[3], " _____/^^\\_____ ");
    strcpy(plane->chars[4], "/=====|PP|=====\\");
    strcpy(plane->chars[5], "======.  .====== ");
    strcpy(plane->chars[6], "      ||||       ");
    strcpy(plane->chars[7], "       ||        ");
    strcpy(plane->chars[8], "       ||        ");
    strcpy(plane->chars[9], "       ||        ");
    strcpy(plane->chars[10],"   ,---||---,    ");
    strcpy(plane->chars[11],"   '---<>---'    ");

    // Set the plane dimensions based on the new design
    plane->width = 16;   // The longest line is 23 characters wide
    plane->height = 12;  // The plane has 12 lines

	// set initial position
	plane->x = 0;
	
    	return plane; 
}







// BOMB
typedef struct Bomb 
{
	int x;
	int y;
} Bomb;

typedef struct bombNode
{
	Bomb* bomb;
	struct bombNode* next;
} bombNode;
typedef struct bombList
{
	bombNode* tail;
	bombNode* head;
} bombList;

bombList* createBombList()
{
	bombList* list = (bombList*) malloc(sizeof(bombList));
	list->tail = NULL;
	list->head = NULL;
	return list;
}

void addBomb(bombList* list, Bomb* bomb)
{
	bombNode* node = (bombNode*) malloc(sizeof(bombNode));
	node->bomb = bomb;
	node->next = NULL;
	if(list->head == NULL)
	{
		list->head = node;
		list->tail = list->head;
	}
	else
	{
		node->next = list->tail;
		list->tail = node;
	}
}

Bomb* spawnBomb()
{
	Bomb* bomb = (Bomb*) malloc(sizeof(Bomb));
	bomb->x = rand() % WIDTH;
	bomb->y = 0;
	return bomb;
}

int is_bomb(bombList* list, int x, int y)
{
	bombNode* node = list->tail;
	while(node != NULL)
	{
		if(node->bomb->x == x && node->bomb->y == y)return 1;
		node = node->next;
	}
	return 0;
}

// BOMB AS FIRE
bombList* createFireList()
{
	return createBombList();
}
Bomb* spawnFire(Plane* plane)
{
	Bomb* bomb = spawnBomb();
	bomb->y = HEIGHT - plane->height - 1;
	bomb->x = plane->x + plane->width/2;
	return bomb;
}


void addFire(bombList* list, Bomb* fire)
{
	addBomb(list, fire);
}

int is_fire(bombList* list, int x, int y)
{
	return is_bomb(list, x, y);
}


void spawnDoubleFire(bombList* fires, Plane* plane)
{
	Bomb* rightFire = spawnFire(plane);
	addFire(fires, rightFire);
	if(plane->x + plane->width/2 >0 )
	{
		Bomb* leftFire = spawnFire(plane);
		leftFire->x--;
		addFire(fires, leftFire);
	}
	else
	{
		// don't create left fire if out of bound
	}
}

// MOVE
void move(int pressedKey, bombList* bombs, bombList* fires, Plane* plane)
{
	// MOVE BOMBS
	bombNode* node = bombs->tail;
	bombNode* prev = NULL;
	while(node != NULL)
	{
		node->bomb->y++;
		
		// if bomb passed out of border, free it!
		if(node->bomb->y == HEIGHT)
		{
			if(prev == NULL)
			{
				//freeing tail
				bombs->tail = node->next;
				bombNode* nodeToFree = node;
				node = node->next;
				free(nodeToFree);
				continue;
			}	
			else
			{
				// freeing not tail 	
				prev->next = node->next;
				bombNode* nodeToFree = node;
				node = node->next;
				free(nodeToFree);
				continue;
			}
		}
		prev = node;
		node = node->next;
	}	


	// MOVE FIRES
        node = fires->tail;
        prev = NULL;
        while(node != NULL)
        {   
                node->bomb->y--;
    
                // if bomb passed out of border, free it!
                if(node->bomb->y == -1)
                {   
                        if(prev == NULL)
                        {   
                                //freeing tail
                                fires->tail = node->next;
                                bombNode* nodeToFree = node;
                                node = node->next;
                                free(nodeToFree);
                                continue;
                        }          
                        else
                        {   
                                // freeing not tail     
                                prev->next = node->next;
                                bombNode* nodeToFree = node;
                                node = node->next;
                                free(nodeToFree);
                                continue;
                        }   
                }   
                prev = node;
                node = node->next;
        } 



	// MOVE PLANE
	
	if(pressedKey == 2 && plane->x < WIDTH - plane->width/2)
	{plane->x++;
		printf("X: %d\n", plane->x); 
	}
	if(pressedKey == 1 && plane->x > -1* plane->width/2)plane->x--;
	
	// ON FIRE PRESSED
	
	if(pressedKey == 3)spawnDoubleFire(fires, plane);
}

int collider(bombList* bombs, bombList* fires, Plane* plane)
{
	// Fire collides with bombs
	bombNode* bNode = bombs->tail;
	bombNode* fNode;
	bombNode* bPrev = NULL;
	bombNode* fPrev;

	int flag_collision = 0;
	while(bNode != NULL)
	{
		if(bNode->bomb->y < HEIGHT - plane->height)
		{
			fPrev = NULL;
			fNode = fires->tail;
			while(fNode != NULL)
			{
				if(fNode->bomb->x == bNode->bomb->x && fNode->bomb->y == bNode->bomb->y
				|| fNode->bomb->x == bNode->bomb->x && fNode->bomb->y == bNode->bomb->y-1)
				{
					// COLLISION !
					score++;
					

					// Destroy Bomb
					if(bPrev == NULL)
					{
						//freeing tail
						bombs->tail = bNode->next;
						bombNode* nodeToFree = bNode;
						bNode = bNode->next;
						free(nodeToFree);
					}
					else
					{
						// freeing not tail
						bPrev->next = bNode->next;
						bombNode* nodeToFree = bNode;
						bNode = bNode->next;
						free(nodeToFree);
					}
						
					
					
					
					// Destroy Fire
					if(fPrev == NULL)
					{
						//freeing tail
						fires->tail = fNode->next;
						bombNode* nodeToFree = fNode;
						fNode = fNode->next;
						free(nodeToFree);
					}
					else
					{
						// freeing not tail     
						fPrev->next = fNode->next;
						bombNode* nodeToFree = fNode;
						fNode = fNode->next;
						free(nodeToFree);
					}
					flag_collision = 1;
					break;
				}
				
				fPrev = fNode;
				fNode = fNode->next;
			}
		}
	
		// else check if collides with plane (done below)
		if(bNode == NULL) break; // iff last headNode was freed
		if(flag_collision){flag_collision = 0; continue;}
		bPrev = bNode;
		bNode = bNode->next;	
	}

	// Plane collides with bomb
	bNode = bombs->tail;
	while(bNode != NULL)
	{
		int i = bNode->bomb->y;
		int j = bNode->bomb->x;
		if(i >= HEIGHT - plane->height && j >= plane->x && j < plane->x + plane->width)
 if(plane->chars[i - (HEIGHT - plane->height)][j - plane->x] != ' ') return 1; // collides with our plane!


		bNode = bNode->next;
	}

	return 0;
}


// DRAWING
void draw(bombList* bombs, bombList* fires, Plane* plane)
{
	// clean terminal
	//printf("\x1b[2J");
	//printf("\x1b[0;0H");
	printf("\x1b[1;1H\x1b[2J");
	// redraw whole map with objects in it
	for(int i=0; i < 10; i++)
	{
		if(i == 6)printf("%s   SCORE: %d\n",art[i], score);
		else if(i == 7)printf("%s   LEVEL: %d\n",art[i], level+1);
		else printf("%s\n",art[i]);
	}
		for(int i=0; i < HEIGHT; i++)
	{
		for(int j=0; j < WIDTH; j++)
		{
	 		if(is_bomb(bombs, j, i)) printf("\033[91m@ \033[m");
			else if(is_fire(fires, j, i)) printf("\033[38;5;46m^ \033[m");
			else if(i >= HEIGHT - plane->height && j >= plane->x && j < plane->x + plane->width)
			{
				char planeChar = 
				plane->chars[i - (HEIGHT - plane->height)][j - plane->x];
				if(planeChar == ' ')printf("\033[48;5;7m. \033[m");
				else printf("\033[34m%c \033[m", planeChar);
			}
			else	printf("\033[48;5;7m. \033[m");
		}
		printf("\n");
	}
}


// INPUT HANDLING

/* MAPS:
 * 	[MEANING]    [Corresponding Value]  [Corresp. Keys]
 *
 *	undefined             0 	       other keys
 *	left 		      1              'A' or '\x1b[C'
 *	right 		      2              'D' or '\x1b[D'
 *      fire     	      3                  ' ' 
 */    


int readKey(char* str, int cursor)
{

	switch(str[cursor]){	
		case 'A': return 1;
		case 'a': return 1;
		case 'D': return 2;
		case 'd': return 2;
		case ' ': return 3;
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
		key = readKey(buf, i);
		if (key == 0) continue; // if nothing from mappings found
	}
	return key;
}

void end()
{
	

	// on end
	// Print the frame and ASCII art with the score
	struct timespec sleepTime = {};

   sleepTime.tv_nsec = 0.3 * 1000000000;
   int count = 0; 
	while(count < 16)
    	{
	    if(exit_loop == 1)return; // SIGINT
	    printf("\x1b[1;1H\x1b[2J");
	    printf("			");
	    for(int i=0; i < 15 - count; i++)printf(" #");
	    printf("\n");
	    printf("	+-------------------------------------------------------------+\n");
	    printf("	|                                                             |\n");
	    printf("	|    ___    __    __  __  ____    _____  _  _  ____  ____     |\n");
	    printf("	|   / __)  /__\\  (  \\/  )( ___)  (  _  )( \\/ )( ___)(  _ \\    |\n");
	    printf("	|  ( (_-. /(__)\\  )    (  )__)    )(_)(  \\  /  )__)  )   /    |\n");
	    printf("	|   \\___/(__)(__)(_/\\/\\_)(____)  (_____)  \\/  (____)(_\\_)     |\n");
	    printf("	|                                                             |\n");
	    printf("	|                          SCORE: %-3d                         |\n", score);
	    printf("	|                                                             |\n");
	    printf("	+-------------------------------------------------------------+\n");
	    printf("			");
	    nanosleep(&sleepTime, NULL);
    	 count++;
	}
}

// MAIN
int main()
{
	configure_terminal();

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	// ITERATION DELAYS for not overkilling cpu
	struct timespec sleepTime = {};
	sleepTime.tv_nsec = 0.1 * 1000000000; // 0.1s, let process rest :)
	
	// SPAWNING TIMERS
	int seconds_gone = 0;
	time_t game_start_time = time(NULL);
	time_t bomb_time = time(NULL);
	int game_time;
	int bomb_spawner_timer;
	
	// Oject lists
	bombList* bombs = createBombList();
	bombList* fires = createFireList();
	// Plane
	Plane* plane = createPlane();
	
	// MAIN LOOP
	while(!exit_loop)
	{
		game_time = (long)(time(NULL) - game_start_time);
		bomb_spawner_timer = (long)(time(NULL) - bomb_time);
		level = game_time/25;
		printf("GAME TIME: %d\n", game_time);
		if(bomb_spawner_timer >= 4 - level*0.3)
		{
			printf("BOMB\n");
			addBomb(bombs, spawnBomb());
			if(level+1>1)addBomb(bombs, spawnBomb());		
			if(level+1>2)addBomb(bombs, spawnBomb());	
			if(level+1>4)addBomb(bombs, spawnBomb());	
			bomb_time = time(NULL);
		}

		int input = checkForInput();
		move(input, bombs, fires, plane);
		if(collider(bombs, fires, plane)){ end(); exit_loop = 1;}
		draw(bombs, fires, plane);

		/* Debugging
		bombNode* node = bombs->tail;
		printf("FROM TAIL OF BOMGS\n");
		while(node != NULL)
		{
			printf("Node: y = %d, x = %d\n", node->bomb->y, node->bomb->x);
			node = node->next;
		}

		node = fires->tail;
		printf("FROM TAIL OF FIRES\n");
		while(node != NULL)
		{
			printf("Node: y = %d, x = %d\n", node->bomb->y, node->bomb->x);
			node = node->next;
		}

		*/
		nanosleep(&sleepTime, NULL);
	}
	printf("\x1b[1;1H\x1b[2J");
    return 0;
}



