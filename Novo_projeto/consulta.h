// consulta.h
#ifndef CONSULTA_H
#define CONSULTA_H

#include <sqlite3.h>

// Função para agendar consulta (usada no menu do paciente)
int agendar_consulta(sqlite3 *db, int id_paciente);

void cancelar_consulta(sqlite3 *db, int id_paciente);

void listar_consultas(sqlite3 *db, int id_paciente);

// Menu exclusivo do paciente
void menu_paciente(sqlite3 *db, int id_paciente);

// Menu exclusivo medico
void ver_consultas_agendadas(sqlite3 *db, int id_medico);

void consultar_paciente(sqlite3 *db, int id_medico);



#endif
