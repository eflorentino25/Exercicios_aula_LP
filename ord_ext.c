#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ARQUIVOS_BLOCOS 1000 // limite de blocos gerados
#define MAX_PATH 128
#define CONGELADO 1
#define ATIVO 0

// Altere aqui para simular diferentes tamanhos de memória
#define MEMORIA_INTERNA 100000000

typedef struct
{
  long long int valor;
  int congelado;
} Registro;

typedef struct
{
  FILE *arquivo;
  int ativo;
  long long int atual;
} Fonte;

Registro memoria[MEMORIA_INTERNA];
int tamanhoHeap;

// Troca dois registros
void swap(Registro *a, Registro *b)
{
  Registro temp = *a;
  *a = *b;
  *b = temp;
}

// Funções de Heap mínimo
void desce(int i)
{
  int menor = i;
  int esq = 2 * i + 1;
  int dir = 2 * i + 2;

  if (esq < tamanhoHeap && memoria[esq].valor < memoria[menor].valor)
    menor = esq;
  if (dir < tamanhoHeap && memoria[dir].valor < memoria[menor].valor)
    menor = dir;

  if (menor != i)
  {
    swap(&memoria[i], &memoria[menor]);
    desce(menor);
  }
}

void constroiHeap()
{
  for (int i = tamanhoHeap / 2 - 1; i >= 0; i--)
  {
    desce(i);
  }
}

// Remove o topo do heap (menor elemento)
Registro removeMinimo()
{
  Registro min = memoria[0];
  memoria[0] = memoria[tamanhoHeap - 1];
  tamanhoHeap--;
  desce(0);
  return min;
}

// Insere um novo registro no heap na última posição e sobe
void insereHeap(Registro novo)
{
  int i = tamanhoHeap;
  memoria[i] = novo;
  tamanhoHeap++;

  while (i > 0 && memoria[(i - 1) / 2].valor > memoria[i].valor)
  {
    swap(&memoria[i], &memoria[(i - 1) / 2]);
    i = (i - 1) / 2;
  }
}

// Fase 1: Distribuição
int distribuicao(const char *nomeEntrada)
{
  FILE *entrada = fopen(nomeEntrada, "r");
  if (!entrada)
  {
    perror("Erro ao abrir entrada");
    exit(1);
  }

  int blocosGerados = 0;
  tamanhoHeap = 0;
  long long int atual;

  // Lê a memória inicial
  for (int i = 0; i < MEMORIA_INTERNA && fscanf(entrada, "%lld", &atual) == 1; i++)
  {
    memoria[tamanhoHeap].valor = atual;
    memoria[tamanhoHeap].congelado = ATIVO;
    tamanhoHeap++;
  }
  constroiHeap();

  while (tamanhoHeap > 0)
  {
    char nomeBloco[MAX_PATH];
    snprintf(nomeBloco, MAX_PATH, "bloco%d.bin", blocosGerados);
    FILE *bloco = fopen(nomeBloco, "wb");
    if (!bloco)
    {
      perror("Erro ao criar bloco");
      exit(1);
    }

    long long int ultimoEscrito = -1;

    while (tamanhoHeap > 0)
    {
      Registro min = removeMinimo();
      fwrite(&min.valor, sizeof(long long int), 1, bloco);
      ultimoEscrito = min.valor;

      if (fscanf(entrada, "%lld", &atual) == 1)
      {
        Registro novo;
        novo.valor = atual;
        novo.congelado = atual < ultimoEscrito ? CONGELADO : ATIVO;
        insereHeap(novo);
      }
    }

    fclose(bloco);
    blocosGerados++;

    tamanhoHeap = 0;
    while (tamanhoHeap < MEMORIA_INTERNA && fscanf(entrada, "%lld", &atual) == 1)
    {
      memoria[tamanhoHeap].valor = atual;
      memoria[tamanhoHeap].congelado = ATIVO;
      tamanhoHeap++;
    }
    constroiHeap();
  }

  fclose(entrada);
  return blocosGerados;
}

// Fase 2: Intercalação
void intercalacao(int blocosGerados, const char *nomeSaida)
{
  Fonte fontes[MAX_ARQUIVOS_BLOCOS];
  Registro heap[MAX_ARQUIVOS_BLOCOS];
  int heapSize = 0;

  for (int i = 0; i < blocosGerados; i++)
  {
    char nome[MAX_PATH];
    snprintf(nome, MAX_PATH, "bloco%d.bin", i);
    fontes[i].arquivo = fopen(nome, "rb");
    fontes[i].ativo = fread(&fontes[i].atual, sizeof(long long int), 1, fontes[i].arquivo) == 1;
  }

  FILE *saida = fopen(nomeSaida, "w");
  if (!saida)
  {
    perror("Erro ao criar arquivo de saida");
    exit(1);
  }

  while (1)
  {
    int menorFonte = -1;
    for (int i = 0; i < blocosGerados; i++)
    {
      if (fontes[i].ativo)
      {
        if (menorFonte == -1 || fontes[i].atual < fontes[menorFonte].atual)
        {
          menorFonte = i;
        }
      }
    }

    if (menorFonte == -1)
      break;

    fprintf(saida, "%lld\n", fontes[menorFonte].atual);
    fontes[menorFonte].ativo = fread(&fontes[menorFonte].atual, sizeof(long long int), 1, fontes[menorFonte].arquivo) == 1;
  }

  for (int i = 0; i < blocosGerados; i++)
  {
    fclose(fontes[i].arquivo);
  }
  fclose(saida);
}

void imprimeUltimos10(const char *nomeSaida)
{
  FILE *f = fopen(nomeSaida, "r");
  if (!f)
  {
    perror("Erro ao abrir arquivo de saida para leitura final");
    return;
  }
  long long int buffer[10];
  int count = 0;

  while (fscanf(f, "%lld", &buffer[count % 10]) == 1)
  {
    count++;
  }

  int start = count >= 10 ? count % 10 : 0;
  int total = count < 10 ? count : 10;

  printf("Ultimos %d elementos ordenados:\n", total);
  for (int i = 0; i < total; i++)
  {
    printf("%lld\n", buffer[(start + i) % 10]);
  }
  fclose(f);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Uso: %s <arquivo_entrada> <memoria_em_registros>", argv[0]);
    return 1;
  }

  const char *nomeEntrada = argv[1];
  int memoriaSimulada = atoi(argv[2]);
  if (memoriaSimulada <= 0 || memoriaSimulada > MEMORIA_INTERNA)
  {
    printf("Memória inválida. Use até %d registros.", MEMORIA_INTERNA);
    return 1;
  }

  clock_t inicio, fim;
  double tempoDistribuicao, tempoIntercalacao;

  inicio = clock();
  int blocos = distribuicao(nomeEntrada);
  fim = clock();
  tempoDistribuicao = (double)(fim - inicio) / CLOCKS_PER_SEC;
  printf("Fase de distribuicao concluida em %.2f segundos. %d blocos gerados.", tempoDistribuicao, blocos);

  inicio = clock();
  intercalacao(blocos, "saida.txt");
  fim = clock();
  tempoIntercalacao = (double)(fim - inicio) / CLOCKS_PER_SEC;
  printf("Fase de intercalacao concluida em %.2f segundos.", tempoIntercalacao);

  imprimeUltimos10("saida.txt");
  return 0;
}
