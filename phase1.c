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
void nullifyProcess(int);
void addToQuitChildList(struct procStruct *);
void removeFromChildList(struct procStruct *);
void clockHandler(int dev, void *arg);
void illegalArgumentHandler(int dev, void *arg);
int getCurrentTime();

/* -------------------------- Structs ------------------------------------- */




/* -------------------------- Globals ------------------------------------- */
// Indexes for block list
struct listNode * blockListHead;
struct listNode * blockListTail;

// Patrick's debugging global variable...
int debugflag = 0;

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
        procTable[i].nextProcPtr = NULL;
        procTable[i].childProcPtr = NULL;
        procTable[i].nextSiblingPtr = NULL;
        procTable[i].parentProcPtr = NULL;
        procTable[i].quitChildPtr = NULL;
        procTable[i].quitSiblingPtr = NULL;
        procTable[i].whoZappedMePtr = NULL;
        procTable[i].nextWhoZappedMePrt = NULL;
        procTable[i].name[0] = '\0';     /* process's name */
        procTable[i].startArg[0] = '\0';  /* args passed to process */
        procTable[i].pid = -1;               /* process id */
        procTable[i].priority = -1;
        procTable[i].startFunc = NULL;   /* function where process begins -- launch */
        procTable[i].stack = NULL;
        procTable[i].stackSize = -1;
        procTable[i].procStartTime = -1;
        procTable[i].zapped = 0;
        procTable[i].quitStatus = -404;
    }
} /* initializeProcessTable */

/* ------------------------------------------------------------------------
 Name - nullifyProcess
 Purpose - Empties the procTable of the process with pid == pidToNullify
 Parameters - int pidToNullify - The PID of the process to zero out.
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void nullifyProcess(int pidToNullify) {
    
    int i = pidToNullify % MAXPROC;
    
    procTable[i].status = NO_PROCESS_ASSIGNED;
    procTable[i].nextProcPtr = NULL;
    procTable[i].childProcPtr = NULL;
    procTable[i].nextSiblingPtr = NULL;
    procTable[i].parentProcPtr = NULL;
    procTable[i].quitChildPtr = NULL;
    procTable[i].quitSiblingPtr = NULL;
    procTable[i].whoZappedMePtr = NULL;
    procTable[i].nextWhoZappedMePrt = NULL;
    procTable[i].name[0] = '\0';     /* process's name */
    procTable[i].startArg[0] = '\0';  /* args passed to process */
    procTable[i].pid = -1;               /* process id */
    procTable[i].priority = -1;
    procTable[i].startFunc = NULL;   /* function where process begins -- launch */
    procTable[i].stack = NULL;
    procTable[i].stackSize = -1;
    procTable[i].procStartTime = -1;
    procTable[i].zapped = 0;
    procTable[i].quitStatus = -404;
} /*nullifyProcess */

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
 Name - dumpProcesses
 Purpose - Outputs the contents of all entries in processTable
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void dumpProcesses() {
    printf("%10s%20s%20s%20s\n", "Name", "PID", "Status", "Priority");
    printf("------------------------------------------------------------------------\n");
    for (int i = 0; i < MAXPROC; i++) {
        printf("%10s%20hi%20d%20d\n", procTable[i].name, procTable[i].pid, procTable[i].status, procTable[i].priority);
    }
} /* dumpProcesses */

