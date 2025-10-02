/******************************************************************************
 *
 * Lib com as funções necessárias para o funcionamento da cozinha
 *
 * Desenvolvido com a assistência da IA Gemini
 * Data: 28 de setembro de 2025
 * 
 *****************************************************************************/



 #include <stdio.h>
 #include <wchar.h>
 #include <locale.h>
 #include <stdlib.h>
 #include <string.h>
 #include <wctype.h>
 #include <time.h>


  // --- ENUMS E CONSTANTES ---
 
typedef enum {
    FAZ_SANDUICHE, FAZ_BATATA, FAZ_BEBIDA, FAZ_MILKSHAKE,
    MONTA_BANDEJA, SEPARA_PEDIDO, CAIXA
} Habilidade;

 // ALTERADO: Novos status para o novo fluxo
 typedef enum {
    NAO_CHEGOU,
    AGUARDANDO_CAIXA,
    ATENDIMENTO_CAIXA,
    AGUARDANDO_DISTRIBUICAO, 
    EM_PRODUCAO, 
    AGUARDANDO_MONTAGEM,    
    EM_MONTAGEM,            
    CONCLUIDO
} Status;

#define TEMPO_MAXIMO_PEDIDO 300
#define TOTAL_FUNCIONARIOS 13
#define MAX_ITENS_POR_PEDIDO 10
#define INTERVALO_CHEGADA_PEDIDOS 45

#define TEMPO_ATENDIMENTO_CAIXA 30
#define TEMPO_DISTRIBUICAO_ITENS 30  
#define TEMPO_MONTAGEM_BANDEJA 30    
#define TEMPO_SAND_SIMPLES 58
#define TEMPO_SAND_MEDIO 88
#define TEMPO_BATATA 190
#define TEMPO_SUCO 38
#define TEMPO_MILKSHAKE 60





// --- ESTRUTURAS DE DADOS ---
 
typedef struct {
    int id_pedido;
    char nome[30];
    Habilidade tipo_preparo;
    int tempo_restante;
    int id_funcionario; // NOVO: ID do funcionário responsável
} Item;

typedef struct Pedido {
    int id;
    int qtd_itens_total;
    int itens_prontos;
    Status status;
    int tempo_chegada;
    int tempo_final;
    int sanduiches_s, sanduiches_m, batatas, sucos, milkshakes;
} Pedido;

typedef struct NoFila {
    void* dado;
    struct NoFila* proximo;
} NoFila;

typedef struct {
    NoFila *inicio;
    NoFila *fim;
} Fila;

typedef struct {
    int id;
    int habilidades[7];
    
    // Capacidade *individual* (calculada pelo nome)
    int cap_chapa;
    int cap_fritadeira;
    int cap_liquidificador;
    
    // Contagem *atual* de itens em preparo
    int ocupado_chapa;
    int ocupado_fritadeira;
    int ocupado_liquidificador;
    
    // Tarefas "exclusivas" (que ocupam 100% do funcionário)
    // CAIXA, SEPARA_PEDIDO, FAZ_BEBIDA (suco)
    int ocupado_tarefa_exclusiva; // 1 se ocupado, 0 se livre
    int tempo_tarefa_atual;       // Timer para a tarefa exclusiva
    Habilidade tipo_tarefa_atual; // Tipo da tarefa exclusiva
    Item *item_atual;             // Para FAZ_BEBIDA
    Pedido *pedido_tarefa_atual;  // Para CAIXA, SEPARA_PEDIDO
    
} Funcionario;


// --- VARIÁVEIS GLOBAIS ---
Funcionario funcionarios[TOTAL_FUNCIONARIOS];
Fila fila_caixa, fila_distribuicao, fila_montagem, fila_chapa, fila_fritadeira, fila_liquidificador, fila_bebidas_gerais;
Fila fila_itens_em_preparo;


 /*
 *
 * FUNCOES DA FILA
 * 
 * inicializar_fila()
 * fila_vazia()
 * enfileirar()
 * desenfileirar()
 * tamanho_fila()
 *
 */

