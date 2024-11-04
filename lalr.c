// CS22B2053
// DHIVYA DHARSHAN V

// <------| LALR PARSER |------>

// NOTE: ACCEPT state  is  denoted with '100'
//       ERROR  states are denoted with '0'
//       SHIFT  states are denoted with '-ve numbers'
//       GOTO   states are denoted with '+ve numbers'  
//       REDUCE states are denoted with '+ve numbers'  

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define SIZE 20
#define ACC 100   
#define ERR 0       

char first[SIZE][SIZE];
int first_idx = 0;
int first_ptr = 0;

int has_ele = 0;
int has_hash = 0;

int limit = 0;

int n_ters;
char ters[SIZE];
int n_non_ters;
char non_ters[SIZE];
int n;
char prods[SIZE][SIZE];
int n_prod[SIZE];   // No.of productions for each 'I'
char prod_I[SIZE][SIZE][SIZE];
int n_I = 0;
int parse_tbl[SIZE][SIZE];
int end = 0;
int goto_comp[SIZE] = {0};  // goto computation completed

char lookahead_I[SIZE][SIZE][SIZE];

int red_sts[SIZE];  // reduced states
int n_red_st = 0;

char stk[SIZE]; // stack
int top = -1;

char inp[SIZE]; // input string
int len = 0;
int ptr = 0;
int iter = 1;   // iteration

int same_I[SIZE][2];
int same_idx = 0;

int merge_parse_tbl[SIZE][SIZE];
int merge_idx = 0;
int mapping[SIZE];

void add_ter(char ter) {    // add terminal
    for(int i = 0; i < strlen(first[first_idx]); i++) {
        if(ter == first[first_idx][i]) {
            return;
        }
    }
    first[first_idx][first_ptr++] = ter;

    return;
} 

void comp_first(char non_ter) { // compute first
    printf("\n<------| comp_first() IN |------>\n");

    char idx;
    for(int i = 0; i < n; i++) {
        if(non_ter == prods[i][0]) {
            idx = i;

            for(int i = 3; i < strlen(prods[idx]); i++) {
                if(prods[idx][i] == '|') {
                    continue;
                }

                if(prods[idx][i] == '#') {
                    if(!has_ele) {
                        if((i + 1 == strlen(prods[idx]))) {
                            add_ter('#');
                        } else if(prods[idx][i + 1] == '|') {
                            add_ter('#');
                            i++;
                        }
                    } else {
                        has_hash = 1;
                    }

                    // for(int i = 0; i < first_ptr; i++) {
                    //     printf("%c ", first[first_idx][i]);
                    // } 
                } else if(isupper(prods[idx][i])) {
                    if(prods[idx][i] == prods[idx][0]) {
                        while((i < strlen(prods[idx])) && (prods[idx][i] != '|')) {
                            i++;
                        } 
                        continue;
                    }

                    if((i + 1 != strlen(prods[idx])) && prods[idx][i + 1] != '|') {
                        has_ele = 1;
                    } else {
                        has_ele = 0;
                    }
                    comp_first(prods[idx][i]);
                
                    if(has_ele && has_hash) {
                        has_ele = 0;
                        has_hash = 0;
                        continue;
                    } else {
                        while((i < strlen(prods[idx])) && (prods[idx][i] != '|')) {
                            i++;
                        } 
                    }
                    has_ele = 0;
                    has_hash = 0;
                    continue;
                    /*if(islower(c)) {
                        first[first_idx][first_ptr++] = c;

                        while((i < strlen(prods[idx])) && (prods[idx][i] != '|')) {
                            i++;
                        } 
                        continue;
                    } else if(c == '#') {
                        if((i + 1 == strlen(prods[idx]))) {
                            first[first_idx][first_ptr++] = c;
                        } else if(prods[idx][i + 1] == '|') {
                            first[first_idx][first_ptr++] = c;
                        } else {
                            continue;
                        }
                    }*/
                } else {
                    add_ter(prods[idx][i]);

                    // for(int i = 0; i < first_ptr; i++) {
                    //     printf("%c ", first[first_idx][i]);
                    // }

                    while((i < strlen(prods[idx])) && (prods[idx][i] != '|')) {
                        i++;
                    } 
                    continue;
                }
            }
            
            // break;
        }
    }

    printf("\n<------| comp_first() OUT |------>\n");

    // if(limit == 10) {
    //     printf("\nhi\n");
    //     exit(EXIT_FAILURE);
    // }
    // printf("\nLimit: %d", limit);
    // limit++;

    return;
}

