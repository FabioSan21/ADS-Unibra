// consulta.c
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include "consulta.h"
#include "utils.h"

#define MAX_IDS 100

int validar_data(const char *data) {
    int d, m, a;
    return sscanf(data, "%d/%d/%d", &d, &m, &a) == 3;
}

int validar_hora(const char *hora) {
    int h, m;
    return sscanf(hora, "%d:%d", &h, &m) == 2;
}

void obter_data_formatada(int dias_a_mais, char *data_formatada, char *dia_semana) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    tm.tm_mday += dias_a_mais;
    mktime(&tm);

    const char *dias_pt[] = {"domingo", "segunda", "terca", "quarta", "quinta", "sexta", "sabado"};
    strftime(data_formatada, 11, "%d/%m/%Y", &tm);
    strcpy(dia_semana, dias_pt[tm.tm_wday]);
}

void listar_horarios_disponiveis(sqlite3 *db, int id_medico, const char *data, const char *horario, char horarios[50][6], int *total) {
    int h_inicio, m_inicio, h_fim, m_fim;
    sscanf(horario, "%d:%d-%d:%d", &h_inicio, &m_inicio, &h_fim, &m_fim);

    *total = 0;
    while (h_inicio < h_fim || (h_inicio == h_fim && m_inicio < m_fim)) {
        char hora[6];
        sprintf(hora, "%02d:%02d", h_inicio, m_inicio);

        const char *sql_verifica = "SELECT COUNT(*) FROM consultas WHERE id_medico = ? AND data = ? AND hora = ? AND status = 'Agendada'";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql_verifica, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 1, id_medico);
        sqlite3_bind_text(stmt, 2, data, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, hora, -1, SQLITE_STATIC);

        int ocupado = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
            ocupado = 1;
        }
        sqlite3_finalize(stmt);

        if (!ocupado) {
            strcpy(horarios[*total], hora);
            (*total)++;
        }

        m_inicio += 30;
        if (m_inicio >= 60) {
            m_inicio -= 60;
            h_inicio++;
        }
    }
}

