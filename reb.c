#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifndef MEMSIZE
#define MEMSIZE 100000
#endif

#ifndef CELLTYPE
#define CELLTYPE char
#endif

#define matchlen(match) 15+match

struct optimization {
        char *pattern;
        char replacement[20];
};

struct optimization optimizations[] = {
        // Minification
        {"[^][+.,<>!#-]",                     {               0}},
        // Duplicates.
        {"\\(+\\{2,\\}\\)",                   {matchlen(1), '+'}},
        {"\\(-\\{2,\\}\\)",                   {matchlen(1), '-'}},
        {"\\(>\\{2,\\}\\)",                   {matchlen(1), '>'}},
        {"\\(<\\{2,\\}\\)",                   {matchlen(1), '<'}},
        // Copying.
        {"\\[\\([0-9]*\\)>+\\1<-\\]",         {1,             '}'}},
        {"\\[\\([0-9]*\\)<+\\1>-\\]",         {1,             '{'}},
        // Misc
        {"\\[[0-9]*[+-]\\]",                  {'0',           '='}},
        {"0=\\([0-9]*\\)+",                   {1,             '='}},
        {"\\[\\([0-9]*\\)>\\]",               {1,             ')'}},
        {"\\[\\([0-9]*\\)<\\]",               {1,             '('}},
        // Questionable: optimize empty loops to nothing. Otherwise
        // these are endless loops, which make no sense, right?
        {"\\[\\]",                            {                 0}},
        // Comment loops
        {"^\\[[^][]*\\]",                     {                 0}},
        {"^\\[.*\\]",                         {                 0}},
        {"\\]\\[[^][]*\\]",                   {']'               }},
        // Other no-ops
        {"<>",                                {                  }},
        {"><",                                {                  }},
        {"\\([0-9]*\\)[=+-],",                {','               }},
};

#define withreg(regvar, matchvar, ...)          \
        regex_t regvar;                         \
        regmatch_t matchvar[1000];              \
        regcomp(&regvar, __VA_ARGS__, 0);

int regmatch(regex_t *preg, char *str, regmatch_t *pmatch)
{
        return !regexec(preg, str, 1000, pmatch, 0);
}

char *replace_pattern (char *str, struct optimization opt)
{
        withreg(reg, rmatch, opt.pattern);
        char copy[10000];
        strcpy(copy, str);
        while (regmatch(&reg, str, rmatch)) {
                int offset = rmatch[0].rm_so;
                for (int i = 0; opt.replacement[i]; ++i) {
                        if (opt.replacement[i] < 15) {
                                regmatch_t match = rmatch[(size_t)opt.replacement[i]];
                                memmove(str + offset, &copy[match.rm_so], match.rm_eo - match.rm_so);
                                offset += match.rm_eo - match.rm_so;
                        } else if (15 < opt.replacement[i] && opt.replacement[i] < ' ') {
                                regmatch_t match = rmatch[opt.replacement[i] - 15];
                                offset += sprintf(str + offset, "%d", match.rm_eo - match.rm_so);
                        } else {
                                str[offset++] = opt.replacement[i];
                        }
                }
                strcpy(str + offset, copy + rmatch[0].rm_eo);
                strcpy(copy, str);
        }
        return str;
}

int minify_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        // Minification pattern is the first optimization.
        struct optimization minification = optimizations[0];
        while (fgets(str, 10000, infile))
                fputs(replace_pattern(str, minification), outfile);
        return EXIT_SUCCESS;
}

int optimize_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        while (fgets(str, 10000, infile)) {
                // Repeat multiple times to make sure everything is optimized.
                for (int iter = 0; iter < 5; ++iter)
                        // Ignoring minimization rule on later passes.
                        for (size_t i = (iter ? 1 : 0);
                             i < sizeof(optimizations) / sizeof(struct optimization);
                             ++i)
                                replace_pattern(str, optimizations[i]);
                fputs(str, outfile);
        }
        return EXIT_SUCCESS;
}

