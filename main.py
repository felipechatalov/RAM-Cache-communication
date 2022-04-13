from distutils.log import error
from json import detect_encoding
from multiprocessing.sharedctypes import Value
from re import T
from secrets import token_bytes
import sys

_BITS = 16
_MAX_INT_SIZE = 2 ** (_BITS-1) - 1

# soma +
# subtracao -
# multi. *
# div. /


def decToBin(n: int):
    # separado em partes caso 1 linha seja confuso

    # coloca o sinal, 0 para positivo e 1 para negativo
    # p1 = ("0" if n > 0 else "1")
    
    # completa o numero com "0" ate o numero desejado de bits
    # p2 = "0" * (_BITS - (len(toBinary(n)))-1)
    
    # transforma um numero inteiro e decimal em binario
    # p3 = toBinary(n)
    
    # apos isso, junta tudo em um string de tamanaho _BITS
    return ("0" if n >= 0 else "1") + ("0" * (_BITS - (len(toBinary(n)))-1)) + toBinary(n)

# funcao recursiva para converter de decimal para binario
def bd_rec(n: str, i: int):
    
    # pega o bit da iteracao atual e ve se esta ativo ou nao
    # p1 = int(n[i])
    
    # se o bit estiver ativo, soma 2**(i-1) ao numero binario, (i-1 ja que o primeiro bit eh de sinal)
    # p2 = (2**(_BITS-i-1))
    
    # caso o sinal seja negativo, multiplica por -1
    # p3 = (-1 if n[0] == "1" else 1)

    # por fim soma este numero com a proxima chamada recursiva com i+=1
    t = int(n[i]) * (2**(len(n)-i-1)) * (-1 if n[0] == "1" else 1)
    # quando chegamos no bit 1, retorna o ultimo numero sem chamada recursiva pois o ultimo bit eh de sinal
    if (i == len(n)-1):
        return t
    return t + bd_rec(n, i+1)

# chamada da funcao recursiva acima, para que n seja necessario passar o 'i' como parametro
def binToDec(n):
    return bd_rec(n, 1)


# funcao para transformar um numero inteiro em binario
# ex: 37 -> 10101
def toBinary(n):
    n = abs(int(n))
    hold = ""
    while n > 0:
        # divide o numero por 2 e pega o resto para a string hold
        b = n//2
        hold += str(n%2)
        n = b
    # retorna a string hold invertida
    return hold[::-1]

# faz a soma de 2 numeros binarios, excluindo sinal
def sum(n1: str, n2: str, co: int, i: int) -> str:
    # t recebe a soma de n1 e n2 e o co(carry out da soma anterior)
    # para a soma eh usado 2 portas XOR
    # print(n1, n2)
    t = str(int(n1[i]) ^ int(n2[i]) ^ int(co))
    
    # 0 ^ 0 = 0
    # 0 ^ 1 = 1
    # 1 ^ 0 = 1
    # 1 ^ 1 = 0
    if i == 0:
        return t
    
    # o terceiro parametro (carry out) precisa ser 1 se 2 ou mais dos numeros forem 1, caso contrario v recebe 0    
    # 1 1 0 OR 1 0 1 OR 0 1 1 OR 1 1 1
   

    # v = 1 if int(n1[i]) == 1 and int(n2[i]) == 1 and co == 0 or int(n1[i]) == 1 and int(n2[i]) == 0 and co == 1 or int(n1[i]) == 0 and int(n2[i]) == 1 and co == 1 or int(n1[i]) == 1 and int(n2[i]) == 1 and co == 1 else 0
    # por fim retorna a soma de i-1 + a iteracao atual

    return sum(n1, n2, 1 if (int(n1[i]) + int(n2[i]) + int(co)) > 1 else 0, i-1) + t

def absGreater(n1: str, n2: str):
    i = 1
    # incrementa o i ate os bits serem diferentes
    while i < _BITS and n1[i] == n2[i] :
        i += 1
    # caso i chegue ao final quer dizer que o numero eh igual
    if i == _BITS:
        return False
    # caso o bit de n1 for 1 retorna true, pois ele eh maior, se nao, retorna false
    if n1[i] == "1":
        return True
    return False
    

def bitshift(n, b, dir):
    # retorna o numero n deslocado para a direita ou esquerda 'b' bits
    if dir == "right":
        return ("0"*b) + n[:-b]
    return n[b:] + ("0"*b)

