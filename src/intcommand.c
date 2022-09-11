#include "intcommand.h"
#include <signal.h>


void lj(Processus_List* pl) {
	for(int i=0; i < pl->size; i++) {
		Processus p = *(pl->proc+i);
		printf("%d  %d  %s ", p.id, p.pid, p.command);
		if (p.state == 0){
			printf("Active\n");
		}
		else {
			printf("Suspended\n");
		}
	}
}

void sj(Processus_List* pl, char* id) {
	if (id != NULL) {
		if (exist(pl, atoi(id))) {
			kill(id_to_pid(pl, atoi(id)), SIGSTOP);
		}
		else {
			printf("proc %s cannot be found \n", id);
		}
	}
	else {
		printf("use : sj *processus id* \n");
	}
}

void bg(Processus_List* pl, char* id) {
	if (id != NULL) {
		if (exist(pl, atoi(id))) {
			kill(id_to_pid(pl, atoi(id)), SIGCONT);
		}
		else {
			printf("proc %s cannot be found \n", id);
		}
	}
	else {
		printf("use : bg *processus id* \n");
	}
}

void fg(Processus_List* pl, char* id, pid_t* pid_fg) {
	if (id != NULL) {
		if (exist(pl, atoi(id))) {
			pid_t pid = id_to_pid(pl, atoi(id));
			*pid_fg = pid;
			kill(pid, SIGCONT);
			while(*pid_fg > 0) {
				pause();
			}
		}
		else {
			printf("proc %s cannot be found \n", id);
		}
	}
	else {
		printf("use : bg *processus id* \n");
	}
}