#############################
# Répertoires et variables
#############################
# Cibles d'installation PC
TARGET_PC   ?= /home/julien/Documents/Cours/LE3/OCC/target-pc
PC_BIN_DIR   = $(TARGET_PC)/bin
PC_LIB_DIR	 = $(TARGET_PC)/lib

# Paramètres pour la compilation sur PC
TARGET_PC_INC = $(TARGET_PC)/include
CFLAGS      = -I$(TARGET_PC_INC)/ncurses -I$(TARGET_PC_INC)
LDFLAGS     = -L$(TARGET_PC)/lib -L$(LIBCA2I) -L$(LIBMCS)


# Cibles d'installation PI
TARGET_PI   ?= /home/julien/Documents/Cours/LE3/OCC/target-pi
PI_BIN_DIR   = $(TARGET_PI)/bin
PI_LIB_DIR	 = $(TARGET_PI)/lib

# Paramètres pour la compilation sur Raspberry Pi
CFLAGS_PI    = -I$(TARGET_PI)/include/ncurses -I$(TARGET_PI)/include
CFLAGS_WPI	 = -I$(TARGET_PI)/include/wiringPi 
LDFLAGS_PI   = -L$(TARGET_PI)/lib -L$(LIBCA2I) -L$(LIBMCS) 
CCC         ?= /home/julien/Documents/Cours/LE3/OCC/CC/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

# Autres variables
LIBCA2I  = libCA2I
LIBMCS   = libMCS
LIBWPI   = libWPI

# IP du serveur qui sera utilisé par les clients
SERVER_IP   = "\"192.168.63.194\"" 

# Cible de l'installation
RPI_IP      = 192.168.63.131
RPI_PASSWORD = "raspberry"

QUESTIONS = "Questions.txt"
ANSWERS   = "Answers.txt"

#############################
# Cibles principales
#############################
all: pc pi

#############################
# Partie PC (compilation native)
#############################
# Pour le PC, on compile server-pc
pc: $(PC_BIN_DIR)/server-pc

$(PC_BIN_DIR)/server-pc: server.c $(PC_LIB_DIR)/libCA2I $(PC_LIB_DIR)/libMCS | $(PC_BIN_DIR)
	gcc $(CFLAGS) $(LDFLAGS) server.c -lCA2I -lMCS -o $(PC_BIN_DIR)/server-pc


#############################
# Construction des bibliothèques (PC)
#############################
$(PC_LIB_DIR)/libCA2I: $(LIBCA2I)/comm.o $(LIBCA2I)/jeu.o
	ar -q $(PC_LIB_DIR)/libCA2I.a $^

$(LIBCA2I)/comm.o: $(LIBCA2I)/comm.c
	gcc $(CFLAGS) -c $< -o $@

$(LIBCA2I)/jeu.o: $(LIBCA2I)/jeu.c
	gcc $(CFLAGS) -c $< -o $@

$(PC_LIB_DIR)/libMCS: $(LIBMCS)/session.o $(LIBMCS)/data.o
	ar -q $(PC_LIB_DIR)/libMCS.a $^

$(LIBMCS)/session.o: $(LIBMCS)/session.c
	gcc $(CFLAGS) -c $< -o $@

$(LIBMCS)/data.o: $(LIBMCS)/data.c
	gcc $(CFLAGS) -c $< -o $@

#############################
# Partie Raspberry Pi (compilation croisée et installation complète)
#############################
# La règle pi compile, crée les répertoires distants et déploie tout (bin, lib, data).
pi: $(PI_BIN_DIR)/client-pi $(PI_BIN_DIR)/server-pi create_remote_dirs deploy-pi

$(PI_BIN_DIR)/client-pi: client.c $(PI_LIB_DIR)/libCA2I $(PI_LIB_DIR)/libMCS $(PI_LIB_DIR)/libWPI | $(PI_BIN_DIR)
	$(CCC) $(CFLAGS_PI) $(CFLAGS_WPI) $(LDFLAGS_PI) -L$(LIBWPI) client.c -lCA2I -lMCS -pthread -lWPI -lncurses -lwiringPi -DINADDR_SVC=$(SERVER_IP) -o $(PI_BIN_DIR)/client-pi
#	$(CCC) $(CFLAGS_PI) $(CFLAGS_WPI) $(LDFLAGS_PI) -L$(LIBWPI) client.c -lCA2I -lMCS -pthread -lWPI -lncurses -lwiringPi -DINADDR_SVC=$(SERVER_IP) -o $(PI_BIN_DIR)/client-pi -Wl,-rpath=\$$ORIGIN/../lib