void find_ters_n_non_ters() {   // find terminals and non terminals
    printf("\n<------| find_ters_n_non_ters() IN |------>\n");

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < strlen(prods[i]); j++) {
            if(prods[i][j] == '|' || prods[i][j] == '#') {
                continue;
            }
            
            if(isupper(prods[i][j])) {
                int flg = 1;
                for(int k = 0; k < n_non_ters; k++) {
                    if(prods[i][j] == non_ters[k]) {
                        flg = 0;
                        break;
                    }
                }
                
                if(flg) {
                    non_ters[n_non_ters++] = prods[i][j];
                }
            } else {
                int flg = 1;
                for(int k = 0; k < n_ters; k++) {
                    if(prods[i][j] == ters[k]) {
                        flg = 0;
                        break;
                    }
                }
                
                if(flg) {
                    ters[n_ters++] = prods[i][j];
                }
            }
            
            if(j == 0) {
                j += 2;
            }
        }
    }
    ters[n_ters++] = '$';

    printf("\n<------| find_ters_n_non_ters() OUT |------>\n");

    return;
}

int find_non_ter_idx(char non_ter) {    // find non terminal index
    for(int i = 0; i < n_non_ters; i++) {
        if(non_ter == non_ters[i]) {
            return i;
        }
    }
    return -1;
}

int find_ter_idx(char ter);

