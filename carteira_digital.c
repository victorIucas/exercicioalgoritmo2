#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENTES 100
#define TAM_NOME 60
#define TAM_HISTORICO 500
#define ARQUIVO_DADOS "banco.dat"

// Tipos de conta: 1 - Corrente, 2 - Salario, 3 - Poupanca

typedef struct {
    int ativa;                      // 1 se a conta existe; 0 caso contrario
    int tipo;                       // Tipo da conta
    float saldo;                     // Saldo da conta
    char historico[TAM_HISTORICO];  // Movimentacoes registradas
} Conta;

typedef struct {
    char nome[TAM_NOME];
    int numero;                       // Identificador do cliente
    Conta contas[3];                  // Corrente, salario e poupanca
    int emprestimoRealizado;          // 1 se ja recebeu emprestimo
    float valorEmprestimo;
} Cliente;


// ------------------------------------------------------------
// FUNCOES AUXILIARES
// ------------------------------------------------------------

// Descarta os caracteres restantes da linha digitada.
void limparBuffer(void) {
    int caractere;
    while ((caractere = getchar()) != '\n' && caractere != EOF) {
        // Apenas descarta os caracteres.
    }
}


// Acrescenta uma mensagem ao historico, sem apagar as anteriores.
void registrarHistorico(Conta *conta, const char *mensagem) {
    size_t tamanhoAtual = strlen(conta->historico);
    size_t espacoRestante = TAM_HISTORICO - tamanhoAtual;

    if (espacoRestante > 1) {
        snprintf(conta->historico + tamanhoAtual,
                 espacoRestante, "%s\n", mensagem);
    }
}


// Retorna o saldo somado de todas as contas ativas do cliente.
float calcularSaldoTotal(Cliente cliente) {
    float total = 0.0f;
    int i;

    for (i = 0; i < 3; i++) {
        if (cliente.contas[i].ativa == 1) {
            total += cliente.contas[i].saldo;
        }
    }

    return total;
}


// Verifica se o cliente possui corrente e poupanca com saldo positivo.
int clienteAptoEmprestimo(Cliente cliente) {
    int temCorrente = 0;
    int temPoupancaComSaldo = 0;
    int i;

    for (i = 0; i < 3; i++) {
        if (cliente.contas[i].ativa == 1) {
            if (cliente.contas[i].tipo == 1) {
                temCorrente = 1;
            }

            if (cliente.contas[i].tipo == 3 &&
                cliente.contas[i].saldo > 0.0f) {
                temPoupancaComSaldo = 1;
            }
        }
    }

    return temCorrente && temPoupancaComSaldo;
}


// ------------------------------------------------------------
// ARQUIVOS: SALVAR E CARREGAR OS DADOS
// ------------------------------------------------------------

void salvarArquivo(Cliente clientes[], int quantidade) {
    FILE *arquivo = fopen(ARQUIVO_DADOS, "wb");

    if (arquivo == NULL) {
        printf("Erro: nao foi possivel salvar os dados.\n");
        return;
    }


    // Grava primeiro a quantidade de clientes.
    if (fwrite(&quantidade, sizeof(int), 1, arquivo) != 1) {
        printf("Erro ao salvar a quantidade de clientes.\n");
        fclose(arquivo);
        return;
    }


    // Depois grava os dados dos clientes.
    if (quantidade > 0 &&
        fwrite(clientes, sizeof(Cliente), quantidade, arquivo) != (size_t)quantidade) {
        printf("Erro ao salvar os clientes.\n");
        fclose(arquivo);
        return;
    }


    fclose(arquivo);
    printf("Dados salvos com sucesso!\n");
}


// Le os dados do arquivo. Se ele nao existir, inicia sem clientes.
int carregarArquivo(Cliente clientes[]) {
    FILE *arquivo = fopen(ARQUIVO_DADOS, "rb");
    int quantidade = 0;

    if (arquivo == NULL) {
        return 0;
    }


    if (fread(&quantidade, sizeof(int), 1, arquivo) != 1) {
        printf("Arquivo vazio ou invalido. Iniciando banco sem clientes.\n");
        fclose(arquivo);
        return 0;
    }


    // Impede que uma quantidade invalida ultrapasse o vetor.
    if (quantidade < 0 || quantidade > MAX_CLIENTES) {
        printf("Quantidade de clientes invalida no arquivo.\n");
        fclose(arquivo);
        return 0;
    }


    if (quantidade > 0 &&
        fread(clientes, sizeof(Cliente), quantidade, arquivo) != (size_t)quantidade) {
        printf("Erro ao ler os dados dos clientes.\n");
        fclose(arquivo);
        return 0;
    }


    fclose(arquivo);
    return quantidade;
}


// ------------------------------------------------------------
// CADASTRO E CONSULTA DE CLIENTES
// ------------------------------------------------------------

