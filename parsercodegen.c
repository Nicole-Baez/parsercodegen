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

#define norw 17                   // num of reserved words
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
    char nameOP[4];
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

int instructions[MAX_SYMBOL_TABLE_SIZE][3];
int cx = 0;
char nameOP_storage[strmax + 1][4] = {""};
int nameOPcounter = 0;

// IMPLEMENT LOGIC FOR SAVING NAMES OF INSTRUCTIONS

// function prototypes
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void expression();
void condition();
void term();
void factor();
void insertSymbolTable(int kind, char name[12], int val, int level, int address, int mark);

// Function that checks for escape sequences (Scanner was turned into an internal module)
void scanner(FILE *ip)
{

    //  Array for reserved words
    char *reservedWord[] = {"null", "begin", "call", "const", "do", "else", "end", "if", "odd", "procedure", "read", "then", "var", "while", "write", "fi", "od"};

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

    // printf("Source Program:\n\n");

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
                // putchar(ch);

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
                // putchar(ch);
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
            // putchar(ch);
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
                // putchar(ch);

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
    // printf("\n");

    // printf("Lexeme Table:\n\n");
    // printf("lexeme\t\ttoken type\n");

    // Prints the name table

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

    for (int i = 0; i < symbolTableCounter; i++)
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

    // printf("INSERTED IN SYMBOL TABLE:\n");
    // printf("kind: %d   name: %s    value: %d    address: %d    mark: %d\n", symbolTable[symbolTableCounter].kind, symbolTable[symbolTableCounter].name, symbolTable[symbolTableCounter].val, symbolTable[symbolTableCounter].addr, symbolTable[symbolTableCounter].mark);

    symbolTableCounter++;
}

// mark for symbol table

// emit
void emit(int num, int l, int m)
{
    // instruction added to the array
    // strcpy(instructions[cx].nameOP, name);
    instructions[cx][0] = num;
    instructions[cx][1] = l;
    instructions[cx][2] = m;

    cx++;
}

// Identifiers and numbers take two entries in tokenList (symbol + value),
//  so we advance tokenCounter by 2 instead of 1.
void getNextToken()
{

    if (tokenList[tokenCounter] == identsym || tokenList[tokenCounter] == numbersym)
    {
        tokenCounter += 2;
    }
    else
    {
        tokenCounter++;
    }
}

