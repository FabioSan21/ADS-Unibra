#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>  // Para usar a função atoi
#include "database.h" 
#include "utils.h"
#include <ctype.h>  // Necessário para isdigit()

#define TOKEN_ACESSO "12345"  // Token de acesso fixo para permitir o cadastro

// Função para validar o token
int validar_token() {
    char token_input[100];  // Agora a variável está declarada

    
    printf("Digite o token de acesso: ");
    scanf("%s", token_input);
    
    if (strcmp(token_input, TOKEN_ACESSO) == 0) {
        return 1;  // Token válido
    } else {
        printf("Token invalido!\n");
        return 0;  // Token inválido
    }
}

// Função para cadastrar a unidade
int cadastrar_unidade(sqlite3 *db) {
    char nome[100], cnpj[20], telefone[20], uf[3], cidade[50], bairro[50];
    char rua[100], complemento[100], numero_str[10];
    int numero, vagas;char vagas_str[10];
    
    // Coletando os dados da unidade
    printf("\nCadastro de Unidade:\n");
    printf("Nome: ");
    scanf(" %[^\n]s", nome);  // Leitura de string com espaços
    printf("CNPJ: ");
    scanf(" %[^\n]s", cnpj);
    do {
        printf("Telefone (apenas numeros, com DDD): ");
        scanf(" %[^\n]s", telefone);

        if (!validar_telefone(telefone)) {
            printf("Telefone invalido. Use apenas numeros com 10 ou 11 digitos.\n");
        } else {
            break;
        }
    } while (1);
    printf("UF (Ex: PE): ");
    scanf(" %[^\n]s", uf);
    formatar_nome(uf); 
    printf("Cidade: ");
    scanf(" %[^\n]s", cidade);
    formatar_nome(cidade); 
    printf("Bairro: ");
    scanf(" %[^\n]s", bairro);
    formatar_nome(bairro); 
    printf("Rua: ");
    scanf(" %[^\n]s", rua);
    formatar_nome(rua); 
    printf("Complemento: ");
    scanf(" %[^\n]s", complemento);
    formatar_nome(complemento); 
    printf("Numero Residencial: ");
    scanf(" %[^\n]s", numero);
    do {
        printf("Vagas: ");
        scanf(" %[^\n]s", vagas_str);

        int valido = 1;
        for (int i = 0; vagas_str[i]; i++) {
            if (!isdigit(vagas_str[i])) {
                valido = 0;
                break;
            }
        }

        if (!valido) {
            printf("Entrada invalida. Digite apenas numeros.\n");
            continue;
        }

        vagas = atoi(vagas_str);
        if (vagas <= 0) {
            printf("Número de vagas deve ser maior que zero.\n");
        }

    } while (vagas <= 0);
    // Convertendo número (que vem como string) para inteiro
    numero = atoi(numero_str);

    // Preparando a query para inserir a unidade no banco de dados
    sqlite3_stmt *stmt;
    const char *sql = 
        "INSERT INTO unidades (nome, cnpj, telefone, uf, cidade, bairro, rua, complemento, numero, vagas) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    
    // Preparar a declaração SQL
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Bind dos parâmetros da query
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cnpj, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, uf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, cidade, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, bairro, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, rua, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, complemento, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, numero);  // Bind de número como inteiro
    sqlite3_bind_int(stmt, 10, vagas);  // Bind de vagas como inteiro

    // Executando a query
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Erro ao cadastrar a unidade: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    printf("Unidade cadastrada com sucesso!\n");
    pause();

    // Finalizar a declaração SQL
    sqlite3_finalize(stmt);
    return 0;
}
