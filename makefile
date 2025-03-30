TARGET_PC ?= /home/julien/Documents/Cours/LE3/OCC/target-pc
LIBCA2I=libCA2I
LIBMCS=libMCS
CFLAGS=-I$(TARGET_PC)/include/ncurses -I$(TARGET_PC)/include
LDFLAGS=-L$(TARGET_PC)/lib -L$(LIBCA2I) -L$(LIBMCS)

all: pc pi 

########## COMPILATION Simple #########
pc : client-pc server-pc
client-pc : client.c libCA2I libMCS
	gcc  $(CFLAGS) $(LDFLAGS) $< -lCA2I -lMCS -lncurses -o $@

server-pc : server.c libCA2I libMCS
	gcc  $(CFLAGS) $(LDFLAGS) $< -lCA2I -lMCS -o $@

# Compilation des fichiers .o dans leurs sous-répertoires
libCA2I : $(LIBCA2I)/comm.o $(LIBCA2I)/jeu.o $(LIBCA2I)/affichage.o
	ar -q $(LIBCA2I)/libCA2I.a $^

$(LIBCA2I)/comm.o: $(LIBCA2I)/comm.c
	gcc $(CFLAGS) -c $< -o $@

$(LIBCA2I)/jeu.o: $(LIBCA2I)/jeu.c libMCS
	gcc $(CFLAGS) -c $< -o $@

$(LIBCA2I)/affichage.o: $(LIBCA2I)/affichage.c $(LIBCA2I)/jeu.o
	gcc $(CFLAGS) -c $< -o $@ -lncurses -DASCII_BORDER

libMCS : $(LIBMCS)/session.o $(LIBMCS)/data.o
	ar -q $(LIBMCS)/libMCS.a $^

session.o: $(LIBMCS)/session.c
	gcc $(CFLAGS) -c $< -o $@

data.o: $(LIBMCS)data.c
	gcc $(CFLAGS) -c $< -o $@

########## COMPILATION CROISEE #########
pi : client-pi server-pi install
TARGET_PI ?= /home/julien/Documents/Cours/LE3/OCC/target-pi
CFLAGS_PI=-I$(TARGET_PI)/include/ncurses -I$(TARGET_PI)/include
LDFLAGS_PI=-L$(TARGET_PI)/lib

CCC=/home/julien/Documents/Cours/LE3/OCC/CC/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

ncurses-pi : ncurses.c
	$(CCC) $(CFLAGS_PI) $(LDFLAGS_PI) $^ -o $@

cards-pi : cards.c
	$(CCC)  $(CFLAGS_PI) $(LDFLAGS_PI) $^ -o $@

install : 
	sshpass -p "raspberry" scp ncurses-pi pi@192.168.1.77:/home/pi/Projet/tests
	sshpass -p "raspberry" scp cards-pi pi@192.168.1.77:/home/pi/Projet/tests

clean: 
	rm -rf client-pc server-pc client-pi server-pi $(LIBCA2I)/*.o $(LIBMCS)/*.o $(LIBCA2I)/*.a $(LIBMCS)/*.a

	

	

	
	
