# AI Usage Disclosure Details

**Student Name:** Nicole Baez Espinosa
**Student ID:** 5544379
**Assignment:** Homework 3

---

## Instructions

Complete this template with detailed information about your AI usage. Submit this
file along with your signed PDF declaration form. Include the conversation log in
order, with each round listing the prompt, AI output, and verification/edits.

---

## AI Tool #1

### Tool Name

Claude

### Version/Model

Sonnet 4.6

### Date(s) Used

March 24th 2026, March 25th 2026, March 26th 2026, March 27th 2026

### Specific Parts of Assignment

Used for understanding parser logic and debugging grammar functions.

### Conversation Log

#### Round 1

**Prompt:** CONST-DECLARATION
if token == const
do
get next token
if token != identsym
error
if SYMBOLTABLECHECK (token) != -1
error
save ident name
get next token
if token != eqlsym
error
get next token
if token != numbersym
error
add to symbol table (kind 1, saved name, number, 0, 0)
get next token
while token == commasym
if token != semicolonsym
error
get next token (how would i save the identifier name? I only have the list of tokens and a separate name table, how can I map them?)
**AI Output:** Since your token list stores the name table index right after identsym, you can use that index to look up the name in the name table. So when you see identsym in the token list, the very next entry is the index into nameTable:
cif (token == identsym)
{
int nameIndex = tokenList[++currentToken]; // grab the name table index
char \*identName = nameTable[nameIndex]; // look up the actual name
// save identName for the symbol table
}
So your const declaration parser would look something like:
cvoid constDeclaration()
{
if (tokenList[currentToken] == constsym)
{
do
{
currentToken++;
if (tokenList[currentToken] != identsym)
error("Expected identifier");

            int nameIndex = tokenList[++currentToken]; // get name table index
            char *savedName = nameTable[nameIndex];    // save the name

            currentToken++;
            if (tokenList[currentToken] != eqsym)
                error("Expected =");

            currentToken++;
            if (tokenList[currentToken] != numbersym)
                error("Expected number");

            int value = tokenList[++currentToken];     // number value is also stored inline

            addToSymbolTable(1, savedName, value, 0, 0);

            currentToken++;
        }
        while (tokenList[currentToken] == commasym);

        if (tokenList[currentToken] != semicolonsym)
            error("Expected semicolon");

        currentToken++;
    }

}
The key insight is that your token list is essentially a flat array where identsym is always followed by its name table index, and numbersym is always followed by its value — so you just need to increment currentToken an extra time to grab the associated data whenever you encounter either of those tokens.
**Verification/Edits:** Claude helped me realize that the name table index is already stored in the token list. I will now do a linear search on the name table as well whenever I have to save the identifier name.

#### Round 2

**Prompt:** When I have to emit an instruction, I have to add it to an array of instructions, the instruction format is SYS 0 3 (name of instruction lexicographical level address), but how would I implement this in C?
**AI Output:** Define a struct for the instruction and an array to hold them:
ctypedef struct {
int op; // operation code
int l; // lexicographical level
int m; // address / modifier
} Instruction;

Instruction code[MAX_CODE_SIZE];
int codeIndex = 0; // tracks how many instructions have been emitted
Then write an emit function that adds to the array:
cvoid emit(int op, int l, int m)
{
code[codeIndex].op = op;
code[codeIndex].l = l;
code[codeIndex].m = m;
codeIndex++;
}
Then whenever you need to emit an instruction you just call:
cemit(9, 0, 3); // SYS 0 3
You'll probably want to define constants or an enum for your operation codes too so the code is readable:
ctypedef enum {
LIT = 1,
OPR = 2,
LOD = 3,
STO = 4,
CAL = 5,
INC = 6,
JMP = 7,
JPC = 8,
SYS = 9
} OpCode;

// Then emit reads much cleaner
emit(SYS, 0, 3); Sonnet 4.6Claude is AI and can make mistakes. Please double-check responses.
**Verification/Edits:** I liked the structure implementation as it is similar to the one done in HW1. I will not do the enum approach. Ultimately, the instructions were stored in a 2D array instead of a struct array.

