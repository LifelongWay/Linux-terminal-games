rel_path = ./src/

installation: $(rel_path)game_skywars $(rel_path)game_snake $(rel_path)game_tictactoe main-screen
	
$(rel_path)game_skywars: $(rel_path)skywars.c
	gcc $(rel_path)skywars.c -o $(rel_path)game_skywars

$(rel_path)game_snake: $(rel_path)snake.c
	gcc $(rel_path)snake.c -o $(rel_path)game_snake

$(rel_path)game_tictactoe: $(rel_path)tictactoe.c
	gcc $(rel_path)tictactoe.c -o $(rel_path)game_tictactoe

main-screen: $(rel_path)main-screen.c
	gcc $(rel_path)main-screen.c -o main-screen
clean:
	rm -r $(rel_path)
rel_path = ./src/

