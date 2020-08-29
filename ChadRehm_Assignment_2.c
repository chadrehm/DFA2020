/*
 * AUthor: Chad Rehm
 * Date: 8/27/2020
 * Description: Build or load a DFA and Accept or Reject user input.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN_STATE 25
#define FALSE 0
#define TRUE 1

typedef unsigned int bool;  // %%twb -- added in place of above include.

/*
 * Struct definitions
 */
typedef struct Transition {
	char * startState;
	char * symbol;
	char * endState;
	struct Transition *next;
} TRANSITION;

typedef struct delta {
	char * symbol;
	struct delta *next;
	TRANSITION * transition;
} DELTA;

typedef struct State {
    bool initialState;
    bool finalState;
    char * stateName;
    DELTA * deltas;
} STATE;

typedef struct Automaton {
	TRANSITION *transitionTop;
	STATE * states[MAX_LEN_STATE];
	int statesCount;
} AUTOMATON;

/*
 * Prototypes
 */
void saveDFA(AUTOMATON *autoPtr);
void readFile(AUTOMATON *autoPtr, char *filepath);
void buildDFA(AUTOMATON *autoPtr);
void setStates(AUTOMATON *autoPtr, char *states);
void setInitialState(AUTOMATON *autoPtr, char *stateName);
void setFinalStates(AUTOMATON *autoPtr, char *states);
struct Transition* buildTransition(char *transition);
DELTA* buildDelta(TRANSITION *transition);
void insertDelta(AUTOMATON *autoPtr, char *startState, DELTA *delta);
struct Transition* insertTransition(struct Transition *top, struct Transition *transition);
int findState(AUTOMATON *autoPtr, char *stateName);
int findInitalState(AUTOMATON *autoPtr);
char* findSymbol(STATE *state, char symbol); // return next state
int getDfaSource();
void processDfa(AUTOMATON *autoPtr);
char *fgetstr(char *string, int n, FILE *stream);

/* 
 * 	Entry point to program.
 *  The processDfa is 
 *  Calling processDfa is a big O of n squared that is the largest 
 *  big O of all the functions called in the method.  The total
 *  big O of the program is n squared.
 */
int main() {
    int dfaSource = getDfaSource();

    AUTOMATON *autoPtr;
	autoPtr = (AUTOMATON *)malloc(sizeof(AUTOMATON));

    if (dfaSource == 1) {
        buildDFA(autoPtr);
        
        char input[100];
        printf("Would you like to save this DFA (y or n)?\n");
    	scanf("%s", input);
    	
		if (input[0] == 'y') {
			saveDFA(autoPtr);
		}
		
    } else if (dfaSource == 2) {
    	char filepath[1000];
    	printf("What file path name:\n"); 
		scanf("%s", filepath);
		  	
        readFile(autoPtr, filepath);
    }
    
    processDfa(autoPtr);

    return 0;
}

/* 
 * 	Save a user built DFA
 *  There is two loops in seires so the big O is O(2*n) or O(n).
 */
void saveDFA(AUTOMATON *autoPtr) {
	char filePath[1000];
	int initialStateIdx;
	FILE *fp;
	int idx;	
	TRANSITION * top;

    printf("What file path name:\n");
    scanf("%s", filePath);
    
    fp = fopen (filePath, "w+");
    
	for (idx = 0; idx < autoPtr->statesCount; idx++) {
		fputs (autoPtr->states[idx]->stateName, fp);
		
		if (autoPtr->statesCount - idx > 1) {
			fputs (",", fp);
		}
		
		if (autoPtr->states[idx]->initialState) {
			initialStateIdx = idx;
		}
	}
	
	fprintf (fp, "\n%s\n", autoPtr->states[initialStateIdx]->stateName);
	
	for (idx = 0; idx < autoPtr->statesCount; idx++) {	
		if (autoPtr->states[idx]->finalState) {
			fputs (autoPtr->states[idx]->stateName, fp);
			
			if (autoPtr->statesCount - idx > 1) {
				fputs (",", fp);
			}
		}
	}
	
	fputs ("\n", fp);
	
	top = autoPtr->transitionTop;

	while (top->next != NULL) {
		fprintf (fp, "%s,%s,%s\n", top->startState, top->symbol, top->endState);
		top = top->next;
	} 
	fprintf (fp, "%s,%s,%s\n", top->startState, top->symbol, top->endState);
} 

