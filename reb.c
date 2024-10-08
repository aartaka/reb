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
#define success 0 ==

#define len(...) \
	sizeof(__VA_ARGS__) / sizeof(__VA_ARGS__[0])

#ifndef MEMSIZE
#define MEMSIZE 100000
#endif

#ifndef CELLTYPE
#define CELLTYPE unsigned char
#endif

#define COMMAND_CHARS "][+.,<>#=(){}-"
#define OP_REGEX                                                        \
        "\\([0-9]*\\)\\(`\\{0,1\\}\\)\\([0-9]*\\)\\([" COMMAND_CHARS "]\\)"

// *INDENT-OFF*
struct replacement {
	char *pattern;
	char *replacement;
} optimizations[] = {
	// Questionable: optimize empty loops to nothing. Otherwise
	// these are endless loops, which make no sense, right?
	{"\\[\\]",                                ""},
	// Comment loops
	{"^\\[[^][]*\\]",                         ""},
	{"\\]\\[[^][]*\\]",                       "]"},
	// Duplicates.
	{"^\\([-+<>]\\)\\1",                      "2\\1"},
	{"\\([^0-9]\\)\\([-+<>]\\)\\2",           "\\12\\2"},
	{"\\<2\\([-+<>]\\)2\\1",                  "4\\1"},
	{"\\<4\\([-+<>]\\)4\\1",                  "8\\1"},
	{"\\<8\\([-+<>]\\)8\\1",                  "16\\1"},
	{"\\<16\\([-+<>]\\)16\\1",                "32\\1"},
	{"\\<32\\([-+<>]\\)32\\1",                "64\\1"},
	{"\\<64\\([-+<>]\\)64\\1",                "128\\1"},
	{"\\<128\\([-+<>]\\)128\\1",              ""},
	// Numbers up to 127, structured around primes
#define X(BASE, ADDEND, ...)                                            \
	{"\\<" #BASE "\\([-+<>]\\)" #ADDEND "\\1", __VA_ARGS__ "\\1"}
// Yes, it's tedious and manually typed in. So what?
	X(2, , "3"), /* 4 */
	X(4, , "5"), X(4, 2, "6"),
	X(4, 3, "7"), /* 8 */ X(8, , "9"), X(8, 2, "10"),
	X(8, 3, "11"), X(8, 4, "12"),
	X(8, 5, "13"), X(8, 6, "14"), X(8, 7, "15"), /* 16 */
	X(16, , "17"), X(16, 2, "18"),
	X(16, 3, "19"), X(16, 4, "20"), X(16, 5, "21"), X(16, 6, "22"),
	X(16, 7, "23"), X(16, 8, "24"), X(16, 9, "25"),
	X(16, 10, "26"), X(16, 11, "27"),
	X(16, 13, "29"), X(16, 14, "30"),
	X(16, 15, "31"), /* 32 */
	X(32, , "33"), X(32, 2, "34"), X(32, 3, "35"), X(32, 4, "36"),
	X(32, 5, "37"), X(32, 6, "38"), X(32, 7, "39"), X(32, 8, "40"),
	X(32, 9, "41"), X(32, 10, "42"),
	X(32, 11, "43"), X(32, 12, "44"), X(32, 13, "45"), X(32, 14, "46"),
	X(32, 15, "47"), X(32, 16, "48"), X(32, 17, "49"),
	X(32, 18, "50"), X(32, 19, "51"), X(32, 20, "52"),
	X(32, 21, "53"), X(32, 22, "54"), X(32, 23, "55"),
	X(32, 24, "56"), X(32, 25, "57"),
	X(32, 27, "59"), X(32, 28, "60"),
	X(32, 29, "61"), X(32, 30, "62"),
	X(32, 31, "63"), /* 64 */ X(64, , "65"), X(64, 2, "66"),
	X(64, 3, "67"), X(64, 4, "68"), X(64, 5, "69"), X(64, 6, "70"),
	X(64, 7, "71"), X(64, 8, "72"),
	X(64, 9, "73"), X(64, 10, "74"), X(64, 11, "75"),
	X(64, 12, "76"), X(64, 13, "77"), X(64, 14, "78"),
	X(64, 15, "79"), X(64, 16, "80"),
	X(64, 17, "81"), X(64, 18, "82"),
	X(64, 19, "83"), X(64, 20, "84"), X(64, 21, "85"), X(64, 22, "86"),
	X(64, 23, "87"), X(64, 24, "88"),
	X(64, 25, "89"), X(64, 26, "90"),
	X(64, 27, "91"), X(64, 28, "92"), X(64, 29, "93"), X(64, 30, "94"),
	X(64, 31, "95"), X(64, 32, "96"),
	X(64, 33, "97"), X(64, 34, "98"), X(64, 35, "99"), X(64, 36, "100"),
	X(64, 37, "101"), X(64, 38, "102"),
	X(64, 39, "103"), X(64, 40, "104"),
	X(64, 41, "105"), X(64, 42, "106"),
	X(64, 43, "107"), X(64, 44, "108"),
	X(64, 45, "109"), X(64, 46, "110"),
	X(64, 47, "111"), X(64, 48, "112"),
	X(64, 49, "113"), X(64, 50, "114"),
	X(64, 51, "115"), X(64, 52, "116"),
	X(64, 53, "117"), X(64, 54, "118"),
	X(64, 55, "119"), X(64, 56, "120"),
	X(64, 57, "121"), X(64, 58, "122"),
	X(64, 59, "123"), X(64, 60, "124"),
	X(64, 61, "125"), X(64, 62, "126"),
	X(64, 63, "127"), /* 128 */
#undef X
	// Copying.
	{"\\[\\([0-9]*\\)>\\([0-9]*\\)+\\1<-\\]", "\\1`\\2}"},
	{"\\[\\([0-9]*\\)<\\([0-9]*\\)+\\1>-\\]", "\\1`\\2{"},
	{"\\[-\\([0-9]*\\)>\\([0-9]*\\)+\\1<\\]", "\\1`\\2}"},
	{"\\[-\\([0-9]*\\)<\\([0-9]*\\)+\\1>\\]", "\\1`\\2{"},
	// Assignment
	{"\\[[0-9]*[+-]\\]",                      "0="},
	{"0=\\([0-9]*\\)+",                       "\\1="},
	// Scan/beacon loops
	{"\\[\\([0-9]*\\)>\\]",                   "\\1`0)"},
	{"\\[\\([0-9]*\\)<\\]",                   "\\1`0("},
	{"-\\[+\\([0-9]*\\)<-\\]",                "\\1`1)"},
	{"-\\[+\\([0-9]*\\)>-\\]",                "\\1`1("},
	{"+\\[-\\([0-9]*\\)<+\\]",                "\\1`255("},
	{"+\\[-\\([0-9]*\\)>+\\]",                "\\1`255)"},
	// No-ops
	{"\\([0-9]*\\)<\\1>",                     ""},
	{"\\([0-9]*\\)>\\1<",                     ""},
	{"\\([0-9]*\\)[=+-],",                    ""},
}, minification = {"[^][+.,<>#-]", ""};
// *INDENT-ON*

#define withreg(regvar, matchvar, ...)          \
        regex_t regvar;                         \
        regmatch_t matchvar[10];                \
        regcomp(&regvar, __VA_ARGS__, 0);
bool
regmatch(regex_t *preg, char *str, regmatch_t *pmatch)
{
	return success regexec(preg, str, 10, pmatch, 0);
}

char *
replace_pattern(char *str, struct replacement re)
{
	withreg(reg, rmatch, re.pattern);
	char *orig = str;
	char buf_[10000];
	char *buf = buf_;
	while (regmatch(&reg, str, rmatch)) {
		memcpy(buf, str, rmatch[0].rm_so);
		buf += rmatch[0].rm_so;
		for (size_t i = 0; re.replacement[i]; ++i) {
			// NOTE: Used to be < 32. 10-31 are undefined.
			if (re.replacement[i] is '\\'
			    and isdigit(re.replacement[i + 1])) {
				regmatch_t match =
				    rmatch[re.replacement[i + 1] - '0'];
				size_t len = match.rm_eo - match.rm_so;
				memcpy(buf, &str[match.rm_so], len);
				buf += len;
				i++;
			} else {
				*buf = re.replacement[i];
				++buf;
			}
		}
		str += rmatch[0].rm_eo;
	}
	strcpy(buf, str);
	strcpy(orig, buf_);
	return orig;
}

int
minify_file(FILE *infile, FILE *outfile)
{
	char str[10000];
	while (fgets(str, 10000, infile))
		fputs(replace_pattern(str, minification), outfile);
	return EXIT_SUCCESS;
}

int
optimize_file(FILE *infile, FILE *outfile)
{
	char str[10000];
	while (fgets(str, 10000, infile)) {
		replace_pattern(str, minification);
		// Repeat multiple times to make sure everything is optimized.
		for (int iter = 0; iter < 5; ++iter)
			for (size_t i = 0; i < len(optimizations); ++i)
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
	char c, str[1000000], *buf = str;
	while (EOF != (c = fgetc(codefile))) {
		if (c is '\r') {
			continue;
		} else if (c is '!') {
			*infile = codefile;
			break;
		} else if (not strchr("\n0123456789`" COMMAND_CHARS, c)) {
			printf("Character '%c' is not recognized by Reb\n\
Clean or minify the input first.\n", c);
			abort();
		}
		*buf++ = c;
	}
	buf = str;
	while (regmatch(&reg, buf, rmatches)) {
		bool tick = regpresent(&rmatches[2]);
		if (tick and regpresent(&rmatches[3]))
			current.argument = atoi(buf + rmatches[3].rm_so);
		else if (not tick and regpresent(&rmatches[1]))
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
run_commands(struct command *commands, FILE *infile, FILE *outfile)
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
			memory[command.offset] += *memory * command.argument;
			*memory = 0;
			break;
		case '{':
			memory[-command.offset] += *memory * command.argument;
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

// *INDENT-OFF*
/* struct replacement compilation[] = { */
/* 	{"\\([0-9]*\\)=",              "\t*memory = \\1;\n"}, */
/* 	{"\\([0-9]*\\)\\([+-]\\)",     "\t*memory \\2= \\1;\n"}, */
/* 	{"\\([0-9]*\\)>",              "\tmemory += \\1;\n"}, */
/* 	{"\\([0-9]*\\)<",              "\tmemory -= \\1;\n"}, */
/* 	{"\\([0-9]*\\)`\\([0-9]*\\))", "\tfor(; *memory != \\2; memory += \\1);\n\t*memory = 0;\n"}, */
/* 	{"\\([0-9]*\\)`\\([0-9]*\\)(", "\tfor(; *memory != \\2; memory -= \\1);\n\t*memory = 0;\n"}, */
/* 	{"\\([0-9]*\\)`\\([0-9]*\\)}", "\tmemory+\\1 += *memory * \\2;\n\t*memory = 0;\n"}, */
/* 	{"\\([0-9]*\\)`\\([0-9]*\\){", "\tmemory-\\1 += *memory * \\2;\n\t*memory = 0;\n"}, */
/* 	{"[",                          "\twhile(*memory) {\n"}, */
/* 	{"]",                          "\t}\n"}, */
/* 	{",",                          "\tif((c=getchar())!=EOF) *memory=c; else *memory = 0;\n"}, */
/* 	{".",                          "\tputchar(*memory);\n"}, */
/* }; */
// *INDENT-ON*

int
compile_commands(struct command *commands, FILE *outfile)
{
	fprintf(outfile,
		"#include <stdio.h>\n#include <string.h>\n#include <stdlib.h>\n");
	fprintf(outfile, "char memory_[%i] = {0};\n", MEMSIZE);
	fprintf(outfile, "int main (void)\n{\n");
	fprintf(outfile, "\tchar *memory = &memory_[%i/2];\n", MEMSIZE);
	fprintf(outfile, "\tchar c;\n");
	for (size_t i = 0; commands[i].command; ++i) {
		struct command command = commands[i];
		switch (command.command) {
		case '+':
			fprintf(outfile, "\t*memory += %i;\n",
				command.argument);
			break;
		case '-':
			fprintf(outfile, "\t*memory -= %i;\n",
				command.argument);
			break;
		case '>':
			fprintf(outfile, "\tmemory += %i;\n", command.argument);
			memory += command.argument;
			break;
		case '<':
			fprintf(outfile, "\tmemory -= %i;\n", command.argument);
			break;
		case ',':
			fprintf(outfile,
				"\tif((c=getchar())!=EOF) *memory=c; else *memory = 0;\n");
			break;
		case '.':
			fprintf(outfile, "\tputchar(*memory);\n");
			break;
		case '[':
			fprintf(outfile, "\twhile(*memory) {\n");
			break;
		case ']':
			fprintf(outfile, "\t}\n");
			break;
		case '=':
			fprintf(outfile, "\t*memory = %i;\n", command.argument);
			break;
		case '}':
			fprintf(outfile, "\tmemory[%u] += *memory * %u;\n\
\t*memory = 0;\n", command.offset, command.argument);
			break;
		case '{':
			fprintf(outfile, "\tmemory[-%u] += *memory * %u;\n\
\t*memory = 0;\n", command.offset, command.argument);
			break;
		case ')':
			fprintf(outfile, "\tfor(; *memory != %u; memory += %u);\n\
\t*memory = 0;\n", command.argument,
				command.offset);
			break;
		case '(':
			fprintf(outfile, "\tfor(; *memory != %u; memory -= %u);\n\
\t*memory = 0;\n", command.argument,
				command.offset);
			break;
		}
	}
	fprintf(outfile, "\treturn EXIT_SUCCESS;\n}\n");
	return EXIT_SUCCESS;
}

int
main(int argc, char *argv[argc])
{
	FILE *infile;
	FILE *bfin = stdin;
	if (argc is 1 or argc >= 2 and not strchr("mforc", argv[1][0])) {
		printf
		    ("Reb is a Brainfuck toolkit using regex for everything.\n\
Available commands:\n\
%s\tf[ormat]   FILE/--\tFormat the FILE as per bf.style and output the result.\n\
%s\tm[inify]   FILE/--\tMinify the FILE and output the result.\n\
%s\to[ptimize] FILE/--\tOutput more efficient Reb format for FILE.\n\
%s\tr[un]      FILE/--\tRun the (minified or optimized) contents of the FILE.\n\
%s\tc[ompile]  FILE/--\tCompile the (minified or optimized) contents of the FILE to C.\n\
\n\
Reb supports:\n\
- Arbitrary precision cells, via compile-time macro CELLTYPE.\n\
- Arbitrary memory size, via compile-time macro MEMSIZE.\n\
- Exclamation mark input in run mode.\n\
- An extended/optimized instruction set (see README for specification.)\n", argv[0], argv[0], argv[0], argv[0], argv[0]);
		return EXIT_SUCCESS;
	}
	if (argc is 2)
		infile = stdin;
	else if (argc > 2 and success strcmp(argv[2], "--"))
		infile = stdin;
	else
		infile = fopen(argv[2], "r");
	struct command commands[100000] = { {0} };
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
		return run_commands(commands, bfin, stdout);
	case 'c':
		parse_file(infile, commands, &bfin);
		return compile_commands(commands, stdout);
	}
	return EXIT_SUCCESS;
}
