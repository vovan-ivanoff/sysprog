#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define INITIAL_SIZE 2
#define MAX_LOGIN_LENGTH 10
#define MAX_PIN 100000
#define HASH_CONST 2654435761U

typedef struct
{
    char login[MAX_LOGIN_LENGTH];
    unsigned int pin_hash;
    int sanctions;
} User;

typedef struct
{
    int size;
    int count;
    User *mem;
} Array;

int array_init(Array *arr)
{
    arr->count = 0;
    arr->size = INITIAL_SIZE;
    arr->mem = (User *)malloc(INITIAL_SIZE * sizeof(User));
    if (!arr->mem)
    {
        printf("malloc error");
        return -1;
    }
return 0;
}
int array_add(Array *arr, User *user)
{
    arr->count++;
    if (arr->count > arr->size)
    {
        if (arr->mem)
        {
            arr->mem = (User *)realloc(arr->mem, arr->size * 2 * sizeof(User));
            if (arr->mem == NULL)
            {
                printf("relloc error");
                return -1;
            }
        }
        arr->size *= 2;
    }
    User *newptr = arr->mem + (arr->count - 1);
    memcpy(newptr, user, sizeof(User));
    return 0;
}
User *array_search(Array *arr, char *username)
{
    User *tmp = NULL;
    for (int i = 0; i < arr->count; i++)
    {
        tmp = (arr->mem + i);
        if (strncmp(tmp->login, username, MAX_LOGIN_LENGTH) == 0)
        {
            return tmp;
        }
    }
    return NULL;
}

int validate_string(const char *str)
{
    int count = 0;
    while (*str)
    {
        if (count > 5)
        {
            return 1;
        }
        if (!((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9')))
        {
            return 1;
        }
        str++;
        count++;
    }
    return 0;
}

int save_users_to_file(Array *arr, const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        return -1;
    }
    fwrite(&(arr->count), sizeof(int), 1, file);
    for (int i = 0; i < arr->count; i++)
    {
        User *tmp = arr->mem + i;
        fwrite(tmp, sizeof(User), 1, file);
    }

    fclose(file);
    return 0;
}

int load_users_from_file(Array *arr, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        return -1;
    }
    int user_count;
    fread(&user_count, sizeof(int), 1, file);
    for (int i = 0; i < user_count; i++)
    {
        User tmp;
        fread(&tmp, sizeof(User), 1, file);
        array_add(arr, &tmp);
    }

    fclose(file);
    return 0;
}

int validate_time(struct tm *tm_info)
{
    int year = tm_info->tm_year + 1900;
    int month = tm_info->tm_mon + 1;
    if (__isleap(year) && month == 2 && tm_info->tm_mday > 29)
    {
        return 1;
    }
    else if (!__isleap(year) && month == 2 && tm_info->tm_mday > 28)
    {
        return 1;
    }
    return 0;
}

unsigned int hash_pin(int pin)
{
    return pin * HASH_CONST % (unsigned int)-1;
}
void flush_input()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int register_user(Array *arr)
{
    User new_user;
    int new_pin = 0;
    printf("Enter login: ");
    scanf("%10s", new_user.login);
    if (validate_string(new_user.login))
    {
        printf("Invalid login.\n");
        return -1;
    }
    printf("Enter PIN: ");
    int ret = scanf("%d", &new_pin);
    if (ret == EOF || ret == 0)
    {
        printf("error reading pin!\n");
        return -1;
    }

    if (new_pin < 0 || new_pin >= MAX_PIN)
    {
        printf("pin is greater than 100000.\n");
        return -1;
    }
    if (array_search(arr, new_user.login))
    {
        printf("user woth this username exists");
        return -1;
    }
    new_user.sanctions = -1;
    new_user.pin_hash = hash_pin(new_pin);
    array_add(arr, &new_user);
    printf("User registered successfully.\n");
    if (save_users_to_file(arr, "./users.dat"))
    {
        printf("Failed to save user to file.\n");
        return -1;
    }
    return 0;
}

User *authenticate_user(Array *arr)
{
    char login[MAX_LOGIN_LENGTH];
    int pin;
    printf("Enter login: ");
    scanf("%6s", login);
    printf("Enter PIN: ");
    scanf("%d", &pin);
    User *ret = array_search(arr, login);
    if (!ret)
    {
        printf("Authentication failed.\n");
        return NULL;
    }
    if (ret->pin_hash == hash_pin(pin))
    {
        return ret;
    }

    printf("Authentication failed.\n");
    return NULL;
}

void show_time()
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buffer[9];
    strftime(buffer, 9, "%H:%M:%S", tm_info);
    printf("Current time: %s\n", buffer);
}

void show_date()
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buffer[11];
    strftime(buffer, 11, "%d:%m:%Y", tm_info);
    printf("Current date: %s\n", buffer);
}