/* ------------------------------------------------------------------------
 Name - dumpReadyList
 Purpose - Outputs the contents of all entries in processTable
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void dumpReadyList() {
    printf("%10s %15s\n", "NAME", "PRIORITY");
    printf("----------------------------\n");
    
    procStruct * curr = ReadyList;
    while (curr != NULL) {
        printf("%10s %15d\n", curr->name, curr->priority);
        curr = curr->nextProcPtr;
    }
    printf("----------------------------\n");
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
    disableInterrupts();
    
    
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
    
    // Call dispatcher if not fork on sentinel
    if (procTable[procSlot].pid != SENTINELPID) {
        dispatcher();
    }
//
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
    int arg;

    if (DEBUG && debugflag)
        USLOSS_Console("launch(): started process %s\n", Current->name);
    

    // Enable interrupts
    enableInterrupts();

    // Call the function passed to fork1, and capture its return value
    arg = Current->startFunc(Current->startArg);

    if (DEBUG && debugflag)
        USLOSS_Console("Process %d returned to launch\n", Current->pid);

    quit(arg);

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
    
    int quitChildPID = -3;
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
    
    
    if (DEBUG && debugflag) {
        USLOSS_Console("Dumping processes from join()...\n");
        dumpProcesses();
    }
    // if Current has no children, return -2
    if (Current->childProcPtr == NULL && Current->quitChildPtr == NULL) {
        return -2;
    }
    
    // if no child of current has quit yet
    if (Current->quitChildPtr == NULL) {
        Current->status = BLOCKED_ON_JOIN;
        dispatcher();
    }
    
    // A child has quit and reactivated its parent
    procPtr childThatQuit = Current->quitChildPtr;
    
    if (DEBUG && debugflag) {
        USLOSS_Console("join(): Child %s has status of quit.\n", childThatQuit->name);
        dumpReadyList();
    }
    
    //Remove quitChild from child quit list and grab it's PID
    procPtr childToQuit = Current->quitChildPtr;
    quitChildPID = childToQuit->pid;
    *status = childToQuit->quitStatus;
    Current->quitChildPtr = childToQuit->quitSiblingPtr;
    
    quitChildPID = childThatQuit->pid;
    *status = childThatQuit->quitStatus;
    // Remove child from parent's quitlist
    childThatQuit->parentProcPtr->quitChildPtr = childThatQuit->quitSiblingPtr;
    nullifyProcess(quitChildPID);
    
    
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
    
    // test if in kernel mode, halt if in user mode
    if (!isKernel()) {
        USLOSS_Console("ERROR: quit(): Process %s - quit() called in User Mode. Halting.", Current->name);
        USLOSS_Halt(1);
    }
    
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
    
    // --- Change Status to QUIT
    Current->quitStatus = status;
    Current->status = QUIT;
    
    // --- If a process zapped this process (multiple?) if (isZapped())
    // --- Unblock that process, change status the READY and add to readyList.
    if (isZapped()) {
        procPtr zapper = Current->whoZappedMePtr;
        while (zapper != NULL) {
            zapper->status = READY;
            pushToReadyList(zapper);
            zapper = zapper->nextWhoZappedMePrt;
        }
    }
    
    int currPID = Current->pid;
    
    // --- If Quitting process is a child and has its own quitChildren
    if (Current->parentProcPtr != NULL && Current->quitChildPtr != NULL) {
        // --- Remove all children on quit list.
        while (Current->quitChildPtr != NULL) {
            int quitChildPID = Current->quitChildPtr->pid;
            Current->quitChildPtr = Current->quitChildPtr->quitSiblingPtr;
            nullifyProcess(quitChildPID);
        }
        // --- Remove self and reactivate parent
        Current->parentProcPtr->status = READY;
        pushToReadyList(Current->parentProcPtr);
        
        addToQuitChildList(Current->parentProcPtr);
        removeFromChildList(Current->parentProcPtr);
        //removeFromChildList(Current);
    }
    
    // --- Else If Quitting Process is a child and not a parent
    else if (Current->parentProcPtr != NULL) {
        // Add quit child to list of quit children
        addToQuitChildList(Current->parentProcPtr);
        removeFromChildList(Current->parentProcPtr);
        //removeFromChildList(Current);
        
        // Once the child has quit, add parent back to the readyList.
        if (Current->parentProcPtr->status != READY) {
            Current->parentProcPtr->status = READY;
            pushToReadyList(Current->parentProcPtr);
        }
    }
    
    // --- Else, current is a parent only
    else {
        while (Current->quitChildPtr != NULL) {
            int quitChildPID = Current->quitChildPtr->pid;
            Current->quitChildPtr = Current->quitChildPtr->quitSiblingPtr;
            nullifyProcess(quitChildPID);
        }
        nullifyProcess(Current->pid);
    }
    
    p1_quit(currPID);
    // --- CALL DISPATCHER
    dispatcher();
} /* quit */

/* ------------------------------------------------------------------------
 Name - addToQuitChildList
 Purpose - Adds a child that has quit to a parent's list of quit-child-
    processes (adds at the end of the list).
 Parameters - procPtr parent: The parent process whose child has quit.
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
/* addToQuitChildList - Adds, in order, the procStruct to the list of children its parent has that have quit.*/
void addToQuitChildList(procPtr parent) {
    if (parent->quitChildPtr == NULL) {
        parent->quitChildPtr = Current;
        return;
    }
    
    procPtr child = parent->quitChildPtr;
    while (child->quitSiblingPtr != NULL) {
        child = child->quitSiblingPtr;
    }
    
    child->quitSiblingPtr = Current;
}/* addToQuitChildList */

