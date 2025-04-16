#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

char buffer[1024];
pthread_mutex_t lock;

typedef struct {
    char* old_word; 
    char* new_word; 
    char* text; 
    int position; 
    int count;
} ThreadArg;

void load_file_content(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }
    fread(buffer, sizeof(char), 1024, file);
    fclose(file);
}

void display_buffer() {
    printf("\nCurrent buffer content:\n");
    printf("%s\n", buffer);
}

void* insert_text(void* arg) {
    ThreadArg* args = (ThreadArg*)arg;
    pthread_mutex_lock(&lock);

    if (args->position < 0 || args->position > strlen(buffer)) {
        printf("Invalid position for insert\n");
    } else {
        int len = strlen(buffer);
        if (len + strlen(args->text) < 1024) {
            memmove(&buffer[args->position + strlen(args->text)], &buffer[args->position], len - args->position);
            memcpy(&buffer[args->position], args->text, strlen(args->text));
        } else {
            printf("Not enough space to insert the text\n");
        }
    }

    pthread_mutex_unlock(&lock);  
    pthread_exit(NULL);
}

void* delete_text(void* arg) {
    ThreadArg* args = (ThreadArg*)arg;
    pthread_mutex_lock(&lock);  

    if (args->position < 0 || args->position + args->count > strlen(buffer)) {
        printf("Invalid position or count for delete\n");
    } else {
        memmove(&buffer[args->position], &buffer[args->position + args->count], strlen(buffer) - (args->position + args->count) + 1);
    }

    pthread_mutex_unlock(&lock);  
    pthread_exit(NULL);
}

void* alter_text(void* arg) {
    ThreadArg* args = (ThreadArg*)arg;
    pthread_mutex_lock(&lock);

    char* old_word = args->old_word;
    char* new_word = args->new_word;
    int old_len = strlen(old_word);
    int new_len = strlen(new_word);
    int buffer_len = strlen(buffer);

    for (int i = 0; i <= buffer_len - old_len; i++) {
        int match = 1;
        for (int j = 0; j < old_len; j++) {
            if (tolower(buffer[i + j]) != tolower(old_word[j])) {
                match = 0;
                break;
            }
        }

        if (match) {
            if (new_len > old_len) {
                if (buffer_len + (new_len - old_len) >= 1024) {
                    printf("Not enough space to replace the word\n");
                    break;
                }
                memmove(&buffer[i + new_len], &buffer[i + old_len], buffer_len - i - old_len + 1);
            } else if (new_len < old_len) {
                memmove(&buffer[i + new_len], &buffer[i + old_len], buffer_len - i - old_len + 1);
            }

            memcpy(&buffer[i], new_word, new_len);
            break;
        }
    }

    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main() {
    pthread_t insert_thread, delete_thread, alter_thread;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialization failed\n");
        return 1;
    }

    load_file_content("input.txt");
    display_buffer();

    char operation[10];
    while (1) {
        printf("\nEnter an operation (insert/delete/alter/exit): ");
        scanf("%s", operation);

        if (strcmp(operation, "insert") == 0) {
            char text[256];
            int position;
            printf("Enter the text to insert: ");
            getchar(); 
            fgets(text, sizeof(text), stdin);
            text[strcspn(text, "\n")] = 0; 

            printf("Enter the position to insert at: ");
            scanf("%d", &position);

            ThreadArg* args = (ThreadArg*)malloc(sizeof(ThreadArg));
            args->text = text;
            args->position = position;
            args->count = 0;  

            pthread_create(&insert_thread, NULL, insert_text, (void*)args);
            pthread_join(insert_thread, NULL);
        }
        else if (strcmp(operation, "delete") == 0) {
            int position, count;
            printf("Enter the position to delete from: ");
            scanf("%d", &position);
            printf("Enter the number of characters to delete: ");
            scanf("%d", &count);

            ThreadArg* args = (ThreadArg*)malloc(sizeof(ThreadArg));
            args->text = NULL;
            args->position = position;
            args->count = count;

            pthread_create(&delete_thread, NULL, delete_text, (void*)args);
            pthread_join(delete_thread, NULL);
        }
        else if (strcmp(operation, "alter") == 0) {
            char word[256], new_word[256];
            printf("Enter the word to replace: ");
            getchar(); 
            fgets(word, sizeof(word), stdin);
            word[strcspn(word, "\n")] = 0;
        
            printf("Enter the new word: ");
            fgets(new_word, sizeof(new_word), stdin);
            new_word[strcspn(new_word, "\n")] = 0;
        
            ThreadArg* args = (ThreadArg*)malloc(sizeof(ThreadArg));
            args->old_word = strdup(word);
            args->new_word = strdup(new_word);
            args->position = 0;
            args->count = 0;
        
            pthread_create(&alter_thread, NULL, alter_text, (void*)args);
            pthread_join(alter_thread, NULL);
        
            free(args->old_word);
            free(args->new_word);
            free(args);
        }
        else if (strcmp(operation, "exit") == 0) {
            break;
        }
        else {
            printf("Invalid operation.\n");
        }
        
        display_buffer();
    }

    pthread_mutex_destroy(&lock);

    return 0;
}