#### Round 3 (optional)

**Prompt:** int numVars = 0;
int error = 0; //cuando detecta el error sale immediatamente de la funcion

    if (tokenList[tokenCounter] == varsym)
    {
        do
        {
            tokenCounter++;
            if (tokenList[tokenCounter] != identsym)
            {
                printf("Error: const, var, and read keywords must be followed by identifier");
                error = -1;
                break;
                // write to output file
            }

            // checks duplicates
            if (symbolTableCheck(nameTable[tokenList[tokenCounter++]]) != -1)
            {
                printf("Error: symbol name has already been declared");
                error = -1; //si detecta el error sale del loop
                break;
            }

            // adds to symbol table
            insertSymbolTable(2, nameTable[tokenList[tokenCounter++]], 0, 0, numVars + 3, 0);

            numVars++;
            tokenCounter++;

        } while (tokenList[tokenCounter] == commasym);


        if(error == -1)
        {
            return error; //no retorna el numvars
        }

        if (tokenList[tokenCount] != semicolonsym)
        {
            printf("Error: constant and variable declarations must be followed by a semicolon");
            // write message to output file
        }

        tokenCounter++;
    }

    return numVars;

I want the program to stop at the first error, meaning it doesnt print additional errors and it goes back to the parent function, is this logic correct

**AI Output:** The logic is mostly right but you have a few issues:
Bug 1: tokenCounter is being incremented too many times
Inside the loop you call tokenCounter++ three separate times plus the do loop increments it at the start. You're skipping tokens. After checking identsym, the next token is the name table index — you only need to grab it once:
ctokenCounter++; // move past identsym
int nameIndex = tokenList[tokenCounter]; // grab name table index
// don't increment again until you're done with this token
Bug 2: tokenCount vs tokenCounter typo
cif (tokenList[tokenCount] != semicolonsym) // wrong variable
if (tokenList[tokenCounter] != semicolonsym) // correct
Bug 3: Semicolon error doesn't set error and return
cif (tokenList[tokenCounter] != semicolonsym)
{
printf("Error: ...");
return -1; // need to actually return here
}
Here's the corrected loop:
cif (tokenList[tokenCounter] == varsym)
{
do
{
tokenCounter++;
if (tokenList[tokenCounter] != identsym)
{
printf("Error: const, var, and read keywords must be followed by identifier");
return -1;
}

        tokenCounter++;
        int nameIndex = tokenList[tokenCounter]; // name table index

        if (symbolTableCheck(nameTable[nameIndex]) != -1)
        {
            printf("Error: symbol name has already been declared");
            return -1;
        }

        insertSymbolTable(2, nameTable[nameIndex], 0, 0, numVars + 3, 0);
        numVars++;
        tokenCounter++;

    } while (tokenList[tokenCounter] == commasym);

    if (tokenList[tokenCounter] != semicolonsym)
    {
        printf("Error: constant and variable declarations must be followed by a semicolon");
        return -1;
    }

    tokenCounter++;

}

return numVars;
Since you're returning -1 directly on any error, you don't need the error flag variable at all.
**Verification/Edits:** I implemented the return -1 strategy and fixed the logic with gathering the name table index. It said the tokenCounter was increased too many times but I ignored that for the moment.

#### Round 4

