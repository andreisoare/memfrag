/* Heap Pictures (bprof)
 * 
 * Copyright (C) 2008 Andrei Soare
 *               contact: andrei.soare@gmail.com
 * 
 * bprof is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bprof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>


#define M 666013


typedef void * Ptr;

typedef struct Node {
	Ptr adr; // key
	size_t size;
	struct Node *lnk;
} Node;

typedef Node *Hash[M];


void initH (Hash h) {
	int i;
	for (i=0; i<M; i++)
		h[i] = NULL;
}

int hash_gen (Ptr key) {
	return (size_t) key % M;
}

void addH (Hash h, Ptr adr, int size) {
	int k = hash_gen (adr);
	Node *p = (Node *) malloc(sizeof(Node));
	p->adr = adr;
	p->size = size;
	p->lnk = h[k];
	h[k] = p;
}

void delH (Hash h, Ptr adr) {
	int k = hash_gen (adr);
	Node *p = h[k], *q;
	if(!p) return;
	if (p->adr == adr) {
		h[k] = p->lnk;
		free(p);
	}
	else {
		while (p->lnk && p->lnk->adr != adr)
			p = p->lnk;
		if (p->lnk) {
			q = p->lnk;
			p->lnk = q->lnk;
			free(q);
		}
	}
}

Node *locH (Hash h, Ptr adr) {
	int k = hash_gen(adr);
	Node *p = h[k];
	while ( p && p->adr != adr )
		p = p->lnk;
	return p;
}

int main (int argc, char **argv) {
	
	FILE *fi = fopen(argv[1], "r");
	FILE *fo = fopen(argv[2], "w");
	Hash h;
	Ptr adr, old;
	size_t size;
	char *s = (char *) malloc(10);
	Node *p;
	int i=0;
	
    if (!fi) { 
        printf("Invalid input file\n");
        return 0;
    }

    if (!fo) {
        printf("Invalid output file\n");
        return 0;
    }

	initH(h);
	while( fgets(s, 10, fi) ) // s will have a maximum size of 9: realloc\n\0
		switch (s[0]) {
			case 'm': {
				fscanf(fi, "%p %lu\n", &adr, &size);
				addH(h, adr, size);
				fprintf(fo, "USED\t%p\t%lu\n", adr, size);
				break;
			}
			case 'f': {
				fscanf(fi, "%p", &adr);
				if (adr) {
                                    p = locH(h, adr);
				    if (p) {
					fprintf(fo, "FREE\t%p\t%lu\n", adr, p->size);
					delH(h,adr);
				    }
				    else fprintf(stderr, "[%d]Free error: %p unallocated before\n", ++i,adr);
				}
				break;
			}
			case 'r': {
				fscanf(fi, "%p %p %lu", &old, &adr, &size);
				if (old==NULL && size==0) break;
				if (old==NULL) {
					fprintf(fo, "USED\t%p\t%lu\n", adr, size);
                                        addH(h,adr,size);
                                }
				else if (size==0) {
					p = locH(h, old);
					if (p) fprintf(fo, "FREE\t%p\t%lu\n", old, p->size);
					else fprintf(stderr, "[%d]Realloc error: %p unallocated before\n", ++i,old);
				}
				else {
					p = locH(h, old);
					if (p) {
						fprintf(fo, "FREE\t%p\t%lu\n", old, p->size);
						fprintf(fo, "USED\t%p\t%lu\n", adr, size);
						delH(h,old);
						addH(h,adr,size);
					}
					else fprintf(stderr, "[%d]Realloc error: %p unallocated before\n", ++i,old);
				}
				break;
			}
		}
	fclose(fi);
	fclose(fo);
	return 0;
}
