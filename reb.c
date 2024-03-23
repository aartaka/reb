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

struct optimization {
        char *pattern;
        char replacement[20];
};

struct optimization optimizations[] = {
        // Minification
        {"[^][+.,<>-]",                       {               0}},
        // Duplicates.
        {"^\\([-+<>]\\)\\1",                  {'2',           1}},
        {"\\([^0-9]\\)\\([-+<>]\\)\\2",       {1, '2',        2}},
        {"\\<2\\([-+<>]\\)2\\1",              {'4',           1}},
        {"\\<4\\([-+<>]\\)4\\1",              {'8',           1}},
        {"\\<8\\([-+<>]\\)8\\1",              {'1', '6',      1}},
        {"\\<16\\([-+<>]\\)16\\1",            {'3', '2',      1}},
        {"\\<32\\([-+<>]\\)32\\1",            {'6', '4',      1}},
        {"\\<64\\([-+<>]\\)64\\1",            {'1', '2', '8', 1}},
        {"\\<128\\([-+<>]\\)128\\1",          {0}},
        // Numbers up to 127, structured around primes
#define X(BASE, ADDEND, ...) \
        {"\\<" #BASE "\\([-+<>]\\)" #ADDEND "\\1", {__VA_ARGS__,   1}}
        // Yes, it's tedious and manually typed in. So what?
        X(2, , '3'), /* 4 */
        X(4, , '5'), X(4, 2, '6'),
        X(4, 3, '7'), /* 8 */ X(8, , '9'), X(8, 2, '1', '0'),
        X(8, 3, '1', '1'), X(8, 4, '1', '2'),
        X(8, 5, '1', '3'), X(8, 6, '1', '4'), X(8, 7, '1', '5'), /* 16 */
        X(16, , '1', '7'), X(16, 2, '1', '8'),
        X(16, 3, '1', '9'), X(16, 4, '2', '0'), X(16, 5, '2', '1'), X(16, 6, '2', '2'),
        X(16, 7, '2', '3'), X(16, 8, '2', '4'), X(16, 9, '2', '5'),
        X(16, 10, '2', '6'), X(16, 11, '2', '7'),
        X(16, 13, '2', '9'), X(16, 14, '3', '0'),
        X(16, 15, '3', '1'), /* 32 */
        X(32, , '3', '3'), X(32, 2, '3', '4'), X(32, 3, '3', '5'), X(32, 4, '3', '6'),
        X(32, 5, '3', '7'), X(32, 6, '3', '8'), X(32, 7, '3', '9'), X(32, 8, '4', '0'),
        X(32, 9, '4', '1'), X(32, 10, '4', '2'),
        X(32, 11, '4', '3'), X(32, 12, '4', '4'), X(32, 13, '4', '5'), X(32, 14, '4', '6'),
        X(32, 15, '4', '7'), X(32, 16, '4', '8'), X(32, 17, '4', '9'),
        X(32, 18, '5', '0'), X(32, 19, '5', '1'), X(32, 20, '5', '2'),
        X(32, 21, '5', '3'), X(32, 22, '5', '4'), X(32, 23, '5', '5'),
        X(32, 24, '5', '6'), X(32, 25, '5', '7'),
        X(32, 27, '5', '9'), X(32, 28, '6', '0'),
        X(32, 29, '6', '1'), X(32, 30, '6', '2'),
        X(32, 31, '6', '3'), /* 64 */ X(64, , '6', '5'), X(64, 2, '6', '6'),
        X(64, 3, '6', '7'), X(64, 4, '6', '8'), X(64, 5, '6', '9'), X(64, 6, '7', '0'),
        X(64, 7, '7', '1'), X(64, 8, '7', '2'),
        X(64, 9, '7', '3'), X(64, 10, '7', '4'), X(64, 11, '7', '5'),
        X(64, 12, '7', '6'), X(64, 13, '7', '7'), X(64, 14, '7', '8'),
        X(64, 15, '7', '9'), X(64, 16, '8', '0'),
        X(64, 17, '8', '1'), X(64, 18, '8', '2'),
        X(64, 19, '8', '3'), X(64, 20, '8', '4'), X(64, 21, '8', '5'), X(64, 22, '8', '6'),
        X(64, 23, '8', '7'), X(64, 24, '8', '8'),
        X(64, 25, '8', '9'), X(64, 26, '9', '0'),
        X(64, 27, '9', '1'), X(64, 28, '9', '2'), X(64, 29, '9', '3'), X(64, 30, '9', '4'),
        X(64, 31, '9', '5'), X(64, 32, '9', '6'),
        X(64, 33, '9', '7'), X(64, 34, '9', '8'), X(64, 35, '9', '9'), X(64, 36, '1', '0', '0'),
        X(64, 37, '1', '0', '1'), X(64, 38, '1', '0', '2'),
        X(64, 39, '1', '0', '3'), X(64, 40, '1', '0', '4'),
        X(64, 41, '1', '0', '5'), X(64, 42, '1', '0', '6'),
        X(64, 43, '1', '0', '7'), X(64, 44, '1', '0', '8'),
        X(64, 45, '1', '0', '9'), X(64, 46, '1', '1', '0'),
        X(64, 47, '1', '1', '1'), X(64, 48, '1', '1', '2'),
        X(64, 49, '1', '1', '3'), X(64, 50, '1', '1', '4'),
        X(64, 51, '1', '1', '5'), X(64, 52, '1', '1', '6'),
        X(64, 53, '1', '1', '7'), X(64, 54, '1', '1', '8'),
        X(64, 55, '1', '1', '9'), X(64, 56, '1', '2', '0'),
        X(64, 57, '1', '2', '1'), X(64, 58, '1', '2', '2'),
        X(64, 59, '1', '2', '3'), X(64, 60, '1', '2', '4'),
        X(64, 61, '1', '2', '5'), X(64, 62, '1', '2', '6'),
        X(64, 63, '1', '2', '7'), /* 128 */
        // Copying.
        /* {"\\[\\([0-9]*\\)>+\\1<-\\]",         {1,             '}'}}, */
        /* {"\\[\\([0-9]*\\)<+\\1>-\\]",         {1,             '{'}}, */
        /* {"\\[-\\([0-9]*\\)>+\\1<\\]",         {1,             '}'}}, */
        /* {"\\[-\\([0-9]*\\)<+\\1>\\]",         {1,             '{'}}, */
        /* // Misc */
        {"\\[[0-9]*-\\]",                     {'0',           '='}},
        {"0=\\([0-9]*\\)+",                   {1,             '='}},
        {"\\[>\\]",                           {               '?'}},
        {"\\[<\\]",                           {          '^', '?'}},
        // Questionable: optimize empty loops to nothing. Otherwise
        // these are endless loops, which make no sense, right?
        {"\\[\\]",                            {                 0}},
        {"\\]\\[[^][]*\\]",                   {']'               }},
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
                for (int i = 0; opt.replacement[i] != 0; ++i) {
                        if (opt.replacement[i] < ' ') {
                                regmatch_t match = rmatch[(size_t)opt.replacement[i]];
                                memmove(str + offset, &copy[match.rm_so], match.rm_eo - match.rm_so);
                                offset += match.rm_eo - match.rm_so;
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
                for (size_t i = 0; i < sizeof(optimizations) / sizeof(struct optimization); ++i)
                        replace_pattern(str, optimizations[i]);
                fputs(str, outfile);
        }
        return EXIT_SUCCESS;
}

struct command {
        int number;
        bool special;
        char command;
};

int parse_file (FILE *codefile, struct command *commands) {
        withreg(reg, rmatches,
                "\\([0-9]\\{0,\\}\\)\\(\\^\\)\\{0,1\\}\\([0-9]\\{0,\\}\\)\\([][+.,<>!#=?{}-]\\)");
        struct command current = {1};
        char str[1000000];
        while (fgets(str, 1000000, codefile)) {
                char *buf = str;
                while (regmatch(&reg, buf, rmatches)){
                        current.number
                                = (rmatches[1].rm_so == rmatches[1].rm_eo)
                                ? 1
                                : atoi(buf + rmatches[1].rm_so);
                        current.special
                                = rmatches[2].rm_so != rmatches[2].rm_eo;
                        // TODO: rmatches[3] is numbered arg to the
                        // special command.
                        current.command
                                = buf[rmatches[4].rm_so];

                        commands->number = current.number;
                        commands->special = current.special;
                        commands->command = current.command;
                        current.command = current.special = 0;
                        current.number = 1;
                        ++commands;
                        buf += rmatches[4].rm_eo;
                }
        }
        return EXIT_SUCCESS;
}

CELLTYPE memory_[MEMSIZE] = {0};
CELLTYPE *memory = &memory_[MEMSIZE/2];

int eval_commands (struct command *commands, FILE *infile, FILE *outfile) {
        int depth = 0;
        for (size_t i = 0; commands[i].command != 0; ++i) {
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
                case '?':
                        if (command.special)
                                for (; *memory; memory--);
                        else
                                memory = memchr(memory, '\0', MEMSIZE);
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
                /* for (int i = 0; commands[i].command != 0; ++i) */
                /*         printf("%s command %c on %d\n", */
                /*                (commands[i].special ? "Special" : "Regular"), */
                /*                commands[i].command, commands[i].number); */
                return eval_commands(commands, stdin, stdout);
        }
        return EXIT_SUCCESS;
}
