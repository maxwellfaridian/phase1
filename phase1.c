/* ------------------------------------------------------------------------
   phase1.c

   University of Arizona
   Computer Science 452
   Fall 2015

   ------------------------------------------------------------------------ */

#include "phase1.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "kernel.h"
#include "LinkedList.h"

/* ------------------------- Prototypes ----------------------------------- */
int sentinel (char *);
extern int start1 (char *);
void dispatcher(void);
void launch();
static void checkDeadlock();
void initializeProcessTable();
void initializeReadyList();
void pushToReadyList();
struct procStruct * popFromReadyList();
void initializeBlockList();
void dumpReadyList();
void pushToReadyList(struct procStruct *);
struct procStruct * popFromReadyList();
int isKernel();
int getNextProcSlot();
void initializeInterrupts();
void disableInterrupts();


/* -------------------------- Structs ------------------------------------- */




/* -------------------------- Globals ------------------------------------- */
// Indexes for block list
struct listNode * blockListHead;
struct listNode * blockListTail;

// Indexes in ReadyList
struct listNode * readyList[6] ;
struct listNode * priorityTailPtrs[6];

// Patrick's debugging global variable...
int debugflag = 1;

// the process table
procStruct procTable[MAXPROC];

// Process lists
static procPtr ReadyList;

// current process ID
procPtr Current;

// the next pid to be assigned
unsigned int nextPid = SENTINELPID;


/* -------------------------- Functions ----------------------------------- */
/* ------------------------------------------------------------------------
   Name - startup
   Purpose - Initializes process lists and clock interrupt vector.
             Start up sentinel process and the test process.
   Parameters - argc and argv passed in by USLOSS
   Returns - nothing
   Side Effects - lots, starts the whole thing
   ----------------------------------------------------------------------- */
void startup(int argc, char *argv[]) {
    int result; /* value returned by call to fork1() */

    /* initialize the process table */
    if (DEBUG && debugflag) {
        USLOSS_Console("startup(): Initializing process table, ProcTable[]\n");
    }
    initializeProcessTable();
    
    // Initialize the Ready list, initialize block list.
    if (DEBUG && debugflag) {
        USLOSS_Console("startup(): Initializing the Ready list & the Block List\n");
    }
    initializeReadyList();
    initializeBlockList();

    // Initialize the clock interrupt handler, etc. (Other required interupts)
    initializeInterrupts();

    // startup a sentinel process
    if (DEBUG && debugflag) {
        USLOSS_Console("startup(): calling fork1() for sentinel\n");
    }
    
    result = fork1("sentinel", sentinel, NULL, USLOSS_MIN_STACK, SENTINELPRIORITY);
    if (result < 0) {
        if (DEBUG && debugflag) {
            USLOSS_Console("ERROR: startup(): fork1 of sentinel returned an error. Halting.\n ");
        }
        USLOSS_Halt(1);
    }
  
    // start the test process
    if (DEBUG && debugflag) {
        USLOSS_Console("startup(): calling fork1() for start1().\n");
    }
    
    result = fork1("start1", start1, NULL, 2 * USLOSS_MIN_STACK, 1);
    if (result < 0) {
        USLOSS_Console("ERROR: startup(): fork1 for start1 returned an error. Halting.\n");
        USLOSS_Halt(1);
    }

    USLOSS_Console("startup(): Should not see this message! ");
    USLOSS_Console("Returned from fork1 call that created start1\n");

    return;
} /* startup */

/* ------------------------------------------------------------------------
 Name - initializeProcessTable
 Purpose - Sets all procStrucs in ProcessTable to have no status (unassigned)
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void initializeProcessTable() {
    for (int i = 0; i < MAXPROC; i++) {
        procTable[i].status = NO_PROCESS_ASSIGNED;
    }
} /* initializeProcessTable */

/* ------------------------------------------------------------------------
 Name - initializeBlockList
 Purpose - Creates the block list as an empty linked list.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void initializeBlockList() {
    blockListHead = NULL;
    blockListTail = blockListHead;
} /* initializeBlockList */

/* ------------------------------------------------------------------------
 Name - initializeReadyList
 Purpose - Builds the ready list. readyList[i] is the "head" reference of 
    the linked-list that serves as priority-i's queue. (Really priority-(i-1)'s).
    These references are initially set to NULL.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void initializeReadyList() {
    
    for (int i = 0; i < 6; i++) {
        readyList[i] = NULL;
    }
} /* initializeReadyList */

