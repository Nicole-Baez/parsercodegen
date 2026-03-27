/*
Assignment:
HW3 - Parser and Code Generator for PL/0
Author(s): Nicole Baez Espinosa, Lianet Martin
Language: C (only)
To Compile:
gcc -O2 -Wall -std=c11 -o parsercodegen parsercodegen.c
To Execute (on Eustis):
./parsercodegen <input_file.txt>
where:
<input_file.txt> is the path to the PL/0 source program
Notes:
- Single integrated program: scanner + parser + code gen
- parsercodegen.c accepts ONE command-line argument
- Scanner runs internally (no intermediate token file)
- Implements recursive-descent parser for PL/0 grammar
- Generates PM/0 assembly code (see Appendix A for ISA)
- All development and testing performed on Eustis
Class: COP 3402 - Systems Software - Spring 2026
Instructor: Dr. Jie Lin
Due Date: See Webcourses for the posted due date and time.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define norw 15                   // num of reserved words
#define idenmax 11                // identifier length
#define strmax 256                // str max length
#define MAX_SYMBOL_TABLE_SIZE 500 // table size

// Structure for enumeration
typedef enum
{
    skipsym = 1,       // Skip / ignore token
    identsym = 2,      // Identifier
    numbersym = 3,     // Number
    beginsym = 4,      // begin
    endsym = 5,        // end
    ifsym = 6,         // if
    fisym = 7,         // fi
    thensym = 8,       // then
    whilesym = 9,      // while
    dosym = 10,        // do
    odsym = 11,        // od
    callsym = 12,      // call
    constsym = 13,     // const
    varsym = 14,       // var
    procsym = 15,      // procedure
    writesym = 16,     // write
    readsym = 17,      // read
    elsesym = 18,      // else
    plussym = 19,      // +
    minussym = 20,     // -
    multsym = 21,      // *
    slashsym = 22,     // /
    eqsym = 23,        // =
    neqsym = 24,       // <>
    lessym = 25,       // <
    leqsym = 26,       // <=
    gtrsym = 27,       // >
    geqsym = 28,       // >=
    lparentsym = 29,   // (
    rparentsym = 30,   // )
    commasym = 31,     // ,
    semicolonsym = 32, // ;
    periodsym = 33,    // .
    becomessym = 34,   // :=
} TokenType;

typedef struct
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;

// Global symbol table
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];

typedef struct
{
    char *nameOP;
    int numOP;
    int L;
    int M;

} instruction;

// This function maps the reserved word
TokenType mapReservedWordAndIdentifier(char *str)
{

    if (strcmp(str, "begin") == 0)
    {
        return beginsym;
    }

    if (strcmp(str, "end") == 0)
    {
        return endsym;
    }

    if (strcmp(str, "if") == 0)
    {
        return ifsym;
    }

    if (strcmp(str, "fi") == 0)
    {
        return fisym;
    }

    if (strcmp(str, "then") == 0)
    {
        return thensym;
    }

    if (strcmp(str, "while") == 0)
    {
        return whilesym;
    }

    if (strcmp(str, "do") == 0)
    {
        return dosym;
    }

    if (strcmp(str, "od") == 0)
    {
        return odsym;
    }

    if (strcmp(str, "call") == 0)
    {
        return callsym;
    }

    if (strcmp(str, "const") == 0)
    {
        return constsym;
    }

    if (strcmp(str, "var") == 0)
    {
        return varsym;
    }

    if (strcmp(str, "procedure") == 0)
    {
        return procsym;
    }

    if (strcmp(str, "write") == 0)
    {
        return writesym;
    }

    if (strcmp(str, "read") == 0)
    {
        return readsym;
    }

    if (strcmp(str, "else") == 0)
    {
        return elsesym;
    }

    return 0; // It is not a reserved word
}

// This function maps the special symbols and alsp detects escape sequences
TokenType mapSpecialSym(char *buff)
{
    if (strcmp(buff, "+") == 0)
    {

        return plussym;
    }

    if (strcmp(buff, "-") == 0)
    {
        return minussym;
    }

    if (strcmp(buff, "/") == 0)
    {
        return slashsym;
    }

    if (strcmp(buff, "*") == 0)
    {
        return multsym;
    }

    if (strcmp(buff, "(") == 0)
    {
        return lparentsym;
    }

    if (strcmp(buff, ")") == 0)
    {
        return rparentsym;
    }

    if (strcmp(buff, "=") == 0)
    {
        return eqsym;
    }

    if (strcmp(buff, ",") == 0)
    {
        return commasym;
    }

    if (strcmp(buff, ";") == 0)
    {
        return semicolonsym;
    }

    if (strcmp(buff, ".") == 0)
    {
        return periodsym;
    }

    if (strcmp(buff, "<") == 0)
    {
        return lessym;
    }

    if (strcmp(buff, ">") == 0)
    {
        return gtrsym;
    }

    if (strcmp(buff, "<=") == 0)
    {
        return leqsym;
    }

    if (strcmp(buff, ">=") == 0)
    {
        return geqsym;
    }

    if (strcmp(buff, "<>") == 0)
    {
        return neqsym;
    }

    if (strcmp(buff, ":=") == 0)
    {
        return becomessym;
    }

    // Returns 0 when either a space or an escape sequence is found
    if (strcmp(buff, " ") == 0 || strcmp(buff, "\n") == 0 || strcmp(buff, "\t") == 0 || strcmp(buff, "\r") == 0)
    {
        return 0;
    }

    return skipsym; // Returns when there is an invalid symbol
}

// Checks whether the buffer contains a reserved word or identifier
TokenType reservedOrIdentifier(char buffer[], int bufferLength, char *reservedWord[], char *nameTable[], int *nameTableLength, int *idenIndex)
{

    // Boolean Flag
    int found = -1;

    // Iterates over the reserved words and checks similarities with buffer
    for (int j = 0; j < norw; j++)
    {
        if (strcmp(reservedWord[j], buffer) == 0)
        {
            *idenIndex = -1;                             // If they match the index identifier remains -1 (no identifier found)
            return mapReservedWordAndIdentifier(buffer); // The token number is returned
        }
    }

    // If it is not a reserved word, this loop checks if the buffer lines up with any identifiers in the name table
    for (int i = 0; i < *nameTableLength; i++)
    {
        // If it is in the name table, break
        if (strcmp(nameTable[i], buffer) == 0)
        {
            found = i; // Found is updated to index i
            break;
        }
    }

    if (found == -1)
    {
        // If it is not in the name table then add it
        nameTable[*nameTableLength] = malloc(bufferLength + 1);

        strcpy(nameTable[*nameTableLength], buffer);
        found = *nameTableLength; // Found is updated
        (*nameTableLength)++;
    }

    // Stores the identifier index from the name table
    *idenIndex = found;

    // Returns the identifier token number
    return identsym;
}

// Token list is global and can be accessed by the main function
int tokenList[strmax + 1] = {0}; // to store all the tokens
int tokenCount = 0;              // Counter to keep track of the token list
int tokenCounter = 0;
char *nameTable[strmax + 1] = {""}; // Array to store the name table
int nameTableLength = 0;
int symbolTableCounter = 0;

instruction instructions[MAX_SYMBOL_TABLE_SIZE] = {""};
int instCount = 0;

// Function that checks for escape sequences (Scanner was turned into an internal module)
void scanner(FILE *ip)
{

    //  Array for reserved words
    char *reservedWord[] = {"null", "begin", "call", "const", "do", "else", "end", "if", "odd", "procedure", "read", "then", "var", "while", "write"};

    // Buffer used for the lexeme grouping process
    char *bufferLexeme = malloc(strmax + 1);

    // Array to store the lexemes (for printing purposes)
    char *lexemes[strmax + 1] = {""};

    // Array for Error Messages
    char *errorMessages[] = {"Identifier too long", "Number too long", "Invalid symbol"};

    // Array for collecting error messages
    char *errorCollect[strmax + 1];

    // Used to read each char
    char ch;

    printf("Source Program:\n\n");

    int i = 0;           // Counter to keep track of the buffer
    int lexLength = 0;   // Counter to keep track of the lexemes array
    int errorMesNum = 0; // Counter to keep track of the error messages throughout the code

    // While true
    while (1)
    {

        // First character is gathered
        ch = fgetc(ip);

        // Checks for the EOF character
        if (ch == EOF)
        {
            break;
        }

        // Char is printed
        // putchar(ch);

        // Checks if char is a letter or a number
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
        {
            i = 0;
            /*
             If the rest of the characters being read are also letters or numbers it can potentially
             be a reserved word or an identifier
            */
            while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
            {
                // Char is printed
                putchar(ch);

                // Chars are added to the buffer until a non letter and non number character is reached
                bufferLexeme[i] = ch;
                // Moves along the ip pointer and gathers the character
                ch = fgetc(ip);

                // Buffer tracker is updated
                i++;
            }

            // Null terminator is added at the end
            bufferLexeme[i] = '\0';

            // If the character is neither a letter or a char, it the pointer skips it
            ungetc(ch, ip);

            // Identifier index variable is declared (it will be sent as a pointer so that it can be updated without it needing to be returned)
            int identifierIndex = -1;

            // Function is called to detect a reserved word or an identifier
            int token = reservedOrIdentifier(bufferLexeme, i, reservedWord, nameTable, &nameTableLength, &identifierIndex);

            // If the identifier token is return
            if (token == identsym)
            {
                // If the length of the identifier exceeds 11 characters
                if (strlen(bufferLexeme) > 11)
                {
                    // If the identifier is too long, it's removed from the name table
                    free(nameTable[nameTableLength - 1]);
                    nameTable[nameTableLength - 1] = NULL;
                    nameTableLength--; // Name table index is decreased

                    // Add error and skipsym
                    errorCollect[errorMesNum] = errorMessages[0]; // "Identifier too long"
                    errorMesNum++;

                    tokenList[tokenCount] = skipsym;
                    tokenCount++;
                }
                else
                {
                    // If it is a valid identifier length, it gets added to the token list
                    tokenList[tokenCount] = identsym;
                    tokenCount++;

                    // If it is not an identifier it also gets added
                    if (identifierIndex != -1)
                    {
                        tokenList[tokenCount] = identifierIndex;
                        tokenCount++;
                    }
                }
            }

            else
            {
                // Token (reserved word) is added to the token list
                tokenList[tokenCount] = token;
                tokenCount++;
            }

            // Memory is allocated for an element in the lexeme array that is the same length as the buffer length
            lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);

            // Buffer is copied to the lexeme array
            strcpy(lexemes[lexLength], bufferLexeme);
            lexLength++; // Lexeme array tracker is updated

            // Buffer is cleared
            bufferLexeme[0] = '\0';
            i = 0;
        }

        // If the character is strictly numbers
        else if (ch >= '0' && ch <= '9')
        {
            i = 0;
            // bufferLexeme[i] = ch;

            // Checks if the next character is also a number
            while (ch >= '0' && ch <= '9')
            {
                putchar(ch);
                bufferLexeme[i] = ch;
                ch = fgetc(ip);
                i++;
            }

            // Null Terminator
            bufferLexeme[i] = '\0';
            ungetc(ch, ip);

            int token = numbersym;

            // If the number is longer than 5 digits
            if (strlen(bufferLexeme) > 5)
            {
                // Collect error message
                errorCollect[errorMesNum] = errorMessages[1]; // "Number too long"
                errorMesNum++;

                // Add skipsym to the token list
                tokenList[tokenCount] = skipsym;
                tokenCount++;

                // Add the lexeme to the lexeme array
                lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);
                strcpy(lexemes[lexLength], bufferLexeme);
                lexLength++;
            }

            else
            {
                // If the number is valid the token is added
                tokenList[tokenCount] = numbersym;
                tokenCount++;

                // The numerical representation of the number is also added to the token list
                tokenList[tokenCount] = atoi(bufferLexeme);
                tokenCount++;

                // Lexeme is added to the lexeme array
                lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);
                strcpy(lexemes[lexLength], bufferLexeme);
                lexLength++;
            }

            // Buffer is cleared
            bufferLexeme[0] = '\0';
            i = 0;
        }

        // If it is not a letter or a number, it could be a special symbol
        else // if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')))
        {
            putchar(ch);
            i = 0;
            bufferLexeme[i] = ch;

            // If the char is any of these three, it could be a <=, >=, or a :=
            if (ch == '<' || ch == '>' || ch == ':')
            {

                ch = fgetc(ip);

                // Cheks if the next character is also a special symbol
                if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')))
                {

                    bufferLexeme[i + 1] = ch;
                }

                // Null terminator
                bufferLexeme[i + 2] = '\0';

                int token = mapSpecialSym(bufferLexeme);

                // If the token is not an escape sequence
                if (token != 0)
                {
                    // Token is added
                    tokenList[tokenCount] = token;
                    tokenCount++;

                    // Lexeme is added to the lexeme array
                    lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);
                    strcpy(lexemes[lexLength], bufferLexeme);
                    lexLength++;

                    // If it is an invalid symbol, token is collected
                    if (token == skipsym)
                    {
                        errorCollect[errorMesNum] = errorMessages[2]; // "Invalid Symbol"
                        errorMesNum++;
                    }

                    // Clear the buffer
                    bufferLexeme[0] = '\0';
                    i = 0;
                    continue;
                }
            }

            // If the character is / it could either be division or a comment
            if (ch == '/')
            {
                ch = fgetc(ip);
                putchar(ch);

                // Checks if it is a comment
                if (ch == '*')
                {

                    // Ignores everything inside the comment
                    while ((ch = fgetc(ip)) != '/')
                    {
                        continue;
                    }
                    continue;
                }

                int token = mapSpecialSym(bufferLexeme);

                // If the token is not an escape sequence
                if (token != 0)
                {
                    // Token is added to the token array
                    tokenList[tokenCount] = token;
                    tokenCount++;

                    // If the token is a skipsym, error message is collected
                    if (token == skipsym)
                    {
                        errorCollect[errorMesNum] = errorMessages[2]; // "Invalid Symbol"
                        errorMesNum++;
                    }

                    // Lexeme is added to the lexeme array
                    lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);
                    strcpy(lexemes[lexLength], bufferLexeme);
                    lexLength++;

                    // Clear the buffer
                    bufferLexeme[0] = '\0';
                    i = 0;
                    continue;
                }
            }

            // If it is a valid special symbol
            else
            {
                // Null terminator
                bufferLexeme[i + 1] = '\0';

                int token = mapSpecialSym(bufferLexeme);

                // If token is not an escape sequence
                if (token != 0)
                {
                    // Token is added to the token list
                    tokenList[tokenCount] = token;
                    tokenCount++;

                    // If the token is a skipsym, error message is added
                    if (token == skipsym)
                    {
                        errorCollect[errorMesNum] = errorMessages[2]; // "Invalid Symbol"
                        errorMesNum++;
                    }

                    // Copies lexeme to the lexeme array
                    lexemes[lexLength] = malloc(strlen(bufferLexeme) + 1);
                    strcpy(lexemes[lexLength], bufferLexeme);
                    lexLength++;

                    // Clear the buffer
                    bufferLexeme[0] = '\0';
                    i = 0;
                    continue;
                }
            }
        }
    }

    // Print portion
    printf("\n");

    printf("Lexeme Table:\n\n");
    printf("lexeme\t\ttoken type\n");

    int token = 0;           // To keep track of the tokens (it has to skip the identifier index)
    int errorPrintIndex = 0; // To keep track of the error messages
    for (int i = 0; i < lexLength; i++)
    {

        // Prints the lexeme
        printf("%s\t\t", lexemes[i]);

        // Checks for an error by checking for skipsym
        if (tokenList[token] == skipsym)
        {
            printf("%s\n", errorCollect[errorPrintIndex]); // Prints error message
            errorPrintIndex++;
        }
        else
        {
            // If not an error, print the token number
            printf("%d\n", tokenList[token]);
        }

        // If the token just printed was an identifier or a number, token is increased by 2
        if (tokenList[token] == identsym || tokenList[token] == numbersym)
        {
            token += 2;
        }

        // Else it's increased by 1
        else
        {
            token++;
        }
    }

    // Prints the name table
    printf("\nName Table:\n\n");
    printf("Index Name\n");
    for (int i = 0; i < nameTableLength; i++)
    {
        printf("%d\t%s\n", i, nameTable[i]);
    }

    // Prints the token list
    printf("\nToken List:\n\n");

    for (int i = 0; i < tokenCount; i++)
    {
        // If the token is an identifier or a number, it also prints the subsequent index or number
        if (tokenList[i] == identsym || tokenList[i] == numbersym)
        {
            printf("%d %d ", tokenList[i], tokenList[i + 1]);
            i++;
        }

        else
        {
            printf("%d ", tokenList[i]);
        }
    }

    printf("\n");

    // File pointer is closed
    fclose(ip);
}