$(PI_BIN_DIR)/server-pi: server.c $(PI_LIB_DIR)/libCA2I $(PI_LIB_DIR)/libMCS | $(PI_BIN_DIR)
	$(CCC) $(CFLAGS_PI) $(LDFLAGS_PI) server.c -lCA2I -lMCS -pthread -o $(PI_BIN_DIR)/server-pi
#	$(CCC) $(CFLAGS_PI) $(LDFLAGS_PI) server.c -lCA2I -lMCS -pthread -o $(PI_BIN_DIR)/server-pi -Wl,-rpath=\$$ORIGIN/../lib

#############################
# Construction des bibliothèques pour la Pi
#############################
# Librairie CA2I pour la Pi
$(PI_LIB_DIR)/libCA2I: $(LIBCA2I)/comm_pi.o $(LIBCA2I)/jeu_pi.o $(LIBCA2I)/affichage_pi.o
	ar -q $(PI_LIB_DIR)/libCA2I.a $^

$(LIBCA2I)/comm_pi.o: $(LIBCA2I)/comm.c
	$(CCC) $(CFLAGS_PI) -c $< -o $@

$(LIBCA2I)/jeu_pi.o: $(LIBCA2I)/jeu.c
	$(CCC) $(CFLAGS_PI) -c $< -o $@

$(LIBCA2I)/affichage_pi.o: $(LIBCA2I)/affichage.c $(LIBCA2I)/jeu_pi.o
	$(CCC) $(CFLAGS_PI) -c $< -o $@ -lncurses

# Librairie MCS pour la Pi
$(PI_LIB_DIR)/libMCS: $(LIBMCS)/session_pi.o $(LIBMCS)/data_pi.o
	ar -q $(PI_LIB_DIR)/libMCS.a $^

$(LIBMCS)/session_pi.o: $(LIBMCS)/session.c
	$(CCC) $(CFLAGS_PI) -c $< -o $@

$(LIBMCS)/data_pi.o: $(LIBMCS)/data.c
	$(CCC) $(CFLAGS_PI) -c $< -o $@

# Librairie WPI pour wiringPi
$(PI_LIB_DIR)/libWPI: $(LIBWPI)/comm_matriceBtn.o $(LIBWPI)/comm_matriceLed.o $(LIBWPI)/comm_7segments.o
	ar -q $(PI_LIB_DIR)/libWPI.a $^

$(LIBWPI)/comm_matriceBtn.o: $(LIBWPI)/comm_matriceBtn.c 
	$(CCC) $(CFLAGS_WPI) -pthread -c $< -o $@

$(LIBWPI)/comm_matriceLed.o: $(LIBWPI)/comm_matriceLed.c
	$(CCC) $(CFLAGS_WPI) -pthread -c $< -o $@

$(LIBWPI)/comm_7segments.o: $(LIBWPI)/comm_7segments.c
	$(CCC) $(CFLAGS_WPI) -pthread -c $< -o $@

#############################
# Création des répertoires distants sur la Pi
#############################
create_remote_dirs:
	sshpass -p $(RPI_PASSWORD) ssh pi@$(RPI_IP) "mkdir -p /home/pi/CA2I /home/pi/CA2I/bin /home/pi/CA2I/lib /home/pi/CA2I/data"

#############################
# Déploiement complet sur la Raspberry Pi
#############################
deploy-pi:
	# Transfert des exécutables
	sshpass -p $(RPI_PASSWORD) scp $(PI_BIN_DIR)/client-pi $(PI_BIN_DIR)/server-pi pi@$(RPI_IP):/home/pi/CA2I/bin
#	# Transfert des bibliothèques (ex. wiringPi.so, ncurses.so, etc.)
#	sshpass -p $(RPI_PASSWORD) scp -r $(PI_LIB_DIR)/* pi@$(RPI_IP):/home/pi/CA2I/lib
	# Transfert des fichiers de données
	sshpass -p $(RPI_PASSWORD) scp $(QUESTIONS) $(ANSWERS) pi@$(RPI_IP):/home/pi/CA2I/data

#############################
# Nettoyage
#############################
clean:
	rm -rf $(PI_BIN_DIR)/* $(PI_LIB_DIR)/*.a
	rm -rf $(PC_BIN_DIR)/* $(PC_LIB_DIR)/*.a
	rm -rf $(LIBCA2I)/*.o $(LIBMCS)/*.o $(LIBWPI)/*.o $(LIBCA2I)/*.a $(LIBMCS)/*.a $(LIBWPI)/*.a
