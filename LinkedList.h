//
//  LinkedList.h
//  phase1XCodeFolder
//
//  Created by Stephen Connolly on 9/3/17.
//  Copyright © 2017 Stephen Connolly. All rights reserved.
//

#ifndef LinkedList_h
#define LinkedList_h

#include <stdio.h>

/* -------------------------- Structs ------------------------------------- */

typedef struct listNode {
    struct listNode * next;
    struct procStruct * process;
} listNode;

extern void addToListTail (struct listNode * head, struct listNode * tail, struct listNode * newNode);
extern struct listNode * removeFromListHead (struct listNode * head);

#endif /* LinkedList_h */
