#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

//structures

enum State_Processus { ACTIVE, SUSPENDED };
typedef enum State_Processus State_Processus;

struct Processus {
	int id;
	pid_t pid;
	State_Processus state;
	char* command;
};
typedef struct Processus Processus;

struct Processus_List {
	Processus* proc;
	int size;
};
typedef struct Processus_List Processus_List;

//procédures 

// créer un nouveau processus
Processus newProcessus(Processus_List* pl, pid_t pid, State_Processus state, char* seq);
// ajouter un processus à la liste de processus
void add(Processus_List* pl, Processus p);
// initialiser le liste des processus
void initializeList(Processus_List* list);
// supprimer un processus
void del(Processus_List* pl, int id);
// voir si un processus exist 
bool exist(Processus_List* pl, int id);
// convertir pid en id
int pid_to_id(Processus_List* pl, pid_t pid);
// marqué un processus comme suspendu 
void suspend(Processus_List* pl, int id);
// marqué un processus comme actif 
void active(Processus_List* pl, int id);
// convertir id en pid
int id_to_pid(Processus_List* pl, int id);

#endif