void inicializar_fila(Fila* f) { f->inicio = f->fim = NULL; }

int fila_vazia(Fila* f) { return f->inicio == NULL; }

void enfileirar(Fila* f, void* dado) {
    NoFila* novo_no = (NoFila*)malloc(sizeof(NoFila));
    novo_no->dado = dado; novo_no->proximo = NULL;
    if (fila_vazia(f)) { f->inicio = f->fim = novo_no; }
    else { f->fim->proximo = novo_no; f->fim = novo_no; }
}

void* desenfileirar(Fila* f) {
    if (fila_vazia(f)) return NULL;
    NoFila* no_removido = f->inicio; void* dado = no_removido->dado;
    f->inicio = f->inicio->proximo;
    if (f->inicio == NULL) f->fim = NULL;
    free(no_removido); return dado;
}

/**
 * @brief Calcula o número de elementos em uma fila.
 * @param f A fila a ser medida.
 * @return O número de elementos na fila.
 */
int tamanho_fila(Fila* f) {
    int contador = 0;
    NoFila* atual = f->inicio;
    while (atual != NULL) {
        contador++;
        atual = atual->proximo;
    }
    return contador;
}

 /*
 *
 *  INICIALIZADORES E IMPRESSÃO DE STATUS
 * 
 *  inicializar_funcionarios()
 *  inicializar_todas_filas()
 *  imprimir_status_funcionarios()
 *
 */

 void inicializar_funcionarios(int cap_chapa, int cap_fritadeira, int cap_milkshake) {
    for (int i = 0; i < TOTAL_FUNCIONARIOS; i++) {
        funcionarios[i] = (Funcionario){
            .id = i+1, .ocupado_tarefa_exclusiva = 0, .item_atual = NULL, .pedido_tarefa_atual = NULL,
            .cap_chapa = cap_chapa, .cap_fritadeira = cap_fritadeira, .cap_liquidificador = cap_milkshake,
            .ocupado_chapa = 0, .ocupado_fritadeira = 0, .ocupado_liquidificador = 0
        };
        for (int j = 0; j < 7; j++) funcionarios[i].habilidades[j] = 0;
    }
    
    // ATENÇÃO: As habilidades definem os especialistas de cada nova tarefa
    funcionarios[0].habilidades[FAZ_SANDUICHE] = 1;
    funcionarios[1].habilidades[FAZ_SANDUICHE] = 1;
    funcionarios[2].habilidades[FAZ_SANDUICHE] = 1; funcionarios[2].habilidades[FAZ_BATATA] = 1;
    funcionarios[3].habilidades[FAZ_SANDUICHE] = 1; funcionarios[3].habilidades[FAZ_BATATA] = 1;
    funcionarios[4].habilidades[FAZ_SANDUICHE] = 1; funcionarios[4].habilidades[FAZ_BEBIDA] = 1; funcionarios[4].habilidades[FAZ_MILKSHAKE] = 1;
    funcionarios[5].habilidades[FAZ_BATATA] = 1;
    funcionarios[6].habilidades[FAZ_BATATA] = 1; funcionarios[6].habilidades[FAZ_SANDUICHE] = 1;
    // Funcionários [7] e [8] são os únicos que MONTAM a bandeja no final
    funcionarios[7].habilidades[FAZ_BEBIDA] = 1; funcionarios[7].habilidades[FAZ_MILKSHAKE] = 1; funcionarios[7].habilidades[MONTA_BANDEJA] = 1;
    funcionarios[8].habilidades[MONTA_BANDEJA] = 1;
    // Funcionários [9] e [10] são os únicos que SEPARAM os pedidos para produção
    funcionarios[9].habilidades[SEPARA_PEDIDO] = 1; funcionarios[9].habilidades[CAIXA] = 1;
    funcionarios[10].habilidades[SEPARA_PEDIDO] = 1; funcionarios[10].habilidades[FAZ_SANDUICHE] = 1;
    // Funcionários [11] e [12] são especialistas de CAIXA
    funcionarios[11].habilidades[CAIXA] = 1;
    funcionarios[12].habilidades[CAIXA] = 1; funcionarios[12].habilidades[FAZ_BEBIDA] = 1; funcionarios[12].habilidades[FAZ_MILKSHAKE] = 1;
}


