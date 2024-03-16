#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define matchlen(match) 15+match

struct optimization {
        char *pattern;
        char replacement[20];
};

struct optimization optimizations[] = {
        // Collapsing operations.
        {"\\(+\\{2,\\}\\)",                   {matchlen(1), '+'}},
        {"\\(-\\{2,\\}\\)",                   {matchlen(1), '-'}},
        {"\\(>\\{2,\\}\\)",                   {matchlen(1), '>'}},
        {"\\(<\\{2,\\}\\)",                   {matchlen(1), '<'}},
        // Copying.
        {"\\[\\([0-9]\\{0,\\}\\)>+\\1<-\\]",  {1,           '@'}},
        {"\\[\\([0-9]\\{0,\\}\\)<+\\1>-\\]",  {1,      '^', '@'}},
        {"\\[-\\([0-9]\\{0,\\}\\)>+\\1<\\]",  {1,           '@'}},
        {"\\[-\\([0-9]\\{0,\\}\\)<+\\1>\\]",  {1,      '^', '@'}},
        // Multiplication by three/first approximation.
        {"\\[\\([0-9]\\{0,\\}\\)>2+\\1<-\\]", {1,           '\''}},
        {"\\[\\([0-9]\\{0,\\}\\)<2+\\1>-\\]", {1,      '^', '\''}},
        {"\\[-\\([0-9]\\{0,\\}\\)>2+\\1<\\]", {1,           '\''}},
        {"\\[-\\([0-9]\\{0,\\}\\)<2+\\1>\\]", {1,      '^', '\''}},
        // Multiplication by three/second approximation.
        {"\\[\\([0-9]\\{0,\\}\\)>3+\\1<-\\]", {1,           '"'}},
        {"\\[\\([0-9]\\{0,\\}\\)<3+\\1>-\\]", {1,      '^', '"'}},
        {"\\[-\\([0-9]\\{0,\\}\\)>3+\\1<\\]", {1,           '"'}},
        {"\\[-\\([0-9]\\{0,\\}\\)<3+\\1>\\]", {1,      '^', '"'}},
        {"\\[[0-9]\\{0,\\}-\\]",              {'0',         '='}},
        {"0=\\([0-9]\\{0,\\}\\)+",            {1,           '='}},
        {"\\[\\([0-9]\\{0,\\}\\)>\\]",        {1,           '?'}},
        {"\\[\\([0-9]\\{0,\\}\\)<\\]",        {1,      '^', '?'}}
};


void replace_pattern (char *str, struct optimization opt) {
        regex_t *reg = malloc(sizeof(regex_t));
        size_t rmax = 1000;
        regmatch_t rmatch[rmax];
        char buf[200];
        strcpy(buf, str);
        regcomp(reg, opt.pattern, 0);
        while (!regexec(reg, str, rmax, rmatch, 0)) {
                size_t buf_idx = 0;
                printf("Another iteration: %s", str);
                for (size_t i = 0; i < strlen(opt.replacement); ++i) {
                        if (opt.replacement[i] < 15) {
                                printf("Replacing with substitution %s\n", opt.pattern);
                                regmatch_t match = rmatch[opt.replacement[i]];
                                strcpy(buf + buf_idx, str + match.rm_so);
                                buf_idx += match.rm_eo - match.rm_so;
                        } else if (15 < opt.replacement[i] && opt.replacement[i] < ' ') {
                                printf("Replacing with length\n");
                                regmatch_t match = rmatch[opt.replacement[i] - 15];
                                buf_idx += sprintf(buf + buf_idx, "%d", match.rm_eo - match.rm_so);
                        } else {
                                printf("Replacing with char %c\n", opt.replacement[i]);
                                buf[buf_idx++] = opt.replacement[i];
                        }
                }
                printf("Buffer is %s at %d\n", buf, buf_idx);
                memcpy(str + rmatch[0].rm_so, buf, buf_idx);
                strcpy(str + rmatch[0].rm_so + buf_idx, str + rmatch[0].rm_eo);
        }
}

int isbf (char c) {
        switch(c) {
        case '+':
        case '-':
        case '<':
        case '>':
        case ',':
        case '.':
        case '[':
        case ']':
        case '#':
        case '!':
                return 1;
        default:
                return 0;
        }
}

int minify_file (FILE *infile, FILE *outfile)
{
        char c;
        while ((c = getc(infile)) != EOF)
                if(isbf(c))
                        putc(c, outfile);
}

/* int optimize_file (FILE *infile, FILE *outfile) */
/* { */
/*         for (int i = 0; i < sizeof(optimizations) / sizeof(struct optimization); ++i) { */
/*                 replace_pattern(str, optimizations[i]); */
/*                 printf("String is: %s", str); */
/*         } */
/* } */

int main (int argc, char **argv)
{
        char str[200];
        minify_file(stdin, stdout);
        /* fgets(str, 200, stdin); */
        /* for (int i = 0; i < sizeof(optimizations) / sizeof(struct optimization); ++i) { */
        /*         replace_pattern(str, optimizations[i]); */
        /*         printf("String is: %s", str); */
        /* } */
        /* fputs(str, stdout); */
}