void cadastrarCliente(Cliente clientes[], int *quantidade) {
    Cliente novo;
    int i, opcao;

    if (*quantidade >= MAX_CLIENTES) {
        printf("Limite de clientes atingido!\n");
        return;
    }


    // Inicializa toda a estrutura antes de preencher os dados.
    memset(&novo, 0, sizeof(Cliente));

    printf("\n===== CADASTRO DE CLIENTE =====\n");
    printf("Nome do cliente: ");
    limparBuffer();
    fgets(novo.nome, sizeof(novo.nome), stdin);
    novo.nome[strcspn(novo.nome, "\n")] = '\0';

    if (strlen(novo.nome) == 0) {
        printf("Nome nao pode ficar vazio. Cadastro cancelado.\n");
        return;
    }


    printf("Numero do cliente: ");
    if (scanf("%d", &novo.numero) != 1) {
        printf("Numero invalido. Cadastro cancelado.\n");
        limparBuffer();
        return;
    }


    // Verifica se o numero escolhido ja pertence a outro cliente.
    for (i = 0; i < *quantidade; i++) {
        if (clientes[i].numero == novo.numero) {
            printf("Esse numero ja esta cadastrado!\n");
            return;
        }
    }


    printf("\nInforme quais contas o cliente possui.\n");
    printf("1 - Conta corrente\n");
    printf("2 - Conta salario\n");
    printf("3 - Conta poupanca\n");

    // Cada posicao do vetor corresponde a um tipo de conta.
    for (i = 0; i < 3; i++) {
        printf("Cliente possui conta %d? (1-Sim / 0-Nao): ", i + 1);

        if (scanf("%d", &opcao) != 1 || (opcao != 0 && opcao != 1)) {
            printf("Opcao invalida. Cadastro cancelado.\n");
            limparBuffer();
            return;
        }

        if (opcao == 1) {
            novo.contas[i].ativa = 1;
            novo.contas[i].tipo = i + 1;

            printf("Saldo inicial: R$ ");
            if (scanf("%f", &novo.contas[i].saldo) != 1) {
                printf("Saldo invalido. Cadastro cancelado.\n");
                limparBuffer();
                return;
            }

            // Registra a abertura da conta no historico.
            registrarHistorico(&novo.contas[i], "Conta cadastrada.");
        }
    }


    // Adiciona o cliente ao vetor e atualiza a quantidade.
    clientes[*quantidade] = novo;
    (*quantidade)++;


    printf("Cliente cadastrado com sucesso!\n");
}


void listarClientes(Cliente clientes[], int quantidade) {
    int i, j;

    printf("\n===== CLIENTES CADASTRADOS =====\n");

    if (quantidade == 0) {
        printf("Nenhum cliente cadastrado.\n");
        return;
    }


    for (i = 0; i < quantidade; i++) {
        printf("\nNome: %s\n", clientes[i].nome);
        printf("Numero: %d\n", clientes[i].numero);

        for (j = 0; j < 3; j++) {
            if (clientes[i].contas[j].ativa == 1) {
                if (clientes[i].contas[j].tipo == 1) {
                    printf("Conta corrente");
                } else if (clientes[i].contas[j].tipo == 2) {
                    printf("Conta salario");
                } else {
                    printf("Conta poupanca");
                }

                printf(" | Saldo: R$ %.2f\n",
                       clientes[i].contas[j].saldo);
            }
        }

        printf("Saldo total: R$ %.2f\n", calcularSaldoTotal(clientes[i]));
    }
}


// ------------------------------------------------------------
// EMPRESTIMO
// ------------------------------------------------------------

void solicitarEmprestimo(Cliente clientes[], int quantidade) {
    int numero, indice = -1, i;
    float saldoTotal, limite, valor;

    printf("\n===== SOLICITAR EMPRESTIMO =====\n");
    printf("Informe o numero do cliente: ");

    if (scanf("%d", &numero) != 1) {
        printf("Numero invalido.\n");
        limparBuffer();
        return;
    }


    // Localiza o cliente pelo numero.
    for (i = 0; i < quantidade; i++) {
        if (clientes[i].numero == numero) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        printf("Cliente nao encontrado!\n");
        return;
    }


    // Cada cliente pode realizar apenas um emprestimo.
    if (clientes[indice].emprestimoRealizado == 1) {
        printf("Este cliente ja realizou um emprestimo.\n");
        return;
    }


    // Verifica os requisitos antes de calcular e liberar o emprestimo.
    if (!clienteAptoEmprestimo(clientes[indice])) {
        printf("Cliente nao esta apto ao emprestimo.\n");
        printf("E necessario ter conta corrente e poupanca com saldo positivo.\n");
        return;
    }


    saldoTotal = calcularSaldoTotal(clientes[indice]);

    // Interpretacao do enunciado: 20% de 10 vezes o saldo total.
    limite = 0.20f * (10.0f * saldoTotal);

    printf("Saldo total: R$ %.2f\n", saldoTotal);
    printf("Limite de emprestimo: R$ %.2f\n", limite);
    printf("Valor solicitado: R$ ");

    if (scanf("%f", &valor) != 1) {
        printf("Valor invalido.\n");
        limparBuffer();
        return;
    }


    if (valor <= 0.0f || valor > limite) {
        printf("Valor fora do limite permitido.\n");
        return;
    }


    // O valor aprovado e depositado na conta corrente.
    for (i = 0; i < 3; i++) {
        if (clientes[indice].contas[i].ativa == 1 &&
            clientes[indice].contas[i].tipo == 1) {

            clientes[indice].contas[i].saldo += valor;
            registrarHistorico(&clientes[indice].contas[i],
                             "Emprestimo recebido na conta corrente.");
            break;
        }
    }


    // Marca o emprestimo para impedir uma segunda solicitacao.
    clientes[indice].emprestimoRealizado = 1;
    clientes[indice].valorEmprestimo = valor;

    printf("Emprestimo aprovado!\n");
    printf("Valor depositado: R$ %.2f\n", valor);
}


