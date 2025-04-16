#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char command[1024];
    char timestamp[100];
} CommandStructure;

CommandStructure history[100];
int history_length = 0;

void getTime(char timestamp[100]) {
    time_t now = time(NULL);
    struct tm tm_info = *localtime(&now);
    strftime(timestamp, 100, "%Y-%m-%d %H:%M:%S", &tm_info);
}

void addHistory(char command[1024]) {
    if (history_length < 100) {
        strcpy(history[history_length].command, command);
        getTime(history[history_length].timestamp);
        history_length++;
    } else {
        for (int i = 1; i < 100; i++) {
            history[i - 1] = history[i];
        }
        strcpy(history[100 - 1].command, command);
        getTime(history[100 - 1].timestamp);
    }
}

void showHistory() {
    printf("\nCommand History:\n");
    for (int i = 0; i < history_length; i++) {
        printf("[%s] %s\n", history[i].timestamp, history[i].command);
    }
}

void clearHistory() {
    history_length = 0;
    printf("Command history cleared.\n");
}

void runCommand(char command[1024]) {
    if (strstr(command, "man") == command) {
        system(command);
    } else if (strstr(command, "clear") == command) {
        system("clear");
    } else if (strstr(command, "tput") == command) {
        system(command);
    } else if (strstr(command, "history") == command) {
        showHistory();
    } else if (strstr(command, "who") == command) {
        system("who");
    } else if (strstr(command, "whoami") == command) {
        system("whoami");
    } else if (strstr(command, "id") == command) {
        system("id");
    } else if (strstr(command, "cd") == command) {
        system(command);
    } else if (strstr(command, "pwd") == command) {
        system("pwd");
    } else if (strstr(command, "mkdir") == command) {
        system(command);
    } else if (strstr(command, "rmdir") == command) {
        system(command);
    } else if (strstr(command, "echo") == command) {
        system(command);
    } else if (strstr(command, "printenv") == command) {
        system(command);
    } else if (strstr(command, "set") == command) {
        system(command);
    } else if (strstr(command, "ping") == command) {
        system(command);
    } else if (strstr(command, "netstat") == command) {
        system("netstat");
    } else if (strstr(command, "nslookup") == command) {
        system("nslookup");
    } else if (strstr(command, "top") == command) {
        system("top");
    } else if (strstr(command, "ps") == command) {
        system("ps");
    } else if (strstr(command, "df") == command) {
        system("df");
    } else {
        printf("Invalid command.\n");
    }
}

void saveOutput(char command[1024]) {
    char filename[1024];
    printf("Enter filename to save the output: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    char redirect_command[1024];
    snprintf(redirect_command, sizeof(redirect_command), "%s > %s", command, filename);
    system(redirect_command);
    printf("Output saved to %s\n", filename);
}

void displayMenu() {
    printf("\nMenu:\n");
    printf("1. Shell Commands\n");
    printf("2. User Information Commands\n");
    printf("3. Directory Commands\n");
    printf("4. Environment Variable Commands\n");
    printf("5. Networking Commands\n");
    printf("6. System Status Commands\n");
    printf("7. Show Command History\n");
    printf("8. Clear Command History\n");
    printf("9. Exit\n");
}

int main() {
    char command[1024];
    int choice;

    while (1) {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printf("\nShell Commands:\nman\nclear\ntput\nhistory\n");
                break;
            case 2:
                printf("\nUser Information Commands:\nwho\nwhoami\nid\n");
                break;
            case 3:
                printf("\nDirectory Commands:\ncd\npwd\nmkdir\nrmdir\n");
                break;
            case 4:
                printf("\nEnvironment Variable Commands:\necho\nprintenv\nset\n");
                break;
            case 5:
                printf("\nNetworking Commands:\nping\nnetstat\nnslookup\n");
                break;
            case 6:
                printf("\nSystem Status Commands:\ntop\nps\ndf\n");
                break;
            case 7:
                showHistory();
                break;
            case 8:
                clearHistory();
                break;
            case 9:
                printf("Exiting the shell...\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }

        printf("Enter command to execute: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        addHistory(command);

        runCommand(command);

        char save_output;
        printf("Do you want to save the output to a file? (y/n): ");
        scanf("%c", &save_output);
        getchar();
        if (save_output == 'y' || save_output == 'Y') {
            saveOutput(command);
        }
    }

    return 0;
}