int agendar_consulta(sqlite3 *db, int id_paciente) {
    setlocale(LC_ALL, "");
    char data[11], hora[6];
    int id_medico, id_unidade;
    int unidades_validas[MAX_IDS], medicos_validos[MAX_IDS];
    char especialidade_selecionada[100], cidade_selecionada[100];
    int total_opcoes = 0;

    sqlite3_stmt *stmt;
    const char *sql_especialidades = "SELECT DISTINCT especialidade FROM medicos";
    if (sqlite3_prepare_v2(db, sql_especialidades, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao buscar especialidades: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("\n--- Especialidades Disponiveis ---\n");
    char especialidades[MAX_IDS][100];
    while (sqlite3_step(stmt) == SQLITE_ROW && total_opcoes < MAX_IDS) {
        const char *esp = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(especialidades[total_opcoes], esp);
        printf("%d - %s\n", total_opcoes + 1, esp);
        total_opcoes++;
    }
    sqlite3_finalize(stmt);

    int escolha;
    printf("Digite o numero da especialidade desejada: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > total_opcoes) {
        printf("Especialidade invalida.\n");
        return -1;
    }
    strcpy(especialidade_selecionada, especialidades[escolha - 1]);
    const char *sql_vagas_esp = "SELECT SUM(u.vagas) FROM unidades u JOIN medicos m ON u.id_unidade = m.id_unidade WHERE m.especialidade = ?";
    sqlite3_prepare_v2(db, sql_vagas_esp, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, especialidade_selecionada, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int total_vagas = sqlite3_column_int(stmt, 0);
        if (total_vagas <= 0) {
            printf("Nenhuma vaga disponivel nesta especialidade no momento. Tente novamente mais tarde.\n");
            sqlite3_finalize(stmt);
            return -1;
        }
    }
    sqlite3_finalize(stmt);


    const char *sql_verifica_existente = "SELECT COUNT(*) FROM consultas c JOIN medicos m ON c.id_medico = m.id_medico WHERE c.id_paciente = ? AND m.especialidade = ? AND c.status = 'Agendada'";
    sqlite3_prepare_v2(db, sql_verifica_existente, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_paciente);
    sqlite3_bind_text(stmt, 2, especialidade_selecionada, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
        printf("Voce ja possui uma consulta agendada nessa especialidade.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    const char *sql_cidades = "SELECT DISTINCT u.cidade FROM unidades u JOIN medicos m ON m.id_unidade = u.id_unidade WHERE m.especialidade = ?";
    if (sqlite3_prepare_v2(db, sql_cidades, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao buscar cidades: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, especialidade_selecionada, -1, SQLITE_STATIC);

    printf("\n--- Cidades Disponiveis ---\n");
    char cidades[MAX_IDS][100];
    total_opcoes = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && total_opcoes < MAX_IDS) {
        const char *cidade = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(cidades[total_opcoes], cidade);
        printf("%d - %s\n", total_opcoes + 1, cidade);
        total_opcoes++;
    }
    sqlite3_finalize(stmt);

    printf("Digite o numero da cidade desejada: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > total_opcoes) {
        printf("Cidade invalida.\n");
        return -1;
    }
    strcpy(cidade_selecionada, cidades[escolha - 1]);

    const char *sql_unidades = "SELECT DISTINCT u.id_unidade, u.nome, u.vagas FROM unidades u JOIN medicos m ON u.id_unidade = m.id_unidade WHERE m.especialidade = ? AND u.cidade = ?";
    if (sqlite3_prepare_v2(db, sql_unidades, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao buscar unidades: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, especialidade_selecionada, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cidade_selecionada, -1, SQLITE_STATIC);

    printf("\n--- Unidades Disponiveis ---\n");
    int total_unidades = 0;
    int unidade_vagas[MAX_IDS];
    while (sqlite3_step(stmt) == SQLITE_ROW && total_unidades < MAX_IDS) {
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        int vagas = sqlite3_column_int(stmt, 2);
        unidades_validas[total_unidades] = id;
        unidade_vagas[total_unidades] = vagas;
        printf("%d - %s (Vagas: %d)\n", total_unidades + 1, nome, vagas);
        total_unidades++;
    }
    sqlite3_finalize(stmt);

    printf("Escolha o numero da unidade desejada: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > total_unidades) {
        printf("Unidade invalida.\n");
        return -1;
    }
    id_unidade = unidades_validas[escolha - 1];
    int vagas_disponiveis = unidade_vagas[escolha - 1];
    if (vagas_disponiveis <= 0) {
        printf("A unidade nao possui vagas disponiveis.\n");
        return -1;
    }

    const char *sql_medicos = "SELECT id_medico, nome, genero, horario FROM medicos WHERE id_unidade = ? AND especialidade = ?";
    if (sqlite3_prepare_v2(db, sql_medicos, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao buscar medicos: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, id_unidade);
    sqlite3_bind_text(stmt, 2, especialidade_selecionada, -1, SQLITE_STATIC);

    printf("\n--- Medicos Disponiveis ---\n");
    char horarios_medico[50][6];
    int total_horarios = 0;
    int total_medicos = 0;
    char horario_trabalho[20];
    while (sqlite3_step(stmt) == SQLITE_ROW && total_medicos < MAX_IDS) {
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        const char *genero = (const char *)sqlite3_column_text(stmt, 2);
        const char *horario = (const char *)sqlite3_column_text(stmt, 3);
        medicos_validos[total_medicos++] = id;
        printf("%d - Dr(a). %s (%s) - Horario: %s\n", total_medicos, nome, genero, horario);
        strcpy(horario_trabalho, horario);
    }
    sqlite3_finalize(stmt);

    printf("Escolha o numero do medico desejado: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > total_medicos) {
        printf("Medico invalido.\n");
        return -1;
    }
    id_medico = medicos_validos[escolha - 1];

    printf("\n--- Datas Disponiveis ---\n");
    char datas[7][11], dias_semana[7][20];
    for (int i = 0; i < 7; i++) {
        obter_data_formatada(i, datas[i], dias_semana[i]);
        printf("%d - %s (%s)\n", i + 1, datas[i], dias_semana[i]);
    }

    printf("Escolha o numero do dia desejado: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > 7) {
        printf("Data invalida.\n");
        return -1;
    }
    strcpy(data, datas[escolha - 1]);

    listar_horarios_disponiveis(db, id_medico, data, horario_trabalho, horarios_medico, &total_horarios);
    if (total_horarios == 0) {
        printf("Nenhum horario disponivel para este dia.\n");
        return -1;
    }

    printf("\n--- Horarios Disponiveis ---\n");
    for (int i = 0; i < total_horarios; i++) {
        printf("%d - %s\n", i + 1, horarios_medico[i]);
    }

    printf("Escolha o numero do horario desejado: ");
    scanf("%d", &escolha);
    if (escolha < 1 || escolha > total_horarios) {
        printf("Horario invalido.\n");
        return -1;
    }
    strcpy(hora, horarios_medico[escolha - 1]);

    const char *sql_insert = "INSERT INTO consultas (id_paciente, id_medico, id_unidade, data, hora, status) VALUES (?, ?, ?, ?, ?, 'Agendada')";
    sqlite3_stmt *stmt_insert;
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) != SQLITE_OK) {
        printf("Erro ao preparar insercao: %s\n", sqlite3_errmsg(db));
        pause();
        return -1;
    }
    sqlite3_bind_int(stmt_insert, 1, id_paciente);
    sqlite3_bind_int(stmt_insert, 2, id_medico);
    sqlite3_bind_int(stmt_insert, 3, id_unidade);
    sqlite3_bind_text(stmt_insert, 4, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 5, hora, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt_insert) != SQLITE_DONE) {
        printf("Erro ao agendar consulta: %s\n", sqlite3_errmsg(db));
        pause();
        sqlite3_finalize(stmt_insert);
        return -1;
    }
    sqlite3_finalize(stmt_insert);

    const char *sql_atualiza_vagas = "UPDATE unidades SET vagas = vagas - 1 WHERE id_unidade = ?";
    sqlite3_stmt *stmt_vagas;
    sqlite3_prepare_v2(db, sql_atualiza_vagas, -1, &stmt_vagas, NULL);
    sqlite3_bind_int(stmt_vagas, 1, id_unidade);
    sqlite3_step(stmt_vagas);
    sqlite3_finalize(stmt_vagas);

    printf("Consulta agendada com sucesso!\n");
    pause();
    return 0;
}

void listar_consultas(sqlite3 *db, int id_paciente) {
    const char *sql = "SELECT c.id_consulta, c.data, c.hora, c.status, u.nome, m.nome, m.especialidade FROM consultas c JOIN unidades u ON c.id_unidade = u.id_unidade JOIN medicos m ON c.id_medico = m.id_medico WHERE c.id_paciente = ?";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, id_paciente);

    printf("\n--- Consultas Agendadas ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *data = (const char *)sqlite3_column_text(stmt, 1);
        const char *hora = (const char *)sqlite3_column_text(stmt, 2);
        const char *status = (const char *)sqlite3_column_text(stmt, 3);
        const char *unidade = (const char *)sqlite3_column_text(stmt, 4);
        const char *medico = (const char *)sqlite3_column_text(stmt, 5);
        const char *especialidade = (const char *)sqlite3_column_text(stmt, 6);

        printf("Código: %d | Data: %s | Hora: %s | Médico: %s | Especialidade: %s | Unidade: %s | Status: %s\n", id, data, hora, medico, especialidade, unidade, status);
    }
    sqlite3_finalize(stmt);
}

void cancelar_consulta(sqlite3 *db, int id_paciente) {
    // Listar apenas consultas com status 'Agendada'
    sqlite3_stmt *stmt_lista;
    const char *sql_lista = "SELECT id_consulta, data, hora FROM consultas "
                            "WHERE id_paciente = ? AND status = 'Agendada'";

    if (sqlite3_prepare_v2(db, sql_lista, -1, &stmt_lista, NULL) != SQLITE_OK) {
        printf("Erro ao listar consultas: %s\n", sqlite3_errmsg(db));
        pause();
        return;
    }

    sqlite3_bind_int(stmt_lista, 1, id_paciente);

    printf("\n--- Consultas Agendadas ---\n");
    int tem_consultas = 0;
    while (sqlite3_step(stmt_lista) == SQLITE_ROW) {
        int id_consulta = sqlite3_column_int(stmt_lista, 0);
        const unsigned char *data = sqlite3_column_text(stmt_lista, 1);
        const unsigned char *hora = sqlite3_column_text(stmt_lista, 2);
        printf("Consulta ID: %d | Data: %s | Hora: %s\n", id_consulta, data, hora);
        tem_consultas = 1;
    }

    sqlite3_finalize(stmt_lista);

    if (!tem_consultas) {
        printf("Nao ha consultas agendadas para cancelar.\n");
        pause();
        return;
    }

    int id_consulta;
    printf("Digite o codigo da consulta que deseja cancelar: ");
    scanf("%d", &id_consulta);

    // Buscar id_unidade antes de cancelar
    int id_unidade = -1;
    sqlite3_stmt *stmt_unidade;
    const char *sql_unidade = "SELECT id_unidade FROM consultas WHERE id_consulta = ? AND id_paciente = ?";
    if (sqlite3_prepare_v2(db, sql_unidade, -1, &stmt_unidade, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt_unidade, 1, id_consulta);
        sqlite3_bind_int(stmt_unidade, 2, id_paciente);
        if (sqlite3_step(stmt_unidade) == SQLITE_ROW) {
            id_unidade = sqlite3_column_int(stmt_unidade, 0);
        }
        sqlite3_finalize(stmt_unidade);
    }

    // Atualizar status da consulta
    const char *sql = "UPDATE consultas SET status = 'Cancelada' WHERE id_consulta = ? AND id_paciente = ?";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_consulta);
    sqlite3_bind_int(stmt, 2, id_paciente);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Consulta cancelada com sucesso!\n");

        // Atualizar vagas da unidade se o id_unidade foi obtido
        if (id_unidade != -1) {
            const char *sql_vagas = "UPDATE unidades SET vagas = vagas + 1 WHERE id_unidade = ?";
            sqlite3_stmt *stmt_vagas;
            if (sqlite3_prepare_v2(db, sql_vagas, -1, &stmt_vagas, NULL) == SQLITE_OK) {
                sqlite3_bind_int(stmt_vagas, 1, id_unidade);
                sqlite3_step(stmt_vagas);
                sqlite3_finalize(stmt_vagas);
            }
        }
    } else {
        printf("Erro ao cancelar consulta.\n");
        pause();
    }

    sqlite3_finalize(stmt);
}


