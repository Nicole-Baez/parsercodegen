# AI Usage Disclosure Details

**Student Name:** Lianet Martin
**Student ID:** 5636501
**Assignment:** Homework 3

---

## Instructions

Complete this template with detailed information about your AI usage. Submit this file along with your signed PDF declaration form. Include the conversation log in order, with each round listing the prompt, AI output, and verification/edits.

---

## AI Tool #1

### Tool Name
Claude

### Version/Model
Sonnet 4.6

### Date(s) Used
March 25, 2026 - April 1, 2026

### Specific Parts of Assignment
Conceptual guidance on PL/0 grammar notation, specifically understanding that square brackets [ ] denote optional elements (zero or one occurrence) in BNF grammar rules such as <const-declaration> and <statement>.
Clarification on the symbol table's mark field — its purpose as an availability flag (0 = active, 1 = out of scope/deleted) and how it is set during symbol insertion and cleaned up at the end of a block.
Conceptual guidance on the role of <factor> in the grammar hierarchy — why identifiers are valid factors, how the parser emits LOD or LIT instructions upon recognizing them, and the recursive relationship between <expression>, <term>, and <factor>.
Code review of the expression() function, identifying a missing unary minus case per the grammar rule <expression> ::= ["-"] <term> { ("+" | "-") <term> }, and guidance on how to handle it with a NEG emit before the first term() call.
Debugging guidance for the skipsym detection loop in program() — specifically the need to increment by 2 (i += 2) when encountering identsym or numbersym tokens, to avoid misreading a name table index as skipsym.

### Conversation Log
[Use one block per round. Add more rounds as needed.]

#### Round 1
**Prompt:** my professor said [] meant 0 or one time , zero doesn't make sense
**AI Output:** Your professor is right, and it does make sense!
"Zero or one time" just means optional. So when you see something like:
<const-declaration> ::= [ "const" ... ]
It means the entire const declaration is optional — a valid PL/0 program doesn't have to have constants declared. Zero times means it just... isn't there at all, and that's still valid.
Same with statement:
<statement> ::= [ <ident> := ... | "begin" ... | "if" ... ]
The whole thing is optional — an empty statement (nothing at all) is legal in PL/0.
So think of [ ] less like "do this zero times" and more like "this might not exist, and that's okay." It's really just a fancy way of saying optional
**Verification/Edits:** Verified by checking the PL/0 grammar provided in the assignment specification

#### Round 2
**Prompt:** explain mark in symboltable
**AI Output:** mark is basically a flag that tells you whether a symbol is still usable or not.

mark = 0 means the symbol is available and can be used
mark = 1 means the symbol is unavailable/deleted and should be ignored

For this assignment since there are no procedures and everything is at level 0, you'll mostly just set mark to 0 when you add a symbol, and then set it to 1 at the end of the block to "clean up" symbols that go out of scope. When you search the symbol table you should skip any entry where mark is 1.
**Verification/Edits:** Verified by checking the insertSymbolTable() and symbolTableCheck() functions. Confirmed that mark = 0 is set on insertion and that the search loop skips entries where mark == 1. The output matched the expected behavior when tested with a simple program.

#### Round 3 (optional)
**Prompt:** why can a factor be an identifier?
**AI Output:** Because in an expression like x + 5, x is a variable whose value needs to be used in the calculation. The factor rule allows identifiers so that when the code generator sees one, it can look it up in the symbol table and emit a LOD instruction to push its value onto the stack, or a LIT if it's a constant.
**Verification/Edits:** The AI's explanation of why identifiers are valid factors matched the grammar definition and the expected code generation behavior. No edits were needed.

#### Round 4 (optional)
**Prompt:**
but aren't factors related by multipliaction
**AI Output:** 
That's a common misconception. Look at the grammar chain:

expression is made of terms connected by + or -
term is made of factors connected by * or /
factor is the base unit — the smallest thing that can be multiplied

