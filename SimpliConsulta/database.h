// database.h
#ifndef DATABASE_H
#define DATABASE_H

int conectar_db(const char *nome_db);
int criar_tabelas();
void fechar_db();

#endif
