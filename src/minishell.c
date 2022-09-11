#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "processus.h"
#include "intcommand.h"
#include <sys/wait.h> /* wait */
#include <stdbool.h>
#include <errno.h>
#include "readcmd.h"
#include <fcntl.h>

typedef struct cmdline cmdline;
Processus_List liste_p;
pid_t pidFils, idFils, pid_fg;
sigset_t new_set;
int codeTerm;

void child_handler(int sig) {

  do { //pour prendre la totalité des changements d'états des fils 
    pidFils = waitpid(-1, &codeTerm, WNOHANG | WCONTINUED | WUNTRACED);
    if (pidFils > 0) {

      if (WIFCONTINUED(codeTerm)) {                   //ne marche pas sur macos (d'après la doc 
        active(&liste_p,pid_to_id(&liste_p,pidFils)); //POSIX certains système n'envoient pas SIGCHLD à la reprise d'un fils)
        printf("[proc %i has been enabled]\n", pid_to_id(&liste_p,pidFils));

      } else if (WIFSTOPPED(codeTerm)) {
        suspend(&liste_p,pid_to_id(&liste_p,pidFils));
        printf("[proc %i has been suspended]\n", pid_to_id(&liste_p,pidFils));
        pid_fg = 0;

      } else if (WIFEXITED(codeTerm) | (WIFSIGNALED(codeTerm))) { //processus terminé 
        del(&liste_p,pid_to_id(&liste_p, pidFils));
        pid_fg = 0;
      }
    }
    else if ((errno != ECHILD) && (pidFils == -1)) {
      perror("error waitpid");
      exit(2);
    }
  } while (pidFils > 0);
}

void ctrlz_handler(int sig) {
  if (pid_fg > 0) {
    kill(pid_fg, SIGSTOP);
    pid_fg = 0;
  }
}

void ctrlc_handler(int sig) {
  if (pid_fg > 0) {
    kill(pid_fg, SIGKILL);
    pid_fg = 0;
  }
}

int change_enter(char* file) {
  int desc = open(file, O_RDONLY);
  if (desc == -1) {
    perror("enter open error");
  }
  if (dup2(desc, 0) == -1) { //redirection
    perror("dup2 enter error");
    desc = -1;
  }
  return desc;
}

int change_exit(char* file) {
  int desc = open(file, O_WRONLY|O_CREAT);
  if (desc == -1) {
    perror("exit open error");
  }
  if (dup2(desc, 1) == -1) { //redirection
    perror("dup2 exit error");
    desc = -1;
  }
  return desc;
}

// remettre les entrees/sorties à 0 et 1
void init_redir(int* desc_enter, int* desc_exit, const int d_0, const int d_1) {

  //par defaut les valeurs sont -1 
  if (*desc_enter != -1) { // si -1 on ne fait rien l'entree est deja celle par defaut 
    if (dup2(d_0,0) == -1) { 
      perror("dup 0 error"); 
    }
    if (close(*desc_enter) == -1) { 
      perror("error close entree"); 
    }
    *desc_enter = -1;
  }

  if (*desc_exit != -1) { // si -1 on ne fait rien la sortie est deja celle par defaut 
    if (dup2(d_1,1) == -1) {  
      perror("dup 1 error"); 
    }
    if (close(*desc_exit) == -1) { 
      perror("erreur close exit"); 
    }
    *desc_exit = -1;
  }
}

void execute_child(cmdline* cmd, int index) {

  if (pidFils < 0) { //erreur du fils
    perror("child error");
    exit(2);
  } 

  else if (pidFils == 0 ) { //on est dans le fils

    sigprocmask(SIG_BLOCK, &new_set, NULL); //on masque les signaux de ctrl-c ctrl-z 

    if (execvp(cmd->seq[index][0], cmd->seq[index]) < 0) {
      perror("execution error");
      exit(2);
    }
    exit(1); //on quitte le fils
  }

  else {  //on est dans le pere 
    Processus p = newProcessus(&liste_p, pidFils, ACTIVE, cmd->seq[index][0]);
    add(&liste_p, p);

    //commande avant plan
    if (cmd->backgrounded == NULL) {
      pid_fg = pidFils;
    }
    else {
      pid_fg = 0;
    }
    //on attend le fils au 1er plan (d'apres retour rendu intermediare)
    while (pid_fg > 0) {
      pause();
    }
  }
  index = 0;
}


