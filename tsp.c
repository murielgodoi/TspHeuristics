/**
 * @file tsp.c
 * @author Muriel de Souza Godoi (muriel@utfpr.edu.br)
 * @brief Implementação da metaheurística GRASP + busca local 2opt 
 * @version 0.2
 * @date 2022-11-10 (Last Version)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

//Constantes
#define BUILDMATRIX false


//Definição as estruturas
typedef struct
{
  float x;
  float y;
  float z;
} Coordenada;

typedef struct
{
  char nome[100];
  char tipo[100];
  char comment[100];
  int dimension;
  char edgeWeightType[20];
  Coordenada *nodes;
  float **distances;
  bool buildMatrix;
} Instance;


//Headers
float fitness(Instance instance, int *rota);
float distance(int, int, Instance);
char* getTimeStamp();


//DataSets filenames
char dataSets[8][20]={
  "star100.tsp",
  "star1k.tsp",
  "star10k.tsp",
  "kj37859.tsp",
  "hyg109399.tsp",
  "star250k.tsp"
};

Instance readTspFile(char *fileName, bool buildMatrix)
{
  char buffer[100];
  char field[100];
  Instance instance;
  instance.buildMatrix = buildMatrix;

  sprintf(buffer,"data/%s",fileName);

  printf("Abrindo arquivo %s\n", fileName);
  // Abre o arquivo .tsp
  FILE *file = fopen(buffer, "r");
  if (file == NULL)
  {
    perror("Erro ao abrir aquivo de entrada:");
    exit(0);
  } // if

  // Lê os atributos da instancia
  fscanf(file, "%*[^:] : %[^\n]\n", instance.nome);
  fscanf(file, "%*[^:] : %[^\n]\n", instance.tipo);
  fscanf(file, "%*[^:] : %[^\n]\n", instance.comment);
  do
  {
    fscanf(file, "%[^:]:%[^\n]\n", field, buffer);
  } while (strcmp(field, "COMMENT ") == 0);
  instance.dimension = strtol(buffer, NULL, 10);
  fscanf(file, "%*[^:]:%[^\n]\n", instance.edgeWeightType);
  fscanf(file, "%*[^\n]\n");

  // Aloca dinamicamente o vetor para a dimensão da instância
  instance.nodes = (Coordenada *)malloc(sizeof(Coordenada) * instance.dimension);

  // Lê os vértices das instancias e salva no vetor de nodos
  printf("Lendo %d vertices...", instance.dimension);
  for (int i = 0; i < instance.dimension; i++)
  {
    fscanf(file, "%*d %f %f %f\n", &instance.nodes[i].x, &instance.nodes[i].y, &instance.nodes[i].z);
  } // for
  printf("OK\n");

  if(BUILDMATRIX){
  // Aloca a metriz de distancias dinamicamente
  printf("Alocando matriz de distancias...");
  instance.distances = (float **)malloc(instance.dimension * sizeof(float *));
  for (int i = 0; i < instance.dimension; i++)
  {
    instance.distances[i] = (float *)malloc(instance.dimension * sizeof(float));
  } // for
  printf("OK\n");

    printf("Calculando matriz de distancias...");
    // Calcula a matriz de distancias entre as estrelas
    for (int i = 0; i < instance.dimension; i++)
    {
      for (int j = i; j < instance.dimension; j++)
      {
        instance.distances[i][j] = sqrt(
            pow(instance.nodes[i].x - instance.nodes[j].x, 2) +
            pow(instance.nodes[i].y - instance.nodes[j].y, 2) +
            pow(instance.nodes[i].z - instance.nodes[j].z, 2));
        instance.distances[j][i] = instance.distances[i][j];
        
      } // for
      printf("\rCalculando matriz de distancias... %.2f%%", 100 * (float)i / instance.dimension);
      fflush(stdout);
    } // for
    printf("\rCalculando matriz de distancias... 100%% OK\n");
  }//if

  return instance;

} // readTspFile


void saveTour(Instance instance, int* rota){
  char filename[120];
  char* timestamp = getTimeStamp();

  sprintf(filename,"results/%s - %s.tour",instance.nome,timestamp);
  FILE* file = fopen(filename,"w");
  
  fprintf(file, "NAME : %s.tour\n",instance.nome);
  fprintf(file, "COMMENT : Lenght %.2f\n", fitness(instance,rota));
  fprintf(file, "TYPE : TOUR\n");
  fprintf(file, "DIMENSION : %i\n",instance.dimension);
  fprintf(file, "TOUR_SECTION\n");
  fprintf(file, "1\n");

  for (int  i = 0; i < instance.dimension-1; i++)
  {
    fprintf(file, "%d\n",rota[i]+1);
  }
  fprintf(file, "-1\n");
  fprintf(file, "EOF\n");

  fclose(file);
  free(timestamp);

}//saveTour

/**
 * @brief Create a Log File object
 * 
 * @param instance 
 * @return FILE* 
 */