void menu_paciente(sqlite3 *db, int id_paciente) {
    int opcao;
    do {
        limpar_terminal();
        printf("\n--- Menu Paciente ---\n");
        printf("1. Agendar Consulta\n");
        printf("2. Consultas Agendadas\n");
        printf("3. Cancelar Consulta\n");
        printf("4. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: agendar_consulta(db, id_paciente); break;
            case 2: listar_consultas(db, id_paciente); break;
            case 3: cancelar_consulta(db, id_paciente); break;
            case 4: printf("Saindo...\n"); break;
            default: printf("Opcao invalida. Tente novamente.\n");
        }
    } while (opcao != 4);
}

void ver_consultas_agendadas(sqlite3 *db, int id_medico) {
    int filtro;
    printf("\n--- Filtro de Consultas ---\n");
    printf("1. Todas\n");
    printf("2. Agendadas\n");
    printf("3. Atendidas\n");
    printf("4. Canceladas\n");
    printf("Escolha uma opcao: ");
    scanf("%d", &filtro);

    const char *sql_base = 
        "SELECT c.data, c.hora, u.nome, p.nome, c.status "
        "FROM consultas c "
        "JOIN unidades u ON c.id_unidade = u.id_unidade "
        "JOIN pacientes p ON c.id_paciente = p.id_paciente "
        "WHERE c.id_medico = ? ";

    char sql[512];
    if (filtro == 1) {
        snprintf(sql, sizeof(sql), "%sORDER BY c.data, c.hora;", sql_base);
    } else if (filtro == 2) {
        snprintf(sql, sizeof(sql), "%sAND c.status = 'Agendada' ORDER BY c.data, c.hora;", sql_base);
    } else if (filtro == 3) {
        snprintf(sql, sizeof(sql), "%sAND c.status = 'Atendida' ORDER BY c.data, c.hora;", sql_base);
    } else if (filtro == 4) {
        snprintf(sql, sizeof(sql), "%sAND c.status = 'Cancelada' ORDER BY c.data, c.hora;", sql_base);
    } else {
        printf("Opcao invalida!\n");
        return;
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao preparar consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id_medico);
    printf("\n--- Consultas ---\n");
    int encontrou = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *data = sqlite3_column_text(stmt, 0);
        const unsigned char *hora = sqlite3_column_text(stmt, 1);
        const unsigned char *unidade = sqlite3_column_text(stmt, 2);
        const unsigned char *paciente = sqlite3_column_text(stmt, 3);
        const unsigned char *status = sqlite3_column_text(stmt, 4);

        printf("Data: %s | Hora: %s | Unidade: %s | Paciente: %s | Status: %s\n",
               data, hora, unidade, paciente, status);
        encontrou = 1;
    }

    if (!encontrou) {
        printf("Nenhuma consulta encontrada.\n");
        pause();
    }

    sqlite3_finalize(stmt);
}

