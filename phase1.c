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
int isZapped(void);
int getNextProcSlot();
void enableInterrupts();
void initializeInterrupts();
void disableInterrupts();
void clock_handler();
void printBinaryHelper(unsigned n);
void printBinary(unsigned n);
void removeFromReadyList(struct procStruct *);


/* -------------------------- Structs ------------------------------------- */




/* -------------------------- Globals ------------------------------------- */
// Indexes for block list
struct listNode * blockListHead;
struct listNode * blockListTail;

// Indexes in ReadyList
// struct listNode * readyList[6] ;
// struct listNode * priorityTailPtrs[6];

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
    ReadyList = NULL;
    initializeBlockList();

    // Initialize the clock interrupt handler
    //USLOSS_IntVec[USLOSS_CLOCK_INT] = clock_handler;

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
        printf("%5s%20d\n", procTable[i].name, procTable[i].priority);
        //printf("%5s%20hi%20d%20d%20s\n", procTable->name, procTable->pid, procTable->status, procTable->priority, procTable->state);
    }
} /* dumpProcessTable */


/* ------------------------------------------------------------------------
 Name - dumpReadyList
 Purpose - Outputs the contents of all entries in processTable
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void dumpReadyList() {
    printf("%5s\n", "NAME");
    printf("-------\n");
    
    procStruct * curr = ReadyList;
    while (curr != NULL) {
        printf("%s\n", curr->name);
        curr = curr->nextProcPtr;
    }
} /* dumpreadyList */

/* ------------------------------------------------------------------------
 Name - pushToReadyList
 Purpose - Adds a process to the readyList
 Parameters - Pointer to process to be added
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void pushToReadyList(struct procStruct * newProcess) {
    
    // Ready List is empty
    if (ReadyList == NULL) {
        ReadyList = newProcess;
    }
    
    // New process is the highest (lowest number) priority
    else if (newProcess->priority < ReadyList->priority) {
        newProcess->nextProcPtr = ReadyList;
        ReadyList = newProcess;
    }
    
    // New process must be inserted at the end of a given priority
    else {
        procStruct * curr = ReadyList;
        procStruct * prev = NULL;
        
        while (curr->priority <= newProcess->priority) {
            prev = curr;
            curr = curr->nextProcPtr;
        }
        prev->nextProcPtr = newProcess;
        newProcess->nextProcPtr = curr;
    }
    return;
    
} /* pushToReadyList */

/* ------------------------------------------------------------------------
 Name - removeFromReadyList
 Purpose - Finds and removes a process pointer from the ready list.
    Does not change the status of the process.
 Parameters - Pointer to process to be added
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void removeFromReadyList(struct procStruct * procToRemove) {
    struct procStruct * curr = ReadyList;
    struct procStruct * prev = NULL;
    
    while (curr != procToRemove && curr != NULL) {
        prev = curr;
        curr = curr->nextProcPtr;
    }
    
    if (curr == procToRemove) {
        prev->nextProcPtr  = curr->nextProcPtr;
        return;
    }
    
    else {
        USLOSS_Console("ERROR: removeFromReadyList(): Failed to remove %s from the ready list.\n", procToRemove->name);
    }
} /* removeFromReadyList */

/* ------------------------------------------------------------------------
 Name - popFromReadyList
 Purpose - Returns a pointer to the next process at the highest priority
 Parameters - nothing
 Returns - a pointer to the next process to be run, 
    NULL if the ready list is empty.
 Side Effects - none
 ----------------------------------------------------------------------- */