// expression
void expression()
{
    // printf("HERE IN EXPRESSION\n");
    //"-" at the start means an expression can begin with a negative sign
    if (tokenList[tokenCounter] == minussym)
    {
        getNextToken();
        term();
        emit(2, 0, 1);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else
    {
        term();
    }
    while (tokenList[tokenCounter] == plussym || tokenList[tokenCounter] == minussym)
    {
        if (tokenList[tokenCounter] == plussym)
        {
            getNextToken();
            term();
            emit(2, 0, 2);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
        else
        {
            getNextToken();
            term();
            emit(2, 0, 3);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
    }
}

// condition
void condition()
{
    // printf("HERE IN CONDITION\n");
    expression();
    if (tokenList[tokenCounter] == eqsym)
    {

        getNextToken();
        expression();
        emit(2, 0, 6);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else if (tokenList[tokenCounter] == neqsym)
    {

        getNextToken();
        expression();
        emit(2, 0, 7);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else if (tokenList[tokenCounter] == lessym)
    {

        getNextToken();
        expression();
        emit(2, 0, 8);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else if (tokenList[tokenCounter] == leqsym)
    {

        getNextToken();
        expression();
        emit(2, 0, 9);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else if (tokenList[tokenCounter] == gtrsym)
    {

        getNextToken();
        expression();
        emit(2, 0, 10);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else if (tokenList[tokenCounter] == geqsym)
    {

        getNextToken();
        expression();
        emit(2, 0, 11);
        strcpy(nameOP_storage[nameOPcounter], "OPR");
        nameOPcounter++;
    }
    else
    {
        printf("condition must contain comparison operator");
    }
}

// factor
void factor()
{

    // printf("HERE IN FACTOR\n");

    int symIdx;

    // if identifier check if it was declared
    if (tokenList[tokenCounter] == identsym)
    {

        symIdx = symbolTableCheck(nameTable[tokenList[tokenCounter + 1]]);

        if (symIdx == -1)
        {

            printf("undeclared identifier");
            return;
        }
        // if it is a constant give instruction LIT
        if (symbolTable[symIdx].kind == 1)
        {
            emit(1, 0, symbolTable[symIdx].val);
            strcpy(nameOP_storage[nameOPcounter], "LIT");
            nameOPcounter++;
        }
        else
        {

            // if variable Load it
            emit(3, 0, symbolTable[symIdx].addr);
            strcpy(nameOP_storage[nameOPcounter], "LOD");
            nameOPcounter++;
        }

        getNextToken();
    }
    else if (tokenList[tokenCounter] == numbersym)
    {
        emit(1, 0, tokenList[tokenCounter + 1]);
        strcpy(nameOP_storage[nameOPcounter], "LIT");
        nameOPcounter++;
        getNextToken();
    }
    else if (tokenList[tokenCounter] == lparentsym)
    {

        getNextToken();
        expression();

        if (tokenList[tokenCounter] != rparentsym)
        {

            printf("Error: right parenthesis must follow left parenthesis\n");
        }

        getNextToken();
    }
    else
    {
        printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
    }
}

// term
void term()
{
    // printf("HERE IN TERM\n");
    factor();

    while (tokenList[tokenCounter] == multsym || tokenList[tokenCounter] == slashsym)
    {

        if (tokenList[tokenCounter] == multsym)
        {

            getNextToken();
            factor();
            emit(2, 0, 4);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
        else
        {

            getNextToken();
            factor();
            emit(2, 0, 5);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
    }
}

// statement
void statement()
{
    // printf("HERE IN STATEMENT\n");

    if (tokenList[tokenCounter] == identsym)
    {
        int symIndex = symbolTableCheck(nameTable[tokenList[tokenCounter + 1]]);
        if (symIndex == -1)
        {
            printf("Error: undeclared identifier\n");
            // write to output file
            return;
        }

        if (symbolTable[symIndex].kind != 2)
        {
            printf("Error: only variable values may be altered\n");
            // write to output file
            return;
        }

        getNextToken();

        if (tokenList[tokenCounter] != becomessym)
        {
            printf("Error: assignment statements must use :=\n");
            // output file
            // write to output file
            return;
        }

        getNextToken();

        // call expression

        expression();
        emit(4, 0, symbolTable[symIndex].addr);
        strcpy(nameOP_storage[nameOPcounter], "STO");
        nameOPcounter++;
        return;
    }

    if (tokenList[tokenCounter] == beginsym)
    {
        getNextToken();
        statement();

        while (tokenList[tokenCounter] == semicolonsym)
        {
            getNextToken();
            statement();
        }

        if (tokenList[tokenCounter] != endsym)
        {
            printf("Error: begin must be followed by end\n");
            // output to file
            return;
        }
        getNextToken();
        return;
    }

    if (tokenList[tokenCounter] == ifsym)
    {
        getNextToken();
        // call condition
        condition();

        int jpcIndex = cx;
        emit(8, 0, 0);
        strcpy(nameOP_storage[nameOPcounter], "JPC");
        nameOPcounter++;

        if (tokenList[tokenCounter] != thensym)
        {
            printf("Error: if must be followed by then\n");
            // output file
            return;
        }

        getNextToken();
        statement();

        instructions[jpcIndex][2] = cx * 3;

        if (tokenList[tokenCounter] == elsesym)
        {
            getNextToken();
            statement();
        }

        if (tokenList[tokenCounter] != fisym)
        {
            printf("Error: if-then statement must end with fi\n");
            // output file
            return;
        }
        getNextToken();
        return;
    }

    if (tokenList[tokenCounter] == whilesym)
    {
        getNextToken();
        int loopIndex = cx * 3;
        condition();

        if (tokenList[tokenCounter] != dosym)
        {
            printf("Error: while must be followed by do\n");
            // output file
            return;
        }

        getNextToken();

        int jpcIndex = cx;
        emit(8, 0, 0);
        strcpy(nameOP_storage[nameOPcounter], "JPC");
        nameOPcounter++;

        statement();
        emit(7, 0, loopIndex);
        strcpy(nameOP_storage[nameOPcounter], "JMP");
        nameOPcounter++;

        instructions[jpcIndex][2] = cx;

        if (tokenList[tokenCounter] != odsym)
        {
            printf("Error: do must be followed by od\n");
            // output file
            return;
        }
        getNextToken();
        return;
    }

    if (tokenList[tokenCounter] == readsym)
    {
        getNextToken();
        if (tokenList[tokenCounter] != identsym)
        {
            printf("Error: const, var, and read keywords must be followed by identifier\n");
            // output file
            return;
        }

        int symIndex = symbolTableCheck(nameTable[tokenList[tokenCounter + 1]]);
        if (symIndex == -1)
        {
            printf("Error: undeclared identifier\n");
            // output
            return;
        }

        if (symbolTable[symIndex].kind != 2)
        {
            printf("Error: only variable values may be altered\n");
            // output
            return;
        }

        getNextToken();
        emit(9, 0, 2);
        strcpy(nameOP_storage[nameOPcounter], "SYS");
        nameOPcounter++;
        emit(4, 0, symbolTable[symIndex].addr);
        strcpy(nameOP_storage[nameOPcounter], "STO");
        nameOPcounter++;
        return;
    }

    if (tokenList[tokenCounter] == writesym)
    {
        getNextToken();
        expression();
        emit(9, 0, 1);
        strcpy(nameOP_storage[nameOPcounter], "SYS");
        nameOPcounter++;
        return;
    }
}

// var-declaration
int varDeclaration()
{
    int numVars = 0;
    // printf("HERE IN VARDECLARATION\n");

    char identName[12];

    if (tokenList[tokenCounter] == varsym)
    {
        do
        {

            getNextToken();

            if (tokenList[tokenCounter] != identsym)
            {
                printf("Error: const, var, and read keywords must be followed by identifier");
                return -1;
                // write to output file
            }

            // printf("Name table variable: %s", nameTable[tokenList[tokenCounter + 1]]);

            // checks duplicates
            if (symbolTableCheck(nameTable[tokenList[tokenCounter + 1]]) != -1)
            {
                printf("Error: symbol name has already been declared");
                return -1;
            }

            strcpy(identName, nameTable[tokenList[tokenCounter + 1]]);

            // adds to symbol table
            insertSymbolTable(2, identName, 0, 0, numVars + 3, 0);

            numVars++;
            getNextToken();

        } while (tokenList[tokenCounter] == commasym);

        if (tokenList[tokenCounter] != semicolonsym)
        {
            printf("Error: constant and variable declarations must be followed by a semicolon");
            // write message to output file
            return -1;
        }

        getNextToken();
    }

    return numVars;
}

// const-declatation
void constDeclaration()
{
    // printf("HERE IN CONSTDECLARATION\n");
    char identName[12];
    if (tokenList[tokenCounter] == constsym)
    {
        do
        {
            getNextToken();
            if (tokenList[tokenCounter] != identsym)
            {
                printf("Error: const, var, and read keywords must be followed by identifier");
            }

            if (symbolTableCheck(nameTable[tokenList[tokenCounter + 1]]) != -1)
            {
                printf("Error: symbol name has already been declared");
            }

            strcpy(identName, nameTable[tokenList[tokenCounter + 1]]);

            getNextToken();

            if (tokenList[tokenCounter] != eqsym)
            {
                printf("Error: constants must be assigned with =");
            }

            getNextToken();

            if (tokenList[tokenCounter] != numbersym)
            {
                printf("Error: constants must be assigned an integer value");
            }

            // add to symbol table
            insertSymbolTable(1, identName, tokenList[tokenCounter + 1], 0, 0, 0);
            getNextToken();

        } while (tokenList[tokenCounter] == commasym);
        if (tokenList[tokenCounter] != semicolonsym)
        {
            printf("Error: constant and variable declarations must be followed by a semicolon");
        }
        getNextToken();
    }
}

// block
void block()
{

    // printf("\nHERE IN BLOCK\n");
    constDeclaration();
    int numVars = varDeclaration();

    if (numVars == -1)
    {
        return; // si detecta un error no se emite la instruccion
    }
    emit(6, 0, numVars + 3);
    strcpy(nameOP_storage[nameOPcounter], "INC");
    nameOPcounter++;

    statement();

    // setting mark to 1
    for (int i = 0; i < symbolTableCounter; i++)
    {
        symbolTable[i].mark = 1;
    }
}

// program
void program()
{

    // call block
    // printf("HERE IN PROGRAM\n");
    block();

    if (tokenList[tokenCounter] != periodsym)
    {
        printf("Error: program must end with period");
    }

    // emit halt,
    emit(9, 0, 3);
    strcpy(nameOP_storage[nameOPcounter], "SYS");
    nameOPcounter++;
}

void printInst()
{

    printf("Assembly code:\n");
    printf("+------+-------+---+-----+\n");
    printf("| Line |   OP  | L |  M  |\n");
    printf("+------+-------+---+-----+\n");
    for (int i = 0; i < cx; i++)
    {

        printf("|  %d  | %s |  %d  |  %d  |\n", i, nameOP_storage[i], instructions[i][1], instructions[i][2]);
    }

    printf("+------+-------+---+-----+\n");

    printf("Symbol Table:\n");
    printf("+------+-------------+-------+-------+---------+------+\n");
    printf("| Kind |     Name    | Value | Level | Address | Mark |\n");
    printf("+------+-------------+-------+-------+---------+------+\n");

    for (int i = 0; i < symbolTableCounter; i++)
    {
        printf("| %d    |     %s       |   %d   |    %d  |    %d    |   %d  |\n", symbolTable[i].kind, symbolTable[i].name, symbolTable[i].val, symbolTable[i].level, symbolTable[i].addr, symbolTable[i].mark);
    }

    printf("+------+-------------+-------+-------+---------+------+\n");
}

/* MAIN */
int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        FILE *ip = fopen(argv[1], "r");
        // FILE *op = fopen("elf.txt", "w");
        scanner(ip);

        // llamar program
        program();
        // printf("INSTRUCTION: %s, %d, %d\n", instructions[0].nameOP, instructions[0].L, instructions[0].M);
        printInst();

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
