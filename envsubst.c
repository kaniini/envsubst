/*
 * Copyright (c) 2021 Ariadne Conill <ariadne@dereferenced.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This software is provided 'as is' and without any warranty, express or
 * implied.  In no event shall the authors be liable for any damages arising
 * from the use of this software.
 */

#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
usage(void)
{
	printf("usage: envsubst [OPTIONS] [VARIABLE-NAMES...]\n");
	printf("       VARIABLE-NAMES can be specified in shell format or bare.\n");
	exit(EXIT_SUCCESS);
}

void
version(void)
{
	printf("envsubst " ENVSUBST_VERSION "\n");
	exit(EXIT_SUCCESS);
}

static char **variables = NULL;
static size_t variable_count = 0;

void
dump_variables(void)
{
	if (!variable_count)
	{
		fprintf(stderr, "envsubst: no variables defined\n");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < variable_count; i++)
	{
		printf("%s\n", variables[i]);
	}
}

const char *
normalize_variable(const char *var, bool require_sigil)
{
	static char scratch[4096];

	if (*var == '$')
	{
		var++;

		if (*var == '{')
		{
			var++;
		}
	}
	else if (require_sigil)
	{
		return NULL;
	}

	strlcpy(scratch, var, sizeof scratch);

	char *p = strchr(scratch, '}');
	if (p != NULL)
	{
		*p = '\0';
	}

	return scratch;
}

void
push_variable(const char *var)
{
	variable_count++;
	variables = reallocarray(variables, variable_count + 1, sizeof (void *));

	if (variables == NULL)
	{
		fprintf(stderr, "envsubst: reallocarray: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	variables[variable_count - 1] = strdup(var);
}

void
process_variable(const char *var)
{
	const char *var_token = normalize_variable(var, false);

	if (var_token == NULL)
	{
		fprintf(stderr, "envsubst: failed to process '%s'\n", var_token);
		return;
	}

	push_variable(var_token);
}

void
process_argument(char *arg)
{
	char *token = strtok(arg, ",");

	for (; token != NULL; token = strtok(NULL, ","))
	{
		process_variable(token);
	}
}

bool
allow_variable(const char *token)
{
	if (!variable_count)
	{
		return true;
	}

	for (size_t i = 0; i < variable_count; i++)
	{
		if (!strcmp(token, variables[i]))
		{
			return true;
		}
	}

	return false;
}

void
print_variable(FILE *stream, const char *token, const char *orig_token)
{
	if (allow_variable(token))
	{
		char *envp = getenv(token);

		if (envp == NULL)
		{
			return;
		}

		fprintf(stream, "%s", envp);
	}
	else
	{
		fprintf(stream, "%s", orig_token);
	}
}

void
process_input(FILE *stream)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	while ((nread = getline(&line, &len, stream)) != -1)
	{
		for (char *p = line; *p != '\0'; p++)
		{
			switch (*p)
			{
			case '$': {
				char *end_p = strpbrk(p, " \t\r\n");
				char ospace = *end_p;
				*end_p = '\0';

				const char *token = normalize_variable(p, true);
				print_variable(stdout, token, p);

				/* we want to still process the whitespace */
				*end_p = ospace;
				p = end_p - 1;

				break;
			}
			default:
				fputc(*p, stdout);
				break;
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	bool want_variables = false;
	const struct option opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"variables", no_argument, NULL, 'v'},
		{"version", no_argument, NULL, 'V'},
		{0, 0, 0, 0},
	};

	for (;;)
	{
		int c = getopt_long(argc, argv, "hvV", opts, NULL);

		if (c == -1)
		{
			break;
		}
		else if (c == 'h')
		{
			usage();
		}
		else if (c == 'v')
		{
			want_variables = true;
		}
		else if (c == 'V')
		{
			version();
		}
	}

	for (int c = optind; c < argc; c++)
	{
		process_argument(argv[c]);
	}

	if (want_variables)
	{
		dump_variables();
		return EXIT_SUCCESS;
	}

	process_input(stdin);

	return EXIT_SUCCESS;
}