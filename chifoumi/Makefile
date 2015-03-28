CC= gcc
FLAGS = -Wall -Wextra

client : common_game.h chifoumi_client.c
	$(CC) $(FLAGS) common_game.h chifoumi_client.c -o client
serveur : common_game.h chifoumi_serveur.c
	$(CC) $(FLAGS) common_game.h chifoumi_serveur.c -o serveur

all : client serveur

clean : 
	rm serveur client 
