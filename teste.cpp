#include <stdio.h>
#include <string.h>

// Definindo a struct carro
struct carro {
    char marca[15];
    int ano;
    char cor[10];
    float preco;
};

// Fun��o para imprimir carros abaixo de um pre�o espec�fico
void imprimeCarrosAbaixoPreco(struct carro vetcarros[], int tamanho, float preco) {
    for (int i = 0; i < tamanho; i++) {
        if (vetcarros[i].preco <= preco) {
            printf("Marca: %s, Ano: %d, Cor: %s, Pre�o: %.2f\n", vetcarros[i].marca, vetcarros[i].ano, vetcarros[i].cor, vetcarros[i].preco);
        }
    }
}

int main() {
    struct carro vetcarros[3] = {
        {"Toyota", 2020, "Azul", 30000},
        {"Ford", 2018, "Preto", 25000},
        {"Honda", 2019, "Branco", 27000}
    };

    float precoMaximo;
    printf("Digite o pre�o m�ximo: ");
    scanf("%f", &precoMaximo);

    imprimeCarrosAbaixoPreco(vetcarros, 3, precoMaximo);

    return 0;
}
