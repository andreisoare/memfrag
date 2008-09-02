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



/* Output format:
malloc
adr size

realloc
old new size

free
adr
*/

#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mutex;
static pthread_mutexattr_t mta;

static void my_init_hook (void);
static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);
static void *my_realloc_hook (void*, size_t, const void*);

void (*__malloc_initialize_hook) (void) = my_init_hook;
static void *old_malloc_hook;
static void *old_free_hook;
static void *old_realloc_hook;

static FILE *log_file;
 
static void
ensure_log_file (void)
{
	char filename[100];

	if (log_file)
		return;

	snprintf (filename, sizeof (filename), "mh-%d.log", getpid());

	log_file = fopen (filename, "w");
}

static void
my_init_hook (void)
{
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_realloc_hook = __realloc_hook;
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook = my_realloc_hook;
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init (&mutex, &mta);
}

static void *
my_malloc_hook (size_t size, const void *caller)
{
	void *result;

	pthread_mutex_lock (&mutex);

	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__realloc_hook= old_realloc_hook;

	/* log_file */
	ensure_log_file ();
	fprintf(log_file, "malloc\n");
	fflush(log_file);

	/* Call recursively */
	result = malloc (size);

	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_realloc_hook= __realloc_hook;

	/* log_file */
	if (result)
		fprintf (log_file,"%p %lu\n", result, size);
	else
		fprintf (log_file,"%d %lu\n", 0, size);
	fflush(log_file);

	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook= my_realloc_hook;

	pthread_mutex_unlock (&mutex);

	return result;
}

static void *
my_realloc_hook (void *ptr, size_t size, const void *caller)
{
	void *result;

	pthread_mutex_lock (&mutex);

	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__realloc_hook= old_realloc_hook; 

	/* log_file */
	ensure_log_file ();
	fprintf(log_file, "realloc\n");
	fflush(log_file);

	/* Call recursively */
	result = realloc (ptr, size);

	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_realloc_hook= __realloc_hook; 

	/* log_file */
	if (ptr && result)
		fprintf (log_file,"%p %p %lu\n", ptr, result, size);
	else if (ptr)
		fprintf (log_file,"%p %d %lu\n", ptr, 0, size);
	else if (result)
		fprintf (log_file,"%d %p %lu\n", 0, result, size);
	else
		fprintf (log_file,"%d %d %lu\n", 0, 0, size);
	fflush(log_file);

	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook= my_realloc_hook;

	pthread_mutex_unlock (&mutex);

	return result;
}

static void
my_free_hook (void *ptr, const void *caller)
{
	pthread_mutex_lock (&mutex);

	/* Restore all old hooks */
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	__realloc_hook= old_realloc_hook; 

	/* log_file */
	ensure_log_file ();
	fprintf(log_file, "free\n");
	fflush(log_file);

	/* Call recursively */
	free (ptr);

	/* Save underlying hooks */
	old_malloc_hook = __malloc_hook;
	old_free_hook = __free_hook;
	old_realloc_hook= __realloc_hook;

	/* log_file */
	if (ptr)
		fprintf (log_file,"%p\n", ptr);
	else
		fprintf (log_file,"%d\n", 0);
	fflush(log_file);

	/* Restore our own hooks */
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook= my_realloc_hook; 

	pthread_mutex_unlock (&mutex);
}

