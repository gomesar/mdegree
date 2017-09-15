/*
 * custom2.c
 * 
 * Copyright 2017 A Gomes <agomes@lasca.ic.unicamp.br>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * Lista 1 - Questao 6
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define smax 33
//#define ALG_NAME "<Custom2>";
//#define DEBUG 1
//#define VERBOSE 1
int gap = -5;
int match = 3;
int ssmatch = -10;	
int memo[smax][smax];

int alignlocal(int idxi, int idxj, char *s1, char *s2) {
	char align1[2*smax-1];
	char align2[2*smax-1];
	
	int j = idxj;	// sequence alpha (columns)
	int i = idxi;	// sequence beta (lines)
	int idx1 = 0;	// alignment 1
	int idx2 = 0;	// alignment 2
	
	while (memo[i][j] > 0) {
		align1[idx1++] = s1[j-1];
		align2[idx2++] = s2[i-1];
		i--;
		j--;

	}
	
	align1[idx1] = '\0';
	align2[idx2] = '\0';
	
	#ifdef DEBUG
	printf("align1: %s. \nalign2: %s. \n", align1, align2);
	#endif
	
	/* Reverse */
	int aux = strlen(align1);
	int tmp;
	
	printf("\t");
	
	for (tmp=0; tmp < j; tmp++) {	/* head */
		printf("-");
	}
	for (tmp=1; tmp <= aux; tmp++){
		printf("%c", toupper(align1[aux-tmp]));
	}
	for (tmp=idxj; tmp <strlen(s1); tmp++) {	/* tail */
		printf("-");
	}
	
	printf("\n\t");
	for (tmp=0; tmp < i; tmp++) {	/* head */
		printf("-");
	}
	for (tmp=1; tmp <= aux; tmp++){
		printf("%c", toupper(align2[aux-tmp]));
	}
	for (tmp=idxi; tmp <strlen(s2); tmp++) {	/* tail */
		printf("-");
	}
	printf("\n\n");
}

int main(int argc, char **argv)
{
	char s1[smax-1];
	char s2[smax-1];
	
	/* Check */
	if (argc < 3) {
		printf("Use: %s string1 string2\n", argv[0]);
		return 1;
	}
	
	int m, n;
	m = (int) strlen(argv[1]); // Long int to int
	n = (int) strlen(argv[2]);
	
	if (m > sizeof(s1)-1) {
		printf("string1 is too big. (%d)\n", m);
		return 1;
	}
	
	if (n > sizeof(s1)-1) {
		printf("string2 is too big. (%d\n", n);
		return 1;
	}
	// Debug
	#ifdef DEBUG
	printf("########## ########## ##########\n");
	printf("Match: %d, Miss-match: %d, Gap: %d.\n", match, ssmatch, gap);
	printf("Max    : %3ld bytes\n", sizeof(s1));
	printf("argv[1]: %3ld bytes\n", strlen(argv[1]));
	printf("argv[2]: %3ld bytes\n", strlen(argv[2]));
	printf("########## ########## ##########\n\n");
	#endif
	
	/* Initialize */
	strcpy(s1, argv[1]);
	strcpy(s2, argv[2]);
	printf("[!] Starting <Custom2> alignment.\n");
	printf("\tSequence 1: %s\n", s1);
	printf("\tSequence 2: %s\n", s2);
	int i, j;
	
	/* Just dont do it ;)
	for (i=0; i<smax; i++) {
		memo[i][0] = i*gap;
		memo[0][i] = i*gap;
	}
	*/
	
	/* Start */
	int max;
	int runi, runj, set;
	for (i=1; i<=n; i++) {
		for (j=i+1; j<=m; j++) {
			max = 0;
			/* Custom change : disallowing position re-use */
			int ret = j-i > i ? i: j-i;
			
			#ifdef DEBUG
			printf("memo[%d][%d] = %d.\n",i-ret, j-ret, memo[i-ret][j-ret]);
			#endif
			
			if (memo[i-ret][j-ret] == 0) {
				/* Only accept matches */
				max = (s1[j-1] == s2[i-1]) ? memo[i-1][j-1] + match : 0;
				
				#ifdef DEBUG
				printf("[%c - %c]\n", s1[j], s2[i]);
				#endif
			} 
;
			/* Save value */
			memo[i][j] = max;
			#ifdef VERBOSE
			printf("%2d ", max);
			#endif
		}
		#ifdef VERBOSE
		printf("\n");
		#endif
	}
	
	printf("##### ##### ##### Results: \n\t");
	for (j=0; j<m; j++) {
		printf("%c\t", s1[j]);
	}
	printf("\n");
	
	/* Find best alignment value */
	int line, col;
	max = 0;
	
	for (i=1; i<=n; i++) {
		for (j=0; j<=m; j++) {
			if (j==0) {
				printf("%c\t", s2[i-1]);
			} else {
				printf("%2d\t", memo[i][j]);
				if (memo[i][j] > max) {
					line = i;
					col = j;
					max  = memo[i][j];
				}
			}
		}
		printf("\n");
	}
	
	printf("Optimal <Custom2> alignment value: %d.\n", memo[line][col]);
	alignlocal(line, col, s1, s2);
	return 0;
}
