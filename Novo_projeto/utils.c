#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "unidade.h"
#include "database.h"
#include "medico.h"
#include "usuario.h"
#include "consulta.h"
#include "structs.h"
#include <conio.h>  // Para getch()
#include <string.h>
#include <ctype.h>

/* 
// Função que calcula a idade com base na data de nascimento no formato dd/mm/aaaa
int calcular_idade(const char *data_nascimento) {
    int dia, mes, ano;

    // Extrai dia, mês e ano da string recebida
    sscanf(data_nascimento, "%d/%d/%d", &dia, &mes, &ano);

    // Obtém a data atual do sistema
    time_t t = time(NULL);            // Pega o tempo atual em segundos
    struct tm *hoje = localtime(&t);  // Converte para estrutura de data/hora

    // Calcula a idade básica: ano atual - ano de nascimento
    int idade = hoje->tm_year + 1900 - ano;

    // Se ainda não fez aniversário este ano, subtrai 1 da idade
    if (mes > (hoje->tm_mon + 1) || (mes == (hoje->tm_mon + 1) && dia > hoje->tm_mday)) {
        idade--;
    }

    return idade;  // Retorna a idade final
}
*/



int validar_telefone(const char *telefone) {
    int tamanho = strlen(telefone);

    // Verifica se o tamanho é válido
    if (tamanho != 10 && tamanho != 11) {
        return 0;
    }

    // Verifica se todos os caracteres são dígitos
    for (int i = 0; i < tamanho; i++) {
        if (!isdigit(telefone[i])) {
            return 0;
        }
    }

    return 1;
}

// Função para verificar se o CPF já existe no banco de dados
int cpf_existe_paciente(sqlite3 *db, const char *cpf) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM pacientes WHERE cpf = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    int existe = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existe = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return existe;
}

int validar_cpf(const char *cpf) {
    int len = strlen(cpf);
    if (len != 11) return 0;
    for (int i = 0; i < len; i++) {
        if (!isdigit(cpf[i])) return 0;
    }
    return 1;
}

// Função para verificar se o CPF já existe no banco de dados
int cpf_existe_medico(sqlite3 *db, const char *cpf) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM medicos WHERE cpf = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    int existe = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existe = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return existe;
}

// Função para verificar se o CRM já existe no banco de dados
int crm_existe(sqlite3 *db, const char *crm) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM medicos WHERE crm = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return -1; // Erro ao preparar a consulta
    }

    sqlite3_bind_text(stmt, 1, crm, -1, SQLITE_STATIC);

    int existe = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existe = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return existe;
}

// Função para limpar o terminal
void limpar_terminal() {
    system("cls");  // Para Windows; use "clear" em sistemas Unix
}