// ------------------------------------------------------------
// RELATORIOS E HISTORICO
// ------------------------------------------------------------

void relatorios(Cliente clientes[], int quantidade) {
    int i;
    float saldoBanco = 0.0f;

    printf("\n===== RELATORIOS DO BANCO =====\n");

    for (i = 0; i < quantidade; i++) {
        saldoBanco += calcularSaldoTotal(clientes[i]);
    }

    printf("Saldo total do banco (soma dos saldos dos clientes): R$ %.2f\n",
           saldoBanco);

    printf("\n===== CLIENTES APTOS A EMPRESTIMO =====\n");
    for (i = 0; i < quantidade; i++) {
        if (clienteAptoEmprestimo(clientes[i]) &&
            clientes[i].emprestimoRealizado == 0) {
            printf("%s | Numero: %d | Saldo: R$ %.2f\n",
                   clientes[i].nome, clientes[i].numero,
                   calcularSaldoTotal(clientes[i]));
        }
    }

    printf("\n===== CLIENTES COM SALDO NEGATIVO =====\n");
    for (i = 0; i < quantidade; i++) {
        if (calcularSaldoTotal(clientes[i]) < 0.0f) {
            printf("%s | Saldo: R$ %.2f\n",
                   clientes[i].nome, calcularSaldoTotal(clientes[i]));
        }
    }

    printf("\n===== CLIENTES CADASTRADOS =====\n");
    for (i = 0; i < quantidade; i++) {
        printf("%s | Numero: %d | Saldo: R$ %.2f\n",
               clientes[i].nome, clientes[i].numero,
               calcularSaldoTotal(clientes[i]));
    }
}


void historicoBanco(Cliente clientes[], int quantidade) {
    int i, j;

    printf("\n===== HISTORICO DO BANCO =====\n");

    if (quantidade == 0) {
        printf("Nenhum cliente cadastrado.\n");
        return;
    }


    for (i = 0; i < quantidade; i++) {
        printf("\nCliente: %s (numero %d)\n",
               clientes[i].nome, clientes[i].numero);

        for (j = 0; j < 3; j++) {
            if (clientes[i].contas[j].ativa == 1) {
                printf("\nConta tipo %d:\n", clientes[i].contas[j].tipo);
                printf("%s", clientes[i].contas[j].historico);
            }
        }
    }
}


// ------------------------------------------------------------
// MENU INTERNO DO BANCO
// ------------------------------------------------------------

void menuBanco(Cliente clientes[], int *quantidade) {
    int opcao;

    do {
        printf("\n========== BANCO ABERTO ==========\n");
        printf("1 - Cadastrar cliente\n");
        printf("2 - Listar clientes\n");
        printf("3 - Solicitar emprestimo\n");
        printf("4 - Relatorios\n");
        printf("5 - Historico do banco\n");
        printf("6 - Salvar e fechar banco\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida.\n");
            limparBuffer();
            opcao = 0;
            continue;
        }

        switch (opcao) {
            case 1:
                cadastrarCliente(clientes, quantidade);
                break;

            case 2:
                listarClientes(clientes, *quantidade);
                break;

            case 3:
                solicitarEmprestimo(clientes, *quantidade);
                break;

            case 4:
                relatorios(clientes, *quantidade);
                break;

            case 5:
                historicoBanco(clientes, *quantidade);
                break;

            case 6:
                salvarArquivo(clientes, *quantidade);
                printf("Banco fechado.\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 6);
}


// ------------------------------------------------------------
// MENU PRINCIPAL
// ------------------------------------------------------------

int main(void) {
    Cliente clientes[MAX_CLIENTES];
    int quantidade = 0;
    int opcao;
    char sair = 'n';

    do {
        printf("\n========== CARTEIRA DIGITAL ==========\n");
        printf("1 - Abrir banco\n");
        printf("2 - Sair\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida.\n");
            limparBuffer();
            opcao = 0;
            continue;
        }


        switch (opcao) {
            case 1:
                // Ao abrir, recupera os dados que foram salvos anteriormente.
                quantidade = carregarArquivo(clientes);
                printf("Banco aberto. Clientes carregados: %d\n", quantidade);
                menuBanco(clientes, &quantidade);
                break;

            case 2:
                printf("Deseja realmente sair? (s/n): ");
                scanf(" %c", &sair);

                if (sair == 's' || sair == 'S') {
                    printf("Encerrando o programa...\n");
                } else {
                    sair = 'n';
                }
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (sair != 's' && sair != 'S');

    return 0;
}
