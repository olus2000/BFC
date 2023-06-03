#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Q - next in linked list
// W - next in tree
// E - ref count
// R - jump address (?)
typedef struct c_ {
    struct c_ *Q, *W;
    int E, R;
} Z;

long ip, _A;

int _B[10000];

int op[60000] = {32};   // Memory
// 0: here
// 1: return stack pointer
// 2: zero (for the sake of word finding)
// 32+: dictionary
int L = 1;              // Dictionary pointer
int t = 64;             // Name space pointer
// Dictionary structure: previous entry, name pointer, execution token
int w;
int STK[10000], *sp = STK;  // Data stack
int top;

char s[10000];          // Stores dictionary names and input buffer

Z _R = {.E=999};

// Three linked lists: recyclable nodes, S nodes and W nodes
// W nodes are terminated with _R
Z *Y, *S, *W = &_R;

FILE *in;

// Garbage Collector
void G(Z *p) {
    for (; p && p != &_R; p = p->W) {
        if (--p->E) break;
        G(p->Q);
        p->Q = Y;
        Y = p;
    }
}

// Make definition
void def(int x) {
    op[op[0]++] = L;            // Compile current head of dictionary
    L = *op - 1;                // Update head of dictionary
    op[op[0]++] = t;            // Compile address of name
    op[op[0]++] = x;            // Compile execution token
    fscanf(in, "%s", s + t);    // Parse name to the name space
    t += strlen(s + t) + 1;     // Update name space pointer
}

char get_digit_parity() {
    int c;
    while ((c = fgetc(in)) != EOF && (c < '0' || c > '9'));
    return c == EOF? -1 : c & 1;
}

// If first is odd then pushes 1 and number of following evens
// If second is odd then pushes 2 and number of numbers pushed by recursive call
// Otherwise pushes 3
//
// 1. Compiles a call
// 2. Compiles creation pointing after the next 1 command
// 3. Compiles moving a cell from S to W and continues
//
// A _P that compiled a 2 must not reach EOF
// Grammar is:
//  Program := 2 { 3 | Program } 1
//  Program := '(' { thing | Program } ')'
int _P() {
    int t, b;
    int s = _A;
    int m = 0;
    for (;;) {
        if ((b = get_digit_parity()) == -1) {
            if (!m) break;
        } else if (b) {
            for (t = 0; get_digit_parity();) ++t;
            _B[_A++] = 1;
            _B[_A++] = t;
            break;
        } else if (get_digit_parity()) {
            t = _A;
            _A += 2;
            _B[t] = 2;
            _B[t+1] = _P();
            m = 1;
        } else {
            _B[_A++] = 3;
        }
    }
    return _A - s;
}

int max_stack_height;

// Execute execution token
int exec(int x) {
    printf("Executing %d at %d top: %d ip: %d rsp: %d\n",
            op[x], x, top, ip, op[1]);
    if (op[0] >= 60000) printf("Out of memory!\n");
    if (op[0] < 0) printf("Memory underflow!\n");
    if (op[1] > max_stack_height) max_stack_height = op[1];
    switch (op[x++]) {
        ;break;case 0:      // Push literal
            *++sp = top;
            top = op[ip++]
        ;break;case 1:      // Compile data field adress?
            op[op[0]++] = x
        ;break;case 2:      // Enter
            op[++op[1]] = ip;
            ip = x
        ;break;case 3:      // Define a word that enters
            def(1);
            op[op[0]++] = 2
        ;break;case 4:      // Make immediate? So jank.
            *op -= 2;
            op[op[0]++] = 2;
        ;break;case 5:      // Read word, find it in the dictionary and call it
            for (w = fscanf(in, "%s", s) < 1?
                    printf("MSH: %d\n", max_stack_height), exit(0), 0 : L;
                 strcmp(s, &s[op[w+1]]);
                 w = op[w]);
            w - 1? exec(w + 2) : (op[op[0]++] = 2, op[op[0]++] = atoi(s));
            //if (!(w-1)) printf("%s %d\n", s, atoi(s));
            // If word not found compiles a FUCKING PUSH WHY AM I SO BAD
        ;break;case 6:      // @ 48
            top = op[top]
        ;break;case 7:      // ! 52
            op[top] = *sp--;
            top = *sp--
        ;break;case 8:      // - 56
            top = *sp-- - top
        ;break;case 9:      // * 60
            top *= *sp--;
        ;break;case 10:     // / 64
            printf("Dividing %d by %d\n", *sp, top);
            top = *sp-- / top
        ;break;case 11:     // 0<= 68
            top = top <= 0;
        ;break;case 12:     // return 72
            ip = op[op[1]--]
        ;break;case 13:     // emit 76
            //putchar(top);
            printf("%d[%d]\n", top, sp - STK);
            top = *sp--
        ;break;case 14:     // getchar 80
            *++sp = top;
            top = getchar();
        ;break;case 15:     // pick 84
            top = sp[-top];
    }
}

