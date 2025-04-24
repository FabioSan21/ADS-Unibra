// database.c
#include <stdio.h>
#include <sqlite3.h>
#include "database.h"

sqlite3 *db;

// Função para conectar ao banco de dados
int conectar_db(const char *nome_db) {
    int rc = sqlite3_open(nome_db, &db);
    if (rc) {
        printf("Nao foi possivel abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    printf("Banco de dados aberto com sucesso.\n");
    return 0;
}

// Função para fechar a conexão com o banco de dados
void fechar_db() {
    sqlite3_close(db);
    printf("Banco de dados fechado.\n");
}

// Função para executar o comando SQL de criação de tabelas
int criar_tabelas(sqlite3 *db) {
    char *sql = 
        "CREATE TABLE IF NOT EXISTS unidades ("
        "id_unidade INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "cnpj TEXT NOT NULL UNIQUE,"
        "telefone TEXT NOT NULL,"
        "uf TEXT NOT NULL,"
        "cidade TEXT NOT NULL,"
        "bairro TEXT NOT NULL,"
        "rua TEXT NOT NULL,"
        "complemento TEXT,"
        "numero INTEGER NOT NULL,"
        "vagas INTEGER NOT NULL);"
        
        "CREATE TABLE IF NOT EXISTS medicos ("
        "id_medico INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "cpf TEXT NOT NULL UNIQUE,"
        "crm TEXT NOT NULL,"
        "telefone TEXT NOT NULL,"
        "senha TEXT NOT NULL,"
        "genero TEXT NOT NULL,"
        "especialidade TEXT NOT NULL,"
        "id_unidade INTEGER NOT NULL,"
        "horario TEXT NOT NULL,"
        "FOREIGN KEY (id_unidade) REFERENCES unidades(id_unidade));"
        
        "CREATE TABLE IF NOT EXISTS pacientes ("
        "id_paciente INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "cpf TEXT NOT NULL UNIQUE,"
        "email TEXT NOT NULL,"
        "telefone TEXT NOT NULL,"
        "idade INTEGER NOT NULL,"
        "genero TEXT NOT NULL,"
        "pcd BOOLEAN NOT NULL,"
        "tipo_pcd TEXT,"
        "uf TEXT NOT NULL,"
        "cidade TEXT NOT NULL,"
        "bairro TEXT NOT NULL,"
        "rua TEXT NOT NULL,"
        "complemento TEXT,"
        "numero INTEGER NOT NULL,"
        "senha TEXT NOT NULL);"
        
        "CREATE TABLE IF NOT EXISTS consultas ("
        "id_consulta INTEGER PRIMARY KEY AUTOINCREMENT,"
        "id_paciente INTEGER,"
        "id_medico INTEGER,"
        "id_unidade INTEGER,"
        "data TEXT NOT NULL,"
        "hora TEXT NOT NULL,"
        "status TEXT NOT NULL, "
        "FOREIGN KEY (id_paciente) REFERENCES pacientes(id_paciente), "
        "FOREIGN KEY (id_medico) REFERENCES medicos(id_medico), "
        "FOREIGN KEY (id_unidade) REFERENCES unidades(id_unidade));";

    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        // A seguir, vamos imprimir o erro detalhado.
        printf("Erro ao criar as tabelas: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("Tabelas criadas com sucesso!\n");
    return 0;
}