void inicializar_todas_filas() {
    inicializar_fila(&fila_caixa); 
    inicializar_fila(&fila_distribuicao); 
    inicializar_fila(&fila_montagem);    
    inicializar_fila(&fila_chapa); 
    inicializar_fila(&fila_fritadeira);
    inicializar_fila(&fila_liquidificador); 
    inicializar_fila(&fila_bebidas_gerais);
    inicializar_fila(&fila_itens_em_preparo);
}
 
/**
 * @brief Imprime o status atual de todos os funcionários no console (Versão Corrigida).
 */
void imprimir_status_funcionarios(int tempo_atual) {
    wprintf(L"\n--- STATUS TEMPO %ds ---\n", tempo_atual);
    
    for (int i = 0; i < TOTAL_FUNCIONARIOS; i++) {
        // Imprime o ID do funcionário (formatado com 2 dígitos)
        wprintf(L"  Func %02d: ", funcionarios[i].id);

        if (funcionarios[i].ocupado_tarefa_exclusiva) {
            // Ocupado com tarefa de Caixa, Distribuição, Montagem ou Suco
            switch (funcionarios[i].tipo_tarefa_atual) {
                case CAIXA:
                    wprintf(L"CAIXA (Pedido %d) [resta: %ds]\n", 
                        funcionarios[i].pedido_tarefa_atual->id, 
                        funcionarios[i].tempo_tarefa_atual);
                    break;
                case SEPARA_PEDIDO: // TEXTO MELHORADO
                    wprintf(L"DISTRIBUINDO ITENS (Pedido %d) [resta: %ds]\n", 
                        funcionarios[i].pedido_tarefa_atual->id, 
                        funcionarios[i].tempo_tarefa_atual);
                    break;
                case MONTA_BANDEJA: // NOVO CASE QUE ESTAVA FALTANDO
                    wprintf(L"MONTANDO BANDEJA (Pedido %d) [resta: %ds]\n", 
                        funcionarios[i].pedido_tarefa_atual->id, 
                        funcionarios[i].tempo_tarefa_atual);
                    break;
                case FAZ_BEBIDA:
                    wprintf(L"FAZENDO SUCO (Pedido %d) [resta: %ds]\n", 
                        funcionarios[i].item_atual->id_pedido, 
                        funcionarios[i].tempo_tarefa_atual);
                    break;
                default:
                     // Este default agora só deve ser atingido se houver um bug real
                     wprintf(L"TAREFA EXCLUSIVA DESCONHECIDA\n");
            }
        } else if (funcionarios[i].ocupado_chapa > 0) {
            // Ocupado com tarefa de Chapa (Sanduíches)
            wprintf(L"CHAPA (%d/%d sanduíches)\n", 
                funcionarios[i].ocupado_chapa, 
                funcionarios[i].cap_chapa);
        } else if (funcionarios[i].ocupado_fritadeira > 0) {
            // Ocupado com tarefa de Fritadeira (Batatas)
            wprintf(L"FRITADEIRA (%d/%d batatas)\n", 
                funcionarios[i].ocupado_fritadeira, 
                funcionarios[i].cap_fritadeira);
        } else if (funcionarios[i].ocupado_liquidificador > 0) {
            // Ocupado com tarefa de Liquidificador (Milkshakes)
            wprintf(L"LIQUIDIFICADOR (%d/%d shakes)\n", 
                funcionarios[i].ocupado_liquidificador, 
                funcionarios[i].cap_liquidificador);
        } else {
            // Totalmente livre
            wprintf(L"LIVRE\n");
        }
    }
}


 /*
 *
 * PRINCIPAIS FUNCOES DA SIMULACAO
 * 
 * criar_itens_para_producao()
 * escalonador_de_tarefas()
 * processar_segundo()
 *
 */


 /**
 * @brief Cria os itens de um pedido e os enfileira nos postos de trabalho apropriados.
 * @param f O pedido a ser processado.
 * Se o pedido não tiver itens, ele é movido diretamente para a fila de separação.
 */