**Prompt:** I keep getting segmentation fault errors when I implement my struct for storing instructions. Why does this keep happening? This is my output
HERE IN PROGRAM
HERE IN BLOCK
HERE IN CONSTDECLARATION
HERE IN VARDECLARATION
tokenCounter before getNextToken: 0
current token: 14
tokenCounter after getNextToken: 1
token after advance: 2
Name table variable: xINSERTED IN SYMBOL TABLE:
kind: 2 name: x value: 0 address: 3 mark: 0
tokenCounter before getNextToken: 3
current token: 31
tokenCounter after getNextToken: 4
token after advance: 2
Name table variable: yINSERTED IN SYMBOL TABLE:
kind: 2 name: y value: 0 address: 4 mark: 0
Segmentation fault: 11
and this is my emit function
// emit
void emit(char \*name, int num, int l, int m)
{
// instruction added to the array
strcpy(instructions[cx].nameOP, name);
instructions[cx].numOP = num;
instructions[cx].L = l;
instructions[cx].M = m;

    cx++;

}
**AI Output:** Your emit function takes 4 parameters so the call just needs the right opcode number. Looking at your call:
cemit("INC", 6, 0, numVars + 3);
The arguments are in the wrong order. Your function signature is (name, num, l, m) so 6 is being passed as num, 0 as l, and numVars + 3 as m. But INC should be opcode 6 with l = 0 and m = numVars + 3, so it should be:
cemit("INC", 6, 0, numVars + 3);
Actually that looks correct for the values. But the segfault is still happening, which means the issue is that instructions array or cx is not declared or initialized properly. Can you share how instructions and cx are declared? The struct needs a nameOP field large enough to hold the opcode name, for example:
ctypedef struct {
char nameOP[4]; // "INC\0" needs 4 chars
int numOP;
int L;
int M;
} Instruction;

Instruction instructions[MAX_CODE_SIZE];
int cx = 0;
If nameOP is declared smaller than the string being copied into it, strcpy will overflow and corrupt memory, which would explain the segfault
**Verification/Edits:** Once I changed the name character array to a static array instead of a pointer, the segmentation fault went away. However, I had to resort to a 2D array to store instructions to go based off the PM/0 virtual machine implementation so the struct completely went away.

#### Round 5

**Prompt:** Assembly code:
+------+-------+---+-----+
| Line | OP | L | M |
+------+-------+---+-----+
| 0 | INC | 0 | 5 |
| 1 | LOD | 0 | 4 |
| 2 | LOD | 0 | 3 |
| 3 | OPR | 0 | 7 |
| 4 | JPC | 0 | 7 |
| 5 | LOD | 0 | 4 |
| 6 | SYS | 0 | 1 |
| 7 | LOD | 0 | 3 |
| 8 | LIT | 0 | 1 |
| 9 | OPR | 0 | 2 |
| 10 | SYS | 0 | 1 |
| 11 | SYS | 0 | 3 |
+------+-------+---+-----+ (This is the output I am getting, and M for JPC is wrong, it should be 21)
**AI Output:** The JPC M value is wrong because of the backpatching. You're saving jpcIndex before emitting JPC, but then instructions[jpcIndex].M = cx is being set too early or cx isn't at the right value when you patch it.
Can you share the relevant section of your statement() where the if/while backpatching happens? Based on your assembly output it looks like a while loop, so the issue is likely here:
cint jpcIndex = cx;
emit("JPC", 8, 0, 0);

statement(); // cx should advance through all the statements inside

emit("JMP", 7, 0, loopIndex); // you have JPC here instead of JMP!

instructions[jpcIndex].M = cx; // patch after the JMP
Looking at your assembly, line 4 is JPC and line 11 is halt — so M should be 11 or wherever execution should jump when the condition is false. But you're getting 7 which is just the next instruction after JPC, meaning cx was never updated after the body was emitted.
Also notice you have:
cemit("JPC", 8, 0, loopIndex); // wrong - should be JMP to loop back
The second jump at the end of a while loop should be JMP not JPC — JMP unconditionally jumps back to re-evaluate the condition, while JPC only jumps if false.
**Verification/Edits:** This output was something I kept getting no matter how many times I tried explaining the problem to Claude. Ultimately, I asked the professor for hints and he suggested a 2D array implentation, which explains why the M field for JPC was 21 (7 \* 3 = 21).

#### Round 6

