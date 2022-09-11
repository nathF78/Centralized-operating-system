#include "processus.h"

Processus newProcessus(Processus_List* pl, pid_t pid, State_Processus state, char* seq) {
	char* cmdline = malloc(sizeof(seq));
	if (cmdline == NULL) {
			perror("realloc error [newProcessus]");
			exit(4);
		}
	strcpy(cmdline, seq);
	int id = 0;
	for(int i=0; i < pl->size; i++) {
		if (id < (pl->proc+i)->id) { 
			id = (pl->proc+i)->id;
		}
	}
	id++;
	Processus p = {.id = id, .pid = pid, .state = state, .command = cmdline};
	return p;
}

void add(Processus_List* pl, Processus p) {
	Processus* new_pl = realloc(pl->proc, (pl->size + 1) * sizeof(Processus));
	if (new_pl == NULL) {
		perror("realloc error [add]");
		exit(4);
	}
	pl->proc = new_pl;
	pl->proc[pl->size] = p;
	pl->size++;
}

void del(Processus_List* pl, int id) {
	bool found = false;
	Processus* new_pl = realloc(pl->proc, (pl->size)* sizeof(Processus));
	if (new_pl == NULL) {
		perror("realloc error [del]");
		exit(4);
	}
	for(int i=0; i < pl->size; i++) {
		if (id == (pl->proc+i)->id) { 
			found = true;
		}
		if (found == true) {
			new_pl[i] = *(pl->proc+i+1); //on decale tout d'un 
		}
	}
	pl->proc = new_pl;
	pl->size--;
}

void initializeList(Processus_List* list) {
	list->size = 0;
	list->proc = malloc(sizeof(Processus)); 
	if (list->proc == NULL) {
		perror("malloc error [initializeList]");
		exit(4);
	}
}

bool exist(Processus_List* pl, int id) {
	for(int i=0; i < pl->size; i++) {
		if (id == (pl->proc+i)->id) { 
			return true;
		}
	}
	return false;
}

int pid_to_id(Processus_List* pl, pid_t pid) {
	for(int i=0; i < pl->size; i++) {
		if (pid == (pl->proc+i)->pid) { 
			return (int)(pl->proc+i)->id;
		}
	}
	return -1; //si proc inexistant
}


void suspend(Processus_List* pl, int id){
for(int i=0; i < pl->size; i++) {
		if (id == (pl->proc+i)->id) { 
			(pl->proc+i)->state = SUSPENDED;
			break;
		}
	}
}

void active(Processus_List* pl, int id){
for(int i=0; i < pl->size; i++) {
		if (id == (pl->proc+i)->id) { 
			(pl->proc+i)->state = ACTIVE;
			break;
		}
	}
}

int id_to_pid(Processus_List* pl, int id) {
	for(int i=0; i < pl->size; i++) {
		if (id == (pl->proc+i)->id) { 
			return (pl->proc+i)->pid;
		}
	}
	return -1; //si proc inexistant
}