int main(int argc, char *argv[]) {

  const int d_0 = dup(0);
  const int d_1 = dup(1);

  int desc_enter = -1;
  int desc_exit = -1;

  int index = 0;

  pid_fg = 0; //pid du processus au 1er plan 

  sigemptyset(&new_set);
  sigaddset(&new_set, SIGINT);
  sigaddset(&new_set, SIGTSTP);

  struct sigaction nv_action;
  sigemptyset(&nv_action.sa_mask);
  nv_action.sa_flags = 0;

  nv_action.sa_handler = ctrlz_handler;
  sigaction(SIGTSTP, &nv_action, NULL);

  nv_action.sa_handler = ctrlc_handler;
  sigaction(SIGINT, &nv_action, NULL);

  nv_action.sa_handler = child_handler;
  sigaction(SIGCHLD, &nv_action, NULL);

  initializeList(&liste_p);


  while(1) {

    init_redir(&desc_enter, &desc_exit, d_0, d_1);
    cmdline* cmd = NULL;
    printf(">>");
    pid_fg = 0;

    while (cmd == NULL) {
     cmd = readcmd();
   }
   if (cmd->err != NULL) {
    printf("erreur : %s", cmd->err);
    cmd = NULL;
  }
  if (*cmd->seq != NULL){
      //gestion des redirections 
    if (cmd->in != NULL) {

      desc_enter = change_enter(cmd->in);
    }
    if (cmd->out != NULL) {
      desc_exit = change_exit(cmd->out);
    }

      //commandes internes

    if (strcmp(cmd->seq[0][0], "cd") == 0) {
      if (cmd -> seq[0][1] == NULL) {
        printf("*cd error* use cd + path\n");
      }
      else {
        chdir(cmd->seq[0][1]);
      }

    } 
    else if (strcmp(cmd->seq[0][0], "exit") == 0) {
      exit(0);
    }
    else if (strcmp(cmd->seq[0][0], "lj") == 0) {
      lj(&liste_p);
    }
    else if (strcmp(cmd->seq[0][0], "sj") == 0) {
      sj(&liste_p, cmd->seq[0][1]);
    }
    else if (strcmp(cmd->seq[0][0], "bg") == 0) {
      bg(&liste_p, cmd->seq[0][1]);
    }
    else if (strcmp(cmd->seq[0][0], "fg") == 0) {
      fg(&liste_p, cmd->seq[0][1],&pid_fg);
    }


    //commandes externes

    else {

      if (cmd->backgrounded != NULL) {
        printf("[backgrounded command] \n");
      }

      if ((cmd->seq[1]) != NULL) { //si il y'a une redirection
        int tube[2];
      index = 1;

      if (pipe(tube) == -1 ) {
        perror("pipe error");
      }

        pidFils = fork(); //on lance le 1er fils
        if (pidFils < 0) { //erreur du fils
          perror("child error");
          exit(2);
        } 
        else if (pidFils == 0) {

          if (close(tube[0]) == -1) { //on ferme l'extremité inutilisée
            perror("error close"); 
          exit(2); 
        }
          if (dup2(tube[1], 1) == -1) { //on redirige 
            perror("error dup2"); 
            exit(2); 
          }
        }

        //on est dans le père 
        execute_child(cmd, 0);
        //on change l'entrée standard 
        close(tube[1]);
        desc_enter = tube[0];

        if (dup2(tube[0], 0) == -1) { 
          perror("erreur dup2"); 
          exit(2); 
        }
      }

      pidFils = fork();
      execute_child(cmd, index);
      index = 0; //on remet l'index à 0

    }

  }
}
return EXIT_SUCCESS;
}
