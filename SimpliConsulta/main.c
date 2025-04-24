#include <stdio.h>
#include <sqlite3.h>
#include "unidade.h"
#include "database.h"
#include "medico.h"
#include "usuario.h"
#include "utils.h"
#include "consulta.h"
#include "structs.h"

// Correção: declaração correta da função menu_medico
void menu_medico(sqlite3 *db, LoginResultado login);

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("consultas.db", &db);
    if (rc) {
        fprintf(stderr, "Nao foi possivel abrir o banco de dados: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    printf("Banco de dados aberto com sucesso.\n");

    if (criar_tabelas(db) != 0) {
        fprintf(stderr, "Erro ao criar as tabelas.\n");
        sqlite3_close(db);
        return 0;
    }

    //limpar_tabelas(db); // ZERA as tabelas - todo banco e zerado

    int opcao;

    do {
        limpar_terminal();
        printf("\n=== SIMPLICONSULTA ===\n");
        printf("\n=== Menu Principal ===\n");
        printf("1. Login\n");
        printf("2. Cadastro\n");
        printf("3. Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: {
                LoginResultado login = realizar_login(db);
                if (login.id_usuario != -1) {
                    if (login.tipo == TIPO_PACIENTE) {
                        menu_paciente(db, login.id_usuario);
                    } else if (login.tipo == TIPO_MEDICO) {
                        // Correção: passar db e login para menu_medico
                        menu_medico(db, login);
                    } else {
                        printf("Tipo de usuario nao reconhecido.\n");
                    }
                }
                break;
            }
            case 2: {
                int cadastro_opcao;
                printf("\n=== Cadastro ===\n");
                printf("1. Cadastrar Medico\n");
                printf("2. Cadastrar Paciente\n");
                printf("3. Cadastrar Unidade\n");
                printf("4. Voltar\n");
                printf("Opcao: ");
                scanf("%d", &cadastro_opcao);

                switch (cadastro_opcao) {
                    case 1:
                        if (validar_token() == 0) {
                            printf("Token invalido. Operacao cancelada.\n");
                            break;
                        }
                        if (cadastrar_medico(db) != 0) {
                            printf("Erro ao cadastrar medico.\n");
                        }
                        break;
                    case 2:
                        if (cadastrar_paciente(db) != 0) {
                            printf("Erro ao cadastrar paciente.\n");
                        }
                        break;
                    case 3:
                        if (cadastrar_unidade(db) != 0) {
                            printf("Erro ao cadastrar unidade.\n");
                        }
                        break;
                    case 4:
                        break;
                    default:
                        printf("Opcao invalida. Tente novamente.\n");
                        break;
                }
                break;
            }
            case 3:
                printf("Saindo do sistema. Ate logo!\n");
                break;
            default:
                printf("Opcao invalida! Tente novamente.\n");
                break;
        }
    } while (opcao != 3);

    sqlite3_close(db);
    printf("Banco de dados fechado.\n");
    return 0;
}