FILE* createLogFile(Instance instance){
  char filename[120];
  char* timestamp = getTimeStamp();

  sprintf(filename,"logs/Log %s.csv",instance.nome);
  FILE* logFile = fopen(filename,"w");

  //fprintf(logFile,"Execution,Alpha, GraspDistance, CurrentDistance, BestCurrentDistance\n");

  return logFile;

}

/**
 * @brief Calcula a distancia entre dois vértices tridimensionais
 * 
 * @param o vértice de origem
 * @param d vértice de destino
 * @param instance instancia utilizada
 * @return float distancia entre os vértices
 */

float distance(int o,int d, Instance instance){
  if(BUILDMATRIX){
    return instance.distances[o][d];
  }else{
    return sqrt(
          (instance.nodes[o].x - instance.nodes[d].x)*(instance.nodes[o].x - instance.nodes[d].x) +
          (instance.nodes[o].y - instance.nodes[d].y)*(instance.nodes[o].y - instance.nodes[d].y) +
          (instance.nodes[o].z - instance.nodes[d].z)*(instance.nodes[o].z - instance.nodes[d].z));
  }//else
}//distance

/**
 * @brief Exibe na tela os dados de uma instancia
 * 
 * @param instance instância a ser exibida
 */
void displayInstance(Instance instance)
{

  printf("Nome: %s\n", instance.nome);
  printf("Tipo: %s\n", instance.tipo);
  printf("Comentário: %s\n", instance.comment);
  printf("Dimensão: %d\n", instance.dimension);
  printf("Tipo de peso da aresta: %s\n", instance.edgeWeightType);

  // Lê os vértices das instancias e salva no vetor de nodos
  for (int i = 0; i < instance.dimension; i++)
  {
    printf("Ponto %i - (%10.6f, %10.6f, %10.6f)\n", i, instance.nodes[i].x, instance.nodes[i].y, instance.nodes[i].z);
  } // for

  for (int i = 0; i < instance.dimension; i++)
  {
    for (int j = 0; j < instance.dimension; j++)
    {
      printf("Distancia %d %d = %f\n", i, j,  distance(i, j, instance));
    } // for
  }   // for

  return;

} // displayInstance method

/**
 * @brief Calcula o custo da rota para a instancia especificada
 * 
 * @param instance instanca a ser utilizada
 * @param rota rota a ser avaliada
 * @return float custo da rota
 */
float fitness(Instance instance, int *rota)
{
  int i;
  float soma = 0;

  for (i = 0; i < instance.dimension - 1; i++){
    soma += distance(rota[i],rota[i + 1], instance);
  }//for

  soma +=  distance(rota[i],rota[0], instance);
  return soma;
} // fitness method

/**
 * @brief Gera uma rota aleatória para uma instancia de tamanho tamanho
 * 
 * @param tamanho tamanho da rota a ser gerada
 * @return int* vetor de inteiros contendo a rota
 */
int *geraRotaAleatoria(int tamanho)
{
  int trocaPos;
  int troca;

  int *rota = (int *)malloc(tamanho * sizeof(int));

  // Inicia a rota em ordem
  for (int i = 0; i < tamanho; i++)
  {
    rota[i] = i;
  } // for

  // Embaralha a rota aleatóriamente
  for (int i = 1; i < tamanho; i++)
  {
    trocaPos = (rand() % 98) + 1;
    troca = rota[i];
    rota[i] = rota[trocaPos];
    rota[trocaPos] = troca;
  } // for
  return rota;
} // geraRotaAleatoria método

/**
 * @brief Cria uma rota utilizando a estrátégia gulosa (vizinho mais próximo)
 * 
 * @param instance instancia a ser utilizada
 * @return int* vetor de inteiros contendo a rota gulosa
 */
