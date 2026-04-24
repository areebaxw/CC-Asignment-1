<div style="color:#000000;">

<h1 align="center">Bottom-Up Parsing Report</h1>
<h3 align="center">SLR(1) and LR(1) Parser Implementation in C++</h3>
<p align="center"><b>Date:</b> April 23, 2026</p>

---

<h2 align="center">1. Introduction</h2>

Bottom-up parsing constructs a parse tree from leaves to root by repeatedly reducing substrings to grammar non-terminals. In shift-reduce parsing, the parser uses a stack and an input buffer, then applies actions from parsing tables:

- `shift`: move next input token to stack
- `reduce`: replace handle with left-hand-side non-terminal
- `accept`: successful parse
- `error`: invalid input

LR parsers are deterministic bottom-up parsers that scan input from left to right and produce a rightmost derivation in reverse. This project implements:

- SLR(1): uses LR(0) items + FOLLOW sets
- LR(1): uses full LR(1) items with lookaheads

Both parsers were built and compared on multiple grammars.

---

<h2 align="center">2. Approach</h2>

### 2.1 Data Structures Used

| Component | File(s) | Data Structure / Class | Purpose |
|---|---|---|---|
| Grammar model | `src/Grammar.h` | `vector<Production>`, `set<Symbol>`, `map<Symbol, set<Symbol>>` | Stores productions, terminals/non-terminals, FIRST/FOLLOW sets |
| LR(0) item | `src/Items.h` | `class LR0Item` | Represents `A -> alpha . beta` |
| LR(1) item | `src/Items.h` | `class LR1Item` | Represents `[A -> alpha . beta, lookahead]` |
| Item sets / states | `src/Items.h` | `LR0ItemSet`, `LR1ItemSet` with `set<Item>` | Canonical collection states |
| Canonical collection | `src/SLRParser.h`, `src/LR1Parser.h` | `vector<LR0ItemSet>` / `vector<LR1ItemSet>` | Stores DFA states |
| Parsing table | `src/ParsingTable.h` | `map<pair<int, Symbol>, Action>`, `map<pair<int, Symbol>, int>` | ACTION and GOTO |
| Parser stack | `src/Stack.h` | `ParsingStack` | State-symbol stack for shift/reduce |
| Parse tree | `src/Tree.h` | `ParseTree`, `TreeNode` | Build and export parse trees |

### 2.2 Algorithm Implementation Details (CLOSURE, GOTO)

#### SLR(1) CLOSURE
For each item with dot before a non-terminal `B`, all productions of `B` are added with dot at start.

#### LR(1) CLOSURE
For each item `[A -> alpha . B beta, a]`, new items are created as `[B -> . gamma, b]` where `b in FIRST(beta a)`.

#### GOTO
For symbol `X`, items with dot before `X` are advanced, then closure is applied.

Pseudo-logic:

```text
CLOSURE(I):
  repeat
    for each item in I with dot before non-terminal B:
      add items for productions of B
  until no new items

GOTO(I, X):
  move dot over X in all matching items
  return CLOSURE(result)
```

### 2.3 Design Decisions and Trade-Offs

| Decision | Benefit | Trade-Off |
|---|---|---|
| Separate parser classes (`SLRParser`, `LR1Parser`) | Clear comparison and modularity | Some duplicated logic |
| Unified `ParsingTable` class | Reuse table display/save logic | Conflict tracking is limited |
| `set`-based item storage | Deterministic ordering and uniqueness | Higher insertion cost than vectors |
| Grammar preprocessing (FIRST/FOLLOW once) | Efficient table generation | More setup complexity |
| Graphviz output integration | Visual debugging and reporting | External dependency for image rendering |

### 2.4 Handling Lookaheads in LR(1) Items

Lookaheads are stored per item (`LR1Item.lookahead`). During closure:

1. Extract `beta` after non-terminal under dot
2. Append current item lookahead `a`
3. Compute `FIRST(beta a)`
4. Create one LR(1) item per terminal in this set

This restricts reduce actions to valid contexts only, avoiding many SLR over-general reductions.

---

<h2 align="center">3. Challenges</h2>

| Challenge | Impact | Solution |
|---|---|---|
| Managing LR(1) state explosion | More states and items for complex grammars | Efficient set-based deduplication and canonical-set equality checks |
| Conflict handling visibility in SLR | Hard to explicitly report conflicts in current table structure | Documented theory conflict cases and compared behavior with LR(1) |
| Input tokenization limitations in interactive mode | Multi-keyword tokens (`if`, `then`, `else`) are hard to test from raw string input | Used grammar-level construction comparison for such grammar; noted parser tokenizer limitation explicitly |
| Maintaining parse tree during reductions | Risk of incorrect child order | Reversed popped node list before attaching children |

---

<h2 align="center">4. Test Cases</h2>

### 4.1 Grammars Tested

| Grammar File | Description |
|---|---|
| `input/grammar1.txt` | Basic expression grammar (`+`) |
| `input/grammar2.txt` | Expression grammar (`+`, `*`, parentheses) |
| `input/grammar3.txt` | Classic LR(1)-stronger-than-SLR grammar (`L=R`, `*R`) |

### 4.2 Test Inputs and Results (5 Strings per Grammar)

#### Grammar 1: `Expr -> Expr + Term | Term`

