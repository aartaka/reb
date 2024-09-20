#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#if !defined(__STDC__) || __STDC_VERSION__ < 199901L
#error "Reb uses C99 features, procure a compliant compiler!"
#endif

#if (__STDC_VERSION__ < 202000L)
#include <stdbool.h>
#endif

#include <iso646.h>
#define is ==

#ifndef MEMSIZE
#define MEMSIZE 100000
#endif

#ifndef CELLTYPE
#define CELLTYPE char
#endif

#define COMMAND_CHARS "][+.,<>#=(){}-"
#define OP_REGEX                                                        \
        "\\([0-9]*\\)\\(`\\{0,1\\}\\)\\([0-9]*\\)\\([" COMMAND_CHARS "]\\)"

#define matchlen(match) 15+match

struct optimization {
	char *pattern;
	char replacement[20];
} optimizations[]
// *INDENT-OFF*
= {
	// Minification
	{"[^][+.,<>!#-]",                         {               0}},
	// Questionable: optimize empty loops to nothing. Otherwise
	// these are endless loops, which make no sense, right?
	{"\\[\\]",                                {               0}},
	// Comment loops
	{"^\\[[^][]*\\]",                         {               0}},
	{"\\]\\[[^][]*\\]",                       {             ']'}},
	// Duplicates.
	{"\\(+\\{2,\\}\\)",                       {matchlen(1), '+'}},
	{"\\(-\\{2,\\}\\)",                       {matchlen(1), '-'}},
	{"\\(>\\{2,\\}\\)",                       {matchlen(1), '>'}},
	{"\\(<\\{2,\\}\\)",                       {matchlen(1), '<'}},
	// Copying.
	{"\\[\\([0-9]*\\)>\\([0-9]*\\)+\\1<-\\]", {1, '`', 2,   '}'}},
	{"\\[\\([0-9]*\\)<\\([0-9]*\\)+\\1>-\\]", {1, '`', 2,   '{'}},
	{"\\[-\\([0-9]*\\)>\\([0-9]*\\)+\\1<\\]", {1, '`', 2,   '}'}},
	{"\\[-\\([0-9]*\\)<\\([0-9]*\\)+\\1>\\]", {1, '`', 2,   '{'}},
	// Assignment
	{"\\[[0-9]*[+-]\\]",                      {'0',         '='}},
	{"0=\\([0-9]*\\)+",                       {1,           '='}},
	// Scan/beacon loops
	{"\\[\\([0-9]*\\)>\\]",                   {1, '`', '0', ')'}},
	{"\\[\\([0-9]*\\)<\\]",                   {1, '`', '0', '('}},
	{"-\\[+\\([0-9]*\\)<-\\]",                {1, '`', '1', '('}},
	{"-\\[+\\([0-9]*\\)>-\\]",                {1, '`', '1', '('}},
	{"+\\[-\\([0-9]*\\)<+\\]",                {1, '`', '2', '5', '5', '('}},
	{"+\\[-\\([0-9]*\\)>+\\]",                {1, '`', '2', '5', '5', ')'}},
	// No-ops
	{"\\([0-9]*\\)<\\1>",                     {               0}},
	{"\\([0-9]*\\)>\\1<",                     {               0}},
	{"\\([0-9]*\\)[=+-],",                    {             ','}},
};
// *INDENT-ON*

#define withreg(regvar, matchvar, ...)          \
        regex_t regvar;                         \
        regmatch_t matchvar[1000];              \
        regcomp(&regvar, __VA_ARGS__, 0);
bool
regmatch(regex_t *preg, char *str, regmatch_t *pmatch)
{
	return !regexec(preg, str, 1000, pmatch, 0);
}