int *geraRotaGulosa(Instance instance)
{
  //printf("Gerando rota inicial gulosa....");
  int *rota = (int *)malloc(instance.dimension * sizeof(int));
  bool *visitados = (bool *)malloc(instance.dimension * sizeof(bool));
  float distProx;
  float distCur;
  int proximo = 0;
  int i, j;

  // Marca vertices como não visitados
  visitados[0] = true;
  for (i = 1; i < instance.dimension; i++)
  {
    visitados[i] = false;
  } // for

  // Para cada vertice
  for (i = 0; i < instance.dimension; i++)
  {
    // Localiza o vértice mais próximo
    distProx = INFINITY;
    for (j = 1; j < instance.dimension; j++)
    {
      
      distCur = distance(proximo, j, instance);
      if (distCur < distProx && !visitados[j])
      {
        distProx = distCur;
        proximo = j;
      } // if
    }   // for

    // Adiciona na rota
    rota[i] = proximo;
    visitados[proximo] = true;

  } // for
  //printf("OK\n");

  return rota;
} // geraRotaGulosa

/**
 * @brief Cria 
 * 
 * @param instance 
 * @param alpha 
 * @return int* vetor de inteiros contendo a rota
 */
int *geraRotaGrasp(Instance instance, float alpha)
{
  int *rota = (int *)malloc(instance.dimension * sizeof(int));
  bool *visitados = (bool *)malloc(instance.dimension * sizeof(bool));

  int i, j;
  float min, max, corte;
  int cardinalidade, sorteio, contaCandidatos;
  int proximo = 0;
  float distCur;

  // Marca vertices como não visitados
  visitados[0] = true;
  for (i = 1; i < instance.dimension; i++)
  {
    visitados[i] = false;
  } // for

  // Para cada vertice
  for (i = 0; i < instance.dimension - 1; i++)
  {
    max = -INFINITY;
    min = INFINITY;

    // Encontra distancia min e max
    for (j = 0; j < instance.dimension; j++)
    {

      distCur = distance(proximo,j,instance);
      
      if (distCur > max && !visitados[j])
      {
        max = distCur;
      }
      if (distCur < min && !visitados[j])
      {
        min = distCur;
      } // if
    }   // for

    //Calcula custo de corte
    corte = min + (alpha * (max - min));

    // Calcula cardinalidade da LRC
    cardinalidade = 0;
    for (j = 0; j < instance.dimension; j++)
    {
      if ( distance(proximo,j,instance) <= corte && !visitados[j])
      {
        cardinalidade++;
      } // if
    }   // for

    //Sorteia um candidato da LRC
    sorteio = rand() % cardinalidade;

    // Procura o candidato sorteado na RLC
    contaCandidatos = 0;
    for (j = 0; j < instance.dimension; j++)
    {
      //Verifica se é um candidato da RLC
      if ( distance(proximo,j,instance) <= corte && !visitados[j])
      {
        //Verifica se é o candidato sorteado
        if (contaCandidatos == sorteio)
        {
          proximo = j;
          break;
        }
        contaCandidatos++;

      } // if
    }   // for
    
    // Adiciona na rota
    rota[i] = proximo;
    visitados[proximo] = true;

  } // for

  return rota;
} // geraRotaGrasp

void run2optReverse(int *rota, int inicio, int fim)
{
  int troca;
  for (int i = inicio + 1, j = fim; i < j; i++, j--)
  {
    troca = rota[i];
    rota[i] = rota[j];
    rota[j] = troca;
  }
}

int runSa2opt(Instance instance, int *rota)
{
  int node1;
  int node2;
  int troca;
  float distancia = fitness(instance, rota);
  float delta;
  bool melhorou = true;
  int temperatura = 1000;

  while (melhorou)
  {
    melhorou = false;
    temperatura *= 0.95;
    for (node1 = 0; node1 < instance.dimension; node1++)
    {
      for (node2 = node1 + 1; node2 < instance.dimension; node2++)
      {
        delta = -distance(rota[node1], rota[(node1 + 1) % instance.dimension], instance)
          -distance(rota[node2], rota[(node2 + 1) % instance.dimension], instance) 
          +distance(rota[node1], rota[(node2) % instance.dimension], instance)
          +distance(rota[(node1 + 1) % instance.dimension],rota[(node2 + 1) % instance.dimension], instance);

        if (delta < -0.00001 || ((float)(rand() % 1000) < temperatura))
        {
          run2optReverse(rota, node1, node2);
          distancia += delta;
          melhorou = true;
        } // if
      }   // for
    }     // for
    // printf("Distancia atual: %f Temperatura: %i\n", distancia, temperatura);
  } // while
  return distancia;
} // run2opt

