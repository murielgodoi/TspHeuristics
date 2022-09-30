#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>

typedef struct{
  float x;
  float y;
  float z;
} Coordinate;

typedef struct{
  char name[100];
  char type[100];
  char comment[100];
  int dimension;
  char edgeWeightType[20];
  Coordinate* nodes;
  float** distances;
} Instance;

Instance readTspFile(char* fileName){
  char buffer[100];
  Instance instance;

  // Abre o arquivo .tsp
  FILE* file = fopen(fileName,"r");
  if(file == NULL){
    perror("Erro ao abrir aquivo de entrada:");
    exit(0);
  }//if

  //Lê os atributos da instancia
  fscanf(file,"%*[^:]:%[^\n]\n",instance.name);
  fscanf(file,"%*[^:]:%[^\n]\n",instance.type);
  fscanf(file,"%*[^:]:%[^\n]\n",instance.comment);
  fscanf(file,"%*[^:]:%[^\n]\n",buffer);
  instance.dimension = strtol(buffer,NULL,10);
  fscanf(file,"%*[^:]:%[^\n]\n",instance.edgeWeightType);
  fscanf(file,"%*[^\n]\n");

  // Aloca dinamicamente o vetor para a dimensão da instância
  instance.nodes = (Coordinate*) malloc(sizeof(Coordinate) * instance.dimension);

  // Lê os vértices das instancias e salva no vetor de nodos
  for (size_t i = 0; i < instance.dimension; i++) {
    fscanf(file,"%*d %f %f %f\n",&instance.nodes[i].x,&instance.nodes[i].y,&instance.nodes[i].z);
  }//for

  // Aloca a metriz de distancias dinamicamente
  instance.distances = (float**) malloc(instance.dimension * sizeof(float*));
  for (int i = 0; i < instance.dimension; i++) {
    instance.distances[i] = (float*) malloc(instance.dimension * sizeof(float));
  }//for

  // Calcula a matriz de distancias entre as estrelas
  for (int i = 0; i < instance.dimension; i++) {
    for (int j = 0; j < instance.dimension; j++) {
      instance.distances[i][j] = sqrt(
        pow(instance.nodes[i].x - instance.nodes[j].x,2)+
        pow(instance.nodes[i].y - instance.nodes[j].y,2)+
        pow(instance.nodes[i].z - instance.nodes[j].z,2));
      }//for
  }//for

  return instance;

}//readTspFile

int* geraRotaInicial(int tamanho){
  int trocaPos;
  int troca;

  int* rota = (int*) malloc(tamanho * sizeof(int));

  // Inicia a rota em ordem
  for (int i = 0; i < tamanho; i++) {
    rota[i] = i;
  }//for

  //Embaralha a rota aleatóriamente
  for (int i = 0; i < tamanho; i++) {
    trocaPos = rand()%99;
    troca = rota[i];
    rota[i] = rota[trocaPos];
    rota[trocaPos] = troca;
  }//for
  return rota;
}//geraRotaInicial método

float fitness(Instance instance, int* rota){
  int i;
  float soma = 0;
  for (i = 0; i < instance.dimension-1; i++) {
    soma += instance.distances[rota[i]][rota[i+1]];
  }
  soma += instance.distances[rota[i]][rota[0]];
  return soma;
}

Instance displayInstance(Instance instance){

  printf("Nome: %s\n",instance.name);
  printf("Tipo: %s\n",instance.type);
  printf("Comentário: %s\n",instance.comment);
  printf("Dimensão: %d\n",instance.dimension);
  printf("Tipo de peso da aresta: %s\n",instance.edgeWeightType);

  // Lê os vértices das instancias e salva no vetor de nodos
  for (size_t i = 0; i < instance.dimension; i++) {
    printf("Ponto %zu - (%10.6f, %10.6f, %10.6f)\n",i, instance.nodes[i].x,instance.nodes[i].y,instance.nodes[i].z);
  }//for

  for (int i = 0; i < instance.dimension; i++) {
    for (int j = 0; j < instance.dimension; j++) {
        printf("Distancia %d %d = %f\n",i,j,instance.distances[i][j]);
    }//for
  }//for

  return instance;

}//readTspFile

void opt2(Instance instance, int* rota, int interacoes){
  int node1;
  int node2;
  int troca;
  float melhorDistancia = fitness(instance, rota);
  float distancia;

  for (int i = 0; i < interacoes; i++) {
    node1 = rand()%(instance.dimension-1);
    node2 = rand()%(instance.dimension-1);

    troca = rota[node1];
    rota[node1] = rota[node2];
    rota[node2] = troca;

    distancia = fitness(instance,rota);
    if(distancia < melhorDistancia){
      melhorDistancia = distancia;
      printf("Distancia %i = %f\n", i, melhorDistancia);

    }else{
      troca = rota[node1];
      rota[node1] = rota[node2];
      rota[node2] = troca;
    }

  }
}

int main(int argc, char** argv){

  srand(time(NULL));

  Instance instance = readTspFile("star1k.tsp");
  displayInstance(instance);
  int* rota = geraRotaInicial(instance.dimension);
  opt2(instance,rota,2000000000);

}//main method
