# c-linked-list

forked from <https://github.com/skorks/c-linked-list>

A single-header C library that provides linked list functionality.

## How to use

- Copy <code>linkedlist.h</code> into your project
- <code>#define LINKEDLIST_IMPLEMENTATION</code> before <code>#include "linkedlist.h"</code> in ONE C source file
- See <code>test.c</code> for basic library usage

## User macros

<code>#define</code> any of the following headers before every <code>#include "linkedlist.h"</code> if desired

- <code>LINKEDLIST_DATA_TYPE</code> | default: <code>uint64_t</code> from <code>stdint.h</code> | the data type of each node in a linked list
- <code>LINKEDLIST_DATA_TYPE_FORMAT</code> | default: <code>"%llu\n"</code> | the <code>printf</code> format of linked list node data followed by a newline (THIS MUST BE DEFINED BY USER IF <code>LINKEDLIST_DATA_TYPE</code> IS DEFINED BY USER)
