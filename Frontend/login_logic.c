#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_GAMES_BEFORE_LOGIN 2
#define LOGIN_FILE "Data/users.txt"
#define STATS_FILE "Data/statistics.txt"

typedef struct {
    char username[50];
    char password[50];
    int games_played;
    int wins;
    int losses;
    int draws;
} User;

static User users[MAX_USERS];
static int num_users = 0;
static User* current_user = NULL;
static int games_played_without_login = 0;

// Load users from file when program starts
void load_users() {
    FILE* file = fopen(LOGIN_FILE, "r");
    if (file == NULL) return;

    while (num_users < MAX_USERS && 
           fscanf(file, "%s %s %d %d %d %d\n", 
                  users[num_users].username,
                  users[num_users].password,
                  &users[num_users].games_played,
                  &users[num_users].wins,
                  &users[num_users].losses,
                  &users[num_users].draws) == 6) {
        num_users++;
    }
    fclose(file);
}

// Save users to file
void save_users() {
    FILE* file = fopen(LOGIN_FILE, "w");
    if (file == NULL) return;

    for (int i = 0; i < num_users; i++) {
        fprintf(file, "%s %s %d %d %d %d\n",
                users[i].username,
                users[i].password,
                users[i].games_played,
                users[i].wins,
                users[i].losses,
                users[i].draws);
    }
    fclose(file);
}

// Log game result to statistics file
void log_game_result(const char* username, int result) {
    FILE* file = fopen(STATS_FILE, "a");
    if (file == NULL) return;

    time_t now;
    time(&now);
    char* date = ctime(&now);
    date[strlen(date)-1] = '\0'; // Remove newline

    char* outcome;
    switch(result) {
        case 1: outcome = "WIN"; break;
        case -1: outcome = "LOSS"; break;
        default: outcome = "DRAW";
    }

    fprintf(file, "[%s] User: %s, Result: %s\n", date, username, outcome);
    fclose(file);
}

int can_play_without_login() {
    return games_played_without_login < MAX_GAMES_BEFORE_LOGIN;
}

void increment_games_without_login() {
    games_played_without_login++;
}

int register_user(const char* username, const char* password) {
    // Check if username already exists
    for(int i = 0; i < num_users; i++) {
        if(strcmp(users[i].username, username) == 0) {
            return 0; // Username already exists
        }
    }

    // Register new user
    if(num_users < MAX_USERS) {
        strcpy(users[num_users].username, username);
        strcpy(users[num_users].password, password);
        users[num_users].games_played = 0;
        users[num_users].wins = 0;
        users[num_users].losses = 0;
        users[num_users].draws = 0;
        num_users++;
        save_users();
        return 1; // Success
    }
    return -1; // User limit reached
}

int login(const char* username, const char* password) {
    for(int i = 0; i < num_users; i++) {
        if(strcmp(users[i].username, username) == 0 &&
           strcmp(users[i].password, password) == 0) {
            current_user = &users[i];
            return 1; // Success
        }
    }
    return 0; // Failed
}

User* get_current_user() {
    return current_user;
}

void update_user_stats(int result) {
    if(current_user != NULL) {
        current_user->games_played++;
        if(result == 1) current_user->wins++;
        else if(result == -1) current_user->losses++;
        else current_user->draws++;
        
        // Save updated stats to files
        save_users();
        log_game_result(current_user->username, result);
    }
}

void show_user_stats() {
    if(current_user != NULL) {
        printf("\nUser Statistics for %s:\n", current_user->username);
        printf("Games Played: %d\n", current_user->games_played);
        printf("Wins: %d\n", current_user->wins);
        printf("Losses: %d\n", current_user->losses);
        printf("Draws: %d\n", current_user->draws);
        if(current_user->games_played > 0) {
            float win_rate = (float)current_user->wins / current_user->games_played * 100;
            printf("Win Rate: %.1f%%\n", win_rate);
        }

        // Show recent game history
        printf("\nRecent Game History:\n");
        FILE* file = fopen(STATS_FILE, "r");
        if (file != NULL) {
            char line[256];
            int count = 0;
            // Create a temporary array to store last 5 games
            char last_games[5][256];
            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, current_user->username)) {
                    strcpy(last_games[count % 5], line);
                    count++;
                }
            }
            // Print last 5 games (or less if fewer games played)
            int games_to_show = count < 5 ? count : 5;
            for (int i = 0; i < games_to_show; i++) {
                printf("%s", last_games[(count - games_to_show + i) % 5]);
            }
            fclose(file);
        }
    }
}
