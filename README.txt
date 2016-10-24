SCC0621 - Multimídia e Hipermídia
Instituto de Ciências Matemáticas e Computação
Universidade de São Paulo
23/10/2016

Projeto 1 - Compressão de Áudio Digital

DUPLA:
Adriano Belfort de Sousa - No. USP 7960706
Henrique de Almeida Machado da Silveira - No. USP 7961089

*** OBJETIVO ***

Este projeto teve por objetivo desenvolver uma ferramenta de compressão e descompressão
de áudio digital, utilizando as técnicas de codificação por diferença, codificação por
carreira (runlength) e codificação Huffman. Dado um arquivo de entrada no formato WAV,
o programa "encode" é responsável pela compressão do aqruivo original. O programa 
"decode" realiza a descompressão, obtendo um outro arquivo WAV muito próximo ao original
(algumas pequenas perdas podem acontecer, por exemplo quando o arquivo original possui
tamanho que o tamanho do header WAV mais o tamanho dos dados, aramazenado no campo 
Subchunk2Size do header WAV).

*** COMPILAÇÃO ***

Para compilar os programas "encode" e "decode", pode-se utilizar o arquivo Makefile
disponível no diretório raiz em que se encontram os outros arquivos .c e .h necessários
para o projeto. Assim, basta fazer:

$ make

E serão compilados os programas "encode" e "decode".

*** EXECUÇÃO ***

Após a compilação, dois executáveis representando os programas supracitados serão gerados:
encode e decode.

Para realizar a compressão, utilize

$ ./encode [-d] [-c] [-h] [-v] <entrada.wav> <codificado.bin>

<entrada.wav> representa o arquivo de entrada a ser lido e <codificado.bin> representa o
arquivo para escrita após compressão.

As flags em [ ] são opcionais. Tais flags significam:

-d : realizar compressão utilizando codificação por diferenças
-c : realizar compressão utilziando codificação por carreira (runlength)
-h : realizar compressão utilizando codificaçnao Huffman
-v : ativar modo "verbose", exibindo mensagens mais detalhadas

As flags podem ser informadas em qualquer ordem. Se nenhuma for informada, nenhuma compressão
é realizada e uma mensagem indicando essa situação é exibida.

Para a opção de ajuda, execute

$ ./encode --help

Para realizar a descompressão, execute

$ ./decode [-v] <entrada.bin> <saida.wav>

<entrada.bin> representa o arquivo de entrada a ser lido e <saida.wav> representa o
arquivo para escrita após descompressão.

A flag -v é opcional e ativa o modo "verbose", exibindo mensagens mais detalhadas.

Para a opção de ajuda, execute

$ ./decode --help

*** ORDEM DE REALIZAÇÃO DE COMPRESSÃO E PERFORMANCE ***

Para a realização de uma combinação de codificações, definimos a seguinte
ordem em "encode":

1) Codificação por diferenças
2) Codificação por carreira (runlength)
3) Codificação Por Huffman

A descompressão acontece em ordem inversa.

Essa ordem foi escolhida porque imaginamos que devido à "localidade" das informações em
um arquivo de áudio, amostras vizinhas seriam semelhantes e assim a codificação por diferenças
seria uma boa escolha para ser realizada primeiro. Logo após isso, como fizemos a suposição
de que essas diferenças podem se repetir sucessivamente, a codificação por carreira 
(runlength) se tornou uma opção natural. Finalmente, a codificação por Huffman é executada
realizando a contagem de frequencias dos dados que chegam até ela.

A performance isolada dessas diferentes técnicas de compressão, para arquivos em geral, como
os sample1.wav e sample2.wav fornecidos, é a seguinte:

- Codificação por diferenças: a compressão é muito pequena, não havendo melhora significativa
de tamanho do arquivo
- Codificação por carreira (runlength): na maioria dos casos, não há compressão, isto é, o tamanho
do arquivo aumenta
- Codificação Huffman: obteve o melhor desempenho, reduzindo o tamanho dos arquivos originais entre
10% e 20% na média