struct command {
        int number;
        char command;
};

int parse_file (FILE *codefile, struct command *commands)
{
        withreg(reg, rmatches,
                "\\([0-9]\\{0,\\}\\)\\([][+.,<>!#=(){}-]\\)");
        struct command current = {1};
        char str[1000000];
        while (fgets(str, 1000000, codefile)) {
                char *buf = str;
                while (regmatch(&reg, buf, rmatches)){
                        current.number
                                = (rmatches[1].rm_so == rmatches[1].rm_eo)
                                ? 1
                                : atoi(buf + rmatches[1].rm_so);
                        current.command
                                = buf[rmatches[2].rm_so];
                        commands->number = current.number;
                        commands->command = current.command;
                        current.command = 0;
                        current.number = 1;
                        ++commands;
                        buf += rmatches[2].rm_eo;
                }
        }
        return EXIT_SUCCESS;
}

CELLTYPE memory_[MEMSIZE] = {0};
CELLTYPE *memory = &memory_[MEMSIZE/2];

int eval_commands (struct command *commands, FILE *infile, FILE *outfile)
{
        int depth = 0;
        for (size_t i = 0; commands[i].command; ++i) {
                /* printf("%c", commands[i].command); */
                struct command command = commands[i];
                char c;
                switch (command.command) {
                case '+':
                        *memory += command.number;
                        break;
                case '-':
                        *memory -= command.number;
                        break;
                case '>':
                        memory += command.number;
                        break;
                case '<':
                        memory -= command.number;
                        break;
                case ',':
                        if ((c = getc(infile)) != EOF)
                                *memory = c;
                        break;
                case '.':
                        putc(*memory, outfile);
                        break;
                case '[':
                        // Stolen from microbf, will refactor later.
                        if (!*memory)
                                while((depth += (commands[i].command=='[') - (commands[i].command==']')))
                                        i++;
                        break;
                case ']':
                        // Stolen from microbf, will refactor later.
                        if (*memory)
                                while((depth += (commands[i].command==']') - (commands[i].command=='[')))
                                        i--;
                        break;
                case '=':
                        *memory = command.number;
                        break;
                case '}':
                        *(memory+command.number) = *memory;
                        *memory = 0;
                        break;
                case '{':
                        *(memory-command.number) = *memory;
                        *memory = 0;
                        break;
                case ')':
                        for (; *memory; memory += command.number);
                        break;
                case '(':
                        for (; *memory; memory -= command.number);
                        break;
                case '#':
                        for (int i = 0, max = (command.number == 1 ? 10 : command.number); i < max; ++i)
                                fprintf(outfile,
                                        "%c%d/%c%c",
                                        ((memory + i - max / 2) == memory ? '[' : ' '),
                                        *(memory + i - max / 2),
                                        *(memory + i - max / 2),
                                        ((memory + i - max / 2) == memory ? ']' : ' '));
                        fputs("\n", outfile);
                        break;
                        // TODO: ! handling (a complicated one!)
                }
        }
        return EXIT_SUCCESS;
}

int main (int argc, char *argv[argc])
{
        FILE *infile;
        if (argc == 2)
                infile = stdin;
        else if (argc > 2 && !strcmp(argv[2], "--"))
                infile = stdin;
        else
                infile = fopen(argv[2], "r");
        struct command commands[100000] = {{0}};
        switch (argv[1][0]) {
        case 'm':
                return minify_file(infile, stdout);
        case 'o':
                return optimize_file(infile, stdout);
        case 'r':
                parse_file(infile, commands);
                /* for (int i = 0; commands[i].command; ++i) */
                /*         printf("Command %c on %d\n", */
                /*                commands[i].command, commands[i].number); */
                return eval_commands(commands, stdin, stdout);
        }
        return EXIT_SUCCESS;
}