char *
replace_pattern(char *str, struct optimization opt)
{
	withreg(reg, rmatch, opt.pattern);
	char copy[10000];
	strcpy(copy, str);
	while (regmatch(&reg, str, rmatch)) {
		int offset = rmatch[0].rm_so;
		for (int i = 0; opt.replacement[i]; ++i) {
			if (opt.replacement[i] < 15) {
				regmatch_t match =
				    rmatch[(size_t)opt.replacement[i]];
				memmove(str + offset, &copy[match.rm_so],
					match.rm_eo - match.rm_so);
				offset += match.rm_eo - match.rm_so;
			} else if (15 < opt.replacement[i]
				   and opt.replacement[i] < ' ') {
				regmatch_t match =
				    rmatch[opt.replacement[i] - 15];
				offset +=
				    sprintf(str + offset, "%d",
					    match.rm_eo - match.rm_so);
			} else {
				str[offset++] = opt.replacement[i];
			}
		}
		strcpy(str + offset, copy + rmatch[0].rm_eo);
		strcpy(copy, str);
	}
	return str;
}

int
minify_file(FILE *infile, FILE *outfile)
{
	char str[10000];
	// Minification pattern is the first optimization.
	struct optimization minification = optimizations[0];
	while (fgets(str, 10000, infile))
		fputs(replace_pattern(str, minification), outfile);
	return EXIT_SUCCESS;
}

int
optimize_file(FILE *infile, FILE *outfile)
{
	char str[10000];
	while (fgets(str, 10000, infile)) {
		// Repeat multiple times to make sure everything is optimized.
		for (int iter = 0; iter < 5; ++iter)
			// Ignoring minimization rule on later passes.
			for (size_t i = (iter ? 1 : 0);
			     i < (sizeof(optimizations)
				  / sizeof(struct optimization)); ++i)
				replace_pattern(str, optimizations[i]);
		fputs(str, outfile);
	}
	return EXIT_SUCCESS;
}

int
format_file(FILE *infile, FILE *outfile)
{
	withreg(blank_reg, blank_matches, "^[[:space:]]*$");
	withreg(normal_reg, normal_matches, "^[[:space:]]*\\(.*\\)$");
	char str[10000];
	int depth = 0;
	while (fgets(str, 10000, infile)) {
		if (regmatch(&blank_reg, str, blank_matches)) {
			fputs("\n", outfile);
		} else if (regmatch(&normal_reg, str, normal_matches)) {
			char *buf = str;
			int leading_close_brackets = 0;
			buf += normal_matches[1].rm_so;
			while (*buf is ']')
				leading_close_brackets++, buf++;
			depth -= leading_close_brackets;
			for (int i = 0; i < depth; ++i)
				fputc(' ', outfile);
			while (leading_close_brackets--)
				fputc(']', outfile);
			while (*buf) {
				depth += (*buf is '[') - (*buf is ']');
				fputc(*buf++, outfile);
			}
		}
	}
	return EXIT_SUCCESS;
}

struct command {
	int argument;
	int offset;
	char command;
};

bool
regpresent(regmatch_t *pmatch)
{
	return pmatch->rm_so != pmatch->rm_eo;
}

void
parse_file(FILE *codefile, struct command *commands, FILE **infile)
{
	withreg(reg, rmatches, OP_REGEX);
	struct command current = { 1, 1 };
	char c, str[1000000] = { 0 }, *buf = str;
	while (EOF != (c = fgetc(codefile))) {
		if (c is '\r') {
			continue;
		} else if (c is '!') {
			*infile = codefile;
			break;
		} else if (!strchr("\n0123456789`" COMMAND_CHARS, *buf++ = c)) {
			printf("Character '%c' is not recognized by Reb\n\
Clean or minify the input first, otherwise expect breakages.\n", c);
			abort();
		}
	}
	buf = str;
	while (regmatch(&reg, buf, rmatches)) {
		bool tick = regpresent(&rmatches[2]);
		if (tick and regpresent(&rmatches[3]))
			current.argument = atoi(buf + rmatches[3].rm_so);
		else if (!tick and regpresent(&rmatches[1]))
			current.argument = atoi(buf + rmatches[1].rm_so);
		// Faulty: offset is there only if there is
		// argument.
		if (tick and regpresent(&rmatches[1]))
			current.offset = atoi(buf + rmatches[1].rm_so);
		current.command = buf[rmatches[4].rm_so];
		commands->argument = current.argument;
		commands->command = current.command;
		commands->offset = current.offset;
		current.argument = current.offset = 1;
		++commands;
		buf += rmatches[4].rm_eo;
	}
}

