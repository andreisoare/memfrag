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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <glib.h>

static FILE *log_file;

static void
ensure_log_file (void)
{
  char filename[100];

  if (log_file)
    return;

  snprintf (filename, sizeof (filename), "gmh-%d.log", getpid ());

  log_file = fopen (filename, "w");
  if (!log_file)
    g_error ("Could not open log file; aborting");
}

static gpointer
mp_malloc (gsize n_bytes)
{
  gpointer buf;

  ensure_log_file ();

  fprintf (log_file, "malloc\n");
  fflush (log_file);

  buf = malloc (n_bytes);
  if (!buf)
    return NULL;

  if (buf)
  	fprintf (log_file, "%p %lu\n", buf, n_bytes);
  else
	fprintf (log_file, "%d %lu\n", 0, n_bytes);
  fflush (log_file);

  return buf;
}
 
static gpointer
mp_realloc(gpointer real, gsize n_bytes)
{
  gpointer new;
  ensure_log_file ();

  fprintf (log_file, "realloc\n");
  fflush (log_file);

  if (n_bytes == 0) {
      if (real)
	free (real);

      new = NULL;
  }
  else {
      new = realloc (real, n_bytes);
      if (!new)
	return NULL;
  }
  if (real && new)
  	fprintf (log_file, "%p %p %lu\n", real, new, n_bytes);
  else if (real)
  	fprintf (log_file, "%p %d %lu\n", real, 0, n_bytes);
  else if (new)
  	fprintf (log_file, "%d %p %lu\n", 0, new, n_bytes);
  else
  	fprintf (log_file, "%d %d %lu\n", 0, 0, n_bytes);
  fflush (log_file);

  return new;
}

static void
mp_free (gpointer buf)
{
  ensure_log_file ();

  fprintf (log_file, "free\n");
  fflush (log_file);

  free (buf);

  if (buf)
  	fprintf (log_file, "%p\n", buf);
  else
  	fprintf (log_file, "%d\n", 0);
  fflush (log_file);
}


/* init/free */

static void
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
__attribute__ ((constructor))
#endif /* !__GNUC__ */
mp_init (void)
{
  GMemVTable mp_mem_table = {
    mp_malloc,
    mp_realloc,
    mp_free,
    NULL,
    NULL,
    NULL,
  };

  g_mem_set_vtable (&mp_mem_table);
}

