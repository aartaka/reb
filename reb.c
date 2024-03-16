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
        {"\\[\\([0-9]\\{0,\\}\\)>+\\1<-\\]",  {1,      '^', '>'}},
        {"\\[\\([0-9]\\{0,\\}\\)<+\\1>-\\]",  {1,      '^', '<'}},
        {"\\[-\\([0-9]\\{0,\\}\\)>+\\1<\\]",  {1,      '^', '>'}},
        {"\\[-\\([0-9]\\{0,\\}\\)<+\\1>\\]",  {1,      '^', '<'}},
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
        // Misc
        {"\\[[0-9]\\{0,\\}-\\]",              {'0',         '='}},
        {"0=\\([0-9]\\{0,\\}\\)+",            {1,           '='}},
        {"\\[\\([0-9]\\{0,\\}\\)>\\]",        {1,      '^', ']'}},
        {"\\[\\([0-9]\\{0,\\}\\)<\\]",        {1,      '^', '['}}
};


void replace_pattern (char *str, struct optimization opt) {
        regex_t reg;
        size_t rmax = 1000;
        regmatch_t rmatch[rmax];
        char buf[10000];
        strcpy(buf, str);
        regcomp(&reg, opt.pattern, 0);
        while (!regexec(&reg, str, rmax, rmatch, 0)) {
                size_t buf_idx = 0;
                for (size_t i = 0; i < strlen(opt.replacement); ++i) {
                        if (opt.replacement[i] < 15) {
                                regmatch_t match = rmatch[(size_t)opt.replacement[i]];
                                strcpy(buf + buf_idx, str + match.rm_so);
                                buf_idx += match.rm_eo - match.rm_so;
                        } else if (15 < opt.replacement[i] && opt.replacement[i] < ' ') {
                                regmatch_t match = rmatch[opt.replacement[i] - 15];
                                buf_idx += sprintf(buf + buf_idx, "%d", match.rm_eo - match.rm_so);
                        } else {
                                buf[buf_idx++] = opt.replacement[i];
                        }
                }
                memcpy(str + rmatch[0].rm_so, buf, buf_idx);
                strcpy(str + rmatch[0].rm_so + buf_idx, str + rmatch[0].rm_eo);
        }
}

int regmatch(char *pattern, char *string)
{
        regex_t reg;
        regcomp(&reg, pattern, 0);
        size_t rmax = 3;
        regmatch_t rmatch[rmax];
        return regexec(&reg, string, rmax, rmatch, 0);
}

int minify_file (FILE *infile, FILE *outfile)
{
        char c;
        while ((c = getc(infile)) != EOF)
                if(!regmatch("[][+-.,<>!#]", (char[]){c}))
                        putc(c, outfile);
        return EXIT_SUCCESS;
}

int optimize_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        while (strlen(fgets(str, 10000, infile))) {
                for (int i = 0; i < sizeof(optimizations) / sizeof(struct optimization); ++i)
                        replace_pattern(str, optimizations[i]);
                fputs(str, outfile);
        }
        return EXIT_SUCCESS;
}

int main (int argc, char **argv)
{
        if (argc == 1
            || strlen(argv[1]) > 1)
                printf("Please use reb with commands like 'm' or 'o'");
        FILE *infile;
        if (argc == 2)
                infile = stdin;
        else if (argc > 2 && !strcmp(argv[2], "--"))
                infile = stdin;
        else
                infile = fopen(argv[2], "r");
        FILE *outfile;
        if (argc <= 3)
                outfile = stdout;
        else if (argc > 3 && !strcmp(argv[3], "--"))
                outfile = stdout;
        else
                outfile = fopen(argv[3], "w");
        switch (argv[1][0]) {
        case 'm':
                return minify_file(infile, outfile);
        case 'o':
                return optimize_file(infile, outfile);
        }
        return EXIT_SUCCESS;
}
