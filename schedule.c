#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_PROCESSES 20
#define QUEUE_SIZE 64

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;

    int remaining_time;
    int start_time;
    int end_time;
    int waiting_time;

    int started;
    int completed;
} Process;

/* ---------------------------------------------------------
   Reset values used during a simulation
   --------------------------------------------------------- */
void reset_processes(Process processes[], int count)
{
    int i;

    for (i = 0; i < count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].waiting_time = 0;
        processes[i].started = 0;
        processes[i].completed = 0;
    }
}

/* ---------------------------------------------------------
   Read process data from input file.

   This program accepts BOTH formats:

   Format 1:
   4
   0 0 12
   1 2 4
   2 3 1
   3 4 2

   Format 2:
   0 0 12
   1 2 4
   2 3 1
   3 4 2
   --------------------------------------------------------- */
int read_input_file(const char *filename, Process processes[])
{
    FILE *file;
    int values[MAX_PROCESSES * 3 + 1];
    int value_count = 0;
    int temp;
    int process_count;
    int start_index;
    int i;

    file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: Could not open input file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%d", &temp) == 1) {
        if (value_count >= (MAX_PROCESSES * 3 + 1)) {
            printf("Error: Too much data in input file.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        values[value_count++] = temp;
    }

    fclose(file);

    if (value_count == 0) {
        printf("Error: Input file is empty.\n");
        exit(EXIT_FAILURE);
    }

    /*
     * If the first number is a process count, the remaining
     * number of integers should equal count * 3.
     */
    if (value_count >= 4 &&
        values[0] >= 1 &&
        values[0] <= MAX_PROCESSES &&
        value_count == 1 + values[0] * 3) {

        process_count = values[0];
        start_index = 1;
    }
    else {
        /*
         * Otherwise assume the entire file contains groups
         * of pid, arrival_time, burst_time.
         */
        if (value_count % 3 != 0) {
            printf("Error: Invalid input file format.\n");
            exit(EXIT_FAILURE);
        }

        process_count = value_count / 3;
        start_index = 0;
    }

    if (process_count > MAX_PROCESSES) {
        printf("Error: Maximum number of processes is %d.\n",
               MAX_PROCESSES);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < process_count; i++) {
        processes[i].pid =
            values[start_index + i * 3];

        processes[i].arrival_time =
            values[start_index + i * 3 + 1];

        processes[i].burst_time =
            values[start_index + i * 3 + 2];

        if (processes[i].arrival_time < 0 ||
            processes[i].burst_time <= 0) {

            printf("Error: Invalid process information for PID %d.\n",
                   processes[i].pid);
            exit(EXIT_FAILURE);
        }
    }

    reset_processes(processes, process_count);

    return process_count;
}

/* ---------------------------------------------------------
   Check whether all processes have completed
   --------------------------------------------------------- */
int all_completed(Process processes[], int count)
{
    int i;

    for (i = 0; i < count; i++) {
        if (!processes[i].completed) {
            return 0;
        }
    }

    return 1;
}

/* ---------------------------------------------------------
   Print statistics table
   --------------------------------------------------------- */
void print_statistics(Process processes[],
                      int count,
                      int order[],
                      const char *algorithm)
{
    int i;
    double total_waiting = 0.0;

    printf("\n%s:\n\n", algorithm);

    printf("%-8s %-14s %-12s %-10s %-14s %-14s\n",
           "PID",
           "Arrival Time",
           "Start Time",
           "End Time",
           "Running Time",
           "Waiting Time");

    printf("--------------------------------------------------------------------------\n");

    for (i = 0; i < count; i++) {
        int index = order[i];

        processes[index].waiting_time =
            processes[index].end_time
            - processes[index].arrival_time
            - processes[index].burst_time;

        total_waiting += processes[index].waiting_time;

        printf("%-8d %-14d %-12d %-10d %-14d %-14d\n",
               processes[index].pid,
               processes[index].arrival_time,
               processes[index].start_time,
               processes[index].end_time,
               processes[index].burst_time,
               processes[index].waiting_time);
    }

    printf("\nAverage Waiting Time: %.2f\n",
           total_waiting / count);
}

/* =========================================================
   FCFS
   ========================================================= */
void simulate_fcfs(Process processes[], int count)
{
    int time = 0;
    int current = -1;
    int completed_count = 0;
    int order[MAX_PROCESSES];
    int order_count = 0;
    int i;

    reset_processes(processes, count);

    printf("\n========================================\n");
    printf("FCFS Scheduling Simulation\n");
    printf("========================================\n");

    while (completed_count < count) {

        /*
         * Select next process only if CPU is currently free.
         */
        if (current == -1) {
            int best_arrival = INT_MAX;

            for (i = 0; i < count; i++) {

                if (!processes[i].completed &&
                    processes[i].arrival_time <= time) {

                    if (processes[i].arrival_time < best_arrival) {
                        best_arrival = processes[i].arrival_time;
                        current = i;
                    }
                }
            }

            if (current != -1 &&
                !processes[current].started) {

                processes[current].started = 1;
                processes[current].start_time = time;

                order[order_count++] = current;
            }
        }

        /*
         * No process available
         */
        if (current == -1) {
            printf("Time %d -> %d: IDLE\n",
                   time, time + 1);

            time++;
            continue;
        }

        /*
         * Run selected process for one time unit
         */
        printf("Time %d -> %d: PID %d\n",
               time,
               time + 1,
               processes[current].pid);

        processes[current].remaining_time--;
        time++;

        /*
         * Process finished
         */
        if (processes[current].remaining_time == 0) {
            processes[current].completed = 1;
            processes[current].end_time = time;

            printf("PID %d completed at time %d\n",
                   processes[current].pid,
                   time);

            completed_count++;
            current = -1;
        }
    }

    print_statistics(processes,
                     count,
                     order,
                     "FCFS");
}

/* =========================================================
   SJF - Non-preemptive Shortest Job First
   ========================================================= */
void simulate_sjf(Process processes[], int count)
{
    int time = 0;
    int current = -1;
    int completed_count = 0;
    int order[MAX_PROCESSES];
    int order_count = 0;
    int i;

    reset_processes(processes, count);

    printf("\n========================================\n");
    printf("SJF Scheduling Simulation\n");
    printf("========================================\n");

    while (completed_count < count) {

        /*
         * If CPU is free, find shortest available job.
         */
        if (current == -1) {
            int shortest_burst = INT_MAX;
            int earliest_arrival = INT_MAX;

            for (i = 0; i < count; i++) {

                if (!processes[i].completed &&
                    processes[i].arrival_time <= time) {

                    /*
                     * Select shortest burst.
                     * If tied, choose earlier arrival.
                     */
                    if (processes[i].burst_time < shortest_burst ||
                        (processes[i].burst_time == shortest_burst &&
                         processes[i].arrival_time < earliest_arrival)) {

                        shortest_burst = processes[i].burst_time;
                        earliest_arrival = processes[i].arrival_time;
                        current = i;
                    }
                }
            }

            if (current != -1 &&
                !processes[current].started) {

                processes[current].started = 1;
                processes[current].start_time = time;

                order[order_count++] = current;
            }
        }

        /*
         * No process ready
         */
        if (current == -1) {
            printf("Time %d -> %d: IDLE\n",
                   time,
                   time + 1);

            time++;
            continue;
        }

        /*
         * Run process for one unit.
         */
        printf("Time %d -> %d: PID %d\n",
               time,
               time + 1,
               processes[current].pid);

        processes[current].remaining_time--;
        time++;

        /*
         * SJF is non-preemptive, so the current process
         * continues until it finishes.
         */
        if (processes[current].remaining_time == 0) {

            processes[current].completed = 1;
            processes[current].end_time = time;

            printf("PID %d completed at time %d\n",
                   processes[current].pid,
                   time);

            completed_count++;
            current = -1;
        }
    }

    print_statistics(processes,
                     count,
                     order,
                     "SJF");
}

/* =========================================================
   ROUND ROBIN QUEUE FUNCTIONS
   ========================================================= */

void enqueue(int queue[],
             int *rear,
             int *queue_count,
             int value)
{
    if (*queue_count >= QUEUE_SIZE) {
        printf("Error: Ready queue overflow.\n");
        exit(EXIT_FAILURE);
    }

    queue[*rear] = value;
    *rear = (*rear + 1) % QUEUE_SIZE;
    (*queue_count)++;
}

int dequeue(int queue[],
            int *front,
            int *queue_count)
{
    int value;

    if (*queue_count <= 0) {
        return -1;
    }

    value = queue[*front];

    *front = (*front + 1) % QUEUE_SIZE;
    (*queue_count)--;

    return value;
}

/*
 * Add newly arrived processes to Round Robin queue.
 */
void add_arrivals(Process processes[],
                  int count,
                  int time,
                  int added[],
                  int queue[],
                  int *rear,
                  int *queue_count)
{
    int i;

    for (i = 0; i < count; i++) {

        if (!added[i] &&
            processes[i].arrival_time <= time) {

            enqueue(queue,
                    rear,
                    queue_count,
                    i);

            added[i] = 1;

            printf("Time %d: PID %d entered ready queue\n",
                   time,
                   processes[i].pid);
        }
    }
}

/* =========================================================
   ROUND ROBIN
   ========================================================= */
void simulate_rr(Process processes[],
                 int count,
                 int quantum)
{
    int queue[QUEUE_SIZE];
    int front = 0;
    int rear = 0;
    int queue_count = 0;

    int added[MAX_PROCESSES] = {0};

    int time = 0;
    int completed_count = 0;
    int current;
    int run_time;
    int i;

    int order[MAX_PROCESSES];

    /*
     * Used for printing execution segments similar
     * to the sample output.
     */
    int segment_pid[500];
    int segment_start[500];
    int segment_end[500];
    int segment_running[500];
    int segment_count = 0;

    double total_waiting = 0.0;

    reset_processes(processes, count);

    if (quantum <= 0) {
        printf("Error: Time quantum must be greater than 0.\n");
        return;
    }

    printf("\n========================================\n");
    printf("Round Robin Scheduling Simulation\n");
    printf("Time Quantum = %d\n", quantum);
    printf("========================================\n");

    while (completed_count < count) {

        /*
         * Add any processes that have arrived.
         */
        add_arrivals(processes,
                     count,
                     time,
                     added,
                     queue,
                     &rear,
                     &queue_count);

        /*
         * Ready queue empty
         */
        if (queue_count == 0) {
            printf("Time %d -> %d: IDLE\n",
                   time,
                   time + 1);

            time++;
            continue;
        }

        current = dequeue(queue,
                          &front,
                          &queue_count);

        /*
         * Record first start time.
         */
        if (!processes[current].started) {
            processes[current].started = 1;
            processes[current].start_time = time;
        }

        segment_pid[segment_count] =
            processes[current].pid;

        segment_start[segment_count] = time;

        run_time = 0;

        /*
         * Run until quantum expires or process completes.
         */
        while (run_time < quantum &&
               processes[current].remaining_time > 0) {

            printf("Time %d -> %d: PID %d\n",
                   time,
                   time + 1,
                   processes[current].pid);

            processes[current].remaining_time--;

            run_time++;
            time++;

            /*
             * Processes may arrive while another process
             * is using the CPU.
             */
            add_arrivals(processes,
                         count,
                         time,
                         added,
                         queue,
                         &rear,
                         &queue_count);
        }

        segment_end[segment_count] = time;
        segment_running[segment_count] = run_time;
        segment_count++;

        /*
         * Process finished
         */
        if (processes[current].remaining_time == 0) {

            processes[current].completed = 1;
            processes[current].end_time = time;

            printf("PID %d completed at time %d\n",
                   processes[current].pid,
                   time);

            completed_count++;
        }
        else {
            /*
             * Quantum expired, put process at end
             * of ready queue.
             */
            enqueue(queue,
                    &rear,
                    &queue_count,
                    current);
        }
    }

    /*
     * Round Robin execution table
     */
    printf("\nRR (Time quantum = %d):\n\n", quantum);

    printf("%-8s %-12s %-12s %-14s\n",
           "PID",
           "Start Time",
           "End Time",
           "Running Time");

    printf("-----------------------------------------------\n");

    for (i = 0; i < segment_count; i++) {

        printf("%-8d %-12d %-12d %-14d\n",
               segment_pid[i],
               segment_start[i],
               segment_end[i],
               segment_running[i]);
    }

    /*
     * Final Round Robin statistics
     */
    printf("\n%-8s %-14s %-14s %-10s %-14s\n",
           "PID",
           "Arrival Time",
           "Running Time",
           "End Time",
           "Waiting Time");

    printf("----------------------------------------------------------------\n");

    /*
     * Print processes by PID/input order.
     */
    for (i = 0; i < count; i++) {

        order[i] = i;

        processes[i].waiting_time =
            processes[i].end_time
            - processes[i].arrival_time
            - processes[i].burst_time;

        total_waiting += processes[i].waiting_time;

        printf("%-8d %-14d %-14d %-10d %-14d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].end_time,
               processes[i].waiting_time);
    }

    printf("\nAverage Waiting Time: %.2f\n",
           total_waiting / count);
}

/* =========================================================
   MAIN
   ========================================================= */
int main(int argc, char *argv[])
{
    Process processes[MAX_PROCESSES];
    int process_count;
    int quantum;

    /*
     * Correct usage:
     *
     * ./schedule_sim testcase1.txt FCFS
     * ./schedule_sim testcase1.txt SJF
     * ./schedule_sim testcase1.txt RR 5
     */
    if (argc < 3) {

        printf("Usage:\n");
        printf("  %s input_file FCFS\n", argv[0]);
        printf("  %s input_file SJF\n", argv[0]);
        printf("  %s input_file RR time_quantum\n", argv[0]);

        return EXIT_FAILURE;
    }

    process_count =
        read_input_file(argv[1], processes);

    if (strcmp(argv[2], "FCFS") == 0) {

        simulate_fcfs(processes,
                      process_count);
    }
    else if (strcmp(argv[2], "SJF") == 0) {

        simulate_sjf(processes,
                     process_count);
    }
    else if (strcmp(argv[2], "RR") == 0) {

        if (argc < 4) {
            printf("Error: RR requires a time quantum.\n");
            printf("Example: %s testcase1.txt RR 5\n",
                   argv[0]);

            return EXIT_FAILURE;
        }

        quantum = atoi(argv[3]);

        if (quantum <= 0) {
            printf("Error: Time quantum must be a positive integer.\n");
            return EXIT_FAILURE;
        }

        simulate_rr(processes,
                    process_count,
                    quantum);
    }
    else {

        printf("Error: Unknown scheduling algorithm '%s'\n",
               argv[2]);

        printf("Valid options are FCFS, SJF, or RR.\n");

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}