/* ------------------------------------------------------------------------
 Name - removeFromChildList
 Purpose - Removes a child that has quit from a parent's list of active-child-
    processes.
 Parameters - procPtr parent: The parent process whose child has quit.
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void removeFromChildList(struct procStruct * parent) {
    
    if (Current == parent->childProcPtr) {
        parent->childProcPtr = parent->childProcPtr->nextSiblingPtr;
    }
    
    else {
        procPtr ptr = parent->childProcPtr;
        while (ptr->nextSiblingPtr != Current) {
            ptr = ptr->nextSiblingPtr;
        }
        ptr->nextSiblingPtr = ptr->nextSiblingPtr->nextSiblingPtr;
    }
    
    if (DEBUG && debugflag) {
        USLOSS_Console("removeFromChildList(): Process %d removed.\n", Current->pid);
    }
    
} /* removeFromChildList */
//void removeFromChildList(struct procStruct * childToRemove) {
//    
//    procPtr ptr = childToRemove;
//    // If childToRemove is the head of the linked list of children
//    if (childToRemove == childToRemove->parentProcPtr->childProcPtr) {
//        childToRemove->parentProcPtr->childProcPtr = childToRemove->nextSiblingPtr;
//    }
//    
//    else {
//        ptr = childToRemove->parentProcPtr->childProcPtr;
//        while (ptr->nextSiblingPtr != childToRemove) {
//            ptr = ptr->nextSiblingPtr;
//        }
//        ptr->nextSiblingPtr = ptr->nextSiblingPtr->nextSiblingPtr;
//    }
//    
//    if (DEBUG && debugflag) {
//        USLOSS_Console("removeFromChildList(): Process %d removed.\n", childToRemove->pid);
//    }
//}

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
    
    disableInterrupts();
    
    
    // First time dispatcher is called is for start1()
    if (Current == NULL) {
        Current = popFromReadyList();
        Current->status = ACTIVE;
        if (DEBUG && debugflag) {
            USLOSS_Console("dispatcher(): dispatcher assigned Current -> Process %s\n", Current->name);
        }
        Current->procStartTime = getCurrentTime();
        enableInterrupts();
        USLOSS_ContextSwitch(NULL, &Current->state);
    }
    
    else {
    // --- Otherwise, The context switch will need old = Current, Current = next (pop).
        struct procStruct * old = Current;
        // --- Change old's status to ready
        if (old->status == ACTIVE) {
            old->status = READY;
            pushToReadyList(old);
        }
        // --- Get next process from ReadyList, change status to running.
        Current = popFromReadyList();                   // If delete testing area, uncomment me
        Current->status = ACTIVE;
        p1_switch(old->pid, Current->pid);
        // --- enableInterrupts() before returning to user code
        Current->procStartTime = getCurrentTime();
        enableInterrupts();
        // --- ContextSwitch
        USLOSS_ContextSwitch(&old->state, &Current->state);
    }
    
    // Do not call context switch if current is the process that would be run. (Only one priority 1 process and it is already Current) FIXME?
    
    
} /* dispatcher */

/* ------------------------------------------------------------------------
 Name - zap
 Purpose - Marks a process with pid as zapped. If zap is called on nonexistent
    process, Halt(1). If a process calls zap on itself, Halt(1). Zap does not
    return until Process_PID has quit.
 Parameters - int pid -> The process to be zapped
 Returns - 0: The zapped process has quit.
 |        -1: The calling process was zapped while in zap.
 Side Effects - The Process calling zap is added to the list of processes
    which called zap on Process_PID. 
              - Process_PID's zappedMarker is set to true;
 ----------------------------------------------------------------------- */
int zap(int pid) {
    
    // Ensure we are in kernel mode before disabling interrupts
    if (!isKernel()) {
        USLOSS_Console("ERROR: zap(): Process %s - zap called while in user mode. Halting.", Current->name);
        USLOSS_Halt(1);
    }
    disableInterrupts();
    
    // If Current tries to zap an nonexistent process
    if (procTable[pid % MAXPROC].status == NO_PROCESS_ASSIGNED) {
        USLOSS_Console("ERROR: zap(): Process %s - process tried to zap a nonexistent process. Halting.", Current->name);
        USLOSS_Halt(1);
    }
    
    // If Current tries to zap itself
    if (Current->pid == pid) {
        USLOSS_Console("ERROR: zap(): Process %s - process tried to call zap on itself. Halting.", Current->name);
        USLOSS_Halt(1);
    }
    
    return -1;
}

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
    if (DEBUG && debugflag) {
        USLOSS_Console("sentinel(): called\n");
    }
    while (1)
    {
        checkDeadlock();
        USLOSS_WaitInt();
    }
} /* sentinel */

