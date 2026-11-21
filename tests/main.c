#include <stdio.h>

int test_var (FILE *out);
int test_puf (FILE *out);
int test_tra (FILE *out);
int test_cmi (FILE *out);
int test_cmo (FILE *out);
int test_dispobj (FILE *out);
int test_menu (FILE *out);
int test_help (FILE *out);

int main (void)
{
	FILE *out = fopen ("RESULT.TXT", "w");
	int failures;

	if (!out) return 1;
	failures = test_var (out);
	failures += test_puf (out);
	failures += test_tra (out);
	failures += test_cmi (out);
	failures += test_cmo (out);
	failures += test_dispobj (out);
	failures += test_menu (out);
	failures += test_help (out);
	fprintf (out, "RESULT %d\n", failures);
	if (fclose (out) != 0) return 1;
	return failures ? 1 : 0;
}
