# AI Usage Disclosure Details

**Student Name:** Nicole Baez Espinosa,Lianet Martin
**Student ID:** 5544379,5636501
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

March 24th 2026, March 25th 2026

### Specific Parts of Assignment

Used for understanding parser logic and debugging grammar function. 

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
    char *identName = nameTable[nameIndex];    // look up the actual name
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
**AI Output:**  Define a struct for the instruction and an array to hold them:
ctypedef struct {
    int op;  // operation code
    int l;   // lexicographical level
    int m;   // address / modifier
} Instruction;

Instruction code[MAX_CODE_SIZE];
int codeIndex = 0;  // tracks how many instructions have been emitted
Then write an emit function that adds to the array:
cvoid emit(int op, int l, int m)
{
    code[codeIndex].op = op;
    code[codeIndex].l = l;
    code[codeIndex].m = m;
    codeIndex++;
}
Then whenever you need to emit an instruction you just call:
cemit(9, 0, 3);  // SYS 0 3
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
**Verification/Edits:** I liked the structure implementation as it is similar to the one done in HW1. I will not do the enum approach
because when I store the instructions, I need the name of the instructions instead of the opcode. I will most likely do a similar approach as I did in HW1 and store the instructions in an array. I will implement an emit function that inserts each instruction into the array of structs.


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

**Prompt:** 
var x, y; begin if y <> x then write y fi; write x+ 1; end. but this input is correct, and this is the tokenlist 14 2 0 31 2 1 32 4 6 2 1 24 2 0 8 16 2 1 7 32 16 2 0 19 3 1 32 5 33 it has skipsym; el sexto numero; so intead of / check for skipsym for (int i = 0; i < tokenCount; i++) { if (tokenList[i] == skipsym) { printf("Error: Scanning error detected by lexer (skipsym present)"); return; } } this I need to i+=2 when an identifier
**AI Output:** 
No it doesn't have skipsym. Skipsym is token 1 and looking at your token list there is no 1 in there. The numbers are all valid token types. Where are you seeing a skipsym?; The sixth number is 4 which is beginsym, not skipsym. Let me count through the list:

14 — varsym 2 0 — identsym, index 0 (x) 31 — commasym 2 1 — identsym, index 1 (y) 32 — semicolonsym 4 — beginsym

No skipsym anywhere in that list.; Yes! Because if an identifier's name table index happens to be 1, your loop would mistake it for skipsym. So you need to skip the second slot for identsym and numbersym the same way getNextToken does. 
**Verification/Edits:** 
Confirmed the bug by printing the index alongside each token value. Fixed the loop by adding i++ when tokenList[i] == identsym || tokenList[i] == numbersym to skip the name table index slot, preventing it from being misread as a token type. Tested with the correct input and confirmed no false scanning error was triggered.





### Learning & Reflection
Working through this assignment with AI assistance helped me develop a deeper understanding of how a compiler pipeline works, from tokenization through parsing and code generation.
Round 1 taught me that the token list is a flat array where identifiers and numbers always occupy two slots — the token type followed by either a name table index or a numeric value. I had been thinking of the name table as a completely separate structure, but realizing the index is already embedded in the token list made the lookup logic click. I verified this by tracing through the token list manually before implementing.
Round 2 introduced me to using a struct array for storing instructions with an emit function. Although I chose not to follow the enum suggestion, understanding the struct approach helped me design my own solution that stored the mnemonic name separately alongside the numeric instruction data. This was a design decision I made myself based on what fit my implementation better.
Round 3 helped me catch a subtle bug where tokenCounter was being incremented too many times inside the var declaration loop, which would cause tokens to be skipped. I also learned the difference between using an error flag with a break versus returning directly — the direct return approach was cleaner and eliminated the need for an extra variable. I verified by tracing through the loop step by step with a sample input.
Round 4 reinforced how easy it is to introduce false positives when iterating over a mixed-format array. The name table index value of 1 for the second identifier was identical to skipsym, causing a valid program to be rejected. This taught me that any loop over the token list must account for the two-slot structure of identifiers and numbers, not just the parser functions. I confirmed the fix by adding debug output and testing against the known correct input from Appendix C.

---

## AI Tool #2 (if applicable)

### Tool Name

[Tool name if you used a different AI tool]

### Version/Model

[Version/model information]

### Date(s) Used

[Dates when this tool was used]

### Specific Parts of Assignment

[Which parts of the assignment involved this tool]

### Conversation Log

[Use one block per round. Add more rounds as needed.]

#### Round 1

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

#### Round 2

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

#### Round 3 (optional)

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

### Learning & Reflection

## [What you learned from using this tool]

## AI Tool #3 (if applicable)

### Tool Name

[Tool name if you used a third AI tool]

### Version/Model

[Version/model information]

### Date(s) Used

[Dates when this tool was used]

### Specific Parts of Assignment

[Which parts of the assignment involved this tool]

### Conversation Log

[Use one block per round. Add more rounds as needed.]

#### Round 1

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

#### Round 2

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

#### Round 3 (optional)

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

### Learning & Reflection

## [What you learned from using this tool]

## Overall Reflection

[Provide an overall reflection on your AI usage for this assignment]
[Consider: How did AI tools help your learning? What did you understand better? How
did you ensure the work remained your own?]

---

## Notes

- Be as specific and detailed as possible in your responses
- Keep the conversation rounds in order
- Include exact prompts and AI outputs when possible
- Explain how you verified and modified AI-generated content
- Reflect on what you learned through the AI interaction
