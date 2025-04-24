#ifndef UTILS_H
#define UTILS_H

#include <sqlite3.h>
#include "structs.h"  //  Importa a definição de LoginResultado

int validar_idade(const char *idade_str); // idade numeros

int cpf_existe_paciente(sqlite3 *db, const char *cpf); // Não permite duplicidade Cpf paciente

int cpf_existe_medico(sqlite3 *db, const char *cpf); // Não permite duplicidade Cpf medico

int validar_telefone(const char *telefone); // validar numeros telefone

int cpf_existe(sqlite3 *db, const char *cpf); // cpf existe já no banco

int validar_cpf(const char *cpf); // cpf tem mais de 11 caratereis?

int crm_existe(sqlite3 *db, const char *crm);

int validar_token(void);  // Declare a função aqui

void digitar_senha(char *senha); // digita senha uma vez 

void formatar_nome(char* nome); // primeira letre maiscula

int validar_email(const char* email);  // validação email


void esconder_senha(char* senha); // Função que lê a senha escondendo os caracteres mais teste de 2 etepas

void pause();


void limpar_terminal(); // Função que limpa o terminal (console)

//int limpar_tabelas(sqlite3 *db);

LoginResultado realizar_login(sqlite3 *db); 

// Função de tratamento de erros (se necessário)
void erro_sqlite3(sqlite3 *db, const char *msg);

#endif // UTILS_H