/* ------------------------------------------------------------------------
 Name - checkDeadlock
 Purpose - Checks the Process Table at the end of a program run to ensure 
           that the sentinel is the only process still in the list.
         - Throws an error, halts if there are more than one processes
    remaining.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
static void checkDeadlock() {
    int numProcs = 0;
    
    // Count the number of processes remaining in the process table.
    for (int i = 0; i < MAXPROC; i++) {
        if (procTable[i].status != NO_PROCESS_ASSIGNED) {
            numProcs++;
        }
    }
    
    // Should be only the sentinel. If not, there is an error
    if (numProcs > 1) {
        USLOSS_Console("ERROR: checkDeadlock(): %d processes in the process table remaining. Halting.\n", numProcs);
        USLOSS_Halt(1);
    }
    
    USLOSS_Console("All processes complete.\n");
    USLOSS_Halt(0);
    
} /* checkDeadlock */

/* ------------------------------------------------------------------------
 Name - initializeinterrupts
 Purpose - Initializes the interrupts required (clock interrupts).
         - Initializes a non-null value for Illegal_Int in IntVec.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void initializeInterrupts() {
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clockHandler;
    USLOSS_IntVec[USLOSS_ILLEGAL_INT] = illegalArgumentHandler;
} /* initializeInterrupts */

/* ------------------------------------------------------------------------
 Name - enableInterrupts
 Purpose - Enables interrupts.
         - Throws an error if USLOSS is passed an invalid PSR
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void enableInterrupts() {
    
    if (USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT) == USLOSS_ERR_INVALID_PSR){
        USLOSS_Console("ERROR: enableInterrupts(): Process %s - Failed to enable interrupts.\n", Current->name);
    }
    
} /* enableInterrupts */

/* ------------------------------------------------------------------------
 Name - disableInterrupts
 Purpose - Disable interrupts
         - Thows an error if USLOSS is passed an invalid PSR
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
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

/* ------------------------------------------------------------------------
 Name - printBinary
 Purpose - Computes and prints a binary representation of an unsigned value.
         - Includes a healper function for recursion purposes.
         - FOR TESTING ONLY! This function is never called in an actual run.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
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


/* ------------------------------------------------------------------------
 Name - getPID
 Purpose - Returns the PID of the currently running process.
         - Required by some testcases.
 Parameters - none
 Returns - int - Current->pid
 Side Effects - none
 ----------------------------------------------------------------------- */
int getpid() {
    return Current->pid;
} /* getpid */

/* ------------------------------------------------------------------------
 Name - isZapped
 Purpose - Returns true if the currently running process was zapped.
 Parameters - none
 Returns - int - Current->zapped
 Side Effects - none
 ----------------------------------------------------------------------- */
int isZapped(void) {
	return Current->zapped;
} /* isZapped */

/* ------------------------------------------------------------------------
 Name - clockHandler
 Purpose - Handles the main functionality of the clock handler. After each 
           time slice (of 20ms), clockHandler checks if the currently running
           process has exceeded its allotted 80ms. If yes, call dispatcher. If no,
           do nothing.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void clockHandler(int dev, void *arg) {
    
    int procTimeUsed = getCurrentTime() - Current->procStartTime;
    
    if (procTimeUsed >= MAXTIMESLOT){
        // FIXME!!! Do I need to add current back to the readyList? I think YES. FIXME!!!
        dispatcher();
    }
} /* clockHandler */

/* ------------------------------------------------------------------------
 Name - getCurrentTime
 Purpose - Returns the time in milliseconds since USLOSS started running
 Parameters - none
 Returns - int - Time in milliseconds since USLOSS started running.
 Side Effects - none
 ----------------------------------------------------------------------- */
int getCurrentTime() {
    int status;
    if (USLOSS_DeviceInput(USLOSS_CLOCK_INT, 0, &status) == USLOSS_DEV_INVALID) {
        USLOSS_Console("ERROR: getCurrentTime(): Encountered error fetching current time. Halting.\n");
        USLOSS_Halt(1);
    }
    return status/1000;
} /* getCurrentTime */

/* ------------------------------------------------------------------------
 Name - illegalArgumentHandler
 Purpose - Basically only here to avoid warnings about uninitialized values.
         - Required by USLOSS.
         - May have to alter this method later. Check Piazza for updates.
 Parameters - none
 Returns - nothing
 Side Effects - none
 ----------------------------------------------------------------------- */
void illegalArgumentHandler(int dev, void *arg) {
    if (DEBUG && debugflag) {
        USLOSS_Console("illegalArgumentHandler(): called\n");
    }
} /* illegalArgumentHandler */