/**
 * @brief Executa o algoritmo 2opt First Improvement
 * 
 * @param instance instancia a ser considerada
 * @param rota rota inicial a ser melhorada
 * @return float custo da rota melhorada
 */
float run2optFirst(Instance instance, int *rota)
{
  int node1;
  int node2;
  float distancia = fitness(instance, rota);
  float delta;
  bool melhorou = true;

  /*
  //Utilizado para gerar o log para o gráfico do 2opt
  int interations = 0; 
  char filename[100];
  char* timestamp = getTimeStamp();
  sprintf(filename,"log2opt-%s.txt",timestamp);
  FILE* logFile = fopen(filename,"w");
  fprintf(logFile,"%f\n",distancia);
  */

  while (melhorou)
  {
    melhorou = false;
    for (node1 = 0; node1 < instance.dimension; node1++)
    {
      for (node2 = node1 + 1; node2 < instance.dimension-1; node2++)
      {
        delta = -distance(rota[node1], rota[(node1 + 1) % instance.dimension], instance)
                -distance(rota[node2], rota[(node2 + 1) % instance.dimension], instance) 
                +distance(rota[node1], rota[(node2) % instance.dimension], instance)
                +distance(rota[(node1 + 1) % instance.dimension],rota[(node2 + 1) % instance.dimension], instance);

        if (delta < -0.001)
        {
          run2optReverse(rota, node1, node2);
          distancia += delta;
          melhorou = true;
          
        } // if
        //interations++;
        //fprintf(logFile,"%i, %f\n",interations, distancia);
      }   // for
    }     // for
    distancia = fitness(instance, rota);
    
    //printf("Distancia atual 2opt %f\n",distancia);
  }       // while
  //fclose(logFile);
  return distancia;
} // run2opt

/**
 * @brief Executa o algoritmo 2opt Best Improvement
 * 
 * @param instance instancia a ser considerada
 * @param rota rota inicial a ser melhorada
 * @return float custo da rota melhorada
 */
float run2optBest(Instance instance, int *rota)
{
  int node1;
  int node2;
  float distancia = fitness(instance, rota);
  float delta;
  bool melhorou = true;
  float bestDelta = 1000;
  int bestNode1;
  int bestNode2;

  while (melhorou)
  {
    melhorou = false;
    for (node1 = 0; node1 < instance.dimension-1; node1++)
    {
      for (node2 = node1 + 1; node2 < instance.dimension; node2++)
      {
        delta = -distance(rota[node1], rota[(node1 + 1) % instance.dimension], instance)
                -distance(rota[node2], rota[(node2 + 1) % instance.dimension], instance) 
                +distance(rota[node1], rota[(node2) % instance.dimension], instance)
                +distance(rota[(node1 + 1) % instance.dimension],rota[(node2 + 1) % instance.dimension], instance);

        if (delta < 0 && delta < bestDelta){
          melhorou = true;
          bestDelta = delta;
          bestNode1 = node1;
          bestNode2 = node2;          
        }// if
      }// for
    }// for
    if(melhorou){
      run2optReverse(rota, bestNode1, bestNode2);
      distancia = fitness(instance, rota);
      //distancia += bestDelta;
    }//
    //printf("Distancia atual 2opt %f - N1: %d N2: %d\n",distancia, bestNode1, bestNode2);
  }// while
  return distancia;
} // run2opt

/**
 * @brief Executa o algoritmo 2opt com estratégia de perturbação (Shake)
 * 
 * @param instance instancia a ser considerada
 * @param rota rota inicial a ser melhorada
 * @return float custo da rota melhorada
 */