/* 
 * 	Read a file to get DFA.
 *  There is a single loop so the big O is O(n).
*/
void readFile(AUTOMATON *autoPtr, char *filepath) {
	
	char line[100];
	DELTA *delta;
	FILE *fp = fopen( filepath, "r");

	if (fp != 0) {
	
		int lineIdx = 0;
		
		TRANSITION *top = NULL;
	 	TRANSITION *transition = NULL;
		
		while( fgetstr( line, sizeof(line), fp ) != 0 ) {
			if (lineIdx == 0) {
				setStates ( autoPtr, line );
			} else if (lineIdx == 1) {
				setInitialState ( autoPtr, line);
			} else if ( lineIdx == 2 ) {
				setFinalStates (autoPtr, line);
			} else {
				transition = buildTransition(line);
				top = insertTransition(top, transition);
				
				delta = (DELTA *)malloc(sizeof(DELTA));
				delta = buildDelta(transition);		
				insertDelta(autoPtr, transition->startState, delta);
			}
			
			lineIdx++;
		}
		
		autoPtr->transitionTop = top;
		fclose(fp);
	} else {
		printf("File %s faild to open\n", filepath);
		exit(0);
	}
}

/* 
 * 	Build DFA from user input
 *  There is a single loop so the big O is O(n) based on
 *  user input.
 */
void buildDFA(AUTOMATON *autoPtr) {
    char input[1000];

    printf("Enter States (comma separated list):\n");
    scanf("%s", input);
    setStates(autoPtr, input);

    printf("Enter Initial state:\n");
    scanf("%s", input);
    setInitialState(autoPtr, input);

    printf("Enter accepting states (comma separated list):\n");
    scanf("%s", input);
    setFinalStates(autoPtr, input);    

    TRANSITION *top = NULL;
 	TRANSITION *transition = NULL;
 	
	char done = 'n';
	do {
		printf("Enter a transition\n");
		scanf("%s", input);
		transition = buildTransition(input);
		top = insertTransition(top, transition);
		
		DELTA *delta = (DELTA *)malloc(sizeof(DELTA));
		delta = buildDelta(transition);		
		insertDelta(autoPtr, transition->startState, delta);
					
		printf("Another transition (y or n)?\n");
		scanf("%s", input);

	} while (input[0] == 'y');
	autoPtr->transitionTop = top;
}

/* 
 * 	Create States and populate stateName of the State.  Add state to 
 *  the Automoton.
 *  There is a single loop so the big O is O(n).
 *  
 */ 
void setStates(AUTOMATON *autoPtr, char *states) {
    char * statePt;
    // Get first token
    statePt = strtok (states,",");

    int statesCount = 0;
    while (statePt != NULL)
    {
    	// initialze new STATE struct
		autoPtr->states[statesCount] = (STATE *)malloc(sizeof(STATE));
		autoPtr->states[statesCount]->initialState = FALSE;
		autoPtr->states[statesCount]->finalState = FALSE;	
		autoPtr->states[statesCount]->deltas = NULL;
		autoPtr->states[statesCount]->stateName = (char *)malloc(strlen(statePt));

        strcpy(autoPtr->states[statesCount]->stateName, statePt);

        statePt = strtok(NULL, ",");
        statesCount += 1;
    }
    autoPtr->statesCount = statesCount;
}

/* 
 * 	Set initialState for the stateName provide.  Calls findState to locate
 *  state pointer.
 *  This function calls findState which has a big O of O(n).  This big O
 *  of this function is O(1) without the call to findState.  The total 
 *  big O of this function is O(n)
 */ 
void setInitialState(AUTOMATON *autoPtr, char *stateName) {
	int stateIdx = findState(autoPtr, stateName);
	
	if (stateIdx >= 0) {
		autoPtr->states[stateIdx]->initialState = TRUE;	
	}
}

/* 
 * 	Sets final state on all states provided.  Calls findState
 *  to locate state pointer.
 *  This has one loop that calls another loop the big O is O 
 *  of n squared.
 */ 
void setFinalStates(AUTOMATON *autoPtr, char *states) {
    char * statePt;
    // Get first token
    statePt = strtok (states,",");

    int stateIdx = 0;
    while (statePt != NULL)
    {
        stateIdx = findState(autoPtr, statePt);

		if (stateIdx >= 0) {
			autoPtr->states[stateIdx]->finalState = TRUE;	
		}

        statePt = strtok(NULL, ",");
        stateIdx += 1;
    }
}

/* 
 * 	Build Transition for user input.
 *  This has no loops the big O is O(1).
 */ 
struct Transition* buildTransition(char *transition) {
	TRANSITION *trans = NULL;
	trans = (TRANSITION *)malloc(sizeof(TRANSITION));
	
	char * transPt;
    // Get startState q1,0,q2
    transPt = strtok(transition,",");
	trans->startState = (char *)malloc(strlen(transPt));
	strcpy(trans->startState, transPt);

	// Get symbol
    transPt = strtok(NULL, ",");
    trans->symbol = (char *)malloc(strlen(transPt));
    strcpy(trans->symbol, transPt);
    
    // Get endState
    transPt = strtok(NULL, ",");
    trans->endState = (char *)malloc(strlen(transPt));
	strcpy(trans->endState, transPt);
	
	trans->next = NULL;
    
    return trans;
}

