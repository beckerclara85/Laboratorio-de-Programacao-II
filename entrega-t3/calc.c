#include "calc.h"
#include "dicionario.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// auxiliares

static bool é_espaço(unichar c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static bool é_digito_ou_ponto(unichar c) {
    return c == 
}