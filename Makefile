
client : common.h appliclient.c 
	gcc common.h appliclient.c -o client

serveur : common.h appliserveur.c
	gcc common.h appliserveur.c -o serveur

clean : rm *.o
