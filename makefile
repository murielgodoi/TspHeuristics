# Comentário do makefile de exemplo

tsp: tsp.o
	@echo "Gerando arquivo executável"
	gcc  *.o -o tsp -lm

tsp.o: tsp.c
	@echo "Compilando e gerando os objetos"
	gcc -O3 -c *.c -lm

clean:
	@echo "Apagando objetos e executáveis antigos..."
	rm -f *.o tsp

run:
	./tsp
