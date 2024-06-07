/*

Gli studenti del corso di Reti di Calcolatori del DMI – UNICT, hanno deciso, di
loro spontanea volontà, di realizzare il gioco dell’impiccato tra macchine
virtuali. Due macchine client si sfideranno tra di loro al fine di indovinare la
parola segreta presente in una macchina server. Ogni macchina client avrà a
disposizione 3 vite. Il conteggio delle vite sarà tenuto dal server. Ad ogni
turno: • Il server prima invierà al client di turno lo stato del gioco: lettere
indovinate, lettere mancanti e la struttura della parola1
.
• Successivamente, ogni client ad ogni turno avrà la possibilità solo di
scrivere da riga di comando o una lettera o dare la risposta completa. In caso
di errore, perderà una vita. Il server notificherà allo specifico client l’esito
del turno. • Se il giocatore di turno perde tutte le vite, dovrà terminare
l’esecuzione del programma2
.
Il server dovrà memorizzare in una struct client il nome del giocatore,
l’indirizzo IP e la porta del client. Notare che, massimo possono giocare solo 2
client a partita e il gioco inizierà solo quando il server avrà registrato
(struct client) i due giocatori3
.
Il server sceglierà randomicamente una stringa da utilizzare per il gioco
dell’impiccato tra quelle presenti in un array bidimensionale. Il server
sceglierà il client che inizierà la partita con modalità a scelta libera dello
studente (random, il primo che si è registrato, ecc.)
1
Se la parola da indovinare è CIAO ed è stata indovinata allo stato attuale solo
la lettera A, verrà inviato _ _ A _ 2 Potrebbero perdere entrambi i client. In
questo caso il server notificherà nel suo terminale il messaggio “Game over” 3
Quindi i client attenderanno nella recvfrom

*/
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define CONNECT_NUM 14
#define FILE_NAME "parole.txt"
#define MAX_STR_LENGTH 1024

typedef struct {
  char *str;
  bool *found_letters;
  int str_len;
} ParolaSegreta;

typedef struct {
  struct sockaddr_in addr;
  int port;
  bool haVinto;
  unsigned vite;
} Client;

char **loadStrArr(const char *filename, int *dim) {
  FILE *fp = fopen(filename, "r");
  char buff[MAX_STR_LENGTH];
  int str_count = 0;
  char **str_arr;

  if (fp == NULL) {
    printf("Nessun file da cui caricare le parole. Creare il file: %s\n",
           filename);
    exit(EXIT_SUCCESS);
  }

  while (!feof(fp)) {
    if (fscanf(fp, "%s\n", buff) != 1)
      continue;
    str_count++;
  }

  if (str_count == 0) {
    printf("Nessuna parola da caricare dal file %s\n", filename);
    exit(EXIT_SUCCESS);
  }

  str_arr = (char **)malloc(sizeof(char *) * str_count);
  rewind(fp);

  int i = 0;
  while (!feof(fp)) {
    if (fscanf(fp, "%s\n", buff) != 1)
      continue;
    str_arr[i] = (char *)malloc((sizeof(char) * strlen(buff)) + 1);
    strcpy(str_arr[i], buff);
    i++;
  }

  fclose(fp);

  *dim = str_count;
  return str_arr;
}

void printStrArr(const char **str_arr, int dim) {
  printf("Parole disponibili:\n");
  for (int i = 0; i < dim; i++)
    printf("%s\n", str_arr[i]);
  printf("\n");
}

ParolaSegreta selectSecretWord(const char **str_arr, int dim) {
  int random_index = rand() % dim;
  ParolaSegreta parola;
  parola.str_len = strlen(str_arr[random_index]);
  parola.str = malloc((sizeof(char) * parola.str_len) + 1);
  strcpy(parola.str, str_arr[random_index]);
  parola.found_letters = malloc(sizeof(bool) * parola.str_len);

  for (int i = 0; i < parola.str_len; i++)
    parola.found_letters[i] = false;

  return parola;
}

bool nessunVincitore(Client *giocatori) {
  for (int i = 0; i < 2; i++)
    if (giocatori[i].haVinto)
      return false;

  return true;
}

bool gameOver(Client *giocatori) {
  return (giocatori[0].vite == 0 && giocatori[1].vite == 0);
}

char *parolaSegretaToStr(ParolaSegreta parola, Client giocatore) {
  char *buff = malloc(sizeof(char) * 1000);
  strcpy(buff, parola.str);
  for (int i = 0; i < parola.str_len; i++) {
    if (!parola.found_letters[i])
      buff[i] = '_';
  }
  return buff;
}