// {} -> loop
// [] -> if condition

/* FUNCIONES */

// symbol table check
int symbolTableCheck(char identName[12])
{
    int index = -1;

    for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
    {
        if (strcmp(identName, symbolTable[i].name) == 0)
        {
            index = i;
            return index;
        }
    }

    // index not found
    return index;
}

// insert to symbol table
void insertSymbolTable(int kind, char name[12], int val, int level, int address, int mark)
{

    symbolTable[symbolTableCounter].kind = kind;
    strcpy(symbolTable[symbolTableCounter].name, name);
    symbolTable[symbolTableCounter].val = val;
    symbolTable[symbolTableCounter].addr = address;
    symbolTable[symbolTableCounter].mark = mark;
}

// mark for symbol table

void emit(char *name, int num, int l, int m)
{
    // instruction added to the array
    strcpy(instructions[instCount].nameOP, name);
    instructions[instCount].numOP = num;
    instructions[instCount].L = l;
    instructions[instCount].M = m;

    instCount++;
}

// statement
void statement()
{

    if (tokenList[tokenCounter] == identsym)
    {
        int symIndex = symbolTableCheck(nameTable[tokenList[tokenCounter++]]);
        if (symIndex == -1)
        {
            printf("Error: undeclared identifier");
            // write to output file
        }

        if (symbolTable[symIndex].kind == 2)
        {
            printf("Error: symbol name has already been declared");
            // output file
        }

        tokenCounter++;

        if (tokenList[tokenCounter] != becomessym)
        {
            printf("Error: assignment statements must use :=");
            // output file
        }

        tokenCounter++;

        // call expression
        // expression()
        emit("STO", 4, symbolTable[symIndex].level, symbolTable[symIndex].addr);
        return;
    }

    if (tokenList[tokenCounter] == beginsym)
    {
        statement();

        do
        {
            tokenCounter++;
            statement();
        } while (tokenList[tokenCounter] == semicolonsym);

        if (tokenList[tokenCounter] != endsym)
        {
            printf("Error: begin must be followed by end");
            // output to file
        }

        return;
    }

    if (tokenList[tokenCounter] == ifsym)
    {
        tokenCounter++;
        // call condition
        // condition()

        // int jpcIndex = current code index?
    }
}