// Entrada de senha com asteriscos
void digitar_senha(char *senha) {
    int i = 0;
    char ch;

    printf("Digite a senha: ");
    while ((ch = getch()) != '\r' && i < 19) { // '\r' é o Enter no Windows
        if (ch == '\b' && i > 0) {
            printf("\b \b");
            i--;
        } else if (ch != '\b') {
            senha[i++] = ch;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

// Função de login para pacientes e médicos
LoginResultado realizar_login(sqlite3 *db) {
    char cpf[20], senha[20];
    LoginResultado resultado = {-1, TIPO_NAO_ENCONTRADO, ""};

    printf("=== LOGIN ===\n");
    printf("Digite o CPF: ");
    scanf(" %s", cpf);
    digitar_senha(senha);

    sqlite3_stmt *stmt;
    const char *sql_paciente = "SELECT id_paciente, nome FROM pacientes WHERE cpf = ? AND senha = ?";
    const char *sql_medico   = "SELECT id_medico, nome FROM medicos WHERE cpf = ? AND senha = ?";

    // Tenta login como paciente
    if (sqlite3_prepare_v2(db, sql_paciente, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, senha, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            resultado.id_usuario = sqlite3_column_int(stmt, 0);
            resultado.tipo = TIPO_PACIENTE;
            strcpy(resultado.nome, (const char *)sqlite3_column_text(stmt, 1));
            sqlite3_finalize(stmt);
            printf("\nLogin como PACIENTE bem-sucedido!\nBem-vindo(a), %s!\n", resultado.nome);
            pause();
            return resultado;
        }
        sqlite3_finalize(stmt);
    } else {
        erro_sqlite3(db, "Erro ao preparar consulta de paciente");

    }

    // Tenta login como médico
    if (sqlite3_prepare_v2(db, sql_medico, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, senha, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            resultado.id_usuario = sqlite3_column_int(stmt, 0);
            resultado.tipo = TIPO_MEDICO;
            strcpy(resultado.nome, (const char *)sqlite3_column_text(stmt, 1));
            sqlite3_finalize(stmt);
            printf("\nLogin como MÉDICO bem-sucedido!\nBem-vindo, Dr(a). %s!\n", resultado.nome);
            pause();
            return resultado;
        }
        sqlite3_finalize(stmt);
    } else {
        erro_sqlite3(db, "Erro ao preparar consulta de médico");
    }

    printf("CPF ou senha incorretos.\n");
    return resultado;
}

// Tratamento de erro do SQLite
void erro_sqlite3(sqlite3 *db, const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
}

int validar_email(const char* email) {
    int at_pos = -1;
    int dot_pos = -1;
    int len = strlen(email);

    if (len < 5) return 0; // Muito curto pra ser um e-mail válido

    for (int i = 0; i < len; i++) {
        if (isspace(email[i])) return 0; // Espaços são inválidos

        if (email[i] == '@') {
            if (at_pos != -1) return 0; // Mais de um @
            at_pos = i;
        } else if (email[i] == '.' && at_pos != -1 && i > at_pos + 1) {
            dot_pos = i;
        }
    }

    // Regras básicas: tem um @, tem um . depois do @, e os dois não estão no início/fim
    return at_pos > 0 && dot_pos > at_pos && dot_pos < len - 1;
}

void formatar_nome(char* nome) {
    int i = 0;
    int novo_nome = 1;

    while (nome[i]) {
        if (isspace(nome[i])) {
            novo_nome = 1;
        } else if (novo_nome && isalpha(nome[i])) {
            nome[i] = toupper(nome[i]);
            novo_nome = 0;
        } else {
            nome[i] = tolower(nome[i]);
        }
        i++;
    }
}

int validar_idade(const char *entrada) {
    for (int i = 0; entrada[i]; i++) {
        if (!isdigit(entrada[i])) {
            return 0; // Tem caractere não numérico
        }
    }

    int idade = atoi(entrada);
    return idade > 0;
}


void esconder_senha(char senha_final[]) {
    char senha[50], confirmacao[50];
    int iguais = 0;

    do {
        int i = 0;
        char ch;

        // Solicita a senha
        printf("Digite a senha: ");
        while ((ch = getch()) != '\r' && i < 49) { // até pressionar Enter
            if (ch == 8) { // backspace
                if (i > 0) {
                    i--;
                    printf("\b \b");
                }
            } else {
                senha[i++] = ch;
                printf("*");
            }
        }
        senha[i] = '\0';

        printf("\n");

        // Solicita a confirmação da senha
        printf("Confirme a senha: ");
        i = 0;
        while ((ch = getch()) != '\r' && i < 49) {
            if (ch == 8) {
                if (i > 0) {
                    i--;
                    printf("\b \b");
                }
            } else {
                confirmacao[i++] = ch;
                printf("*");
            }
        }
        confirmacao[i] = '\0';

        printf("\n");

        // Verifica se as senhas coincidem
        if (strcmp(senha, confirmacao) == 0) {
            strcpy(senha_final, senha);  // A senha final é a senha inserida
            iguais = 1;
        } else {
            printf("\nAs senhas nao conferem. Tente novamente.\n\n");
        }
    } while (!iguais);
}

// Função de pausa, esperando o ENTER para continuar
void pause() {
    printf("Pressione ENTER para continuar...");
    getchar(); // Limpa o buffer
    getchar(); // Espera o ENTER
}

// database.c
/*int limpar_tabelas(sqlite3 *db) {
    const char *sql = 
        "DELETE FROM consultas;"
        "DELETE FROM pacientes;"
        "DELETE FROM medicos;"
        "DELETE FROM unidades;";

    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Erro ao limpar as tabelas: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    printf("Todas as tabelas foram esvaziadas com sucesso.\n");
    return 0;
}*/
