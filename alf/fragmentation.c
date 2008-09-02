/* Allocation Lifetime and Fragmentation (alf)
 * 
 * Copyright (C) 2008 Andrei Soare
 *               contact: andrei.soare@gmail.com
 * 
 * alf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/********************************************************
 * This program determines the fragmentation percentage *
 * after each allocation is deallocated                 *
 *                                                      *
 * Input lines can be one of the following:             *
 *    (1) m szB ptr                                     *
 *    (2) f ptr                                         *
 *    (3) time_allocated lifetime heap_szB where        *
 *                                                      *
 * Output lines have the following format:              *
 *    time_allocated lifetime fragmentationi where      *
 *                                                      *
 * After each free comes a line (3)                     *
 *                                                      *
 ********************************************************/ 

#include <stdio.h>
#include <stdlib.h>

#define MAX_H 200                             // maximum level of skip list

typedef struct _Allocation_List {
    struct _Allocation_List **next, *prev;
    size_t szB;
    void *ptr;
} Allocation_List;

int H = 0, k=1;                               // current maximum level of skip list
Allocation_List *first = NULL, *last = NULL;  // first and last elements of skip list
Allocation_List *path[MAX_H+1];               // path[i]=element on level i, previous 
                                              // to the searched element

Allocation_List *build_node (size_t szB, void *ptr, int *nodeH)
{
   int r, i;
   Allocation_List *node = malloc(sizeof(Allocation_List));

   node->szB = szB;
   node->ptr = ptr;
   node->prev = NULL;
   *nodeH = rand() % (MAX_H+1);
   if (*nodeH > MAX_H) *nodeH = MAX_H;
   node->next = (Allocation_List **) malloc(sizeof(Allocation_List *) * (*nodeH+1)); 
   for (i = 0; i <= *nodeH; i++)
      node->next[i] = NULL;
   return node;
}

int search_node (void *ptr)
{
   Allocation_List *node;
   int i;

   if (first == NULL) return 0;

   for (i = H+1; i <= MAX_H; i++) path[i] = first;
   for (node = first, i = H; i >= 0; i--) {
      while (node->next[i] != NULL && node->next[i]->ptr < ptr)
         node = node->next[i];
      path[i] = node;
   }
   if (path[0]->next[0] != NULL && path[0]->next[0]->ptr == ptr) return 1;
   else return 0;
} 
    
void add_node (size_t szB, void *ptr)
{
   Allocation_List *node;
   int i, nodeH;

   if (first == NULL) {        // first build a sentinel node
      first = (Allocation_List *) malloc(sizeof(Allocation_List));
      first->szB = 0;
      first->ptr = NULL;
      first->prev = NULL;
      first->next = (Allocation_List **) malloc(sizeof(Allocation_List *) * (MAX_H+1));
      for (i = 0; i <= MAX_H; i++) first->next[i] = NULL; 
   }
   
   if (search_node(ptr)) return;
   node = build_node (szB, ptr, &nodeH);
   if (path[0]->next[0] != NULL)
       path[0]->next[0]->prev = node;
   node->prev = path[0];
   for (i = 0; i <= nodeH; i++) {
       node->next[i] = path[i]->next[i];
       path[i]->next[i] = node;
   }

   if (node->next[0] == NULL) last = node;

   if (nodeH > H) H = nodeH;
}
   
void del_node (void *ptr)
{
   Allocation_List *node;
   int i;

   if (!search_node(ptr)) return;

   node = path[0]->next[0];
   if (node == last) last = node->prev;
   if (node->next[0] != NULL) 
      node->next[0]->prev = path[0];
   for (i = 0; i <= MAX_H; i++)
      if (path[i]->next[i] == node)
         path[i]->next[i] = node->next[i];
   free(node);
}

int main (int argc, char **argv) {   
    FILE *fi = fopen (argv[1], "r");
    FILE *fo = fopen (argv[2], "w");
    char *line = (char *) malloc (200), *where = (char *) malloc (200);
    double fragmentation;
    size_t real_heap_szB, heap_szB, szB, time, lifetime;
    void *ptr;
    
    while (fgets (line, 200, fi)) {
        if (line[0] == 'm') {
            sscanf (line, "m %lu %p", &szB, &ptr);
            add_node (szB, ptr);
        }
        else if (line[0] == 'f') {
            k++;
            sscanf (line, "f %p", &ptr);
            del_node (ptr);
            fscanf (fi, "%lu %lu %lu", &time, &lifetime, &heap_szB);
            fgets (where, 200, fi);
            real_heap_szB = last->ptr - first->next[0]->ptr + last->szB;
            fragmentation = real_heap_szB < heap_szB ? 0 : (double)(real_heap_szB - heap_szB) / real_heap_szB * 100;
            fprintf(fo, "%12lu %12lu %12.2lf %s", time, lifetime, fragmentation, where);
        }
        else if (line[0] == '=' || line[0] == '*' || line[0] == '\n') ;
        else { 
            printf("error - bad input at line %d: %s\n\n", k, line);
            return 1;
        }
        k++;
    }

    fclose(fi);
    fclose(fo);

    return 0;
}

