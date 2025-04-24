#ifndef USUARIO_H
#define USUARIO_H

#include <sqlite3.h>

// Função para cadastrar um paciente no banco de dados
int cadastrar_paciente(sqlite3 *db);

#endif  // USUARIO_H