float run2optShake(Instance instance, int* rota, int intensity, int shake){
  float distancia;
  float minDistancia;
  float distanciaInicial;
  int node1, node2, troca;

  int* melhorRota = (int*) malloc(instance.dimension * sizeof(int));
  
  distancia = run2optFirst(instance, rota);
  distanciaInicial = distancia;
  minDistancia = distancia;

  for (int i = 0; i < shake; i++){

    for (int j = 0; j < intensity; j++){
    
      node1 = (rand()% instance.dimension-1)+1;
      node2 = (rand()% instance.dimension-1)+1;

      troca = rota[node1];
      rota[node1] = rota[node2];
      rota[node2] = troca;
    }//for shake intensity

    distancia = run2optFirst(instance, rota);
    //printf("Distancia atual shake %f\n",distancia);

   //Save best results, if found
   if (distancia < minDistancia)
    {
      minDistancia = distancia;
      memcpy(melhorRota,rota,instance.dimension * sizeof(int));
    }
  }//for shake interations

  memcpy(rota,melhorRota,instance.dimension * sizeof(int));
  //free(melhorRota);
  if(minDistancia < distanciaInicial ){
    printf("Shake melhorou: de %f para %f\n",distanciaInicial, minDistancia);
  }
  return minDistancia;
}//run2optShake


float run2vert(Instance instance, int *rota)
{
  int node1;
  int node2;
  int troca;
  float distancia = fitness(instance, rota);
  float delta;

  bool melhorou = true;

  while (melhorou)
  {
    melhorou = false;
    for (node1 = 1; node1 < instance.dimension-1; node1++)
    {
      for (node2 = node1 + 2; node2 < instance.dimension; node2++)
      {
        delta = -distance(rota[node1], rota[(node1 + 1) % instance.dimension], instance)
                -distance(rota[node1], rota[(node1 - 1) ], instance)
                -distance(rota[node2], rota[(node2 + 1) % instance.dimension], instance)
                -distance(rota[node2], rota[(node2 - 1) ], instance)
                +distance(rota[node1], rota[(node2 + 1) % instance.dimension], instance)
                +distance(rota[node1], rota[(node2 - 1) ], instance)
                +distance(rota[node2], rota[(node1 + 1) % instance.dimension], instance)
                +distance(rota[node2], rota[(node1 - 1) ], instance);
        
        
        if (delta < -0.001)
        {
          troca = rota[node1];
          rota[node1] = rota[node2];
          rota[node2] = troca;

          distancia += delta;  
          melhorou = true;        
        }


      }// for
    }// for
  }// while
  distancia = fitness(instance, rota);
  return distancia;
} // run2vert

//Funções auxiliares para exibir o tempo durante a execução


/**
 * @brief Calcula a diferença de tempo em segundos um timestamp e o tempo atual
 * 
 * @param initialTick tempo inicial
 * @return double diferença entre os tempos em segundos
 */
double calculaTempo(clock_t initialTick){
  return (double)(clock() - initialTick) / (CLOCKS_PER_SEC);
}

/**
 * @brief Calcula a diferença de tempo em segundos entre 2 timestamps
 * 
 * @param initialTick tempo inicial
 * @param finalTick tempo final
 * @return double diferença entre os tempos em segundos
 */
double calculaDiferencaTempo(clock_t initialTick, clock_t finalTick){
  return (double)(finalTick - initialTick) / (CLOCKS_PER_SEC);
}

/**
 * @brief Gera e retora uma string contendo o timestamp
 * 
 * @return char* string contendo o timestamp com data e hora
 */