/* ------------------------------------------------------------------------
 Name - dumpProcessTable
 Purpose - Outputs the contents of all entries in processTable
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void dumpProcessTable() {
    printf("%5s %20s %20s %20s %20s\n", "Name", "PID", "Status", "Priority", "State");
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAXPROC; i++) {
        printf("%5s%20hi%20d%20d%20s\n", procTable->name, procTable->pid, procTable->status, procTable->priority, procTable->state);
    }
} /* dumpProcessTable */

/* ------------------------------------------------------------------------
 Name - pushToReadyList
 Purpose - Adds a process to the readyList
 Parameters - Pointer to process to be added
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void pushToReadyList(struct procStruct * newProcess) {
    struct listNode * newNode = malloc(sizeof(listNode));          // Create a new node to insert into the readyList.
    newNode->process = newProcess;
    addToListTail(readyList[newProcess->priority -1], priorityTailPtrs[newProcess->priority -1], newNode);
    return;
    
} /* pushToReadyList */

/* ------------------------------------------------------------------------
 Name - popFromReadyList
 Purpose - Returns a pointer to the next process at the highest priority
 Parameters - nothing
 Returns - a pointer to the next process to be run, 
    NULL if the ready list is empty.
 Side Effects - none
 ----------------------------------------------------------------------- */
struct procStruct * popFromReadyList() {
    for (int i = 0; i < 6; i++) {
        if (readyList[i] != NULL) {
            return removeFromListHead(readyList[i])->process;
        }
    }
    return NULL;
} /* popFromReadyList */

/* ------------------------------------------------------------------------
   Name - finish
   Purpose - Required by USLOSS
   Parameters - none
   Returns - nothing
   Side Effects - none
   ----------------------------------------------------------------------- */
void finish(int argc, char *argv[]) {
    if (DEBUG && debugflag)
        USLOSS_Console("in finish...\n");
} /* finish */

/* ------------------------------------------------------------------------
   Name - fork1
   Purpose - Gets a new process from the process table and initializes
             information of the process.  Updates information in the
             parent process to reflect this child process creation.
   Parameters - the process procedure address, the size of the stack and
                the priority to be assigned to the child process.
   Returns - the process id of the created child or -1 if no child could
             be created or if priority is not between max and min priority.
   Side Effects - ReadyList is changed, ProcTable is changed, Current
                  process information changed
   ------------------------------------------------------------------------ */
