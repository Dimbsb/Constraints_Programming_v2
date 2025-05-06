// compile with: gcc -o output mc3.c
// run with: ./output
// Results will be saved in results.txt
// test.csv is a testing csv to see if everything works ok... i mean 73x73
// constraints.csv is the one that will be used in the algorithm and it is 73x73
// as our courses... We must implement it as Stergiou said constraints types are
// (0,1,2,3,4) 0 = no constraint 1 = Xi != Xj 2 = Xi / 3 != Xj / 3 3 = abs(Xi /
// 3 - Xj / 3) > 6 4 = (Xi / 3 == Xj / 3 && Xi % 3 < Xj % 3) Do not care about
// what gets printed at output(just debugging)...check only .txt

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define TABU_SIZE 10 // ADJUSTABLE
typedef struct {
  int variable; // X
  int value;    // a

} TabuEntry;
typedef struct {
  TabuEntry entries[TABU_SIZE];
  int front;
  int rear;
  int count;
} TabuQueue;

// Functions signature
void readConstraintsMatrix(const char *filename, int constraints[73][73]);
int satisfies(int *Xvalue, int numberofvariables, int numberofvalues);
void addToTabuList(TabuQueue *queue, int value, int variable);
void initTabuQ(TabuQueue *queue);
int isInTabuList(TabuQueue *queue, int variable, int value);
int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues);
int AlternativeAssignment(int *Xvalue, int numberofvariables, int variable, int numberofvalues);
void Tabu_Min_Conflicts(int *Xvalue, int numberofvariables, int numberofvalues, int maxTries, int maxChanges, TabuQueue *TabuList, FILE *outputFile, int *moves, int *bestConflicts);
int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile);

int main()
{
    int maxTries = 100;
    int maxChanges = 100;
    int numberofvariables = 73;
    int days = 20;
    int Xvalue[numberofvariables];
    int numberofvalues = days * 3;
    int PrecedureRestarts = 2;

    FILE *outputFile = fopen("THIRD.txt", "w");
    if (outputFile == NULL)
    {
        printf("ERROR OPENING TXT FILE.\n");
        return 1;
    }

    fprintf(outputFile, "RUN RESULTS:\n");
    fprintf(outputFile, "----------------------------------------------\n");

    srand(time(NULL));

    TabuQueue TabuList;

    // Statistics
    int totalMoves = 0;
    int totalBestConflicts = 0;
    double totalExecutionTime = 0.0;
    int solutionsFound = 0;

    for (int run = 0; run < PrecedureRestarts; run++)
    {
        initTabuQ(&TabuList);

        int moves = 0;
        int bestConflicts = 0;

        clock_t start = clock(); // Start timing
        Tabu_Min_Conflicts(Xvalue, numberofvariables, numberofvalues, maxTries, maxChanges, &TabuList, outputFile, &moves, &bestConflicts);
        clock_t end = clock(); // End timing
        double executionTime = (double)(end - start) / CLOCKS_PER_SEC;

        totalMoves += moves;
        totalBestConflicts += bestConflicts;
        totalExecutionTime += executionTime;

        if (bestConflicts == 0)
        {
            solutionsFound++;
        }

        fprintf(outputFile, "Run %d: Moves = %d, Best Conflicts = %d, Execution Time = %.2f seconds\n", run + 1, moves, bestConflicts, executionTime);
    }

    // Calculate averages
    double averageMoves = (double)totalMoves / 20;
    double averageBestConflicts = (double)totalBestConflicts / 20;
    double averageExecutionTime = totalExecutionTime / 20;

    // Print summary
    fprintf(outputFile, "\nSUMMARY:\n");
    fprintf(outputFile, "----------------------------------------------\n");
    fprintf(outputFile, "Solutions Found: %d/20\n", solutionsFound);
    fprintf(outputFile, "Average Moves: %.2f\n", averageMoves);
    fprintf(outputFile, "Average Best Conflicts: %.2f\n", averageBestConflicts);
    fprintf(outputFile, "Average Execution Time: %.2f seconds\n", averageExecutionTime);

    fclose(outputFile);
    printf("RESULTS SAVED TO THIRD.txt\n");

    return 0;
}

int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile){
    // A := initial complete assignment of the variables in Problem
    for (int i = 0; i < numberofvariables; i++)
    {
        Xvalue[i] = rand() % numberofvalues + 1;
    }
    // Print initial assignment
    fprintf(outputFile, "INITIAL ASSIGNMENT:\n");
    for (int i = 0; i < numberofvariables; i++)
    {
        fprintf(outputFile, "X%d = %d\n", i, Xvalue[i]);
    }
    return Xvalue;
}