def sumOperator(n1: str, n2: str):
    # operador de soma
    
    # caso o sinal seja diferente, coloque o maior em valor absoluto na frente
    # e fazemos a subtracao
    # por ser uma subtracao, nao ha overflow
    
    print(f'\t  {n1[0]} {n1[1:]}\n\t +\n\t  {n2[0]} {n2[1:]}')
    
    if (n1[0] != n2[0]):
        if absGreater(n1, n2):
            # 3 + -2 = pos
            # -3 + 2 = neg            
            return n1[0] + sub(n1[1:], n2[1:], 0, _BITS-2)
        else:
            # 2 + -3 = neg
            # -2 + 3 = pos            
            return n2[0] + sub(n2[1:], n1[1:], 0, _BITS-2)

    # caso contrario somamos os numeros
    # 2 + 3 = pos
    # 3 + 2 = pos
    # -2 + -3 = neg
    # -3 + -2 = neg
    t = sum("0"+n1[1:], "0"+n2[1:], 0, _BITS-1)

    if t[0] == "1":
        raise error("OVERFLOW")
    return n1[0] + t[1:]


def sub(n1, n2, co, i):
    # equacao de subtracao de numeros binarios

    # n1  n2 cin cout/r 
    # 0 - 0 - 0 = 0   0
    # 0 - 0 - 1 = 1   1    
    # 0 - 1 - 0 = 1   1       
    # 0 - 1 - 1 = 1   0     
    # 1 - 0 - 0 = 0   1    
    # 1 - 0 - 1 = 0   0
    # 1 - 1 - 0 = 0   0
    # 1 - 1 - 1 = 1   1  
    
    # r -> n1 ^ n2 ^ cin
    # cout -> n1 - (n2 + cin) < 0

    t = str((int(n1[i]) ^ int(n2[i])) ^ co)
    # print(f'n1: {n1[i]} n2: {n2[i]} co: {co} t: {t}')
    if i == 0:
        return t
    return sub(n1, n2, 1 if (int(n1[i]) - (int(n2[i]) + co)) < 0 else 0, i-1) + t

     

def subOperator(n1, n2):
    # operador de subtracao


    # n1 - n2 -> maior abs na frente e subtracao sem sinal, sinal positivo caso nao precise
    # trocar posicoes, se nao, negativo
    # n1 - -n2 -> sum n1 + n2 com sinal positivo sempre
    # -n1 - n2 -> sum n1 + n2 com sinal negativo sempre
    # -n1 - -n2 -> maior na frente e subtracao sem sinal, sinal negativo caso nao precise
    # trocar posicoes, se nao, positivo

    print(f'\t  {n1[0]} {n1[1:]}\n\t -\n\t  {n2[0]} {n2[1:]}')


    # como eh usado a soma, tem chance do resultado dar um overflow
    if n1[0] != n2[0]:
        t = sum("0"+n1[1:], "0"+n2[1:], 0, _BITS-1)
        if t[0] == "1":
            raise error("OVERFLOW")
        return n1[0] + t[1:]

    # sinais iguais
    # subtracao de 2 positivos, ou seja, nao ha overflow
    if absGreater(n1, n2):
        return n1[0] + sub(n1[1:], n2[1:], 0, _BITS-2)
    else:
        return ("0" if n2[0] == "1" else "1") + sub(n2[1:], n1[1:], 0, _BITS-2)


def mulOperator(n1, n2):
    if n1 == "0"*_BITS or n1 == "1" + ("0"*(_BITS-1)) or n2 == "0"*_BITS or n2 == "1" + ("0"*(_BITS-1)):
        return "0"*_BITS

    count = _BITS-1
    c = "0"
    a = "0"*15
    q = n1[1:]
    m = n2[1:]

    print(f'C        A               Q              M')
    print(f'{c}|{a}|{q} {m}')

    while count != 0:
        if q[-1] == "1":
           a = sum("0" + a, "0" + m, 0, len(a))
           c, a = a[0], a[1:]

        full = bitshift(c+a+q, 1, "right")
        c = full[0]
        a = full[1:16]
        q = full[16:]
        count -= 1
        print(f'C|       A       |       Q              M')
        print(f'{c}|{a}|{q} {m}')

    return ("0" if n1[0] == n2[0] else "1") + (a+q)