void howmuch(char *time_str, char flag)
{
    struct tm tm_info;
    time_t t = time(NULL);
    double diff;

    if (strptime(time_str, "%d:%m:%Y", &tm_info) == NULL)
    {
        printf("Invalid time format.\n");
        return;
    }
    tm_info.tm_sec = 0;
    tm_info.tm_min = 0;
    tm_info.tm_hour = 0;
    if (validate_time(&tm_info))
    {
        printf("Invalid date.\n");
        return;
    };
    diff = difftime(t, mktime(&tm_info));
    if (diff < 0)
    {
        printf("Future time!\n");
        return;
    }

    switch (flag)
    {
    case 's':
        printf("Elapsed time: %.0f seconds\n", diff);
        break;
    case 'm':
        printf("Elapsed time: %.0f minutes\n", diff / 60);
        break;
    case 'h':
        printf("Elapsed time: %.0f hours\n", diff / 3600);
        break;
    case 'y':
        printf("Elapsed time: %.2f years\n", diff / (3600 * 24 * 365.25));
        break;
    default:
        printf("Invalid flag.\n");
        break;
    }
}

void sanctions(Array *arr, char *username, int number)
{
    User *user = array_search(arr, username);
    if (!user)
    {
        printf("User not found");
        return;
    }
    int confirmation = 0;
    printf("Enter confirmation code: ");
    scanf("%d", &confirmation);
    if (confirmation == 12345)
    {
        user->sanctions = number;
        printf("Sanctions applied to user %s.\n", username);
        save_users_to_file(arr, "./users.dat");
    }
    else
    {
        printf("Invalid confirmation code.\n");
    }
    return;
}

int shell(Array *arr, User *user)
{
    int request_count = 0;
    char *line = NULL;
    size_t len = 0;
    char command[256];
    while (1)
    {
        if (user == NULL)
        {
            return 0;
        }
        if (user != NULL && user->sanctions != -1 && request_count >= user->sanctions)
        {
            printf("Request limit reached. Logging out.\n");
            user = NULL;
            return 2;
        }

        printf("shell> ");
        if (getline(&line, &len, stdin) == -1)
        {
            printf("EOF caught. Exiting.\n");
            free(line);
            line = NULL;
            user = NULL;
            return 1;
        }
        line[strcspn(line, "\n")] = '\0'; // Remove trailing newline
        strncpy(command, line, sizeof(command) - 1);
        command[sizeof(command) - 1] = '\0'; // Ensure null-termination
        free(line);
        line = NULL;

        if (strcmp(command, "Time") == 0)
        {
            show_time();
        }
        else if (strcmp(command, "Date") == 0)
        {
            show_date();
        }
        else if (strncmp(command, "Howmuch", 7) == 0)
        {
            char date_str[40];
            char flag;
            if (sscanf(command + 7, "%20s -%c", date_str, &flag) < 2)
            {
                printf("Invalid arguments.\n");
                continue;
            };
            howmuch(date_str, flag);
        }
        else if (strcmp(command, "Logout") == 0)
        {
            user = NULL;
        }
        else if (strncmp(command, "Sanctions", 9) == 0)
        {
            char username[MAX_LOGIN_LENGTH];
            int number;
            if (sscanf(command + 9, "%7s %d", username, &number) < 2)
            {
                printf("Invalid arguments.\n");
                continue;
            };
            sanctions(arr, username, number);
            flush_input();
        }
        else if (strcmp(command, "Help") == 0)
        {
            printf("Available commands:\n");
            printf("Time - prints surrent time\n");
            printf("Date - prints current date\n");
            printf("Howmuch DD:MM:YYYY -[s|m|h|y] - prints how much time is elapsed\n");
            printf("Logout - logs user out\n");
            printf("Sanctions <username> <number> - applies sanctins to specified user\n");
            printf("Help - prints this help\n");
        }
        else
        {
            printf("Unknown command.\n");
        }

        request_count++;
    }
    return 0;
}

int main()
{
    Array arr;
    array_init(&arr);
    if (load_users_from_file(&arr, "./users.dat"))
    {
        printf("Users file not found. starting from scratch!\n");
    }
    int flag = 0;
    while (1)
    {
        if (flag == 1) // EOF catched in stdin
        {
            break;
        }
        printf("1. Register\n2. Login\n3. Exit\n");
        int choice = 0;
        char *line = NULL;
        size_t len = 0;

        if (getline(&line, &len, stdin) == -1)
        {
            choice = 3;
        }
        else if (strlen(line) == 2 && line[0] >= '1' && line[0] <= '3' && line[1] == '\n')
        {
            choice = line[0] - '0';
        }
        else
        {
            choice = -1;
        }

        free(line);

        switch (choice)
        {
        case 1:
            register_user(&arr);
            flush_input();
            break;
        case 2:
        {
            User *usr = authenticate_user(&arr);
            if (usr)
            {
                flush_input();
                flag = shell(&arr, usr);
            }
            break;
        }
        case 3:
            return 0;
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
    }
    free(arr.mem);
    return 0;
}