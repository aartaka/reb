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
        // Primes
        {"\\<2\\([-+<>]\\)\\1",               {'3',           1}},
        {"\\<4\\([-+<>]\\)\\1",               {'5',           1}},
        {"\\<4\\([-+<>]\\)3\\1",              {'7',           1}},
        {"\\<8\\([-+<>]\\)\\1",               {'9',           1}}, // Not a prime
        {"\\<8\\([-+<>]\\)3\\1",              {'1', '1',      1}},
        {"\\<8\\([-+<>]\\)5\\1",              {'1', '3',      1}},
        {"\\<8\\([-+<>]\\)7\\1",              {'1', '5',      1}},         // Not a prime
        {"\\<16\\([-+<>]\\)\\1",              {'1', '7',      1}},
        {"\\<16\\([-+<>]\\)3\\1",             {'1', '9',      1}},
        {"\\<16\\([-+<>]\\)5\\1",             {'2', '1',      1}}, // Not a prime
        {"\\<16\\([-+<>]\\)9\\1",             {'2', '5',      1}}, // Not a prime
        {"\\<16\\([-+<>]\\)7\\1",             {'2', '3',      1}},
        {"\\<16\\([-+<>]\\)11\\1",            {'2', '7',      1}}, // Not a prime
        {"\\<16\\([-+<>]\\)13\\1",            {'2', '9',      1}},
        {"\\<16\\([-+<>]\\)15\\1",            {'3', '1',      1}},
        {"\\<31\\([-+<>]\\)\\1",              {'3', '3',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)5\\1",             {'3', '7',      1}},
        {"\\<32\\([-+<>]\\)7\\1",             {'3', '9',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)9\\1",             {'4', '1',      1}},
        {"\\<32\\([-+<>]\\)11\\1",            {'4', '3',      1}},
        {"\\<32\\([-+<>]\\)13\\1",            {'4', '5',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)15\\1",            {'4', '7',      1}},
        {"\\<32\\([-+<>]\\)17\\1",            {'4', '9',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)21\\1",            {'5', '3',      1}},
        {"\\<32\\([-+<>]\\)23\\1",            {'5', '5',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)25\\1",            {'5', '7',      1}}, // Not a prime
        {"\\<32\\([-+<>]\\)27\\1",            {'5', '9',      1}},
        {"\\<32\\([-+<>]\\)29\\1",            {'6', '1',      1}},
        {"\\<32\\([-+<>]\\)31\\1",            {'6', '3',      1}},
        {"\\<64\\([-+<>]\\)3\\1",             {'6', '7',      1}},
        {"\\<64\\([-+<>]\\)5\\1",             {'6', '9',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)7\\1",             {'7', '1',      1}},
        {"\\<64\\([-+<>]\\)9\\1",             {'7', '3',      1}},
        {"\\<64\\([-+<>]\\)11\\1",            {'7', '5',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)13\\1",            {'7', '7',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)15\\1",            {'7', '9',      1}},
        {"\\<64\\([-+<>]\\)17\\1",            {'8', '1',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)19\\1",            {'8', '3',      1}},
        {"\\<64\\([-+<>]\\)21\\1",            {'8', '5',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)23\\1",            {'8', '7',      1}}, // Not a prime
        {"\\<64\\([-+<>]\\)25\\1",            {'8', '9',      1}},
        {"\\<64\\([-+<>]\\)27\\1",            {'9', '1',      1}},
        {"\\<64\\([-+<>]\\)29\\1",            {'9', '3',      1}},
        {"\\<64\\([-+<>]\\)31\\1",            {'9', '5',      1}},
        {"\\<64\\([-+<>]\\)33\\1",            {'9', '7',      1}},
        {"\\<64\\([-+<>]\\)37\\1",            {'1', '0', '1', 1}},
        {"\\<64\\([-+<>]\\)39\\1",            {'1', '0', '3', 1}},
        {"\\<64\\([-+<>]\\)43\\1",            {'1', '0', '7', 1}},
        {"\\<64\\([-+<>]\\)45\\1",            {'1', '0', '9', 1}},
        {"\\<64\\([-+<>]\\)47\\1",            {'1', '1', '1', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)49\\1",            {'1', '1', '3', 1}},
        {"\\<64\\([-+<>]\\)53\\1",            {'1', '1', '7', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)55\\1",            {'1', '1', '9', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)57\\1",            {'1', '2', '1', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)59\\1",            {'1', '2', '3', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)61\\1",            {'1', '2', '5', 1}}, // Not a prime
        {"\\<64\\([-+<>]\\)63\\1",            {'1', '2', '7', 1}},
        // Copying.
        /* {"\\[\\([0-9]\\{0,\\}\\)>+\\1<-\\]",  {1,             '}'}}, */
        /* {"\\[\\([0-9]\\{0,\\}\\)<+\\1>-\\]",  {1,             '{'}}, */
        /* {"\\[-\\([0-9]\\{0,\\}\\)>+\\1<\\]",  {1,             '}'}}, */
        /* {"\\[-\\([0-9]\\{0,\\}\\)<+\\1>\\]",  {1,             '{'}}, */
        /* // Misc */
        {"\\[[0-9]\\{0,\\}-\\]",              {'0',           '='}},
        {"0=\\([0-9]\\{0,\\}\\)+",            {1,             '='}},
        /* {"\\[\\([0-9]\\{0,\\}\\)>\\]",        {1,             '?'}}, */
        /* {"\\[\\([0-9]\\{0,\\}\\)<\\]",        {1,        '^', '?'}}, */
        // Questionable: optimize empty loops to nothing. Otherwise
        // these are endless loops, which make no sense, right?
        {"\\[\\]",                            {                 0}},
};

#define withreg(regvar, matchvar, ...)          \
        regex_t regvar;                         \
        regmatch_t matchvar[1000];              \
        regcomp(&regvar, __VA_ARGS__, 0);

int regmatch(regex_t *preg, char *str, regmatch_t *pmatch)
{
        return !regexec(preg, str, 1000, pmatch, 0);
}

void replace_pattern (char *str, struct optimization opt)
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
}

int minify_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        while (fgets(str, 10000, infile)) {
                // Remove newline.
                str[strlen(str)] = '\0';
                // Minification pattern is the first optimization.
                replace_pattern(str, optimizations[0]);
                fputs(str, outfile);
        }
        return EXIT_SUCCESS;
}

int optimize_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        while (fgets(str, 10000, infile)) {
                // Remove newline.
                str[strlen(str)] = '\0';
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
                                memory = memchr(memory, '\0', strlen(memory));
                        break;
                case '#':
                        for (int i = 0, max = (command.number == 1 ? 10 : command.number); i < max; ++i)
                                printf("%c%d/%c%c",
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
