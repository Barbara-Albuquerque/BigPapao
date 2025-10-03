/******************************************************************************
 *
 * Simulador da Cozinha do BigPapão
 *
 * Desenvolvido com a assistência da IA Gemini
 * Data: 28 de setembro de 2025
 *
 * Descrição:
 * Main principal com todas as chamadas de função
 *****************************************************************************/


#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>


#include "definir_capacidade.c"
#include "cozinha.c"

FILE *log_filas;


int main(){

    setlocale(LC_ALL, "");

     // Capacidades *base* (por funcionário)
     int cap_chapa, cap_fritadeira, cap_milkshake;

     setar_capacidade(&cap_chapa, &cap_fritadeira, &cap_milkshake);
     
     wprintf(L"Usando nome: %ls\n\n--- Capacidades por Funcionário ---\n", nome_completo);
     wprintf(L"Chapa: %d | Fritadeira: %d | Liquidificador: %d\n\n", cap_chapa, cap_fritadeira, cap_milkshake);
     
     inicializar_funcionarios(cap_chapa, cap_fritadeira, cap_milkshake);
     inicializar_todas_filas();

     // --- Geração automática de pedidos ---
        Pedido *lista_pedidos = NULL;

        // Exemplo: loja aberta de 0 a 600 segundos (10 minutos).
        // Ajuste a janela conforme a sua simulação.
        int tempo_abre = 0;
        int tempo_fecha = 1000;

        // Gera automaticamente dentro da janela [tempo_abre, tempo_fecha]
        int total_de_pedidos = gerar_pedidos_automaticamente(
            &lista_pedidos,
            tempo_abre,
            tempo_fecha,
            1,                           // id inicial
            (unsigned)time(NULL)         // semente aleatória
        );

     wprintf(L"--- Iniciando Simulação com Fila de Caixa ---\n");
     
     // CSV com o estado de cada fila a cada segundo
     log_filas = fopen("log_filas2.csv", "w");

     if (log_filas == NULL) {
         wprintf(L"ERRO: Não foi possível criar o arquivo de log.\n");
         return 1;
     }
        // Cabeçalho do CSV
     fprintf(log_filas, "Tempo,Caixa,Distribuicao,Montagem,Chapa,Fritadeira,Liquidificador,BebidasGerais,EmPreparo\n"); // ALTERADO: Novas filas

     int tempo_simulacao = 0, concluidos = 0;
     while (concluidos < total_de_pedidos) {
         for(int i = 0; i < total_de_pedidos; i++) {
             if(lista_pedidos[i].status == NAO_CHEGOU && tempo_simulacao >= lista_pedidos[i].tempo_chegada) {
                 lista_pedidos[i].status = AGUARDANDO_CAIXA;
                 enfileirar(&fila_caixa, &lista_pedidos[i]);
                 wprintf(L"Tempo %ds: Pedido %d chegou e entrou na fila do caixa.\n", tempo_simulacao, lista_pedidos[i].id);
             }
         }
         escalonador_de_tarefas();
         processar_segundo(lista_pedidos, total_de_pedidos, tempo_simulacao + 1);
        


         // Imprime o status de cada funcionário, o que estão fazendo, a cada 20 segundos
        if ((tempo_simulacao + 1) % 20 == 0) {
            imprimir_status_funcionarios(tempo_simulacao + 1);
        }


        // Registra o estado atual de cada fila no CSV
        fprintf(log_filas, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
            tempo_simulacao + 1,
            tamanho_fila(&fila_caixa),
            tamanho_fila(&fila_distribuicao),
            tamanho_fila(&fila_montagem),
            tamanho_fila(&fila_chapa),
            tamanho_fila(&fila_fritadeira),
            tamanho_fila(&fila_liquidificador),
            tamanho_fila(&fila_bebidas_gerais),
            tamanho_fila(&fila_itens_em_preparo)
        );

         tempo_simulacao++;
         concluidos = 0;
         for (int i = 0; i < total_de_pedidos; i++) {
             if (lista_pedidos[i].status == CONCLUIDO) concluidos++;
         }
         
     }
     
     fclose(log_filas);

     wprintf(L"\n--- Fim da Simulação ---\n");
     wprintf(L"Tempo total para concluir todos os pedidos: %d segundos.\n\n", tempo_simulacao);
     for (int i = 0; i < total_de_pedidos; i++) {
         int tempo_final = lista_pedidos[i].tempo_final;
         int tempo_total_pedido = tempo_final - lista_pedidos[i].tempo_chegada;
         wprintf(L"Pedido %d: Chegou em %ds, Concluído em %ds (duração: %ds). ",
             lista_pedidos[i].id, lista_pedidos[i].tempo_chegada, tempo_final, tempo_total_pedido);
              // Descrição do conteúdo do pedido
    wprintf(L"   Itens: ");
    if (lista_pedidos[i].sanduiches_s > 0)
        wprintf(L"%d sanduíche(s) simples, ", lista_pedidos[i].sanduiches_s);
    if (lista_pedidos[i].sanduiches_m > 0)
        wprintf(L"%d sanduíche(s) médio(s), ", lista_pedidos[i].sanduiches_m);
    if (lista_pedidos[i].batatas > 0)
        wprintf(L"%d batata(s), ", lista_pedidos[i].batatas);
    if (lista_pedidos[i].sucos > 0)
        wprintf(L"%d suco(s), ", lista_pedidos[i].sucos);
    if (lista_pedidos[i].milkshakes > 0)
        wprintf(L"%d milkshake(s), ", lista_pedidos[i].milkshakes);
    if (lista_pedidos[i].refrigerantes > 0)
        wprintf(L"%d refrigerante(s), ", lista_pedidos[i].refrigerantes);

    wprintf(L"\n");
         if (tempo_total_pedido > TEMPO_MAXIMO_PEDIDO || tempo_final == 0) {
             wprintf(L"(ESTOUROU O PRAZO OU FALHOU!)\n");
         } else {
             wprintf(L"(Entregue a tempo.)\n");
         }
     }
     free(lista_pedidos);


     return 0;

}