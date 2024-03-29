/* Patrick's DEBUG printing constant... */
#define DEBUG 1


typedef struct procStruct procStruct;

typedef struct procStruct * procPtr;

struct procStruct {
    procPtr         nextProcPtr;
    procPtr         childProcPtr;
    procPtr         nextSiblingPtr;
    procPtr         parentProcPtr;
    procPtr         quitChildPtr;
    procPtr         quitSiblingPtr;
    procPtr         whoZappedMePtr;
    procPtr         whoZappedMeSiblingPtr;
    char            name[MAXNAME];     /* process's name */
    char            startArg[MAXARG];  /* args passed to process */
    USLOSS_Context  state;             /* current context for process */
    short           pid;               /* process id */
    int             priority;
    int (* startFunc) (char *);   /* function where process begins -- launch */
    char           *stack;
    unsigned int    stackSize;
    int             status;        /* READY, BLOCKED, QUIT, etc. */
   /* other fields as needed... */
    int             procStartTime;
    int             zapped;
    int             quitStatus;
    int             totalTime;
};

struct psrBits {
    unsigned int curMode:1;
    unsigned int curIntEnable:1;
    unsigned int prevMode:1;
    unsigned int prevIntEnable:1;
    unsigned int unused:28;
};

union psrValues {
   struct psrBits bits;
   unsigned int integerPart;
};

/* Some useful constants.  Add more as needed... */
#define NO_CURRENT_PROCESS       NULL
#define NO_PROCESS_ASSIGNED     -1
#define ACTIVE                   1
#define READY                    2
#define QUIT                     3

#define BLOCKED_ON_ZAP           8
#define BLOCKED_ON_JOIN          9
#define BLOCKED                 10
#define BLOCKED_ON_ME           11

#define MINPRIORITY              5
#define MAXPRIORITY              1
#define SENTINELPID              1
#define SENTINELPRIORITY         (MINPRIORITY + 1)