/* 
 * 	Build delta (symbol).  This is a tuple with the sybmol
 *  Being the lockup value and the second value points to the
 *  Transition.
 *  This has no loops the big O is O(1).
 */ 
DELTA* buildDelta(TRANSITION *transition) {
	DELTA *delta = NULL;
	delta = (DELTA *)malloc(sizeof(DELTA));
	
	delta->symbol = (char *)malloc(strlen(transition->symbol));
	strcpy(delta->symbol, transition->symbol);
	
	delta->next = NULL;
	
	delta->transition = transition;
	return delta;
}

/* 
 * 	Inserts Delta to the appropreate state.  Uses findState to get the state
 *  index.
 *  There is a single loop so the big O is O(n).
 */ 
void insertDelta(AUTOMATON *autoPtr, char *startState, DELTA *delta) {
	int stateIdx = findState(autoPtr, startState);
	DELTA *current = NULL; 
	current = autoPtr->states[stateIdx]->deltas;
	
	if (current == NULL) {
		autoPtr->states[stateIdx]->deltas = delta;
	} else {
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = delta;
	}
}

/* 
 * 	Inserts a transition into the transition list.
 *  This has no loops the big O is O(1).
 */ 
struct Transition* insertTransition(struct Transition *top, struct Transition *transition) {
  transition->next = top;	       	/* insert new node at top of list */
  top = transition;

  return top;
}

/* 
 * 	Sets final state on all states provided.  Calls findState
 *  to locate state pointer.
 *  There is a single loop so the big O is O(n).
 */ 
int findState(AUTOMATON *autoPtr, char *stateName) {
     int idx;

    for(idx = 0; idx < autoPtr->statesCount; idx++) {
        if (strcmp(autoPtr->states[idx]->stateName, stateName) == 0) {
    		return idx;    	
        }
    }
    return -1;
}

/* 
 * 	Find and return a states index in the Automoton states array.
 *  There is a single loop so the big O is O(n).
 */ 
int findInitalState(AUTOMATON *autoPtr) {
	int idx;
	for(idx = 0; idx < MAX_LEN_STATE; idx++) {
		if (autoPtr->states[idx]->initialState) {
			return idx;
		}
	}
}

/* 
 * 	Confirms that the user input character is a symbol on the current
 *  State.  Returns the name of the next state.
 *  There is a single loop so the big O is O(n).
 */ 
char* findSymbol(STATE *state, char symbol) {
	DELTA *current = NULL; 
	current = state->deltas;
	
	while (current != NULL) {
		if (current->symbol[0] == symbol) {
			return current->transition->endState;
		}
		current = current->next;
	}
	printf("Invalid character found.\n");
	return NULL;
}

/* 
 * 	Prompt user for source of DFA.  File or Console.
 *  This has no loops the big O is O(1).
 */ 
int getDfaSource () {
    int dfaSource;

    printf("Please create or load a DFA:\n");
    printf("1 - Enter DFA 2 - Read DFA\n");
    scanf("%d", &dfaSource);
    fflush (stdin);
    return dfaSource;
}

/* 
 * 	Prompt user for input to DFA. Repeat until user doesn't input 'y'.
 *  Accept or Reject input.
 *  This has one loop that runs each input that calls another loop that
 *  big O(n) this is big O of n squared.
 */ 
void processDfa(AUTOMATON *autoPtr) {
	char *userInput;
	
	char done = 'n';
	do {
		printf("Please enter an input string to test your DFA:\n");
    	scanf("%s", userInput);
    	int inputLength = strlen(userInput);
    	int stateIdx = findInitalState(autoPtr);
    	char *nextStateName;
    	
    	STATE *state = (STATE *)malloc(sizeof(STATE));
    	state = autoPtr->states[stateIdx]; //initalState
    	
    	int idx;
    	for (idx = 0; idx < inputLength; idx++) {
    		nextStateName = findSymbol(state, userInput[idx]);
    		if(nextStateName == NULL) {
    			break;
			} else {
    			stateIdx = findState(autoPtr, nextStateName);
    			state = autoPtr->states[stateIdx];
			}
		}
		
		if (state->finalState && nextStateName != NULL) {
			printf("Accepted\n");
		} else {
			printf("Rejected\n");
		}
					
		printf("Would you like to test another string (y or n)?\n");
    	fflush (stdin);
		done = getchar();
		fflush (stdin);
	} while (done == 'y');
	free(autoPtr);
}

//	fgetstr() - mimics behavior of fgets(), but removes new-line
//  character at end of line if it exists
//  This function was sampled from http://www.siafoo.net/snippet/75
char *fgetstr(char *string, int n, FILE *stream) {
	char *result;
	result = fgets(string, n, stream);
	if(!result)
		return(result);

	if(string[strlen(string) - 1] == '\n')
		string[strlen(string) - 1] = 0;

	return(string);
}