So factor isn't multiplication itself, it's just what gets multiplied. It can be an identifier, a number, or a whole parenthesized expression — anything that produces a single value to be used in a term.
**Verification/Edits:** Cross-referenced with the parse tree examples from lecture. Sketched a quick parse tree for x * 5

#### Round 5 (optional)
**Prompt:** 
yes example(x+5)(x+4) are two factors inside each there is a expression with two terms
**AI Output:** 
Exactly! Breaking it down:

(x+5) is one factor, and inside it x and 5 are each a term
(x+4) is another factor, and inside it x and 4 are each a term
The two factors together form a term connected by *

So the grammar is recursive — a factor can contain an expression, which can contain terms, which can contain factors, and so on going as deep as needed.
**Verification/Edits:** Simplified to (2+3) * 4 and traced it manually before applying the same logic to the original example. Confirmed the recursive structure held in both cases, which validated the AI's breakdown.

#### Round 6 (optional)
**Prompt:**
```c
void expression()
{
    term();
    while(tokenList[tokenCounter] == plussym || tokenList[tokenCounter] == minussym){
        if(tokenList[tokenCounter] == plussym){
            getNextToken();
            term();
            emit("ADD", 2, 0, 2);
        }else{
            getNextToken();
            term();
            emit("SUB", 2, 0, 3);
        }
    }
} CHECK
```
**AI Output:** 
Almost correct, but you're missing the unary minus case. The grammar says:
<expression> ::= ["-"] <term> { ("+" | "-") <term> }
The optional "-" at the start means an expression can begin with a negative sign, like -x or -5. You need to check for minussym before the first term() call, and if found, call getNextToken(), call term(), then emit NEG (sub-operation 1).
**Verification/Edits:** Ran the parser on a PL/0 program containing a negative value like x := -5 and observed incorrect output before the fix.

#### Round 7 (optional)
**Prompt:**
intead of / check for skipsym
    for (int i = 0; i < tokenCount; i++)
    {
        if (tokenList[i] == skipsym)
        {
            printf("Error: Scanning error detected by lexer (skipsym present)");
            return;
        }
    }
this I need to i+=2 when an identifier
**AI Output:** 
Yes! Because if an identifier's name table index happens to be 1, your loop would mistake it for skipsym. So you need to skip the second slot for identsym and numbersym the same way getNextToken does.
**Verification/Edits:** Reviewed the getNextToken() function provided in the assignment and confirmed that it skips an extra slot when encountering identsym or numbersym tokens. Applied the same i += 2 logic to the skipsym detection loop to keep it consistent, which resolved the false positive issue.

### Learning & Reflection
I learned how recursive descent parsing maps directly to BNF grammar rules, and how grammar notation like [ ] and { } has precise meaning that directly drives parsing logic — misreading it leads to real implementation errors, like missing the unary minus case in expression(). AI assistance helped me connect abstract grammar notation to concrete C code decisions, particularly around the recursive relationship between <expression>, <term>, and <factor>, and how each function corresponds to exactly one grammar production. I also learned that the token list's two-slot format for identifiers and numbers affects any loop iterating over it, a subtle structural detail I had overlooked that was caught through the skipsym detection discussion. Understanding of symbol table design — specifically the role of the mark field in managing scope — and how code generation integrates with parsing was enhanced through AI explanations that consistently tied each decision back to a specific grammar rule or PM/0 instruction like LOD, LIT, and NEG, rather than just describing behavior in isolation.

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
[What you learned from using this tool]

---

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
[What you learned from using this tool]

---

## Overall Reflection

[Provide an overall reflection on your AI usage for this assignment]
[Consider: How did AI tools help your learning? What did you understand better? How did you ensure the work remained your own?]

---

## Notes

- Be as specific and detailed as possible in your responses
- Keep the conversation rounds in order
- Include exact prompts and AI outputs when possible
- Explain how you verified and modified AI-generated content
- Reflect on what you learned through the AI interaction
