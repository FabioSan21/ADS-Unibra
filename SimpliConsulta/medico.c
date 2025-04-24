#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "consulta.h"
#include "structs.h"
#include "utils.h"

// Função para cadastrar o médico
int cadastrar_medico(sqlite3 *db) {
    char nome[100], cpf[20], crm[20], telefone[20], senha[20], genero[20];
    char especialidade[50], horario[50];
    int id_unidade;

    // Listar especialidades
    const char* especialidades[] = {
        "Pneumologia", "Cardiologia", "Urologia", "Psiquiatrico", "ClinicoGeral",
        "Neurologia", "Ginecologia", "Oftalmologia", "Ortopedia", "Pediatra"
    };

    // Listar horários
    const char* horarios_manha[] = {"06:00-12:00", "07:00-13:00", "08:00-14:00"};
    const char* horarios_tarde[] = {"12:00-18:00", "13:00-19:00", "14:00-20:00"};
    const char* horarios_noite[] = {"18:00-00:00", "19:00-01:00", "20:00-02:00"};

    // Seleção de gênero
    const char* generos[] = {"Masculino", "Feminino", "Outros"};
    
    // Coletando dados do médico
    printf("\nCadastro de Medico:\n");
    printf("Nome: ");
    scanf(" %[^\n]s", nome);
    formatar_nome(nome); 
    do {
        printf("CPF: ");
        scanf(" %[^\n]s", cpf);
        if (!validar_cpf(cpf)) {
            printf("CPF invalido. Deve conter 11 digitos numericos.\n");
            continue;
        }
        if (cpf_existe_medico(db, cpf)) {
            printf("CPF ja cadastrado.\n");
        } else {
            break;
        }
    } while (1);
    do {
        printf("CRM: ");
        scanf(" %[^\n]s", crm);
    
        if (crm_existe(db, crm)) {
            printf("Este CRM ja esta cadastrado. Tente outro.\n");
        } else {
            break;
        }
    
    } while (1);
    do {
        printf("Telefone (apenas numeros, com DDD): ");
        scanf(" %[^\n]s", telefone);

        if (!validar_telefone(telefone)) {
            printf("Telefone invalido. Use apenas numeros com 10 ou 11 digitos.\n");
        } else {
            break;
        }
    } while (1);
    esconder_senha(senha); 
    
    // Seleção do gênero
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

    // Seleção da especialidade
    printf("Escolha a Especialidade:\n");
    for (int i = 0; i < 10; i++) {
        printf("%d. %s\n", i + 1, especialidades[i]);
    }
    int especialidade_opcao;
    printf("Opcao: ");
    scanf("%d", &especialidade_opcao);
    if (especialidade_opcao >= 1 && especialidade_opcao <= 10) {
        strcpy(especialidade, especialidades[especialidade_opcao - 1]);
    } else {
        printf("Opcao invalida. Especialidade nao selecionada.\n");
        return -1;
    }

    // Seleção da unidade (supondo que já tenha unidades cadastradas no banco)
    printf("Escolha a Unidade:\n");

    // Buscar unidades no banco de dados (somente unidades existentes)
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id_unidade, nome FROM unidades";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao buscar unidades: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    int unidade_opcao = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id_unidade_db = sqlite3_column_int(stmt, 0);
        const char *nome_unidade = (const char *)sqlite3_column_text(stmt, 1);
        printf("%d. %s\n", unidade_opcao, nome_unidade);
        unidade_opcao++;
    }

    sqlite3_finalize(stmt);

    printf("Opcao: ");
    scanf("%d", &id_unidade);

    // Seleção do horário de trabalho
    printf("Escolha o horario de trabalho (Manha/Tarde/Noite):\n");
    printf("1. Manha\n2. Tarde\n3. Noite\n");
    int horario_opcao;
    scanf("%d", &horario_opcao);
    switch (horario_opcao) {
        case 1:
            printf("Escolha um horario de manha:\n");
            for (int i = 0; i < 3; i++) {
                printf("%d. %s\n", i + 1, horarios_manha[i]);
            }
            printf("Opcao: ");
            scanf("%d", &horario_opcao);
            strcpy(horario, horarios_manha[horario_opcao - 1]);
            break;
        case 2:
            printf("Escolha um horario da tarde:\n");
            for (int i = 0; i < 3; i++) {
                printf("%d. %s\n", i + 1, horarios_tarde[i]);
            }
            printf("Opcao: ");
            scanf("%d", &horario_opcao);
            strcpy(horario, horarios_tarde[horario_opcao - 1]);
            break;
        case 3:
            printf("Escolha um horario da noite:\n");
            for (int i = 0; i < 3; i++) {
                printf("%d. %s\n", i + 1, horarios_noite[i]);
            }
            printf("Opcao: ");
            scanf("%d", &horario_opcao);
            strcpy(horario, horarios_noite[horario_opcao - 1]);
            break;
        default:
            printf("Opcao invalida! Horario nao selecionado.\n");
            return -1;
    }

    // Preparando a query para inserir o médico no banco de dados
    sqlite3_stmt *stmt_inserir;
    const char *sql_inserir = "INSERT INTO medicos (nome, cpf, crm, telefone, senha, genero, especialidade, id_unidade, horario) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, sql_inserir, -1, &stmt_inserir, 0) != SQLITE_OK) {
        printf("Erro ao preparar a consulta de insercao: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Bind dos parâmetros
    sqlite3_bind_text(stmt_inserir, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 2, cpf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 3, crm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 4, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 5, senha, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 6, genero, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_inserir, 7, especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_inserir, 8, id_unidade);
    sqlite3_bind_text(stmt_inserir, 9, horario, -1, SQLITE_STATIC);

    // Executando a query
    if (sqlite3_step(stmt_inserir) != SQLITE_DONE) {
        printf("Erro ao cadastrar o médico: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt_inserir);
        return -1;
    }

    printf("Medico cadastrado com sucesso!\n");
    pause();

    // Finalizar a declaração SQL
    sqlite3_finalize(stmt_inserir);
    return 0;
}

// Menu para o médico
void menu_medico(sqlite3 *db, LoginResultado login) {
    int opcao;
    do {
        limpar_terminal();
        printf("\n--- Menu do Medico ---\n");
        printf("1. Ver Consultas Agendadas\n");
        printf("2. Consultar Paciente\n");
        printf("3. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                ver_consultas_agendadas(db, login.id_usuario);
                break;
            case 2:
                consultar_paciente(db, login.id_usuario);
                break;
            case 3:
                printf("Saindo do sistema.\n");
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
                break;
        }
    } while (opcao != 3);
}
