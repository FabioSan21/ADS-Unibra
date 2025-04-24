#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "utils.h"
#include <ctype.h>     // Para isdigit
#include <stdlib.h>    // Para atoi

// Função para cadastrar o paciente
int cadastrar_paciente(sqlite3 *db) {
    char nome[100], cpf[20], email[100], telefone[20], senha[20], genero[20];
    char tipo_pcd[50];char idade_str[10];
    int idade, pcd; // 1 para sim, 0 para não

    // Dados de endereço
    char uf[3], cidade[50], bairro[50], rua[100], complemento[100], numero[10];

    // Lista de gêneros
    const char* generos[] = {"Masculino", "Feminino", "Outros"};
    const char* pcd_options[] = {"sim", "nao"};

    // Coletando os dados do paciente
    printf("\nCadastro de Paciente:\n");
    printf("Nome Completo: ");
    scanf(" %[^\n]s", nome);
    formatar_nome(nome); 
    do {
        printf("CPF: ");
        scanf(" %[^\n]s", cpf);
        if (!validar_cpf(cpf)) {
            printf("CPF invalido. Deve conter 11 digitos numericos.\n");
            continue;
        }
        if (cpf_existe_paciente(db, cpf)) {
            printf("CPF ja cadastrado.\n");
        } else {
            break;
        }
    } while (1);
    
    do {
        printf("Email: ");
        scanf(" %[^\n]s", email);
        if (!validar_email(email)) {
            printf("Email invalido. Tente novamente.\n");
        } else {
            break;
        }
    } while (1);
    
    do {
        printf("Telefone (apenas numeros, com DDD): ");
        scanf(" %[^\n]s", telefone);

        if (!validar_telefone(telefone)) {
            printf("Telefone invalido. Use apenas números com 10 ou 11 digitos.\n");
        } else {
            break;
        }
    } while (1);

    esconder_senha(senha); 
    do {
        printf("Idade: ");
        scanf(" %[^\n]s", idade_str);
    
        if (!validar_idade(idade_str)) {
            printf("Idade invalida. Digite apenas números maiores que zero.\n");
        } else {
            idade = atoi(idade_str);
            break;
        }
    
    } while (1);
    // Seleção de gênero
    printf("Escolha o Genero:\n");
    for (int i = 0; i < 3; i++) {
        printf("%d. %s\n", i + 1, generos[i]);
    }
    int genero_opcao;
    printf("Opcao: ");
    scanf("%d", &genero_opcao);
    if (genero_opcao >= 1 && genero_opcao <= 3) {
        strcpy(genero, generos[genero_opcao - 1]);
    } else {
        printf("Opcao invalida. Genero nao selecionado.\n");
        return -1;
    }

    // Perguntar se o paciente é PCD
    printf("O voce e PCD? (sim/nao):\n");
    for (int i = 0; i < 2; i++) {
        printf("%d. %s\n", i + 1, pcd_options[i]);
    }
    int pcd_opcao;
    printf("Opcao: ");
    scanf("%d", &pcd_opcao);
    if (pcd_opcao == 1) {
        pcd = 1;
        printf("Tipo de PCD (ex: visual, auditiva, etc): ");
        scanf(" %[^\n]s", tipo_pcd);
    } else if (pcd_opcao == 2) {
        pcd = 0;
        tipo_pcd[0] = '\0'; // Nenhum tipo de PCD
    } else {
        printf("Opcao invalida. PCD nao selecionado.\n");
        return -1;
    }

    // Coletando dados de endereço
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

    // Preparando a query para inserir o paciente no banco de dados
    sqlite3_stmt *stmt_inserir;
    const char *sql_inserir = "INSERT INTO pacientes (nome, cpf, email, telefone, idade, genero, pcd, tipo_pcd, uf, cidade, bairro, rua, complemento, numero, senha) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql_inserir, -1, &stmt_inserir, 0) != SQLITE_OK) {
        printf("Erro ao preparar a consulta de inserção: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Bind dos parâmetros
    sqlite3_bind_text(stmt_inserir, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 2, cpf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 3, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 4, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_inserir, 5, idade);
    sqlite3_bind_text(stmt_inserir, 6, genero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_inserir, 7, pcd);
    sqlite3_bind_text(stmt_inserir, 8, tipo_pcd, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 9, uf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 10, cidade, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 11, bairro, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 12, rua, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 13, complemento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 14, numero, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 15, senha, -1, SQLITE_STATIC);

    // Executando a query
    if (sqlite3_step(stmt_inserir) != SQLITE_DONE) {
        printf("Erro ao cadastrar o paciente: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_inserir);
        return -1;
    }

    printf("Paciente cadastrado com sucesso!\n");
    pause();

    // Finalizar a declaração SQL
    sqlite3_finalize(stmt_inserir);
    return 0;
}