CELLTYPE memory_[MEMSIZE] = { 0 };

CELLTYPE *memory = &memory_[MEMSIZE / 2];

int
eval_commands(struct command *commands, FILE *infile, FILE *outfile)
{
	int depth = 0;
	for (size_t i = 0; commands[i].command; ++i) {
		/* printf("%c", commands[i].command); */
		struct command command = commands[i];
		char c;
		switch (command.command) {
		case '+':
			*memory += command.argument;
			break;
		case '-':
			*memory -= command.argument;
			break;
		case '>':
			memory += command.argument;
			break;
		case '<':
			memory -= command.argument;
			break;
		case ',':
			if (EOF != (c = getc(infile)))
				*memory = c;
			else
				*memory = 0;
			break;
		case '.':
			putc(*memory, outfile);
			break;
		case '[':
			// Stolen from microbf, will refactor later.
			if (!*memory)
				while ((depth += (commands[i].command is '[')
					- (commands[i].command is ']')))
					i++;
			break;
		case ']':
			// Stolen from microbf, will refactor later.
			if (*memory)
				while ((depth += (commands[i].command is ']')
					- (commands[i].command is '[')))
					i--;
			break;
		case '=':
			*memory = command.argument;
			break;
		case '}':
			*(memory + command.offset) +=
			    *memory * command.argument;
			*memory = 0;
			break;
		case '{':
			*(memory - command.offset) +=
			    *memory * command.argument;
			*memory = 0;
			break;
		case ')':
			if (command.offset is 1)
				memory =
				    memchr(memory, command.argument,
					   MEMSIZE / 2);
			else
				for (; *memory != command.argument;
				     memory += command.offset) ;
			*memory = 0;
			break;
		case '(':
			for (; *memory != command.argument;
			     memory -= command.offset) ;
			*memory = 0;
			break;
		case '#':
			for (int i = 0, max =
			     (command.argument == 1 ? 10 : command.argument);
			     i < max; ++i)
				// Print the []-wrapped num/char cells
				// with ^-prefixed current cell
				fprintf(outfile,
					"[%s%d/%c] ",
					((memory + i - max / 2) ==
					 memory ? "^" : ""),
					*(memory + i - max / 2),
					*(memory + i - max / 2));
			fputs("\n", outfile);
			break;
		}
	}
	return EXIT_SUCCESS;
}

int
main(int argc, char *argv[argc])
{
	FILE *infile;
	FILE *bfin = stdin;
	if (argc == 1 or (argc >= 2 and !strchr("mfor", argv[1][0]))) {
		printf
		    ("Reb is a Brainfuck toolkit using regex for everything.\n\
Available commands:\n\
%s\tm[inify]   FILE/--\tMinify the FILE and output the result.\n\
%s\tf[ormat]   FILE/--\tFormat the FILE as per bf.style and output the result.\n\
%s\to[ptimize] FILE/--\tOutput more efficient Reb format for FILE.\n\
%s\tr[un]      FILE/--\tRun the (minified or optimized) contents of the FILE.\n", argv[0], argv[0], argv[0], argv[0]);
		return EXIT_SUCCESS;
	}
	if (argc == 2)
		infile = stdin;
	else if (argc > 2 and !strcmp(argv[2], "--"))
		infile = stdin;
	else
		infile = fopen(argv[2], "r");
	struct command commands[100000] = { { 0 } };
	switch (argv[1][0]) {
	case 'm':
		return minify_file(infile, stdout);
	case 'f':
		return format_file(infile, stdout);
	case 'o':
		return optimize_file(infile, stdout);
	case 'r':
		parse_file(infile, commands, &bfin);
		/* for (int i = 0; commands[i].command; ++i) */
		/*         printf("Command %c on %d over %d\n", */
		/*                commands[i].command, commands[i].argument, commands[i].offset); */
		return eval_commands(commands, bfin, stdout);
	}
	return EXIT_SUCCESS;
}