int is_dup(int i_val, int j_val) {
    // printf("\n<--| I %d |-->\n", i_val);
    for(int i = 0; i < n_I; i++) {
        if(i_val > i) {
            for(int j = 0; j < n_prod[i]; j++) {
                if(!strcmp(prod_I[i_val][j_val], prod_I[i][j])) {
                    if(!strcmp(lookahead_I[i_val][j_val], lookahead_I[i][j])) {
                        // printf("\n<--| i %d |-->\n", i);
                        // printf("\nprod_I: %s | prod_I_tbl: %s\n", prod_I[i_val][j_val], prod_I[i][j]);
                        // printf("\nlook_I: %s | look_I_tbl: %s\n", lookahead_I[i_val][j_val], lookahead_I[i][j]);
                        char c;
                        for(int k = 0; k < strlen(prod_I[i_val][j_val]); k++) {
                            if(prod_I[i_val][j_val][k] == '.') {
                                if(k + 1 < strlen(prod_I[i_val][j_val])) {
                                    c = prod_I[i_val][j_val][k + 1];
                                    // printf("%c\n", c);
                                }
                            } 
                        }
                        if(isupper(c)) {
                            parse_tbl[i_val][find_non_ter_idx(c) + n_ters] = parse_tbl[i][find_non_ter_idx(c) + n_ters];
                        } else {
                            parse_tbl[i_val][find_ter_idx(c)] = parse_tbl[i][find_ter_idx(c)];
                            // printf("\ni: %d | idx: %d\n", i, find_ter_idx(c));
                            // printf("val: %d\n", parse_tbl[i][find_ter_idx(c)]);
                        }

                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void closure_other();

void closure(char* prod) {  // I0 computation
    if(n_I == 0) {
        prod_I[n_I][0][0] = 'Z';
        prod_I[n_I][0][1] = '-';
        prod_I[n_I][0][2] = '>';
        prod_I[n_I][0][3] = '.';
        prod_I[n_I][0][4] = prod[0];
        // char lookahead[] = "$";
        lookahead_I[n_I][0][0] = '$';
    }
    n_prod[n_I]++;
    // printf("check");
    closure_other();
/*
    char curr = prod[0];
    char nxt_look[SIZE];
    strcpy(nxt_look, "$");

    n_prod[n_I] = 0;
    while(isupper(curr)) {
        n_prod[n_I]++;

        // printf("\n%s", nxt_look);
        int look_idx = 0;
        for(int i = 0; i < strlen(nxt_look); i++) {
            char nxt_look_ter[SIZE];
            // printf("%c\n", nxt_look[i]);
            if(isupper(nxt_look[i])) {
                strcpy(nxt_look_ter, first[find_non_ter_idx(nxt_look[i])]);

                for(int j = 0; j < strlen(nxt_look_ter); j++) {
                    lookahead_I[n_I][n_prod[n_I]][look_idx++] = nxt_look_ter[j];
                }
            } else {
                // printf("%c", nxt_look[i]);
                nxt_look_ter[0] = nxt_look[i];
                lookahead_I[n_I][n_prod[n_I]][look_idx++] = nxt_look_ter[0];
            }
        }
        memset(nxt_look, 0, sizeof(nxt_look));
        // printf("check");
        int idx;
        for(int i = 0; i < n; i++) {
            if(prods[i][0] == curr) {
                idx = i;
                break;
            }
        }

        int ptr = 0;
        for(int i = 0; i < strlen(prods[idx]); i++) {
            prod_I[n_I][n_prod[n_I]][ptr++] = prods[idx][i];

            if(prods[idx][i] == '>') {
                prod_I[n_I][n_prod[n_I]][ptr++] = '.';

                curr = prods[idx][i + 1];
            }
        }
        prod_I[n_I][n_prod[n_I]][ptr] = '\0';

        int j = 0;
        // printf("check");
        while(prod_I[n_I][n_prod[n_I]][j] != '.') {
            j++;
        }
        j++;
        // printf("check");
        if(j + 1 <= strlen(prod_I[n_I][n_prod[n_I]])) {
            nxt_look[0] = prod_I[n_I][n_prod[n_I]][j + 1];
        } else {
            strcpy(nxt_look, lookahead_I[n_I][n_prod[n_I]]);
        }
        // printf("check");
    }
    n_prod[n_I]++;
    n_I++;
*/
    return;
}

void closure_other() {    // computation of Is other than I0
    // printf("\n<--| nI: %d |-->\n", n_I);

    int curr_n_prod = 0;
    while(curr_n_prod < n_prod[n_I]) {
        char prod[SIZE];
        strcpy(prod, prod_I[n_I][curr_n_prod]);
        // printf("Prod: %s\n", prod);

        char curr;
        for(int i = 0; i < strlen(prod); i++) {
            if(prod[i] == '.') {
                curr = prod[i + 1];
                break;
            }
        }
        // printf("prodI: %s\n", prod_I[n_I][n_prod[n_I]]);
        char nxt_look[SIZE];
        memset(nxt_look, 0, sizeof(nxt_look));
        int j = 0;
        // printf("check");
        while(prod_I[n_I][curr_n_prod][j] != '.') {
            j++;
        }
        j++;
        // printf("check");
        if(j + 1 <= strlen(prod_I[n_I][curr_n_prod]) - 1) {
            // printf("check");
            nxt_look[0] = prod_I[n_I][curr_n_prod][j + 1];
        } else {
            strcpy(nxt_look, lookahead_I[n_I][curr_n_prod]);
        }
        // n_prod[n_I]++;
        // strcpy(nxt_look, lookahead_I[n_I][0]);
        // printf("Next look: %s\n", nxt_look);

        // int look_idx = 0;
        char nxt_look_ter[SIZE];
        memset(nxt_look_ter, 0, sizeof(nxt_look_ter));
        for(int i = 0; i < strlen(nxt_look); i++) {
            // printf("%c\n", nxt_look[i]);
            if(isupper(nxt_look[i])) {
                strcpy(nxt_look_ter, first[find_non_ter_idx(nxt_look[i])]);

                // for(int j = 0; j < strlen(nxt_look_ter); j++) {
                //     lookahead_I[n_I][n_prod[n_I]][look_idx++] = nxt_look_ter[j];
                // }
                break;
            } else {
                // printf("%c\n", nxt_look[i]);
                nxt_look_ter[i] = nxt_look[i];
                // printf("Look: %s\n", nxt_look_ter);
                // lookahead_I[n_I][n_prod[n_I]][look_idx++] = nxt_look_ter[0];
            }
        }

        char new_curr;
        for(int i = 0; i < n; i++) {
            // printf("Nonter: %c | Curr: %c\n", prods[i][0], curr);
            if(prods[i][0] == curr) {
                // printf("Prod: %s\n", prods[i]);
                int idx = i;
                    
                int ptr = 0;
                for(int i = 0; i < strlen(prods[idx]); i++) {
                    prod_I[n_I][n_prod[n_I]][ptr++] = prods[idx][i];

                    if(prods[idx][i] == '>') {
                        prod_I[n_I][n_prod[n_I]][ptr++] = '.';

                        new_curr = prods[idx][i + 1];
                        // printf("Curr: %c\n", curr);
                    }
                }
                prod_I[n_I][n_prod[n_I]][ptr] = '\0';
                strcpy(lookahead_I[n_I][n_prod[n_I]], nxt_look_ter);
                // if(!is_dup()) {
                    // printf("ProdI: %s\n", prod_I[n_I][n_prod[n_I]]);
                n_prod[n_I]++;
                // } else {
                //     memset(prod_I[n_I][n_prod[n_I]], 0, sizeof(prod_I[n_I][n_prod[n_I]]));
                //     memset(lookahead_I[n_I][n_prod[n_I]], 0, sizeof(lookahead_I[n_I][n_prod[n_I]]));
                // }
            }
        }
        curr = new_curr;
        // memset(nxt_look, 0, sizeof(nxt_look));
        curr_n_prod++;
    }
    n_I++;

    return;
}

void go_to(char *prod, int i_val, int j_val) {  // goto
    strcpy(lookahead_I[n_I][n_prod[n_I]], lookahead_I[i_val][j_val]);

    char goto_ele;
    n_prod[n_I] = 0;
    int flag = 0;
    int ptr = 0;

    if(!is_dup(i_val, j_val)) {
        for(int i = 0; i < strlen(prod); i++) {
            if(prod[i] == '.') {
                goto_ele = prod[i + 1];
                prod_I[n_I][n_prod[n_I]][ptr++] = prod[i + 1];        
                prod_I[n_I][n_prod[n_I]][ptr++] = '.';
                if((i < strlen(prod)) && (isupper(prod[i + 2]))) {
                    flag = 1;
                }

                i++;
            } else {
                prod_I[n_I][n_prod[n_I]][ptr++] = prod[i];
            } 
        }
        prod_I[n_I][0][ptr] = '\0';
        n_prod[n_I]++;

        for(int i = 0; i < n_ters; i++) {
            if(goto_ele == ters[i]) {
                parse_tbl[i_val][i] = -n_I;
            }
        }

        for(int i = 0; i < n_non_ters; i++) {
            if(goto_ele == non_ters[i]) {
                parse_tbl[i_val][n_ters + i] = n_I;
            }
        }

        if(flag) {
            closure_other();
            // n_prod[n_I]++;
        } else {
            n_I++;
        }
    } else {
        memset(lookahead_I[n_I][n_prod[n_I]], 0, sizeof(lookahead_I[n_I][n_prod[n_I]]));
    }

    return;
}

void init_parse_tbl() { // initialize parse table
    printf("\n<------| init_parse_tbl() IN |------>\n");

    for(int i = 0; i < n_I; i++) {
        for(int j = 0; j < n_ters + n_non_ters; j++) {
            parse_tbl[i][j] = ERR; 
        }
    }

    parse_tbl[1][n_ters - 1] = ACC; 

    printf("\n<------| init_parse_tbl() OUT |------>\n");

    return;
}

void display_parse_tbl() {  // display parse table
    printf("\n<------| display_parse_table() IN |------>\n\n");

    printf("  \t");
    for(int i = 0; i < n_ters; i++) {
        printf("%c\t", ters[i]);
    }
    for(int i = 0; i < n_non_ters; i++) {
        printf("%c\t", non_ters[i]);
    }
    printf("\n");
    for(int i = 0; i < n_I; i++) {
        printf("I%d\t", i);

        for(int j = 0; j < n_ters + n_non_ters; j++) {
            printf("%d\t", parse_tbl[i][j]);
        }
        printf("\n");
    }

    printf("\n<------| display_parse_table() OUT |------>\n");

    return;
}

int find_ter_idx(char ter) {    // find terminal index
    for(int i = 0; i < n_ters; i++) {
        if(ter == ters[i]) {
            return i;
        }
    }
    return -1;
}

void find_red_st() {    // find reduced states
    printf("\n<------| find_red_st() IN |------>\n");

    for(int i = 0; i < n_I; i++) {
        if(n_prod[i] == 1 && i != 1) {
            if(prod_I[i][0][strlen(prod_I[i][0]) - 1] == '.') {
                red_sts[n_red_st++] = i;
            }
        }
    }

    // for(int i = 0; i < n_red_st; i++) {
    //     printf("%d\n", red_sts[i]);
    // }

    for(int i = 0; i < n_red_st; i++) {
        char red_st[SIZE];
        strcpy(red_st, prod_I[red_sts[i]][0]);
        red_st[strlen(red_st) - 1] = '\0';
        // printf("%s\n", red_st);
        for(int j = 0; j < n; j++) {
            if(!strcmp(red_st, prods[j])) {
                for(int k = 0; k < strlen(lookahead_I[red_sts[i]][0]); k++) {
                    parse_tbl[red_sts[i]][find_ter_idx(lookahead_I[red_sts[i]][0][k])] = j + 1; 
                }
            }
        }
    }

    printf("\n<------| find_red_st() OUT |------>\n");

    return;
}

void push(char ter) {
    stk[++top] = ter;

    return;
}

void pop() {
    stk[top--] = '\0';

    return;
}

char peek() {
    return stk[top];
}

void display_parse() {
    printf("\n\t\t\t<--| ITERATION %d |-->\n", iter++);
    printf("STACK: %s", stk);
    printf("\t\tSTRNG: ");
    for(int i = 0; i < strlen(inp); i++) {
        if(i == ptr) {
            printf("~>");
        }
        printf("%c", inp[i]);
    }
    printf("\t\tACTION: ");
    // printf("\n");

    return;
}

int get_map_idx(int num) {
    for(int i = 0; i < merge_idx; i++) {
        if(num == mapping[i]) {
            return i;
        }
    }
    return -1;
}

int parsing() {
    printf("\n<------| parsing() IN |------>\n");

    push('$');
    push('0');

    int action;
    while(action != ACC) {  // ACCEPT = 100
        display_parse();

        action = merge_parse_tbl[get_map_idx(peek() - '0')][find_ter_idx(inp[ptr])];
        // printf("%d\n", action);
        if(action == 100) {
            printf("ACCEPT\n");

            return 1;
        } else if(action < 0) { // shift operation
            printf("SHIFT %d\n", abs(action));

            push(inp[ptr]);
            push(abs(action) + '0');
            ptr++;
        } else if(action > 0) { // reduce operation
            printf("REDUCE %d\n", action);

            int pop_count = 0;
            for(int i = 0; i < strlen(prods[action - 1]); i++) {
                if(prods[action - 1][i] == '>') {
                    pop_count = -1;
                }
                pop_count++;
            }
            pop_count *= 2;

            for(int i = 0; i < pop_count; i++) {
                pop();
            }
            char r = peek();
            push(prods[action - 1][0]);
            char c = peek();

            push(merge_parse_tbl[get_map_idx(r - '0')][find_non_ter_idx(c) + n_ters] + '0');
        } else {    // ERROR
            printf("ERROR\n");

            return 0;
        }
    }
    display_parse();

    printf("\n<------| parsing() OUT |------>\n");

    return 0;
}

void display_merge_parse_tbl() {  // display parse table
    printf("\n<------| display_merge_parse_table() IN |------>\n\n");

    printf("  \t");
    for(int i = 0; i < n_ters; i++) {
        printf("%c\t", ters[i]);
    }
    for(int i = 0; i < n_non_ters; i++) {
        printf("%c\t", non_ters[i]);
    }
    printf("\n");
    for(int i = 0; i < n_I; i++) {
        int flag = 1;
        for(int k = 0; k < same_idx; k++) {
            if(i == same_I[k][1]) {
                printf("I%d\t", same_I[k][0]);
                flag = 0;
            }
        }
        if(flag) {
            printf("I%d\t", i);
        }

        for(int j = 0; j < n_ters + n_non_ters; j++) {
            printf("%d\t", parse_tbl[i][j]);
        }
        printf("\n");
    }

    printf("\n<------| display_merge_parse_table() OUT |------>\n");

    return;
}

void display_merged_parse_tbl() {  // display parse table
    printf("\n<------| display_merged_parse_table() IN |------>\n\n");

    printf("  \t");
    for(int i = 0; i < n_ters; i++) {
        printf("%c\t", ters[i]);
    }
    for(int i = 0; i < n_non_ters; i++) {
        printf("%c\t", non_ters[i]);
    }
    printf("\n");
    for(int i = 0; i < merge_idx; i++) {
        printf("I%d\t", mapping[i]);

        for(int j = 0; j < n_ters + n_non_ters; j++) {
            printf("%d\t", merge_parse_tbl[i][j]);
        }
        printf("\n");
    }

    printf("\n<------| display_merged_parse_table() OUT |------>\n");

    return;
}


void merging() {
    printf("\n<------| merging() IN |------>\n");

    int same = 1;
    for(int i1 = 0; i1 < n_I; i1++) {
        for(int i2 = i1 + 1; i2 < n_I; i2++) {
            same = 1;
            if(n_prod[i1] == n_prod[i2]) {
                // printf("\nN_prod1 = %d | N_prod2 = %d | i1 = %d | i2 = %d\n", n_prod[i1], n_prod[i2], i1, i2);
                for(int j = 0; j < n_prod[i1]; j++) {
                    if(strlen(prod_I[i1][j]) == strlen(prod_I[i2][j])) {
                        // printf("\nprod_I[i1][j] = %s | prod_I[i2][j] = %s | i1 = %d | i2 = %d\n", prod_I[i1][j], prod_I[i2][j], i1, i2);
                        for(int k = 0; k < strlen(prod_I[i1][j]); k++) {
                            if(prod_I[i1][j][k] == prod_I[i2][j][k]) {
                                continue;
                            } else {
                                same = 0;
                                // break;
                            }
                        }
                    } else {
                        same = 0;
                        // break;
                    }
                }
            } else {
                same = 0;
                // break;
            }

            if(same) {
                same_I[same_idx][0] = i1;
                same_I[same_idx][1] = i2;
                same_idx++;
            }
        }
    }

    printf("\n<--| Is TO CLUB |-->\n");
    for(int i = 0; i < same_idx; i++) {
        printf("I%d\tI%d\n", same_I[i][0], same_I[i][1]);
    }

    for(int i = 0; i < n_I; i++) {
        for(int j = 0; j < n_ters + n_non_ters; j++) {
            for(int k = 0; k < same_idx; k++) {
                if(abs(parse_tbl[i][j]) == same_I[k][1]) {
                    if(parse_tbl[i][j] > 0) {
                        parse_tbl[i][j] = same_I[k][0];
                    } else {
                        parse_tbl[i][j] = -same_I[k][0];
                    }
                }
            }
        }
    }

    display_merge_parse_tbl();

    for(int i = 0; i < n_I; i++) {
        int flag = 0, k = 0;
        for(k = 0; k < n_ters + n_non_ters; k++) {
            if(parse_tbl[i][k] != 0) {
                flag = 1;
            }
        }
        if(!flag) {
            continue;
        }

        for(k = 0; k < same_idx; k++) {
            if(i == same_I[k][0]) {
                flag = 1;
                break;
            }
        }
        if(flag) {
            for(int j = 0; j < n_ters + n_non_ters; j++) {
                if(parse_tbl[i][j] == parse_tbl[same_I[k][1]][j]) {
                    merge_parse_tbl[merge_idx][j] = parse_tbl[i][j];
                } else if(parse_tbl[i][j] == 0) {
                    merge_parse_tbl[merge_idx][j] = parse_tbl[same_I[k][1]][j];
                } else {
                    merge_parse_tbl[merge_idx][j] = parse_tbl[i][j];
                }
            }
            memset(parse_tbl[same_I[k][1]], 0, sizeof(parse_tbl[same_I[k][1]]));

            mapping[merge_idx] = i;
            merge_idx++;
        } else {
            for(int j = 0; j < n_ters + n_non_ters; j++) {
                merge_parse_tbl[merge_idx][j] = parse_tbl[i][j];
            }
            mapping[merge_idx] = i;
            merge_idx++;
        }
    }

    printf("\n<--| MAPPING |-->\n");
    for(int i = 0; i < merge_idx; i++) {
        printf("%d -> %d\n", i, mapping[i]);
    }

    display_merged_parse_tbl();

    printf("\n<------| merging() OUT |------>\n");

    return;
}

int main() {
    printf("\n<------| main() IN |------>\n");

    printf("\nEnter no.of productions (Ex: 3) : ");
    scanf("%d", &n);

    printf("\n<--| ENTER PRODUCTIONS | Ex: S->AB {enter} A->a {enter} B->b |-->\n");
    for(int i = 0; i < n; i++) {
        scanf("%s", prods[i]);
    }

    find_ters_n_non_ters();
    printf("\nTerminals: ");
    for(int i = 0; i < n_ters; i++) {
        printf("%c ", ters[i]);
    }
    printf("\nNon-terminals: ");
    for(int i = 0; i < n_non_ters; i++) {
        printf("%c ", non_ters[i]);
    }
    printf("\n");

    for(int i = 0; i < n_non_ters; i++) {
        comp_first(non_ters[i]);
        first_idx++;
        first_ptr = 0;
    }

    printf("\n<--| FIRST |-->\n");
    for(int i = 0; i < n_non_ters; i++) {
        printf("First(%c) = {%s}\n", non_ters[i], first[i]);
    }

    init_parse_tbl();

    printf("\n\n<--| I0 |-->\n");
    closure(prods[0]);
    for(int i = 0; i < n_I; i++) {
        for(int j = 0; j < n_prod[i]; j++) {
            printf("%s", prod_I[i][j]);
            printf(", %s\n", lookahead_I[i][j]);
        }
    }

    int temp = n_I;
    while(!end) {
        end = 1;
        for(int i = 0; i < temp; i++) {
            if(!goto_comp[i]) {
                end = 0;
    
                for(int j = 0; j < n_prod[i]; j++) {
                    if(prod_I[i][j][strlen(prod_I[i][j]) - 1] != '.') {
                        go_to(prod_I[i][j], i, j);
                    }
                }
            }
            goto_comp[i] = 1;
        }

        temp = n_I;
    }

    for(int i = 0; i < n_I; i++) {
        printf("\n<--| I%d |-->\n", i);
        for(int j = 0; j < n_prod[i]; j++) {
            printf("%s", prod_I[i][j]);
            printf(", %s\n", lookahead_I[i][j]);
        }
    }

    find_red_st();

    display_parse_tbl();

    merging();

    printf("\nEnter input string (Ex: ab) : ");
    scanf("%s", inp);
    inp[strlen(inp)] = '$';
    len = strlen(inp);
    inp[len] = '\0';
    // printf("%s", inp);

    int success = parsing();
    if(success) {
        printf("\n<------| STRING PARSED SUCCESSFULLY |------>\n");
    } else {
        printf("\n------UNABLE TO PARSE GIVEN STRING------\n");
        
        return 1;
    }

    printf("\n<------| main() OUT |------>\n");

    return 0;
}