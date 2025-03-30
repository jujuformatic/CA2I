# makefile pour compiler les exemples 
# hello.c et menu.c venant du ncurses HOWTO
# https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html

TARGET_PC ?= /home/julien/Documents/Cours/LE3/OCC/target-pc
CFLAGS=-I$(TARGET_PC)/include/ncurses -I$(TARGET_PC)/include
LDFLAGS=-L$(TARGET_PC)/lib

all: pc pi 

########## COMPILATION Simple #########
pc : cards_ncurses-pc
cards_ncurses-pc : cards_ncurses.c 
	@echo "edition de liens avec la lib ncurses : option -lncurses"
	@echo "localisation des fichiers d'entete non standards : option -I"
	@echo "localisation des librairies non standards : option -L"
	gcc  $(CFLAGS) $(LDFLAGS) $^ -o $@ -lncurses -DASCII_BORDER

########## COMPILATION CROISEE #########
pi : cards_ncurses-pi install
TARGET_PI ?= /home/julien/Documents/Cours/LE3/OCC/target-pi
CFLAGS_PI=-I$(TARGET_PI)/include/ncurses -I$(TARGET_PI)/include
LDFLAGS_PI=-L$(TARGET_PI)/lib

CCC=/home/julien/Documents/Cours/LE3/OCC/CC/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

cards_ncurses-pi : cards_ncurses.c
	$(CCC) $(CFLAGS_PI) $(LDFLAGS_PI) $^ -o $@ -lncurses

install : 
	sshpass -p "raspberry" scp cards_ncurses-pi pi@192.168.1.77:/home/pi/Projet/tests

clean: 
	rm -rf cards_ncurses-pc cards_ncurses-pi

	

	

	
	
