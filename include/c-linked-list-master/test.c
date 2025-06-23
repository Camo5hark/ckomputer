#include <stdio.h>
#include <stdlib.h>

#define LINKEDLIST_IMPLEMENTATION
#include "linkedlist.h"

int main() {
  ll_List *list = ll_makelist();

  ll_add(1, list);
  ll_add(20, list);
  ll_add(2, list);
  ll_add(5, list);
  ll_add(8, list);
  ll_add(9, list);
  ll_add(13, list);
  ll_display(list);
  puts("");

  ll_delete(2, list);
  ll_display(list);
  puts("");

  ll_delete(1, list);
  ll_display(list);
  puts("");

  ll_delete(20, list);
  ll_display(list);
  puts("");

  ll_reverse(list);
  printf("Reversed: using three pointers. \n");
  ll_display(list);

  ll_reverse_using_two_pointers(list);
  printf("Reversed: using to pointers. \n");
  ll_display(list);

  ll_destroy(list);
  return 0;
}
