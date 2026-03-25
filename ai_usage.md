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

March 24th, 2026

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

**Prompt:** [Exact prompt/question]
**AI Output:** [Summarize or paste the output]
**Verification/Edits:** [How you verified accuracy or modified the output]

#### Round 3 (optional)

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
