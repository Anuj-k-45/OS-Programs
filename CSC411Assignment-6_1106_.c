#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

typedef struct {
    int account_id;
    int balance;
} Account;

typedef struct {
    int transaction_no;
    int account_id;
    char type[10];
    int amount;
    int resulting_balance;
} Transaction;

typedef struct {
    Account accounts[5];
    Transaction transactions[20];
    int transaction_count;
} Shared_Memory;

void create_accounts(Shared_Memory *data) {
    srand(time(NULL));
    for (int i = 0; i < 5; i++) {
        data->accounts[i].account_id = 1000 + i;
        data->accounts[i].balance = 100 + rand() % 1000;
        printf("Created ---- > Account No : %d with Initial Balance : %d\n",data->accounts[i].account_id,data->accounts[i].balance);
    }
    data->transaction_count = 0;
}

void deposit(Shared_Memory *data) {
    srand(time(NULL) + getpid());
    for (int i = 0; i < 10; i++) {
        int idx = rand() % 5;
        int amount = 1 + rand() % 200;
        data->accounts[idx].balance += amount;
        
        Transaction *t = &data->transactions[data->transaction_count++];
        t->transaction_no = data->transaction_count;
        t->account_id = data->accounts[idx].account_id;
        strcpy(t->type, "Deposit");
        t->amount = amount;
        t->resulting_balance = data->accounts[idx].balance;
    }
}

void withdraw(Shared_Memory *data) {
    srand(time(NULL) + getpid());
    for (int i = 0; i < 10; i++) {
        int idx = rand() % 5;
        int amount = 1 + rand() % 200;
        
        Transaction *t = &data->transactions[data->transaction_count++];
        t->transaction_no = data->transaction_count;
        t->account_id = data->accounts[idx].account_id;
        strcpy(t->type, "Withdraw");
        t->amount = amount;

        if (data->accounts[idx].balance >= amount) {
            data->accounts[idx].balance -= amount;
            t->resulting_balance = data->accounts[idx].balance;
        } else {
            printf("Transaction declined for account %d: Insufficient funds.\n", data->accounts[idx].account_id);
            t->resulting_balance = data->accounts[idx].balance;
        }
    }
}

void print_transactions(Shared_Memory *data) {
    printf("\nTransaction History:\n");
    for (int i = 0; i < data->transaction_count; i++) {
        printf("Transaction %d ---> Account No : %d ---> Performed : %s ---> Amount : %d ---> Current Balance : %d\n", 
               data->transactions[i].transaction_no,
               data->transactions[i].account_id,
               data->transactions[i].type,
               data->transactions[i].amount,
               data->transactions[i].resulting_balance);
    }
}

void clear_transaction_history(Shared_Memory *data) {
    data->transaction_count = 0;
    printf("Transaction history cleared.\n");
}

int main() {
    int shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    Shared_Memory *data = (Shared_Memory *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    create_accounts(data);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        withdraw(data);
        shmdt(data);
        exit(0);
    } else {
        deposit(data);
        wait(NULL);
        print_transactions(data);
        
        int choice;
        printf("\nEnter 1 to clear transaction history, 0 to keep: ");
        scanf("%d", &choice);
        if (choice == 1) {
            clear_transaction_history(data);
        }
        
        shmdt(data);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}