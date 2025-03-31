DESTDIR=/home/simon/Documents/libs/wiringPi/target-wpi
CCC=/home/simon/Documents/libs/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc
CFLAGS=-I$(DESTDIR)/include
LDFLAGS=-L$(DESTDIR)/lib -lpthread
IPJOYPI=192.168.109.94

all: install

comm_matriceBtn.o: comm_matriceBtn.c 
	$(CCC) $(CFLAGS) $(LDFLAGS) -c comm_matriceBtn.c -o comm_matriceBtn.o -lwiringPi

comm_matriceBtn: testBtn.c comm_matriceBtn
	$(CCC) $(CFLAGS) $(LDFLAGS)  comm_matriceBtn.o testBtn.c -o comm_matriceBtn -lwiringPi
	sshpass -v -p raspberry  scp comm_matriceBtn pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/

comm_matriceLed.o: comm_matriceLed.c
	$(CCC) $(CFLAGS) $(LDFLAGS) -c comm_matriceLed.c -o comm_matriceLed.o -lwiringPi

comm_matriceLed: comm_matriceLed.c
	$(CCC) $(CFLAGS) $(LDFLAGS)  comm_matriceLed.c -o comm_matriceLed -lwiringPi
	sshpass -v -p raspberry  scp comm_matriceLed pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/

comm_7segments.o: comm_7segments.c
	$(CCC) $(CFLAGS) $(LDFLAGS) -c comm_7segments.c -o comm_7segments.o -lwiringPi

test: test.c
	$(CCC) $(CFLAGS) $(LDFLAGS)  test.c -o test -lwiringPi
	sshpass -v -p raspberry  scp test pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/

install: comm_matriceBtn.o comm_matriceLed.o comm_7segments.o
	sshpass -v -p raspberry  scp comm_matriceLed.o pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/
	sshpass -v -p raspberry  scp comm_matriceBtn.o pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/
	sshpass -v -p raspberry  scp comm_7segments.o pi@$(IPJOYPI):/home/pi/Desktop/Projet/comm_modules/

clean: 
	rm -rf comm_matriceBtn.o
	rm -rf comm_matriceLed.o
	rm -rf comm_7segments.o