| Input String | Expected | SLR(1) | LR(1) |
|---|---|---|---|
| `id` | Valid | Accept | Accept |
| `id+id` | Valid | Accept | Accept |
| `id+id+id` | Valid | Accept | Accept |
| `+id` | Invalid | Reject | Reject |
| `id+` | Invalid | Reject | Reject |

#### Grammar 2: includes `*` and parentheses

| Input String | Expected | SLR(1) | LR(1) |
|---|---|---|---|
| `id` | Valid | Accept | Accept |
| `id+id` | Valid | Accept | Accept |
| `id*id` | Valid | Accept | Accept |
| `(id+id)*id` | Valid | Accept | Accept |
| `id+*id` | Invalid | Reject | Reject |

#### Grammar 3: `Start -> L = R | R`, `L -> * R | id`, `R -> L`

| Input String | Expected | SLR(1) | LR(1) |
|---|---|---|---|
| `id=id` | Valid | Accept | Accept |
| `*id=id` | Valid | Accept | Accept |
| `id` | Valid | Accept | Accept |
| `*id` | Valid | Accept | Accept |
| `*=id` | Invalid | Reject | Reject |

### 4.3 Conflict Case: SLR(1) Conflict Resolved by LR(1)

For grammar 3, theoretical parser construction shows that SLR(1) can introduce shift/reduce ambiguity because reductions use FOLLOW sets globally. LR(1) separates contexts using item lookaheads, so reduce actions are placed only for valid lookahead terminals.

In this implementation, this difference appears as larger LR(1) automata/table detail and context-specific reductions.

---

<h2 align="center">5. Comparison Analysis</h2>

### 5.1 SLR(1) vs LR(1) Parsing Power

| Aspect | SLR(1) | LR(1) |
|---|---|---|
| Item type | LR(0) items | LR(1) items with lookahead |
| Reduce condition | FOLLOW(A) | Specific lookahead of each item |
| Conflict tendency | Higher | Lower |
| Grammar coverage | Weaker | Stronger |

### 5.2 Number of States Comparison

Measured from parser runs:

| Grammar | SLR(1) States | LR(1) States | Difference |
|---|---:|---:|---:|
| `grammar1` | 7 | 7 | 0 |
| `grammar2` | 12 | 22 | +10 |
| `grammar3` | 10 | 14 | +4 |
| `grammar4` (build comparison only) | 10 | 17 | +7 |

### 5.3 Table Construction Time (Approx.)

Measured with scripted runs (`Measure-Command`) on the same machine.

| Grammar | SLR(1) Build Time (ms) | LR(1) Build Time (ms) |
|---|---:|---:|
| `grammar1` | 69.20 | 38.50 |
| `grammar2` | 35.90 | 43.08 |
| `grammar3` | 34.83 | 33.27 |

Note: values are approximate CLI timings and include program startup overhead.

### 5.4 Memory Usage Comparison (Proxy by Item Count)

Exact heap profiling was not instrumented; item-count and state-count were used as practical memory proxies.

| Grammar | SLR Items | LR(1) Items | Relative Memory Trend |
|---|---:|---:|---|
| `grammar1` | 14 | 26 | LR(1) higher |
| `grammar2` | 34 | 158 | LR(1) much higher |
| `grammar3` | 22 | 38 | LR(1) higher |

Interpretation: LR(1) consumes more memory due to state splitting and lookahead-carrying items.

---

<h2 align="center">6. Sample Outputs</h2>

### 6.1 Screenshots of Program Execution

Insert screenshots below (generated during runs):

- Menu and parser build output
- Acceptance/rejection traces
- Comparison section output

Example placeholders:

```text
[Insert screenshot: console_run_grammar2.png]
[Insert screenshot: comparison_grammar3.png]
```

### 6.2 Item Sets and Parsing Tables

Generated files:

- `output/slr_items.txt`
- `output/lr1_items.txt`
- `output/slr_parsing_table.txt`
- `output/lr1_parsing_table.txt`

Sample excerpt (format):

```text
I0:
  [ExprPrime -> . Expr, $]
  [Expr -> . Term, $]
  ...
```

### 6.3 Parse Trees for Accepted Strings

Generated parse tree images:

- `output/slr_parse_tree.png`
- `output/lr1_parse_tree.png`

Suggested examples to include in final document:

- Parse tree for `id+id` (grammar1)
- Parse tree for `(id+id)*id` (grammar2)

### 6.4 Conflict Examples and Resolutions

- Conflict grammar used: `input/grammar3.txt`
- SLR(1): broader reduce placement from FOLLOW-based logic
- LR(1): context-specific reduce placement via lookaheads
- Observed implementation effect: LR(1) canonical collection and table are larger but more precise

---

<h2 align="center">7. Conclusion</h2>

This project demonstrates practical implementation of bottom-up parsing with both SLR(1) and LR(1). Key takeaways:

1. LR(1) is more powerful because lookaheads preserve local context for reductions.
2. SLR(1) is simpler and often smaller, but can be less precise for ambiguous contexts.
3. Canonical collection growth is the main cost in LR(1), affecting table size and memory.
4. Visualization (state diagrams, parse trees, parsing traces) significantly improves debugging and understanding.
5. Robust tokenization is important for realistic grammars (especially keyword-heavy grammars like dangling-else).

Overall, implementing both parsers provided strong insight into compiler front-end construction, practical trade-offs, and conflict handling strategies.

</div>