char* getTimeStamp(){
  char* buffer = (char*) malloc(100* sizeof(char));
  time_t rawtime;
  struct tm * timeinfo;


  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  sprintf(buffer, "%02d %02d %04d %02d:%02d:%02d", timeinfo->tm_mday,
            timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  return buffer;

}//timeStamp

/**
 * @brief Função criada para avaliar o alpha do GRASP
 * 
 * @param instance instancia a ser avaliada
 * @param runs quantidade de execuções para cada valor de alpha
 */
void evaluateAlpha(Instance instance, int runs){
  FILE* logFile;

  float alpha;
  int* melhorRota = (int*) malloc(instance.dimension * sizeof(int));

  int *rota = NULL;
  float distancia;
  float distanciaGrasp;
  float minDistancia = INFINITY;
 
    logFile = createLogFile( instance );
    //Escreve o cabeçalho no arquivo de log
    for ( alpha = 0; alpha <= 1; alpha = alpha + 0.025)
    {
      if(alpha==0){
        fprintf(logFile,"%f",alpha);
      }else{
        fprintf(logFile,",%f",alpha);
      }//else
    }
    fprintf(logFile,"\n");
    minDistancia = INFINITY;
    printf("Alpha %f\n",alpha);

  //Executa o algoritmo "runs" vezes
  for (int i = 1; i <= runs; i++)
  {
    //Para cada valor de alpha
    for ( alpha = 0; alpha <= 1; alpha = alpha + 0.025)
    {
    free(rota);

    rota = geraRotaGrasp(instance, alpha);
    
    distanciaGrasp = fitness(instance, rota);

    distancia = run2optFirst(instance, rota);
  
    if (distancia < minDistancia)
    {
      minDistancia = distancia;
      memcpy(melhorRota,rota,instance.dimension * sizeof(int));
      saveTour(instance,melhorRota);
    }

      //Salva os valores obtidos no log
      if(alpha==0){
        fprintf(logFile,"%f", distancia);// primeita coluna csv
      }else{
        fprintf(logFile,",%f", distancia);// demais colunas
      }//else
      fflush(logFile);
      
    }//forAlpha
    fprintf(logFile,"\n" );
  } // forExecutions

  //Fecha arquivo de log
  fclose(logFile);
}


int main(int argc, char **argv)
{
  srand(1);
  int execucoes;
  int base;

  if (argc == 3){// Verifica se os argumentos foram passados
    execucoes = atoi(argv[2]);
    base = atoi(argv[1]);
    if(execucoes < 0){ //
      printf("Erro: O número de execuções não pode ser negativa!\n");
      exit(1);
    }//if
    if(base < 0 || base > 5){
      printf("Erro: A instancia escolhida deve estar entre 0 e 5\n");
      exit(1);
    }
  }else{ // Exibe instruções caso não receber os parâmetros
    printf("Heuristicas TSP usando GRASP+2opt\n");
    printf("Autor: Muriel de Souza Godoi\n\n");
    printf("Para utilizar esse sistema execute com os parâmetros:\n");
    printf("\n./tsp <instancia> <qte de execuções> (linux)\n");
    printf("As instancias devem estar entre 0 e 5\n\n");
    printf("Instancia 0 ->     100 estrelas\n");
    printf("Instancia 1 ->   1.000 estrelas\n");
    printf("Instancia 2 ->  10.000 estrelas\n");
    printf("Instancia 3 ->  37.859 estrelas\n");
    printf("Instancia 4 -> 109.399 estrelas\n");
    printf("Instancia 5 -> 250.000 estrelas\n\n");

    exit(1);
  }//else

  clock_t initialTick = clock();
  clock_t loopStartTick;
  clock_t graspTick;
  clock_t optTick;
  float alpha;

  int *rota = NULL;
  float distancia;
  float distanciaGrasp;
  float minDistancia = INFINITY;
  float minGrasp = INFINITY;

  Instance instance = readTspFile(dataSets[base], false);

  int* melhorRota = (int*) malloc(instance.dimension * sizeof(int));
  // displayInstance(instance);

  for (int i = 1; i <= execucoes; i++)
  {
    free(rota);

    loopStartTick = clock();
    //rota = geraRotaAleatoria(instance.dimension);
    //rota = geraRotaGulosa(instance);
    
    if(i == 1){ //Executa uma vez com a busca 100% gulosa
      rota = geraRotaGrasp(instance, 0);
    }else{
      rota = geraRotaGrasp(instance, 0.01 + (rand()%5) * 0.01);
    }//else
    
    distancia = fitness(instance, rota);

    if (distancia < minGrasp){
      minGrasp = distancia;
    }//if
    printf("%i - %.1lfs - GRASP %f, ", i, calculaTempo(initialTick), distancia,  minDistancia);
    fflush(stdout);

    //distancia = run2optShake(instance, rota, 1, 10);
    //distancia = fitness(instance, rota);
    
    distancia = run2optFirst(instance, rota);
  
    if (distancia < minDistancia)
    {
      minDistancia = distancia;
      memcpy(melhorRota,rota,instance.dimension * sizeof(int));
      saveTour(instance,melhorRota);
    }
    printf("%.1lfs - 2OPT= %f - BG: %f - BL: %f\n", calculaTempo(initialTick),  distancia, minGrasp, minDistancia);
    fflush(stdout);

  } // forExecutions

  printf("\nDistancia Minima: %f\n", minDistancia);

  return 0;
} // main method