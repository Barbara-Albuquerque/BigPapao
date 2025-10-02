import pandas as pd
import matplotlib.pyplot as plt

# Nome do arquivo CSV gerado pelo seu programa em C
nome_arquivo_csv = 'log_filas2.csv' # Lembre-se de ajustar se o seu for 'log_filas2.csv'

# Tenta carregar os dados do arquivo
try:
    df = pd.read_csv(nome_arquivo_csv)
except FileNotFoundError:
    print(f"ERRO: Arquivo '{nome_arquivo_csv}' não encontrado.")
    print("Por favor, execute o programa em C primeiro para gerar o arquivo de dados.")
    exit()

# --- Separa as filas em grupos para plots diferentes ---
filas_de_producao = ['EmPreparo', 'Chapa', 'Fritadeira', 'Liquidificador']
filas_de_transicao = ['Caixa', 'Distribuicao', 'Montagem', 'BebidasGerais']

# Garante que as colunas existem no DataFrame para evitar erros
filas_de_producao = [f for f in filas_de_producao if f in df.columns]
filas_de_transicao = [f for f in filas_de_transicao if f in df.columns]


# --- Criação do Gráfico Dividido (Subplots) ---

# Cria uma figura com 2 linhas e 1 coluna de gráficos. Eles compartilharão o mesmo eixo X (Tempo).
# Aumentamos o figsize vertical para acomodar os dois gráficos.
fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(15, 12), sharex=True)

# Define um título geral para a janela inteira
fig.suptitle('Análise das Filas da Cozinha do BigPapão', fontsize=20, fontweight='bold')


# --- Gráfico 1: Filas de Produção (Alto Volume) ---
ax1 = axes[0]
for coluna in filas_de_producao:
    ax1.plot(df['Tempo'], df[coluna], label=coluna, linewidth=2.5) # Linhas mais grossas

ax1.set_title('Filas de Produção Contínua', fontsize=16)
ax1.set_ylabel('Número de Itens em Produção', fontsize=12)
ax1.legend(title='Filas de Produção', fontsize=10)
ax1.grid(True)


# --- Gráfico 2: Filas de Transição (Baixo Volume) ---
ax2 = axes[1]
for coluna in filas_de_transicao:
    # Adicionamos MARCADORES para destacar os picos de 1 ou 2 itens
    ax2.plot(df['Tempo'], df[coluna], label=coluna, linestyle='--', marker='o', markersize=4)

ax2.set_title('Filas de Transição (Entrada/Saída)', fontsize=16)
ax2.set_ylabel('Número de Itens na Fila', fontsize=12)
ax2.set_xlabel('Tempo (segundos)', fontsize=14)
ax2.legend(title='Filas de Transição', fontsize=10)
ax2.grid(True)
# Ajusta o eixo Y para focar nos números pequenos
ax2.set_ylim(bottom=-0.5, top=max(5, df[filas_de_transicao].max().max() + 1))


# --- Finalização e Salvamento ---

# Ajusta o layout para evitar sobreposição
plt.tight_layout(rect=[0, 0, 1, 0.96]) # Deixa espaço para o supertítulo

# Salva o gráfico em um arquivo PNG de alta qualidade
nome_arquivo_saida = 'grafico_filas.png'
plt.savefig(nome_arquivo_saida, dpi=300) # dpi=300 para alta resolução

print(f"Gráfico salvo como '{nome_arquivo_saida}'")
print("Mostrando o gráfico...")
plt.show()
print("Janela do gráfico fechada.")