**Prompt:** VAR-DECLARATION - returns number of variables
numVars = 0
if token == varsym
do
get next token
if token != identsym
error
if SYMBOLTABLECHECK (token) != -1
error
add to symbol table (kind 2, ident, 0, 0, numVars + 3)
numVars++
get next token
while token == commasym
if token != semicolonsym
error
get next token
return numVars (is there something wrong with this pseudo code? its for a recursive descent parser in c)
**AI Output:** Your pseudocode looks mostly correct, but there's a missing loop structure for handling multiple variable declarations after the commas. Here's the issue:
After you check while token == commasym, you never get the next token or re-validate the identifier inside that loop. The loop body is empty/implied but not stated.
Problematic section:
while token == commasym ← loops, but does nothing inside
if token != semicolonsym ← this is OUTSIDE the while, not in it
Fixed pseudocode:
VAR-DECLARATION - returns number of variables
numVars = 0
if token == varsym
get next token
if token != identsym
error
if SYMBOLTABLECHECK(token) != -1
error
add to symbol table (kind 2, ident, 0, 0, numVars + 3)
numVars++
get next token
while token == commasym
get next token ← consume the comma, move to next ident
if token != identsym
error
if SYMBOLTABLECHECK(token) != -1
error
add to symbol table (kind 2, ident, 0, 0, numVars + 3)
numVars++
get next token ← advance past the identifier
if token != semicolonsym
error
get next token ← consume the semicolon
return numVars
Key fixes:

Loop body filled in — after consuming the comma, you must re-check for a valid identifier, check the symbol table for duplicates, add it, increment, and advance the token.
get next token after semicolon — you should consume the semicolon before returning, so the caller receives the next meaningful token (this is standard practice in recursive descent parsers to keep the token stream advancing properly).
Indentation clarity — the if token != semicolonsym check belongs after the while loop, not inside it.

The overall structure and logic (duplicate checking, kind 2, offset numVars + 3) looks correct for a PL/0-style parser.
**Verification/Edits:** Since the assignment instructions state that the pseudo code has errors, I decided to verify with the professor. I asked him about ways that could help me figure out what parts of the pseudo code were incorrect and I started comparing the grammar with the pseudo code line by line. This way, I found that the statement and expression functions had the most errors.

#### Round 7

**Prompt:** Error: program must end with period • Complete Example: See Appendix E for detailed walkthrough
Error: const, var, and read keywords must be followed by identifier
Error: symbol name has already been declared
Error: constants must be assigned with =
Error: constants must be assigned an integer value
Error: constant and variable declarations must be followed by a semicolon
Error: undeclared identifier 9
Error: only variable values may be altered
Error: assignment statements must use :=
Error: begin must be followed by end
Error: if must be followed by then
Error: while must be followed by do
Error: do must be followed by od
Error: if-then statement must end with fi
Error: condition must contain comparison operator
Error: right parenthesis must follow left parenthesis
Error: arithmetic equations must contain operands, parentheses, numbers, or symbols (These are the current error messages implemented in my lexical analyzer + parser code, what are some custom messages I can implement in my code?)
**AI Output:** Here are some good custom error messages you could add:
Semantic Errors

"division by zero is not allowed"
"constant value exceeds maximum integer size"
"variable declared but never used"
"constant declared but never used"

Statement/Structure Errors

"empty block — begin must contain at least one statement"
"nested procedure depth exceeds maximum allowed level"
"read must be followed by a variable, not a constant"
"write/print must be followed by an expression"

Expression Errors

"unary minus is not supported"
"expression cannot begin with an operator"
"missing operand before operator"
"consecutive operators are not allowed"

Symbol Table Errors

"symbol table full — too many declarations"
"procedure called with wrong number of arguments" (if your language supports params)

General Structure

"unexpected end of file"
"statement cannot begin with this token"
"maximum nesting depth exceeded for if/while blocks"

The most practical ones to actually implement given your current error list would be:

Division by zero (easy to catch during parsing of expressions)
Empty begin/end block
Unexpected end of file
Variable declared but never used (tracked via symbol table flag)

These fill natural gaps in what your current errors already cover.
**Verification/Edits:** I implemented the empty begin end block error in my code and an empty file error in my code as part of custom errors. I tested every test case after implenting these new ones to ensure the logic remained unaffected.

#### Round 1

