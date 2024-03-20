#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define matchlen(match) 15+match

struct optimization {
        char *pattern;
        char replacement[20];
};

struct optimization optimizations[] = {
        // Duplicates.
        {"^\\([-+<>]\\)\\1",                     {'2',    1}},
        {"\\([^0-9]\\)\\([-+<>]\\)\\2",          {1, '2',    2}},
        {"2\\([-+<>]\\)2\\1",                    {'4',         1}},
        {"4\\([-+<>]\\)4\\1",                    {'8',         1}},
        {"8\\([-+<>]\\)8\\1",                 {'1', '6',    1}},
        {"16\\([-+<>]\\)16\\1",               {'3', '2',    1}},
        {"32\\([-+<>]\\)32\\1",               {'6', '4',    1}},
        {"64\\([-+<>]\\)64\\1",               {'1', '2', '8', 1}},
        {"128\\([-+<>]\\)128\\1",             {}},
        // Copying.
        {"\\[\\([0-9]\\{0,\\}\\)>+\\1<-\\]",  {1,           '}'}},
        {"\\[\\([0-9]\\{0,\\}\\)<+\\1>-\\]",  {1,           '{'}},
        {"\\[-\\([0-9]\\{0,\\}\\)>+\\1<\\]",  {1,           '}'}},
        {"\\[-\\([0-9]\\{0,\\}\\)<+\\1>\\]",  {1,           '{'}},
        // Multiplication by two/first approximation.
        {"\\[\\([0-9]\\{0,\\}\\)>2+\\1<-\\]", {1,      '^', '}'}},
        {"\\[\\([0-9]\\{0,\\}\\)<2+\\1>-\\]", {1,      '^', '{'}},
        {"\\[-\\([0-9]\\{0,\\}\\)>2+\\1<\\]", {1,      '^', '}'}},
        {"\\[-\\([0-9]\\{0,\\}\\)<2+\\1>\\]", {1,      '^', '{'}},
        // Misc
        {"\\[[0-9]\\{0,\\}-\\]",              {'0',         '='}},
        {"0=\\([0-9]\\{0,\\}\\)+",            {1,           '='}},
        {"\\[\\([0-9]\\{0,\\}\\)>\\]",        {1,           '?'}},
        {"\\[\\([0-9]\\{0,\\}\\)<\\]",        {1,      '^', '?'}}
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
        withreg(reg, rmatch, "[][+.,<>!#-]");
        char c;
        while ((c = getc(infile)) != EOF)
                if(regmatch(&reg, (char[]){c, 0}, rmatch))
                        putc(c, outfile);
        return EXIT_SUCCESS;
}

int optimize_file (FILE *infile, FILE *outfile)
{
        char str[10000];
        while (fgets(str, 10000, infile)) {
                // Remove newline.
                str[strlen(str)-1] = '\0';
                for (size_t i = 0; i < sizeof(optimizations) / sizeof(struct optimization); ++i)
                        replace_pattern(str, optimizations[i]);
                fputs(str, outfile);
                // Compensate the initial newline stripping.
                putc('\n', outfile);
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
                "\\([0-9]\\{0,\\}\\)\\(\\^\\)\\{0,1\\}\\([][+.,<>!#-]\\)");
        int parsed_commands = 0;
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
                                = buf[rmatches[3].rm_so];

                        commands[0].number = current.number;
                        commands[0].special = current.special;
                        commands[0].command = current.command;
                        current.command = current.special = 0;
                        current.number = 1;
                        ++commands;
                        ++parsed_commands;
                        buf += rmatches[3].rm_eo;
                }
        }
        return EXIT_SUCCESS;
}

int eval_commands (struct command *commands, FILE *infile, FILE *outfile) {
        char memory_[100000] = {0};
        char *memory = &memory_[50000];
        int depth = 0;
        for (size_t i = 0; commands[i].command != 0; ++i) {
                /* printf("%c", commands[i]->command); */
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