bool parolaTrovata(ParolaSegreta parola) {
  for (int i = 0; i < parola.str_len; i++)
    if (!parola.found_letters[i])
      return false;
  return true;
}

void checkParola(int sockfd, const char *attempt, Client *giocatore,
                 ParolaSegreta *parola) {
  if (!strcmp(attempt, parola->str)) {
    for (int i = 0; i < parola->str_len; i++)
      parola->found_letters[i] = true;
    giocatore->haVinto = true;
    sendto(sockfd, "HAI VINTO!", strlen("HAI VINTO") + 1, 0,
           (struct sockaddr *)&giocatore->addr, sizeof(struct sockaddr_in));
    return;
  }

  giocatore->vite--;
  sendto(sockfd, "Sbagliato!", strlen("Sbagliato!") + 1, 0,
         (struct sockaddr *)&giocatore->addr, sizeof(struct sockaddr_in));
}

void checkLettereParola(int sockfd, const char attempt, Client *giocatore,
                        ParolaSegreta *parola) {
  int count = 0;
  for (int i = 0; i < parola->str_len; i++) {
    if (parola->str[i] == '\0')
      break;
    if (parola->str[i] == attempt) {
      parola->found_letters[i] = true;
      count++;
    }
  }

  if (count == 0) {
    giocatore->vite--;
    sendto(sockfd, "Sbagliato!", strlen("Sbagliato") + 1, 0,
           (struct sockaddr *)&giocatore->addr, sizeof(struct sockaddr_in));
    return;
  }

  if (parolaTrovata(*parola)) {
    giocatore->haVinto = true;
    sendto(sockfd, "HAI VINTO!", strlen("HAI VINTO") + 1, 0,
           (struct sockaddr *)&giocatore->addr, sizeof(struct sockaddr_in));
    return;
  }

  sendto(sockfd, "Corretto!", strlen("Corretto") + 1, 0,
         (struct sockaddr *)&giocatore->addr, sizeof(struct sockaddr_in));
}

int main(int argc, char **argv) {
  int sockfd, n, number_received;
  socklen_t len = sizeof(struct sockaddr_in);
  struct sockaddr_in local_addr;
  char msg[1000];
  Client giocatori[2];
  int giocatori_collegati = 0;
  char **str_arr;
  int arr_dim;
  ParolaSegreta parola;
  int current = 0;

  srand(time(NULL));

  if (argc != 2) {
    printf("Uso: receiver <porta>\n");
    return 0;
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("Errore nell'apertura del socket.\n");
    return -1;
  }

  memset(&local_addr, 0, len);
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(atoi(argv[1]));

  if (bind(sockfd, (struct sockaddr *)&local_addr, len) < 0) {
    printf("Errore nell'assegnamento della porta.\n");
    return -1;
  }

  str_arr = loadStrArr(FILE_NAME, &arr_dim);
  printStrArr(str_arr, arr_dim);
  parola = selectSecretWord(str_arr, arr_dim);
  printf("Parola Segreta: %s\n", parola.str);

  while (giocatori_collegati < 2) {
    n = recvfrom(sockfd, &number_received, sizeof(int), 0,
                 (struct sockaddr *)&giocatori[giocatori_collegati].addr, &len);
    if (number_received != CONNECT_NUM)
      continue;
    giocatori[giocatori_collegati].port =
        ntohs(giocatori[giocatori_collegati].addr.sin_port);
    giocatori[giocatori_collegati].haVinto = false;
    giocatori[giocatori_collegati].vite = 3;
    giocatori_collegati++;
  }

  while (nessunVincitore(giocatori) && !gameOver(giocatori)) {
    if (giocatori[current % 2].vite == 0){
      current++;
      continue;
    }
    sendto(sockfd, parolaSegretaToStr(parola, giocatori[current % 2]),
           parola.str_len, 0, (struct sockaddr*)&giocatori[current % 2].addr,
           len);
    n = recvfrom(sockfd, msg, 999, 0,
                 (struct sockaddr *)&giocatori[current % 2].addr, &len);
    msg[n] = 0;
    if (strlen(msg) != 1)
      checkParola(sockfd, msg, &giocatori[current % 2], &parola);
    else
      checkLettereParola(sockfd, msg[0], &giocatori[current % 2], &parola);
    current++;
  }
  sendto(sockfd, "L'altro giocatore ha vinto",
           strlen("L'altro giocatore ha vinto"), 0, (struct sockaddr*)&giocatori[current % 2].addr,
           len);
 
  close(sockfd);
  return 0;
}
