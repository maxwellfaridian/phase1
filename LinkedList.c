//
//  LinkedList.c
//  phase1XCodeFolder
//
//  Created by Stephen Connolly on 9/3/17.
//  Copyright © 2017 Stephen Connolly. All rights reserved.
//

#include "LinkedList.h"




void addToListTail (struct listNode * head, struct listNode * tail, struct listNode * newNode) {
    if (head == NULL) {
        newNode->next = NULL;
        head = newNode;
        tail = newNode;
    }
    
    else {
        newNode->next = NULL;
        tail->next = newNode;
        tail = newNode;
    }
}

struct listNode * removeFromListHead (struct listNode * head) {
    struct listNode * returnNode = head;
    head = head->next;
    return returnNode;
}