def divOperator(n1, n2):
    # D dividido por V, resultado em Q e resto em R
    # d = n1
    # v = n2

    if n2 == "0"*_BITS or n2 == "1" + ("0"*(_BITS-1)):
        raise ValueError(f"Divisao por zero")
    if n1 == "0"*_BITS or n1 == "1" + ("0"*(_BITS-1)):
        print(f'0 divided by {n2} = 0')
        return "0"*_BITS, "0"*_BITS

    d = "0"+n1[1:]
    v = "0"+n2[1:]

    print(f'dividendo: {d}, divisor: {v}')

    q = 0; r = d
    # caso d >= v subraimos v de d ate d < v, cada vez q subtraimos
    # adicionamos 1 ao 'q' e por fim o nosso resto fica em r
    while absGreater(d, v) or d == v:
        print(f'{d}({binToDec(d)}) >= {v}({binToDec(v)}), logo:')
        
        d = "0"+sub(d[1:], v[1:], 0, _BITS-2)
        q += 1
        
        print(f'resultado : {q} e resto : {binToDec(n1[0] + d[1:])} ')
        
        r = d
    return ("0" if n1[0] == n2[0] else "1") + decToBin(q)[1:], n1[0] + r[1:]


def handleInput():
    print("$ digite um numero em base decimal, seguido por uma operacao (+, -, *, /) e outro numero decimal.")
    print("$ ex: '2 * 3' ou digite 'quit' para sair")
    usr = input().split(" ")
    if usr[0] == "quit":
        return 0, "quit", 0
    if len(usr) != 3:
        raise ValueError("Invalid input, ex: 2 * 3")
    if usr[1] not in ["+", "-", "*", "/"]:
        raise ValueError("Invalid operator, ex: +, -, *, /")
    try:
        n1, n2 = int(usr[0]), int(usr[2])
    except:
        raise ValueError("Invalid input, values must be integers")
    if abs(n1) > _MAX_INT_SIZE or abs(n2) > _MAX_INT_SIZE:
        raise ValueError('n1 or n2 exceeded max int size for {_BITS} bits')

    return decToBin(n1), usr[1], decToBin(n2)

def main():
    
    
    n1, op, n2 = handleInput()
    print(n1, op, n2)
    
    while op != "quit":
        
        # print(f'numero 1: {binToDec(n1)}', end='')
        # print(f' \tbinario: {n1}')
        # print(f'numero 2: {binToDec(n2)}', end='')
        # print(f' \tbinario: {n2}')
        print()
        if op == "+":
            r = sumOperator(n1, n2)
            print('----------------------------------------------------')
            print(f'resultado: {r}, ({binToDec(r)})')
            print('----------------------------------------------------')
        elif op == "-":
            r = subOperator(n1, n2)
            print('----------------------------------------------------')
            print(f'resultado: {r}, ({binToDec(r)})')
            print('----------------------------------------------------')
        elif op == "*":
            r = mulOperator(n1, n2)
            print('----------------------------------------------------')
            print(f'resultado: {r}, ({binToDec(r)})')
            print('----------------------------------------------------')
        elif op == "/":
            r, re = divOperator(n1, n2)
            print('----------------------------------------------------')
            print(f'resultado: {r}, resto: {re}  ({binToDec(r), binToDec(re)})')
            print('----------------------------------------------------')

        n1, op, n2 = handleInput()    

 
def test(n1, n2, op):
    b1 = decToBin(n1)
    b2 = decToBin(n2)


    a, r = 0, 0
    if op == '+':
        a = binToDec(sumOperator(b1, b2))
        r = n1 + n2
    elif op == '-':
        a = binToDec(subOperator(b1, b2))
        r = n1 - n2
    elif op == '*':
        a = binToDec(mulOperator(b1, b2))
        r = n1 * n2
    elif op == '/':
        if n2 == 0:
            print(f'divisao por 0, nao entrou')
            return
        a, b = divOperator(b1, b2)
        # print(a)
        a = binToDec(a)
        r = int(n1 / n2)
    print(f'\t{n1} {op} {n2} = {a}, should be {r}    \t{a == r}')
    if op == '/':
        print(f'\treminder = {binToDec(b)} \t\t')
    return






if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == "-x":
        print("test mode")
        # n1 = 2413 # "0000100101101101"
        # n2 = 5549 # "0001010110101101" 
        n1 = 2
        n2 = 3
        ops = ["+", "-", "*", "/"]
        print(n1, n2)
        for j in range(4):
            print(ops[j])

            print('-------------------')
            for i in range(4):
                test(n1*(1 if i < 2 else -1), n2*(1 if i%2 == 0 else -1), ops[j])
            for i in range(4):
                test(n2*(1 if i < 2 else -1), n1*(1 if i%2 == 0 else -1), ops[j])
            
            #-n1 0
            #n1 0
            #0 -n2
            #0 n2
            print("with 0's")
            for i in range(4):
                test(n1*(1 if i < 2 else 0)*(-1 if i%2 == 0 else 1), n2*(1 if i > 1 else 0)*(-1 if i%2 == 0 else 1), ops[j])
            print('-------------------')
    else:
        print('user mode')
        main()

