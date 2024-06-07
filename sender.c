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
*
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int sockfd, n, vite = 3;
  const int connect_num = 14;
  char sendline[1000];
  char readline[1000];
  struct sockaddr_in remote_addr;
  socklen_t len = sizeof(struct sockaddr_in);

  if (argc != 3) {
    printf("Uso: sender <indirizzo> <porta>\n");
    return 0;
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("Errore nell'apertura del socket.\n");
    return -1;
  }

  memset(&remote_addr, 0, len);
  remote_addr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(remote_addr.sin_addr));
  remote_addr.sin_port = htons(atoi(argv[2]));

  sendto(sockfd, &connect_num, sizeof(int), 0, (struct sockaddr *)&remote_addr,
         len);
  while (vite > 0) {
    n = recvfrom(sockfd, readline, 999, 0, (struct sockaddr *)&remote_addr,
                 &len);
    readline[n] = 0;
    printf("%s\n", readline);
    if (!strcmp(readline, "L'altro giocatore ha vinto"))
      break;
    printf("Hai %d vite\nInserisci una lettera o una parola: ", vite);
    scanf("%s", sendline);
    sendto(sockfd, sendline, strlen(sendline) + 1, 0,
           (struct sockaddr *)&remote_addr, len);
    n = recvfrom(sockfd, readline, 999, 0, (struct sockaddr *)&remote_addr,
                 &len);
    readline[n] = 0;
    printf("%s\n", readline);
    if (!strcmp(readline, "Sbagliato!"))
      vite--;
    else if (!strcmp(readline, "HAI VINTO!"))
      break;
  }

  if (vite == 0)
    printf("Hai perso!");

  close(sockfd);

  return 0;
}
