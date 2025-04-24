#ifndef UNIDADE_H
#define UNIDADE_H

#include <sqlite3.h>

// Função para validar o token de acesso
int validar_token();

// Função para cadastrar uma unidade no banco de dados
int cadastrar_unidade(sqlite3 *db);

#endif // UNIDADE_H