int fork1(char *name, int (*startFunc)(char *), char *arg,
          int stacksize, int priority) {
    
    int procSlot = -1;          // The location in process table to store PCB
    
    // test if in kernel mode, halt if in user mode
    if (!isKernel()) {
        USLOSS_Console("ERROR: fork1(): Process %s - fork1() called in User Mode. Halting.", name);
        USLOSS_Halt(1);
    }
    
    //disable interrupts
    if (DEBUG && debugflag) {
        USLOSS_Console("fork1(): Process %s - disabling interrupts. FIXME!!!\n", name);
    }
    disableInterrupts();    // FIXME!!! disableInterrupts() not finished
    
    
    if (DEBUG && debugflag) {
        USLOSS_Console("fork1(): Process %s - creating process.\n", name);
    }

    // Return -2 if stack size is too small
    if (stacksize < USLOSS_MIN_STACK) {
        if (DEBUG && debugflag) {
            USLOSS_Console("ERROR: fork1(): Process %s - Process stack size is too small.\n", name);
        }
        return -2;
    }

    // Is there room in the process table? What is the next PID?
    // Get next open slot in process table.
    procSlot = getNextProcSlot();
    
    // Check if ProcTable is full, if priority is out of bounds, if startFunc is NULL, if name is NULL
    if ((procSlot == -1) ||
        (nextPid != SENTINELPID && priority == SENTINELPRIORITY) ||
        (nextPid != SENTINELPID && (priority > MINPRIORITY || priority < MAXPRIORITY)) ||
        (startFunc == NULL) ||
        (name == NULL)) {
        
        if (DEBUG && debugflag) {
            if (name == NULL) {
                USLOSS_Console("ERROR: fork1(): Process PID %d - Process name cannot be NULL.\n", Current->pid);
            }
            if (procSlot == -1) {
                USLOSS_Console("ERROR: fork1(): Process %s - Process Table is full.\n", name);
            }
            if (nextPid != SENTINELPID && priority == SENTINELPRIORITY) {
                USLOSS_Console("ERROR: fork1(): Process %s - Only sentinel may have priority %d.\n", name, priority);
            }
            if (nextPid != SENTINELPID && (priority > MINPRIORITY || priority < MAXPRIORITY)) {
                USLOSS_Console("ERROR: fork1(): Process %s - Process priority %d is out of bounds.\n", name, priority);
            }
            if (startFunc == NULL) {
                USLOSS_Console("ERROR: fork1(): Process %s - Process startFunc cannot be NULL.\n", name);
            }
        }
        return -1;
    }
    
    // Assign process the next pid
    procTable[procSlot].pid = nextPid;
    nextPid++;              // Increment nextPID for the next time fork1 is called

    // fill-in entry in process table */
    // if name is too long...
    if ( strlen(name) >= (MAXNAME - 1) ) {
        USLOSS_Console("ERROR: fork1(): Process %s - %s is too long a name.  Halting.\n", name, name);
        USLOSS_Halt(1);
    }
    
    // Initialize process name and startFunc
    strcpy(procTable[procSlot].name, name);
    procTable[procSlot].startFunc = startFunc;
    
    // Initialize process stackSize and stack
    procTable[procSlot].stackSize = stacksize;
    
    // if malloc fails
    if ((procTable[procSlot].stack = malloc(stacksize)) == NULL) {
        USLOSS_Console("ERROR: fork1(): Process %s - stack malloc failed", name);
        USLOSS_Halt(1);
    }
    
    // Assign process priority
    procTable[procSlot].priority = priority;
    
    
    // Initialize and error check for process arg
    if ( arg == NULL ) {
        procTable[procSlot].startArg[0] = '\0';
    }
    else if ( strlen(arg) >= (MAXARG - 1) ) {
        USLOSS_Console("ERROR: fork1(): Argument too long. Halting.\n");
        USLOSS_Halt(1);
    }
    else{
        strcpy(procTable[procSlot].startArg, arg);
    }
    
    if (Current != NULL) {              // "This" process is a child
        if (Current->childProcPtr == NULL) {                // This is current's first child
            Current->childProcPtr = &procTable[procSlot];
        }
        else {                                              // Current already has >=1 child
            procTable[procSlot].nextSiblingPtr = Current->childProcPtr;
            Current->childProcPtr = &procTable[procSlot];               // FIXME: New child might need to be at end of list, probably not.
        }
    }
    
    // Assign pointer to parent if the is one. If current == null, parent == null.
    procTable[procSlot].parentProcPtr = Current;
    

    // Initialize context for this process, but use launch function pointer for
    // the initial value of the process's program counter (PC)
    USLOSS_ContextInit(&(procTable[procSlot].state),
                       procTable[procSlot].stack,
                       procTable[procSlot].stackSize,
                       NULL,
                       launch);

    // for future phase(s)
    p1_fork(procTable[procSlot].pid);

    // Change process status to ready and add to process list
    procTable[procSlot].status = READY;
    pushToReadyList(&procTable[procSlot]);

    // Call dispatcher
    if (procTable[procSlot].pid != SENTINELPID) {
        dispatcher();
    }
    
    return procTable[procSlot].pid;  // -1 is not correct! Here to prevent warning.
} /* fork1 */

/* ------------------------------------------------------------------------
 Name - getNextProcSlot
 Purpose - Finds the next available index for insertion in the process table
 Parameters - none
 Returns - The index next empty slot in the process table,
    or -1 if there is no available slot (table is full)
 Side Effects - enable interrupts
 ------------------------------------------------------------------------ */
int getNextProcSlot() {
    
    int currSlot = nextPid % MAXPROC;       // Get the hashed index of the "ideal" slot.
    int numQueries = 0;
    
    // Traverse procTable until an empty slot is found
    while (procTable[currSlot].status != NO_PROCESS_ASSIGNED) {
        
        // If we have inspected each potential slot in the procTable, return -1
        if (numQueries >= MAXPROC) {
            return -1;
        }
        
        nextPid++;
        currSlot = nextPid % MAXPROC;
        numQueries++;
    }
    return currSlot;
} /* getNextProcSlot */

