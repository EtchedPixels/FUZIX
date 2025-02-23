/* Functions dealing with lists of values
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

void 
appendval(l, v)
  struct vallist *l;
  struct val *v;
{
  v->next = NULL;
  if (l->head == NULL) l->head = l->tail = v;
  else
  { l->tail->next = v; l->tail = v; }
}



/* Searchval searches through a val list for a given val.
 * When a position is found, searchval operates according
 * to the mode value: if true, return the val, else delete
 * the val. If sub is true, substring matches are used.
 */

struct val *
searchval(l, name, mode, sub)
  struct vallist *l;
  char *name;
  int mode;
  bool sub;
{
  struct val *last, *this;
  int len;

  switch (sub)
  {
    case TRUE:
      len = strlen(name);
      for (last = this = l->head; this; last = this, this = this->next)
	if (!strncmp(this->name, name, len))
	  break;
      break;
    case FALSE:
      for (last = this = l->head; this; last = this, this = this->next)
	if (!strcmp(this->name, name))
	  break;
      break;
    default:
      fatal("Bad sub value in searchval");
  }
  if (this == NULL) return (NULL);

  switch (mode)
  {
    case FALSE:
      if (l->head == l->tail)
      { l->head = l->tail = NULL; return (this); }

      if (this == l->head) l->head = this->next;
      else last->next = this->next;

      if (this == l->tail) l->tail = last;
      this->next = NULL;
    case TRUE:
      return (this);
    default:
      fatal("Bad mode value in searchval");
  }
}

/* Save the value into the list in the correct order
 */
void 
saveval(l, v)
  struct vallist *l;
  struct val *v;
{
  struct val *last, *this;

  if (l->head == NULL)
  { l->head = l->tail = v; v->next = NULL; return; }

  for (last = this = l->head; this; last = this, this = this->next)
  { if (!strcmp(this->name, v->name))
    { free(this->val); free(v->name);		/* Overwrite the old val */
      this->val = v->val; this->exported = v->exported;
      free(v); return;
    }
    if (strcmp(this->name, v->name) > 0) break;
  }

  if (this == NULL)
  { l->tail->next = v; v->next = NULL;
    l->tail = v; return;
  }

  if (this == last)
  { v->next = l->head; l->head = v; return; }

  v->next = this; last->next = v;
}

struct val *
pullval(l)
  struct vallist *l;
{
  struct val *temp;

  if (l->head == NULL) return (NULL);

  temp = l->head;
  if (l->head == l->tail)
    l->head = l->tail = NULL;
  else
    l->head = l->head->next;
  temp->next = NULL;
  return (temp);
}

void 
setval(name, val, l)
  char *name, *val;
  struct vallist *l;
{
  struct val *v;

  v = (struct val *) Malloc(sizeof(struct val), "setval val malloc");
  v->name = Malloc(strlen(name) + 1, "setval name malloc");
  v->val = Malloc(strlen(val) + 1, "setval val malloc");
  strcpy(v->name, name); strcpy(v->val, val);
  v->exported = FALSE; saveval(l, v);
}
