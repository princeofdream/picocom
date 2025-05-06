/*
 * =====================================================================================
 *
 *       Filename:  ppcom_cli.cpp
 *
 *    Description: ppcom cli
 *
 *        Version:  1.0
 *        Created:  2021年09月17日 14时31分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <basic.h>



void ppcom_cli_print_usage(const char *pname)
{
    printf("%s -s /tmp/proc-x.sock\n", pname);
}

int main(int argc, char *argv[])
{
	int ch;

	while (true)
	{
		ch = getopt(argc, argv, "tip:hs:");
		if (ch < 0)
			break;
		printf("optind: %d, ch: %#x\n", optind, ch);
		switch (ch)
		{
		case 't':
			break;
		case 'i':
			break;
		case 'p':
			break;
		case 's':
			break;
		case 'h':
            ppcom_cli_print_usage(argv[0]);
			exit(0);
		}
	}


	return 0;

}