void consultar_paciente(sqlite3 *db, int id_medico) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT c.id_consulta, p.nome, p.cpf, c.data, c.hora "
                      "FROM consultas c "
                      "JOIN pacientes p ON c.id_paciente = p.id_paciente "
                      "WHERE c.id_medico = ? AND c.status = 'Agendada'";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id_medico);

    int index = 1;
    int ids_consultas[100];
    printf("\n--- Consultas Agendadas ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id_consulta = sqlite3_column_int(stmt, 0);
        const unsigned char *nome = sqlite3_column_text(stmt, 1);
        const unsigned char *cpf = sqlite3_column_text(stmt, 2);
        const unsigned char *data = sqlite3_column_text(stmt, 3);
        const unsigned char *hora = sqlite3_column_text(stmt, 4);

        printf("%d. %s | CPF: %s | Data: %s | Hora: %s\n", index, nome, cpf, data, hora);
        ids_consultas[index - 1] = id_consulta;
        index++;
    }

    if (index == 1) {
        printf("Nenhuma consulta agendada encontrada.\n");
        pause();
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    int opcao;
    printf("Escolha uma consulta para ver detalhes (0 para sair): ");
    scanf("%d", &opcao);
    if (opcao < 1 || opcao >= index) return;

    int id_consulta = ids_consultas[opcao - 1];

    // Buscar dados detalhados do paciente
    const char *sql_detalhes = "SELECT p.nome, p.cpf, p.telefone, p.idade, p.pcd, p.tipo_pcd, c.id_unidade "
                               "FROM consultas c "
                               "JOIN pacientes p ON c.id_paciente = p.id_paciente "
                               "WHERE c.id_consulta = ?";

    if (sqlite3_prepare_v2(db, sql_detalhes, -1, &stmt, 0) != SQLITE_OK) {
        printf("Erro ao preparar detalhes: %s\n", sqlite3_errmsg(db));
        pause();
        return;
    }

    sqlite3_bind_int(stmt, 1, id_consulta);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 0);
        const unsigned char *cpf = sqlite3_column_text(stmt, 1);
        const unsigned char *telefone = sqlite3_column_text(stmt, 2);
        int idade = sqlite3_column_int(stmt, 3);
        int pcd = sqlite3_column_int(stmt, 4);
        const unsigned char *tipo_pcd = sqlite3_column_text(stmt, 5);
        int id_unidade = sqlite3_column_int(stmt, 6);

        printf("\n--- Detalhes do Paciente ---\n");
        printf("Nome: %s\nCPF: %s\nTelefone: %s\nIdade: %d\n", nome, cpf, telefone, idade);
        if (idade > 60 || pcd) {
            printf("Paciente Prioridade\n");
            if (pcd) {
                printf("Tipo de PCD: %s\n", tipo_pcd);
            }
        }

        // Perguntar se deseja alterar o status
        int acao;
        printf("\n1. Cancelar Consulta\n2. Marcar como Atendida\n3. Voltar\nEscolha: ");
        scanf("%d", &acao);

        if (acao == 1 || acao == 2) {
            const char *novo_status = (acao == 1) ? "Cancelada" : "Atendida";

            sqlite3_stmt *update;
            const char *sql_update = "UPDATE consultas SET status = ? WHERE id_consulta = ?";
            if (sqlite3_prepare_v2(db, sql_update, -1, &update, 0) == SQLITE_OK) {
                sqlite3_bind_text(update, 1, novo_status, -1, SQLITE_STATIC);
                sqlite3_bind_int(update, 2, id_consulta);
                if (sqlite3_step(update) == SQLITE_DONE) {
                    printf("Consulta marcada como %s.\n", novo_status);

                    // Devolver vaga à unidade
                    const char *sql_devolver = "UPDATE unidades SET vagas = vagas + 1 "
                                               "WHERE id_unidade = ?";
                    sqlite3_stmt *devolver;
                    if (sqlite3_prepare_v2(db, sql_devolver, -1, &devolver, 0) == SQLITE_OK) {
                        sqlite3_bind_int(devolver, 1, id_unidade);
                        sqlite3_step(devolver);
                        sqlite3_finalize(devolver);
                    }
                } else {
                    printf("Erro ao atualizar status.\n");
                    pause();
                }
                sqlite3_finalize(update);
            }
        }
    } else {
        printf("Erro ao buscar dados do paciente.\n");
        pause();
    }

    sqlite3_finalize(stmt);
}
