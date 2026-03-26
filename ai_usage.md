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

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]





### Learning & Reflection

[Reflect on what you learned and how the AI assistance contributed to your
understanding]
[e.g., I learned about proper error handling techniques and improved my debugging
skills]
[e.g., Understanding of cryptographic concepts was enhanced through AI
explanations]

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
