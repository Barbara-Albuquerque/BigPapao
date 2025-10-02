/******************************************************************************
 *
 * Lib com as funções necessárias para o cálculo da capacidade dos postos de trabalho
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

wchar_t *nome_completo = L"Andrey Noewertton Ferreira";   // Variavel global com o nome da simulacao


// ----PSEUDO DICIONARIO PARA O ALFABETO COMPLETO --------
typedef struct { wchar_t chave[4]; int valor; } chave_valor;
chave_valor tabela[] = {
    {L"q",1},{L"w",6},{L"e",7},{L"r",6},{L"t",5},{L"y",2},{L"u",3},{L"i",8},{L"o",9},{L"p",4},
    {L"á",3},{L"ã",4},{L"a",2},{L"s",5},{L"d",8},{L"f",7},{L"g",4},{L"h",1},{L"j",4},{L"k",7},
    {L"l",8},{L"ç",5},{L"é",2},{L"í",3},{L"z",3},{L"x",4},{L"c",9},{L"v",8},{L"b",3},{L"n",2},
    {L"m",5},{L"ó",6},{L"õ",7},{L"ô",6},{L"â",1},{L"ê",2}
};


int calcular_pesos(wchar_t nome[][50], int index) {
    int total = 0;
    for (int i = 0; nome[index][i] != L'\0'; i++) {
        wchar_t letra[2] = {towlower(nome[index][i]), L'\0'};
        for (int j = 0; j < sizeof(tabela)/sizeof(tabela[0]); j++) {
            if (wcscmp(tabela[j].chave, letra) == 0) { total += tabela[j].valor; break; }
        }
    }
    return total;
}

 /**
  * @brief Define a capacidade *base* que cada funcionário terá.
  */
 void definir_capacidades(int p1, int p2, int p3, int* cap_chapa, int* cap_fritadeira, int* cap_milkshake) {
    // Esta é a capacidade POR FUNCIONÁRIO
    switch (p1 % 3) { case 0: *cap_chapa=1; break; case 1: *cap_chapa=2; break; case 2: *cap_chapa=3; break; }
    switch (p2 % 3) { case 0: *cap_fritadeira=6; break; case 1: *cap_fritadeira=4; break; case 2: *cap_fritadeira=2; break; } // Ex: 4 batatas por funcionário
    switch (p3 % 3) { case 0: *cap_milkshake=2; break; case 1: *cap_milkshake=6; break; case 2: *cap_milkshake=4; break; }
}


void setar_capacidade(int *cap_chapa, int *cap_fritadeira, int *cap_milkshake){

    setlocale(LC_ALL, "");
    //wchar_t nome_completo[] = L"Andrey Noewertton Ferreira";
    wchar_t nomes[3][50] = {{0}};
    int index_nome=0, index_caract=0;
    for(int i=0; nome_completo[i] != L'\0' && index_nome < 3; i++){
        if(nome_completo[i] == L' '){
            nomes[index_nome][index_caract]='\0'; index_nome++; index_caract=0;
        } else { nomes[index_nome][index_caract++] = nome_completo[i]; }
    }
    nomes[index_nome][index_caract]='\0';
    int p1 = calcular_pesos(nomes, 0); int p2 = calcular_pesos(nomes, 1); int p3 = calcular_pesos(nomes, 2);
    
    definir_capacidades(p1, p2, p3, cap_chapa, cap_fritadeira, cap_milkshake);

}