/* ------------------------------------------------------------------------
 Name - isKernel
 Purpose - Checks the current OS mode.
 Parameters - none
 Returns - Returns 0 if in kernel mode, 
    !0 if in user mode.
 Side Effects - enable interrupts
 ------------------------------------------------------------------------ */
int isKernel() {
    return (USLOSS_PSR_CURRENT_MODE & USLOSS_PsrGet());
} /* isKernel */

/* ------------------------------------------------------------------------
   Name - launch
   Purpose - Dummy function to enable interrupts and launch a given process
             upon startup.
   Parameters - none
   Returns - nothing
   Side Effects - enable interrupts
   ------------------------------------------------------------------------ */
void launch() {
    int result;

    if (DEBUG && debugflag)
        USLOSS_Console("launch(): started\n");

    // Enable interrupts

    // Call the function passed to fork1, and capture its return value
    result = Current->startFunc(Current->startArg);

    if (DEBUG && debugflag)
        USLOSS_Console("Process %d returned to launch\n", Current->pid);

    quit(result);

} /* launch */

/* ------------------------------------------------------------------------
   Name - join
   Purpose - Wait for a child process (if one has been forked) to quit.  If 
             one has already quit, don't wait.
   Parameters - a pointer to an int where the termination code of the 
                quitting process is to be stored.
   Returns - the process id of the quitting child joined on.
             -1 if the process was zapped in the join
             -2 if the process has no children
   Side Effects - If no child process has quit before join is called, the 
                  parent is removed from the ready list and blocked.
   ------------------------------------------------------------------------ */
int join(int *status) {
    return -1;  // -1 is not correct! Here to prevent warning.
} /* join */

/* ------------------------------------------------------------------------
   Name - quit
   Purpose - Stops the child process and notifies the parent of the death by
             putting child quit info on the parents child completion code
             list.
   Parameters - the code to return to the grieving parent
   Returns - nothing
   Side Effects - changes the parent of pid child completion status list.
   ------------------------------------------------------------------------ */
void quit(int status) {
    p1_quit(Current->pid);
} /* quit */

/* ------------------------------------------------------------------------
   Name - dispatcher
   Purpose - dispatches ready processes.  The process with the highest
             priority (the first on the ready list) is scheduled to
             run.  The old process is swapped out and the new process
             swapped in.
   Parameters - none
   Returns - nothing
   Side Effects - the context of the machine is changed
   ----------------------------------------------------------------------- */
void dispatcher(void) {
    procPtr nextProcess = NULL;

    p1_switch(Current->pid, nextProcess->pid);
    
    
    ///////////////////////////
    
    
    
    ///////////////////////////
} /* dispatcher */

/* ------------------------------------------------------------------------
   Name - sentinel
   Purpose - The purpose of the sentinel routine is two-fold.  One
             responsibility is to keep the system going when all other
             processes are blocked.  The other is to detect and report
             simple deadlock states.
   Parameters - none
   Returns - nothing
   Side Effects -  if system is in deadlock, print appropriate error
                   and halt.
   ----------------------------------------------------------------------- */
int sentinel (char *dummy) {
    if (DEBUG && debugflag)
        USLOSS_Console("sentinel(): called\n");
    while (1)
    {
        checkDeadlock();
        USLOSS_WaitInt();
    }
} /* sentinel */

/* check to determine if deadlock has occurred... */
static void checkDeadlock() {

} /* checkDeadlock */

/*
Initializes the interrupts.
*/
void initializeInterrupts() {

} /* initializeInterrupts */

/*
Enable the interrupts.
*/
void enableInterrupts() {
    
} /* enableInterrupts */

/*
Disables the interrupts.
*/
void disableInterrupts() {
    // turn the interrupts OFF iff we are in kernel mode
    // if not in kernel mode, print an error message and
    // halt USLOSS
    if (!isKernel()) {
        USLOSS_Console("ERROR: disableInterrupts(): Called while not in kernel mode.");
        USLOSS_Halt(1);
    }
    else {
        USLOSS_Console("ERROR: disableInterrupts(): FIXME!!! FINISH FUNCTION!!!");
        // This is going to be a bitwise operation with psr and 0x2?
    }
    return;
} /* disableInterrupts */