void criar_itens_para_producao(Pedido* pedido) {
    pedido->qtd_itens_total = pedido->sanduiches_s + pedido->sanduiches_m + pedido->batatas + pedido->sucos + pedido->milkshakes;
    pedido->itens_prontos = 0;
    if (pedido->qtd_itens_total == 0) {
        pedido->status = AGUARDANDO_MONTAGEM; enfileirar(&fila_montagem, pedido); return;
    }
    for(int i=0; i < pedido->sanduiches_s; i++) {
        Item* item = (Item*)malloc(sizeof(Item));
        *item = (Item){pedido->id, "Sanduiche simples", FAZ_SANDUICHE, TEMPO_SAND_SIMPLES, 0};
        enfileirar(&fila_chapa, item);
    }
    for(int i=0; i < pedido->sanduiches_m; i++) {
       Item* item = (Item*)malloc(sizeof(Item));
       *item = (Item){pedido->id, "Sanduiche medio", FAZ_SANDUICHE, TEMPO_SAND_MEDIO, 0};
       enfileirar(&fila_chapa, item);
    }
    for(int i=0; i < pedido->batatas; i++) {
        Item* item = (Item*)malloc(sizeof(Item));
        *item = (Item){pedido->id, "Batata", FAZ_BATATA, TEMPO_BATATA, 0};
        enfileirar(&fila_fritadeira, item);
    }
    for(int i=0; i < pedido->sucos; i++) {
        Item* item = (Item*)malloc(sizeof(Item));
        *item = (Item){pedido->id, "Suco", FAZ_BEBIDA, TEMPO_SUCO, 0};
        enfileirar(&fila_bebidas_gerais, item);
    }
    for(int i=0; i < pedido->milkshakes; i++) {
        Item* item = (Item*)malloc(sizeof(Item));
        *item = (Item){pedido->id, "Milk Shake", FAZ_MILKSHAKE, TEMPO_MILKSHAKE, 0};
        enfileirar(&fila_liquidificador, item);
    }
}

/**
 * @brief Escalonador de tarefas que aloca tarefas para funcionários disponíveis.
 * Prioriza tarefas exclusivas (Caixa, Separação, Suco) e depois tenta encher
 * as capacidades de tarefas em lote (Sanduíches, Batatas, Milkshakes).
 * Implementa a regra de "trava" de tipo para tarefas em lote.
 */
