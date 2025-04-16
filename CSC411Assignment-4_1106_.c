#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int find_max(int *arr, int l, int r) {
    int max_val = arr[l];
    for (int i = l + 1; i <= r; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

void process(int *arr, int l, int r) {
    pid_t left_pid, right_pid;
    int statusR, statusL;
    int m = (l + r) / 2;

    if (r - l + 1 < 10) {
        int max_val = find_max(arr, l, r);
        printf("Process ID: %d, Parent Process ID: %d, Max: %d\n", getpid(), getppid(), max_val);
        exit(max_val);
    }

    left_pid = fork();
    if (left_pid == 0) {
        process(arr, l, m);
    }
    
    right_pid = fork();
    if (right_pid == 0) {
        process(arr, m + 1, r);
    }

    waitpid(left_pid, &statusR, 0);
    waitpid(right_pid, &statusL, 0);

    int max_left = WEXITSTATUS(statusR);
    int max_right = WEXITSTATUS(statusL);
    int max_val = (max_left > max_right) ? max_left : max_right;
    
    printf("Process ID: %d, Parent Process ID: %d, Max: %d\n", getpid(), getppid(), max_val);
    exit(max_val);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please provide the array size as ./filename <array_size>\n");
        exit(1);
    }
    
    int n = atoi(argv[1]);

    if (n <= 0) {
        printf("Size cannot be negetive\n");
        exit(1);
    }

    int arr[n];

    srand(time(NULL) + getpid());
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 128;
    }
    
    printf("Array: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    process(arr, 0, n - 1);

    return 0;
}