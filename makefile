todo: mensajeria
mensajeria: mensajeria.o
	g++ -Wall -o mensajeria mensajeria.o
mensajeria.o: mensajeria.c
	g++ -Wall -c mensajeria.c
limpiar: clean
clean:
	rm -f *.o
	rm -f mensajeria