struct procStruct * popFromReadyList() {
    procStruct * returnProc = ReadyList;
    ReadyList = ReadyList->nextProcPtr;
    return returnProc;
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
int fork1(char *name, int (*startFunc)(char *), char *arg, int stacksize, int priority) {
    
    int procSlot = -1;          // The location in process table to store PCB
    
    // test if in kernel mode, halt if in user mode
    if (!isKernel()) {
        USLOSS_Console("ERROR: fork1(): Process %s - fork1() called in User Mode. Halting.", name);
        USLOSS_Halt(1);
    }
    
    //disable interrupts
    if (DEBUG && debugflag) {
        USLOSS_Console("fork1(): Process %s - disabling interrupts.\n", name);
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
        USLOSS_Console("ERROR: fork1(): Process %s - stack malloc failed\n", name);
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
 Side Effects - none
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
    enableInterrupts();

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
    
    // Processor must be in kernel mode.
    if (!isKernel()){
        USLOSS_Console("ERROR: join(): Process %s - Join called in user mode. Halting.\n", Current->name);
        USLOSS_Halt(1);
    }
    
    /* Disable Interrupts */
    if (DEBUG && debugflag) {
        USLOSS_Console("join(): Process %s - disabling interrupts.\n", Current->name);
    }
    disableInterrupts();
    
    // if Current has no un-quit children, return -2
    if (Current->childProcPtr == NULL) {
        return -2;
    }
    
    // if no child of current has quit yet
    if (Current->quitChildPtr == NULL) {
        Current->status = BLOCKED_ON_JOIN;
        removeFromReadyList(Current);
        dispatcher();
    }
    
    int quitChildPID = -404;
    
    //Remove quitChild from child quit list and grab it's PID
    procPtr childToQuit = Current->quitChildPtr;
    quitChildPID = childToQuit->pid;
    *status = childToQuit->quitStatus;
    Current->quitChildPtr = childToQuit->quitSiblingPtr;
    
    // --- if Current was blocked, but child has reactivated it
        // --- Get quit child
        // --- *status = child.quitStatus
        // --- Remove child from list of list of quitChildren
    
    
    // if Current was zapped in join -1
    if (isZapped()) {
        return -1;
    }
    
    
    return quitChildPID;  // Return PID of first quit child
} /* join */

/* ------------------------------------------------------------------------
   Name - quit
   Purpose - Stops the child process and notifies the parent of the death by
             putting child quit info on the parents child completion code
             list.
   Parameters - the code to return to the grieving parent <- LOL
   Returns - nothing
   Side Effects - changes the parent of pid child completion status list.
   ------------------------------------------------------------------------ */
void quit(int status) {
    
    // --- make sure we are in kernel mode
    // test if in kernel mode, halt if in user mode
    if (!isKernel()) {
        USLOSS_Console("ERROR: quit(): Process %s - quit() called in User Mode. Halting.", Current->name);
        USLOSS_Halt(1);
    }
    // --- disable interrupts.
    /* Disable Interrupts */
    if (DEBUG && debugflag) {
        USLOSS_Console("quit(): Process %s - disabling interrupts.\n", Current->name);
    }
    disableInterrupts();
    
    // --- Ensure that the process does not have any running children
        // --- If this happens, print an error message and USLOSS_Halt(1)
    if (Current->childProcPtr != NULL) {
        USLOSS_Console("ERROR: quit(): Process %s - Process called quit with active children.", Current->name);
        USLOSS_Halt(1);
    }
    
    // --- Change Status to QUIT and remove from ready list
    Current->quitStatus = status;
    Current->status = QUIT;
    removeFromReadyList(Current);
    
    // --- If a process zapped this process (multiple?) if (isZapped())
        // --- Unblock that process, change status the READY and add to readyList.
    
    // --- If Quitting process is and child and has quitChildren
        // Do some stuff here.
    // --- Else If Quitting Process is a child and not a parent
        // --- Do some stuff here.
    // --- Else, current is a parent
        // --- Do some stuff here.
    
    // --- CALL DISPATCHER
    dispatcher();
    //p1_quit(Current->pid);
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
    if (DEBUG && debugflag) {
        USLOSS_Console("dispatcher(): Started\n");
    }
    
    // First time dispatcher is called is for start1()
    if (Current == NULL) {
        Current = popFromReadyList();
        if (DEBUG && debugflag) {
            USLOSS_Console("dispatcher(): dispatcher assigned Current -> Process %s\n", Current->name);
        }
        //Current->procStartTime = USLOSS_DeviceInput(USLOSS_CLOCK_INT, 0, 0);    // FIXME
        USLOSS_Console("dispatcher(): Current = NULL\n");
        enableInterrupts();
        USLOSS_ContextSwitch(NULL, &Current->state);
    }
    
    else {
    // --- Otherwise, The context switch will need old = Current, Current = next (pop).
        struct procStruct * old = Current;
        // --- Change old's status to ready
        old->status = READY;
        // --- Get next process from ReadyList, change status to running.
        Current = popFromReadyList();
        // --- Get start time for new Current           // FIXME!!! Still don't understand how to retrieve time
        p1_switch(old->pid, Current->pid);
        // --- enableInterrupts() before returning to user code
        enableInterrupts();
        // --- ContextSwitch
        USLOSS_ContextSwitch(&old->state, &Current->state);
    
    
    }
    // Do not call context switch if current is the process that would be run. (Only one priority 1 process and it is already Current)
    
    
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
    
    if (USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT) == USLOSS_ERR_INVALID_PSR){
        USLOSS_Console("ERROR: enableInterrupts(): Process %s - Failed to enable interrupts.\n", Current->name);
    }
    
} /* enableInterrupts */

/*
Disables the interrupts.
*/
void disableInterrupts() {
    // turn the interrupts OFF iff we are in kernel mode
    // if not in kernel mode, print an error message and
    // halt USLOSS
    if (!isKernel()) {
        USLOSS_Console("ERROR: disableInterrupts(): Called while not in kernel mode.\n");
        USLOSS_Halt(1);
    }
    else {
        if (USLOSS_PsrSet(USLOSS_PsrGet() ^ USLOSS_PSR_CURRENT_INT) == USLOSS_ERR_INVALID_PSR){
            USLOSS_Console("ERROR: disableInterrupts(): Failed to disable interrupts.\n");
        }
    }
    return;
} /* disableInterrupts */

/*
 Prints the binary representation of unsigned value n.
*/
void printBinaryHelper(unsigned n) {
    /* step 1 */
    if (n > 1)
        printBinaryHelper(n/2);
    
    /* step 2 */
    printf("%d", n % 2);
}
void printBinary(unsigned n) {
    printf("Binary representation of %u is: ", n);
    printBinaryHelper(n);
    printf("\n");
} /* printBinary */

/*
 * This function simply checks if the Current process has been zapped
 */
int isZapped(void) {
	return Current->zapped;
}