int fth() {
    printf("Running \"\"\"Forth\"\"\" system...\n");
    def(3);  // Define a defining word (32-34)
    def(4);  // Define IMMEDIATE (35-37)
    def(1);  // Define a word that compiles address of `execute loop` (38-40)
    w = *op;
    op[op[0]++] = 5;    // 41
    op[op[0]++] = 2;    // 42
    ip = *op;           // Start execution from address of `execute loop`
    op[op[0]++] = w;    // 43
    op[op[0]++] = ip - 1;   // 44   Address of `loop` in `execute loop`
    // Define words that compile basic ops
    for (w = 6; w < 16;) def(1), op[op[0]++] = w++;
    // 45, 49, 53, 57, 61, 65, 69, 73, 77, 81 - 85
    op[1] = *op;    // Allocate 512 cells for return stack 85-596
    *op += 512;     // 597
    for (;;) exec(op[ip++]);
}

int main (int _, char *v[]) {
    int XXX = 2;
    printf("Does this even do anything?\n");
    long i;
    char *k = "CVDCJBADCDCCBADCECBBAAAC";
    // Set _B to [2 21 3 2 9 1 0 3 2 3 2 2 1 0 3 2 4 2 1 1 0 0 0 2 _P()]
    for (_A = 0; _A < 24; ++_A) _B[_A] = k[_A] - 'A';
    printf("_B processed\n");
    ++_A; // _A = 25
    in = fopen(v[1], "rb");
    _B[24] = _P();
    printf("Pee done\n");
    char o[ftell(in)];
    int po = 0;
    rewind(in);
    Z *t,*e;
    for (;;) switch (_B[ip]) {
        ;break;case 0:
            //printf("0 %d >>> %d\n", ip, ip & 1);
            //if (!--XXX) exit(0);
            o[po++] = ip & 1;   // Push parity of ip to o
            ip = 2;             // Jump to 2
            G(W);               // Collect W
            W = &_R;            // Reset W
            break;
        ;break;case 1:  // Move W x times down the W list and jump
            //printf("1 %d\n", ip);
            e = t = W;          // Copy W twice
            // Make e go deep into its Q
            for (int i = 0; i < _B[ip+1] && e != &_R; ++i) e = e->Q;
            ip = e->R;
            W = (++(e->W)->E, e->W);    // Set W to e->W and inc its ref count
            G(t);               // Collect old W
            break;
        ;break;case 2:  // Add cell to S list
            //printf("2 %d\n", ip);
            // If can't be recycled then create new
            if (!(t = Y)) {
                t = calloc(1, sizeof(Z));
            }
            Y = t->Q;
            t->E = 1; // Ref count!
            t->R = ip + 2 + _B[ip+1]; // First one gets 23
            t->W = t->R > _B[1] && t->R != _A? (++W->E, W) : &_R;
            t->Q = S;
            S = t;
            ip += 2;
        ;break;case 3: // Move cell from S list to W list or do The Test
            //printf("3 %d\n", ip);
            if (!S) {
                printf("Running The Test\n");
                long s = 0L, ind = 0, c;
                while (!feof(in)) {
                    c = fgetc(in);
                    if (c == EOF) break;
                    s += c;
                    //if (isdigit(c))
                    //    printf("Digit is %c and o is %d\n", c, o[ind]);
                    if (isdigit(c)) if (o[ind++] != (c & 1)) {
                        printf("Failed at char %d : %d\n", ftell(in), ind-1);
                        exit(1);
                    }
                }
                printf("Sum: %d, o: %d\n", s, o[ind]);
                if (s % 2 != o[ind]) exit(1);
                rewind(in);
                ip = 0;
                fth();
                exit(0);
            }
            Z *t = S;
            S = t->Q;
            t->Q = W;
            W = t;
            ++ip;
    }
}
