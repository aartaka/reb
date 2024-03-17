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
        // Collapsing operations.
        {"\\(+\\{2,\\}\\)",                   {matchlen(1), '+'}},
        {"\\(-\\{2,\\}\\)",                   {matchlen(1), '-'}},
        {"\\(>\\{2,\\}\\)",                   {matchlen(1), '>'}},
        {"\\(<\\{2,\\}\\)",                   {matchlen(1), '<'}},
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

#define withreg(regvar, max, matchvar, ...) \
        regex_t regvar;                        \
        regmatch_t matchvar[max];             \
        regcomp(&regvar, __VA_ARGS__, 0);

void replace_pattern (char *str, struct optimization opt) {
        withreg(reg, 1000, rmatch, opt.pattern);
        char buf[10000];
        strcpy(buf, str);
        while (!regexec(&reg, str, 1000, rmatch, 0)) {
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

int minify_file (FILE *infile, FILE *outfile)
{
        withreg(reg, 3, rmatch, "[][+-.,<>!#]");
        char c;
        while ((c = getc(infile)) != EOF)
                if(!regexec(&reg, (char[]){c, 0}, 3, rmatch, 0))
                        putc(c, outfile);
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

int parse_file (FILE *codefile, struct command **commands) {
        char c;
        char next;
        int parsed_commands = 0;
        struct command current = {1};
        while ((c = getc(codefile)) != EOF) {
                switch (c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                        current.number = (c-48);
                        while ((next = getc(codefile))) {
                                if ('^' == next) {
                                        goto special;
                                } else if ('+' == next
                                           || '-' == next
                                           || '<' == next
                                           || '>' == next
                                           || ',' == next
                                           || '.' == next
                                           || '[' == next
                                           || ']' == next) {
                                        c = next;
                                        goto command;
                                } else if (isdigit(next)) {
                                        current.number *= 10;
                                        current.number += (next-48);
                                } else {
                                        break;
                                }
                        }
                        break;
                case '^':
                special:
                        current.special = true;
                        break;
                case '+': case '-':
                case '<': case '>':
                case ',': case '.':
                case '[': case ']':
                case '{': case '}':
                case '?': case '=':
                command:
                        current.command = c;
                        commands[0] = malloc(sizeof(struct command));
                        commands[0]->number = current.number;
                        commands[0]->special = current.special;
                        commands[0]->command = current.command;
                        current.command = current.special = 0;
                        current.number = 1;
                        ++commands;
                        ++parsed_commands;
                default:
                        break;
                }
        }
        return EXIT_SUCCESS;
}

int eval_commands (struct command **commands, FILE *infile, FILE *outfile) {
        unsigned int pointer = 0;
        char memory_[100000] = {0};
        char *memory = &memory_[50000];
        size_t brackets_[200] = {0};
        size_t *brackets = brackets_;
        for (size_t i = 0; commands[i] != 0;) {
                printf("Command %c\n", commands[i]->command);
                struct command *command = commands[i];
                switch (command->command) {
                case '+':
                        *memory += command->number;
                        break;
                case '-':
                        *memory -= command->number;
                        break;
                case '>':
                        memory += command->number;
                        break;
                case '<':
                        memory -= command->number;
                        break;
                case ',':
                        *memory = getc(infile);
                        break;
                case '.':
                        putc(*memory, outfile);
                        break;
                case '[':
                        if (*memory) {
                                *(brackets++) = i;
                        } else {
                                int j = i+1;
                                for (int depth = 1; depth > 0; ++j) {
                                        if (commands[j]->command == ']')
                                                depth--;
                                        else if (commands[j]->command == '[')
                                                depth++;
                                        i = j;
                                }
                        }
                        break;
                case ']':
                        if (memory[pointer])
                                i = *(--brackets);
                        break;
                }
                i++;
        }
        return EXIT_SUCCESS;
}

int main (int argc, char **argv)
{
        /* if (argc == 1 */
        /*     || strlen(argv[1]) > 1) */
        /*         printf("Please use reb with commands like 'm' or 'o'"); */
        /* FILE *infile; */
        /* if (argc == 2) */
        /*         infile = stdin; */
        /* else if (argc > 2 && !strcmp(argv[2], "--")) */
        /*         infile = stdin; */
        /* else */
        /*         infile = fopen(argv[2], "r"); */
        /* FILE *outfile; */
        /* if (argc <= 3) */
        /*         outfile = stdout; */
        /* else if (argc > 3 && !strcmp(argv[3], "--")) */
        /*         outfile = stdout; */
        /* else */
        /*         outfile = fopen(argv[3], "w"); */
        /* switch (argv[1][0]) { */
        /* case 'm': */
        /*         return minify_file(infile, outfile); */
        /* case 'o': */
        /*         return optimize_file(infile, outfile); */
        /* } */
        struct command **commands = calloc(10000, sizeof(void*));
        parse_file(fopen("test.bf", "r"), commands);
        for (int i = 0; commands[i] != 0; ++i)
                printf("%s command %c on %d\n",
                       (commands[i]->special ? "Special" : "Regular"),
                       commands[i]->command, commands[i]->number);
        // eval_commands(commands, stdin, stdout);
        return EXIT_SUCCESS;
}