void escalonador_de_tarefas() {
    for (int i = 0; i < TOTAL_FUNCIONARIOS; i++) {
        
        if (funcionarios[i].ocupado_tarefa_exclusiva) {
            continue;
        }

        // PRIORIDADE 1: Atender no Caixa
        if (funcionarios[i].habilidades[CAIXA] && !fila_vazia(&fila_caixa)) {
            Pedido* p = (Pedido*)desenfileirar(&fila_caixa);
            p->status = ATENDIMENTO_CAIXA; 
            funcionarios[i].ocupado_tarefa_exclusiva = 1;
            funcionarios[i].pedido_tarefa_atual = p; 
            funcionarios[i].tipo_tarefa_atual = CAIXA;
            funcionarios[i].tempo_tarefa_atual = TEMPO_ATENDIMENTO_CAIXA;
            continue;
        }
        
        // NOVO - PRIORIDADE 2: Separar/Distribuir Itens para produção
        if (funcionarios[i].habilidades[SEPARA_PEDIDO] && !fila_vazia(&fila_distribuicao)) {
            Pedido* p = (Pedido*)desenfileirar(&fila_distribuicao);
            p->status = EM_PRODUCAO; // O pedido entra em produção assim que seus itens são separados
            funcionarios[i].ocupado_tarefa_exclusiva = 1;
            funcionarios[i].pedido_tarefa_atual = p;
            funcionarios[i].tipo_tarefa_atual = SEPARA_PEDIDO; // Nova tarefa
            funcionarios[i].tempo_tarefa_atual = TEMPO_DISTRIBUICAO_ITENS;
            continue;
        }

        // PRIORIDADE 3: Montar Bandeja Final 
        if (funcionarios[i].habilidades[MONTA_BANDEJA] && !fila_vazia(&fila_montagem)) {
            Pedido* p = (Pedido*)desenfileirar(&fila_montagem);
            if (p->status == AGUARDANDO_MONTAGEM) {
                p->status = EM_MONTAGEM; 
                funcionarios[i].ocupado_tarefa_exclusiva = 1;
                funcionarios[i].pedido_tarefa_atual = p;
                funcionarios[i].tipo_tarefa_atual = MONTA_BANDEJA; // Tarefa específica
                funcionarios[i].tempo_tarefa_atual = TEMPO_MONTAGEM_BANDEJA;
                continue;
            } else {
                enfileirar(&fila_montagem, p);
            }
        }

        // PRIORIDADE 4 (Parte 1): Fazer Bebida/Suco (Exclusivo)
        if (funcionarios[i].habilidades[FAZ_BEBIDA] && !fila_vazia(&fila_bebidas_gerais)) {
            Item* item = (Item*)desenfileirar(&fila_bebidas_gerais);
            funcionarios[i].ocupado_tarefa_exclusiva = 1;
            funcionarios[i].item_atual = item;
            funcionarios[i].tipo_tarefa_atual = FAZ_BEBIDA;
            funcionarios[i].tempo_tarefa_atual = item->tempo_restante;
            continue;
        }

        // PRIORIDADE 4 (Parte 2): Tarefas em Lote
        // Verificar se o funiconário já possui uma tarefa em lote em andamento, evitando que comece outra
        if (funcionarios[i].ocupado_chapa > 0) {
            while (funcionarios[i].habilidades[FAZ_SANDUICHE] && !fila_vazia(&fila_chapa) && funcionarios[i].ocupado_chapa < funcionarios[i].cap_chapa) {
                Item* item = (Item*)desenfileirar(&fila_chapa);
                item->id_funcionario = funcionarios[i].id;
                enfileirar(&fila_itens_em_preparo, item);
                funcionarios[i].ocupado_chapa++;
            }
        } else if (funcionarios[i].ocupado_fritadeira > 0) {
            while (funcionarios[i].habilidades[FAZ_BATATA] && !fila_vazia(&fila_fritadeira) && funcionarios[i].ocupado_fritadeira < funcionarios[i].cap_fritadeira) {
                Item* item = (Item*)desenfileirar(&fila_fritadeira);
                item->id_funcionario = funcionarios[i].id;
                enfileirar(&fila_itens_em_preparo, item);
                funcionarios[i].ocupado_fritadeira++;
            }
        } else if (funcionarios[i].ocupado_liquidificador > 0) {
             while (funcionarios[i].habilidades[FAZ_MILKSHAKE] && !fila_vazia(&fila_liquidificador) && funcionarios[i].ocupado_liquidificador < funcionarios[i].cap_liquidificador) {
                Item* item = (Item*)desenfileirar(&fila_liquidificador);
                item->id_funcionario = funcionarios[i].id;
                enfileirar(&fila_itens_em_preparo, item);
                funcionarios[i].ocupado_liquidificador++;
            }
        } else {
            if (funcionarios[i].habilidades[FAZ_SANDUICHE] && !fila_vazia(&fila_chapa)) {
                while (funcionarios[i].habilidades[FAZ_SANDUICHE] && !fila_vazia(&fila_chapa) && funcionarios[i].ocupado_chapa < funcionarios[i].cap_chapa) {
                    Item* item = (Item*)desenfileirar(&fila_chapa);
                    item->id_funcionario = funcionarios[i].id;
                    enfileirar(&fila_itens_em_preparo, item);
                    funcionarios[i].ocupado_chapa++;
                }
            } else if (funcionarios[i].habilidades[FAZ_BATATA] && !fila_vazia(&fila_fritadeira)) {
                while (funcionarios[i].habilidades[FAZ_BATATA] && !fila_vazia(&fila_fritadeira) && funcionarios[i].ocupado_fritadeira < funcionarios[i].cap_fritadeira) {
                    Item* item = (Item*)desenfileirar(&fila_fritadeira);
                    item->id_funcionario = funcionarios[i].id;
                    enfileirar(&fila_itens_em_preparo, item);
                    funcionarios[i].ocupado_fritadeira++;
                }
            } else if (funcionarios[i].habilidades[FAZ_MILKSHAKE] && !fila_vazia(&fila_liquidificador)) {
                 while (funcionarios[i].habilidades[FAZ_MILKSHAKE] && !fila_vazia(&fila_liquidificador) && funcionarios[i].ocupado_liquidificador < funcionarios[i].cap_liquidificador) {
                    Item* item = (Item*)desenfileirar(&fila_liquidificador);
                    item->id_funcionario = funcionarios[i].id;
                    enfileirar(&fila_itens_em_preparo, item);
                    funcionarios[i].ocupado_liquidificador++;
                }
            }
        }
     }
 }

  /**
  * @brief Processa 1 segundo de simulação.
  */
 void processar_segundo(Pedido pedidos[], int total_pedidos, int tempo_atual) {
    for (int i = 0; i < TOTAL_FUNCIONARIOS; i++) {
        if (funcionarios[i].ocupado_tarefa_exclusiva) {
            funcionarios[i].tempo_tarefa_atual--;
            
            if (funcionarios[i].tempo_tarefa_atual <= 0) {
                // Tarefa de CAIXA concluída -> Envia para fila de DISTRIBUIÇÃO
                if (funcionarios[i].tipo_tarefa_atual == CAIXA) {
                    funcionarios[i].pedido_tarefa_atual->status = AGUARDANDO_DISTRIBUICAO;
                    enfileirar(&fila_distribuicao, funcionarios[i].pedido_tarefa_atual);
                    funcionarios[i].ocupado_tarefa_exclusiva = 0; 
                    funcionarios[i].pedido_tarefa_atual = NULL;
                }
                // NOVO: Tarefa de SEPARA_PEDIDO concluída -> Cria os itens e os enfileira para produção
                else if (funcionarios[i].tipo_tarefa_atual == SEPARA_PEDIDO) {
                    criar_itens_para_producao(funcionarios[i].pedido_tarefa_atual);
                    funcionarios[i].ocupado_tarefa_exclusiva = 0;
                    funcionarios[i].pedido_tarefa_atual = NULL;
                }
                // Tarefa de MONTAGEM DE BANDEJA concluída -> Pedido finalizado
                else if (funcionarios[i].tipo_tarefa_atual == MONTA_BANDEJA) {
                    funcionarios[i].pedido_tarefa_atual->status = CONCLUIDO;
                    funcionarios[i].pedido_tarefa_atual->tempo_final = tempo_atual;
                    funcionarios[i].ocupado_tarefa_exclusiva = 0; 
                    funcionarios[i].pedido_tarefa_atual = NULL;
                }
                // Tarefa de BEBIDA (Suco) concluída
                else if (funcionarios[i].tipo_tarefa_atual == FAZ_BEBIDA) {
                    for (int j = 0; j < total_pedidos; j++) {
                        if (pedidos[j].id == funcionarios[i].item_atual->id_pedido) {
                            pedidos[j].itens_prontos++;
                            if (pedidos[j].itens_prontos >= pedidos[j].qtd_itens_total) {
                                pedidos[j].status = AGUARDANDO_MONTAGEM;
                                enfileirar(&fila_montagem, &pedidos[j]);
                            }
                            break;
                        }
                    }
                    free(funcionarios[i].item_atual);
                    funcionarios[i].ocupado_tarefa_exclusiva = 0; 
                    funcionarios[i].item_atual = NULL;
                }
            }
        }
    }

    Fila nova_fila_preparo;
    inicializar_fila(&nova_fila_preparo);

    while (!fila_vazia(&fila_itens_em_preparo)) {
        Item* item = (Item*)desenfileirar(&fila_itens_em_preparo);
        item->tempo_restante--;

        if (item->tempo_restante <= 0) {
            for(int i=0; i < TOTAL_FUNCIONARIOS; i++) {
                if(funcionarios[i].id == item->id_funcionario) {
                    if (item->tipo_preparo == FAZ_SANDUICHE) funcionarios[i].ocupado_chapa--;
                    else if (item->tipo_preparo == FAZ_BATATA) funcionarios[i].ocupado_fritadeira--;
                    else if (item->tipo_preparo == FAZ_MILKSHAKE) funcionarios[i].ocupado_liquidificador--;
                    break;
                }
            }
            for (int j = 0; j < total_pedidos; j++) {
                if (pedidos[j].id == item->id_pedido) {
                    pedidos[j].itens_prontos++;
                    if (pedidos[j].itens_prontos >= pedidos[j].qtd_itens_total) {
                        pedidos[j].status = AGUARDANDO_MONTAGEM;
                        enfileirar(&fila_montagem, &pedidos[j]);
                    }
                    break;
                }
            }
            free(item);
        } else {
            enfileirar(&nova_fila_preparo, item);
        }
    }
    fila_itens_em_preparo = nova_fila_preparo;
}
int gerar_pedidos_automaticamente(
    Pedido **out_lista,
    int tempo_abre,
    int tempo_fecha,
    int id_inicial,
    unsigned seed
) {
    srand(seed);

    int cap = 32;                        // capacidade inicial do vetor
    int n = 0;                           // quantidade gerada
    Pedido *v = (Pedido*)malloc(cap * sizeof(Pedido));
    if (!v) { fprintf(stderr, "Falha de memoria.\n"); exit(1); }

    int t = tempo_abre;
    int id = id_inicial;

    while (t <= tempo_fecha) {
        if (n == cap) {
            cap *= 2;
            v = (Pedido*)realloc(v, cap * sizeof(Pedido));
            if (!v) { fprintf(stderr, "Falha de memoria.\n"); exit(1); }
        }
        // ---- NOVO MODELO: máx 2 comidas e máx 2 bebidas ----
        int s_s = 0, s_m = 0, bat = 0, suc = 0, milk = 0;

        // total de COMIDAS: 1..2  (garante ao menos 1 item no pedido)
        int total_comidas = 1 + (rand() % 2);

        // total de BEBIDAS: 0..2
        int total_bebidas = rand() % 3;

        // distribui COMIDAS entre: sanduíche simples, sanduíche médio, batata
        for (int k = 0; k < total_comidas; k++) {
            int r = rand() % 3; // 0=s_s, 1=s_m, 2=bat
            if (r == 0) s_s++;
            else if (r == 1) s_m++;
            else bat++;
        }

        // distribui BEBIDAS entre: suco, milkshake
        for (int k = 0; k < total_bebidas; k++) {
            if (rand() % 2 == 0) suc++;
            else milk++;
        }

        // (Opcional) se quiser evitar sanduíche médio sem simples, remodele acima.
        // Aqui, por regra, o pedido tem no máx 4 itens: total_comidas(≤2) + total_bebidas(≤2).

        v[n] = (Pedido){
            .id = id++,
            .qtd_itens_total = 0,
            .itens_prontos = 0,
            .status = NAO_CHEGOU,
            .tempo_chegada = t,
            .tempo_final = 0,
            .sanduiches_s = s_s,
            .sanduiches_m = s_m,
            .batatas = bat,
            .sucos = suc,
            .milkshakes = milk
        };
        n++;

        // Próxima chegada:
        //   Avanço aleatório de 1..INTERVALO_CHEGADA_PEDIDOS segundos.
        //   (Troque por 'INTERVALO_CHEGADA_PEDIDOS' fixo se quiser determinístico)
        int intervalo_min = 10;
        int intervalo_max = INTERVALO_CHEGADA_PEDIDOS;
        if (intervalo_max < intervalo_min) intervalo_max = intervalo_min; // segurança
        t += intervalo_min + (rand() % (intervalo_max - intervalo_min + 1));

    }

    *out_lista = v;
    return n;
}