void initTabuQ(TabuQueue *queue) {
  queue->front = 0;
  queue->rear = -1;
  queue->count = 0;
}
int isInTabuList(TabuQueue *queue, int variable, int value) {
  for (int i = 0; i < queue->count; i++) {
    int idx = (queue->front + i) % TABU_SIZE;
    if (queue->entries[idx].variable == variable &&
        queue->entries[idx].value == value)
      return 1;
  }
  return 0;
}
void addToTabuList(TabuQueue *queue, int value, int variable) {
  if (queue->count == TABU_SIZE) {
    queue->front = (queue->front + 1) % TABU_SIZE;
    queue->count--;
  }
  queue->rear = (queue->rear + 1) % TABU_SIZE;
  queue->entries[queue->rear].variable = variable;
  queue->entries[queue->rear].value = value;
  queue->count++;
}

void readConstraintsMatrix(const char *filename, int constraints[73][73])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("ERROR OPENING CSV FILE.\n");
        exit(1);
    }

    char buffer[2048];
    int row = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, ",");
        int col = 0;

        while (token != NULL)
        {
            constraints[row][col] = atoi(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    fclose(file);
}

int satisfies(int *Xvalue, int numberofvariables, int numberofvalues)
{
    int conflicts = 0;
    int constraints[73][73] = {0};

    readConstraintsMatrix("BetterCSVview.csv", constraints);

    for (int i = 0; i < numberofvariables; i++)
    {
        for (int j = i+1; j < numberofvariables; j++)
        {
            int constraint = constraints[i][j];

            if (constraint == 1)
            {
                if (Xvalue[i] == Xvalue[j])
                {
                    conflicts++;
                }
            }
            else if (constraint == 2)
            {
                int diff = abs((Xvalue[i] / 3) - (Xvalue[j] / 3));
                if (diff < 2)
                {
                    conflicts++;
                }
            }
            else if (constraint == 3)
            {
                if ((Xvalue[i] / 3) == (Xvalue[j] / 3))
                {
                    conflicts++;
                }
            }
            else if (constraint == 4)
            {
                if ((Xvalue[i] / 3 == Xvalue[j] / 3) && (Xvalue[i] % 3 >= Xvalue[j] % 3))
                {
                    conflicts++;
                }
            }
        }
    }

    return conflicts;
}

int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues)
{
    int VariableWithConflicts[numberofvariables];
    int Counter = 0;

    for (int i = 0; i < numberofvariables; i++)
    {
        if (satisfies(Xvalue, numberofvariables, numberofvalues) > 0)
        {
            VariableWithConflicts[Counter] = i;
            Counter++;
        }
    }

    if (Counter == 0)
    {
        return -1;
    }

    return VariableWithConflicts[rand() % Counter];
}

int AlternativeAssignment(int *Xvalue, int numberofvariables, int variable, int numberofvalues)
{
    int BetterValue = Xvalue[variable];
    int TotalInitialConflicts = satisfies(Xvalue, numberofvariables, numberofvalues);

    for (int NewValue = 0; NewValue < numberofvalues; NewValue++)
    {
        Xvalue[variable] = NewValue;

        int NewConflicts = satisfies(Xvalue, numberofvariables, numberofvalues);

        if (NewConflicts < TotalInitialConflicts)
        {
            TotalInitialConflicts = NewConflicts;
            BetterValue = NewValue;
        }
    }

    Xvalue[variable] = BetterValue;

    return BetterValue;
}

void Tabu_Min_Conflicts(int *Xvalue, int numberofvariables, int numberofvalues, int maxTries, int maxChanges, TabuQueue *TabuList, FILE *outputFile, int *moves, int *bestConflicts)
{
    *moves = 0;
    *bestConflicts = INT_MAX;

    for (int tries = 0; tries < maxTries; tries++)
    {
        Xvalue = initialize(Xvalue, numberofvariables, numberofvalues, outputFile);

        for (int changes = 0; changes < maxChanges; changes++)
        {
            int conflicts = satisfies(Xvalue, numberofvariables, numberofvalues);
            if (conflicts == 0)
            {
                *bestConflicts = 0;
                fprintf(outputFile, "Solution found after %d tries and %d changes.\n", tries, changes);
                return;
            }

            if (conflicts < *bestConflicts)
            {
                *bestConflicts = conflicts;
            }

            int variableInConflict = RandomVariableConflict(Xvalue, numberofvariables, numberofvalues);

            int bestValue = AlternativeAssignment(Xvalue, numberofvariables, variableInConflict, numberofvalues);

            if (!isInTabuList(TabuList, variableInConflict, bestValue) || satisfies(Xvalue, numberofvariables, numberofvalues) < conflicts)
            {
                int previousValue = Xvalue[variableInConflict];
                Xvalue[variableInConflict] = bestValue;

                addToTabuList(TabuList, previousValue, variableInConflict);

                (*moves)++; // Increment the move counter

                fprintf(outputFile, "Variable X%d changed from %d to %d. Conflicts: %d\n",
                        variableInConflict, previousValue, bestValue, satisfies(Xvalue, numberofvariables, numberofvalues));
            }
        }
    }

    fprintf(outputFile, "No solution found after %d tries and %d changes.\n", maxTries, maxChanges);
}