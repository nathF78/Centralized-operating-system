#ifndef INTCOMMAND_H
#define INTCOMMAND_H

#include <string.h>
#include "processus.h"

//procedure

void lj(Processus_List* pl);
void sj(Processus_List* pl, char* id);
void bg(Processus_List* pl, char* id);
void fg(Processus_List* pl, char* id, pid_t* pid_fg);

#endif