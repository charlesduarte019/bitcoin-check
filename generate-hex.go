package main

import (
	"bufio"
	"fmt"
	"math/big"
	"os"
	"strings"
)

func main() {
	if len(os.Args) < 3 {
		fmt.Println("Uso: go run main.go <inicio_hex> <fim_hex>")
		fmt.Println("Exemplo: go run main.go 1000000000000000000 10000000000000000FF")
		os.Exit(1)
	}

	inicioStr := strings.TrimSpace(os.Args[1])
	fimStr := strings.TrimSpace(os.Args[2])

	// Instancia os números de precisão arbitrária
	inicioVal := new(big.Int)
	fimVal := new(big.Int)

	// Converte da base 16 (hexadecimal)
	_, okInicio := inicioVal.SetString(inicioStr, 16)
	_, okFim := fimVal.SetString(fimStr, 16)

	if !okInicio || !okFim {
		fmt.Println("Erro: Certifique-se de que ambos os argumentos sejam hexadecimais válidos.")
		os.Exit(1)
	}

	// Compara se o início é maior que o fim (retorna > 0 se inicioVal > fimVal)
	if inicioVal.Cmp(fimVal) > 0 {
		fmt.Println("Erro: O valor inicial não pode ser maior que o valor final.")
		os.Exit(1)
	}

	// Preserva a quantidade de dígitos da entrada para o padding
	tamanhoDigitos := len(inicioStr)

	writer := bufio.NewWriter(os.Stdout)
	defer writer.Flush()

	// Formato dinâmico com padding (ex: %019X)
	formato := fmt.Sprintf("%%0%dX\n", tamanhoDigitos)

	// Loop usando math/big
	um := big.NewInt(1)
	atual := new(big.Int).Set(inicioVal)

	for atual.Cmp(fimVal) <= 0 {
		fmt.Fprintf(writer, formato, atual)
		atual.Add(atual, um) // atual++
	}
}