// var-declaration
int varDeclaration()
{
    int numVars = 0;

    if (tokenList[tokenCounter] == varsym)
    {
        do
        {
            tokenCounter++;
            if (tokenList[tokenCounter] != identsym)
            {
                printf("Error: const, var, and read keywords must be followed by identifier");
                return -1;
                // write to output file
            }

            // checks duplicates
            if (symbolTableCheck(nameTable[tokenList[tokenCounter++]]) != -1)
            {
                printf("Error: symbol name has already been declared");
                return -1;
            }

            tokenCounter++;
            int varIndex = tokenList[tokenCounter];

            // adds to symbol table
            insertSymbolTable(2, nameTable[varIndex], 0, 0, numVars + 3, 0);

            numVars++;
            tokenCounter++;

        } while (tokenList[tokenCounter] == commasym);

        if (tokenList[tokenCounter] != semicolonsym)
        {
            printf("Error: constant and variable declarations must be followed by a semicolon");
            // write message to output file
            return -1;
        }

        tokenCounter++;
    }

    return numVars;
}

// const-declatation
void constDeclaration()
{
    char identName[12];

    if (tokenList[tokenCounter] == constsym)
    {
        // esto va adentro del do while
        tokenCounter++;
        if (tokenList[tokenCounter] != identsym)
        {
            // ERROR
        }
        if (symbolTableCheck(nameTable[tokenList[tokenCounter++]]) != -1)
        {
            // ERROR
        }
    }
}

// block
void block()
{

    constDeclaration();
    int numVars = varDeclaration();

    if (numVars == -1)
    {
        return; // si detecta un error no se emite la instruccion
    }
    emit("INC", 6, 0, numVars + 3);
}

// program
void program()
{

    // call block
    block();

    if (tokenList[tokenCount] != periodsym)
    {
        printf("Error: program must end with period");
    }

    // emit halt,
    emit("SYS", 9, 0, 3);
}

// condition
// expression
// term
// factor

/* MAIN */
int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        FILE *ip = fopen(argv[1], "r");

        scanner(ip);

        // llamar program

        // print instructions
        // print symbol table
    }

    else
    {
        printf("Wrong number of arguments.\n");
        printf("Try: ./00_args 123\n");
    }

    return 0;
}
