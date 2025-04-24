// structs.h
#ifndef STRUCTS_H
#define STRUCTS_H

// Definição do tipo de usuário
typedef enum {
    TIPO_NAO_ENCONTRADO = 0,
    TIPO_PACIENTE = 1,
    TIPO_MEDICO = 2
} TipoUsuario;

// Estrutura do resultado do login
typedef struct {
    int id_usuario;         // Serve tanto para paciente quanto para médico
    TipoUsuario tipo;       // Enum que indica o tipo de usuário
    char nome[100];         // Nome do usuário logado
} LoginResultado;

#endif // STRUCTS_H