**Prompt:** // Term
void term()
{

    factor(); //Factor is called
    if (errorFlag == 1)
        return;

    //Loop checks for multiplication or division symbol
    while (tokenList[tokenCounter] == multsym || tokenList[tokenCounter] == slashsym)
    {

        //If token is a multiplication symbol, then factor is called and MULT instruction is emitted
        if (tokenList[tokenCounter] == multsym)
        {

            getNextToken();
            factor();
            if (errorFlag == 1)
                return;
            emit(2, 0, 4);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
        //Division symbol is emitted
        else
        {

            getNextToken();

            //Custom check for division by 0
            if(tokenList[tokenCounter] == 0)
            {
                printf("Error: division by 0");
                strcpy(errorMessage, "Error: division by 0");
                errorFlag = 1;
                return;
            }

            factor();
            if (errorFlag == 1)
                return;
            emit(2, 0, 5);
            strcpy(nameOP_storage[nameOPcounter], "OPR");
            nameOPcounter++;
        }
    }

}
This is how I edited my term function to try to catch the division by 0 error but this is the output I get. Assembly code:
+------+-------+---+-----+
| Line | OP | L | M |
+------+-------+---+-----+
| 0 | INC | 0 | 4 |
| 1 | LIT | 0 | 5 |
| 2 | LIT | 0 | 0 |
| 3 | OPR | 0 | 5 |
| 4 | STO | 0 | 3 |
| 5 | SYS | 0 | 3 |
+------+-------+---+-----+
Symbol Table:
+------+-------------+-------+-------+---------+------+
| Kind | Name | Value | Level | Address | Mark |
+------+-------------+-------+-------+---------+------+
| 1 | m | 5 | 0 | 0 | 1 |
| 2 | x | 0 | 0 | 3 | 1 |
+------+-------------+-------+-------+---------+------+
**AI Output:** The problem is your check:
cif(tokenList[tokenCounter] == 0)
This is checking if the token type is 0, not if the value is the number zero. By the time you're in term(), you're working with token codes (like numbersym, identsym, etc.), not the actual numeric values.
You need to check the actual number value from your number list/array. It should be something like:
celse
{
getNextToken();

    if(tokenList[tokenCounter] == numbersym && numberList[tokenCounter] == 0)
    {
        printf("Error: division by 0");
        strcpy(errorMessage, "Error: division by 0");
        errorFlag = 1;
        return;
    }

    factor();
    ...

}
The exact array name depends on how your lexer stores literal values — it might be numberList, valueList, tokenValue, etc. The key point is you need two conditions:

The current token is a number literal (numbersym)
That number's actual value is 0

Without condition 1, you might accidentally match something that isn't a number at all but happens to have token code 0.
**Verification/Edits:** This response helped me remember that numbers are declared with the token 3 before the actual number is placed in the tokenlist. Instead of doing what the AI suggested, I tweaked the statement slightly by doing if(tokenList[tokenCounter] == numbersym && tokenList[tokenCounter + 1] == 0).

### Learning & Reflection

AI was mostly useful with debugging. It encouraged me to place debugging print statements throughout my code that I felt were very helpful in terms of visualizing each function. However, I found that AI was not optimal for conceptual questions as it often focused on the coding details rather than the big picture. For instance, it could not examine different approaches that would produce different instruction fields and it kept repeating the same answer.

---

## AI Tool #2 (if applicable)

### Tool Name

Not applicable

## AI Tool #3 (if applicable)

### Tool Name

Not applicable.

## Overall Reflection

Overall, AI helped me mostly with the technical aspects of coding. It helped me debug segmentation fault errors and it reinforced a lot of debugging practices that I had not implemented in a long time. I also attended office hours to ensure that I understood the conceptual portions of the assignment in order to implement the grammar. There were other strategies in my code that I implemented without the help of AI such as the global flag errorFlag that supports immediate exit from the program once an error is encountered.
Through this Homework assignment, I learned to always double check AI generated content with a professor or TA as they have more knowledge and context of the assignment than AI tools.

---

## Notes

- Be as specific and detailed as possible in your responses
- Keep the conversation rounds in order
- Include exact prompts and AI outputs when possible
- Explain how you verified and modified AI-generated content
- Reflect on what you learned through the AI interaction
