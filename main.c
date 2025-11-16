#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototypes for functions defined in other source files (no headers)
void initialize_game();
int play_round(int player_choice);
void show_score();
int get_player_score();
int get_computer_score();
int get_computer_score();

// Function declarations from login_logic.c
int register_user(const char* username, const char* password);
int login(const char* username, const char* password);
int can_play_without_login();
void increment_games_without_login();
void update_user_stats(int result);
void show_user_stats();
typedef struct {
    char username[50];
    char password[50];
    int games_played;
    int wins;
    int losses;
    int draws;
} User;
User* get_current_user();

void clear_screen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

#define PAGE_LOGIN 0
#define PAGE_REGISTER 1
#define PAGE_GAME_MODES 2
#define PAGE_PLAYING 3
#define PAGE_STATS 4
#define PAGE_RULES 5
#define PAGE_GUEST 6

// Add these to your function declarations at the top
void navigate_to(int page);
int get_current_page();
void show_navigation_header();
void go_back();

// Track guest games across all functions
static int guest_games_completed = 0;

void show_login_menu() {
    
    show_navigation_header();
    printf("\n1. Login\n");
    printf("2. Register\n");
    if (guest_games_completed < 2) {
        printf("3. Play as Guest\n");
        printf("4. Exit\n");
        printf("Enter your choice (1-4): ");
    } else {
        printf("3. Exit\n");
        printf("Enter your choice (1-3): ");
    }
}

void show_main_menu() {
    printf("\nROCK PAPER SCISSORS GAME - MAIN MENU\n");
    printf("1. Local Multiplayer\n");
    printf("2. VS Computer\n");
    printf("3. View Rules\n");
    printf("4. View Statistics\n");
    printf("5. Logout\n");
    printf("6. Exit\n");
    printf("Enter your choice (1-6): ");
}

void show_guest_menu() {
    printf("\nROCK PAPER SCISSORS GAME - GUEST MODE\n");
    printf("1. Play vs Computer\n");
    printf("2. View Rules\n");
    printf("3. Login/Register\n");
    printf("4. Exit\n");
    printf("Enter your choice (1-4): ");
}

// Game-mode functions are implemented in Frontend/game_mode_logic.c
void show_game_options();

void show_rules() {
    clear_screen();
    printf("\nRULES OF THE GAME:\n");
    printf("1. Rock crushes Scissors\n");
    printf("2. Scissors cuts Paper\n");
    printf("3. Paper covers Rock\n");
    printf("\nGuest Mode: You can play 2 games before login is required\n");
    printf("\nPress Enter to continue...");
    getchar();
    getchar();
}

void handle_registration() {
    char username[50], password[50];
    
    clear_screen();
    printf("\nREGISTRATION\n");
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    
    if(register_user(username, password)) {
        printf("\nRegistration successful! Please login.\n");
    } else {
        printf("\nRegistration failed. Username might already exist.\n");
    }
    printf("\nPress Enter to continue...");
    getchar();
    getchar();
    navigate_to(PAGE_LOGIN);
    clear_screen();
    show_login_menu();
    return; /* exit guest_mode */
}

void handle_login() {
    char username[50], password[50];

    clear_screen();
    printf("\nLOGIN\n");
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    if(login(username, password)) {
        printf("\nLogin successful! Welcome %s!\n", username);
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
    } else {
        printf("\nLogin failed. Invalid username or password.\n");
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
    }
}

void show_game_modes();

void play_best_of_three();

void play_survival_mode();

void play_custom_match();

void play_game();

void guest_mode() {
    int running = 1;
    static int best_of_three_sets_played = 0;
    
    while (running) {
        clear_screen();
        show_guest_menu();
        
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                if (best_of_three_sets_played < 2) {
                    navigate_to(PAGE_PLAYING);
                    play_best_of_three();
                    best_of_three_sets_played++;
                    if (best_of_three_sets_played >= 2) {
                        printf("\nYou've played your 2 guest games.\n");
                        printf("Please login or register to continue playing!\n");
                        printf("\nPress Enter to continue...");
                        getchar();
                        /* redirect immediately to login menu */
                        navigate_to(PAGE_LOGIN);
                        guest_games_completed = 2; /* Ensure guest option is hidden */
                        clear_screen();
                        show_login_menu();
                        return; /* exit guest_mode */
                    }
                    navigate_to(PAGE_GUEST);
                } else {
                    printf("\nYou've played your 2 guest games.\n");
                    printf("Please login or register to continue playing!\n");
                    printf("\nPress Enter to continue...");
                    /* consume leftover newline and wait for Enter */
                    getchar();
                    navigate_to(PAGE_LOGIN);
                    clear_screen();
                    show_login_menu();
                    return;
                }
                break;
            case 2:
                show_rules();
                break;
            case 3:
                running = 0;  // Return to login menu
                break;
            case 4:
                exit(0);
            default:
                printf("Invalid choice! Press Enter to try again...");
                getchar();
                getchar();
        }
    }
}

// Forward declare the handler functions
void handle_local_multiplayer_mode();
void handle_vs_computer_mode();

void logged_in_mode() {
    int running = 1;

    while (running) {
        clear_screen();
        show_main_menu();

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                handle_local_multiplayer_mode();
                break;
            case 2:
                handle_vs_computer_mode();
                break;
            case 3:
                show_rules();
                break;
            case 4:
                show_user_stats();
                printf("\nPress Enter to continue...");
                getchar();
                getchar();
                break;
            case 5:
                running = 0;  // Logout
                break;
            case 6:
                exit(0);
            default:
                printf("Invalid choice! Press Enter to try again...");
                getchar();
                getchar();
        }
    }
}

// Add this to the function declarations at the top
void load_users();

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    #ifdef _WIN32
    // Set console output to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    // Load saved user data when program starts
    load_users();
    navigate_to(PAGE_LOGIN);
    
    while (1) {
        clear_screen();
        int current_page = get_current_page();
        
        switch(current_page) {
            case PAGE_LOGIN:
                show_login_menu();
                int choice;
                scanf("%d", &choice);
                switch(choice) {
                    case 1:
                        handle_login();
                        if (get_current_user() != NULL) {
                            navigate_to(PAGE_GAME_MODES);
                            logged_in_mode();
                        }
                        break;
                    case 2:
                        navigate_to(PAGE_REGISTER);
                        handle_registration();
                        navigate_to(PAGE_LOGIN);
                        break;
                    case 3:
                        if (guest_games_completed < 2) {
                            navigate_to(PAGE_GUEST);
                            guest_mode();
                        } else {
                            printf("\nThanks for playing! Goodbye!\n");
                            return 0;
                        }
                        break;
                    case 4:
                        printf("\nThanks for playing! Goodbye!\n");
                        return 0;
                }
                break;

            case PAGE_GAME_MODES:
                play_game();
                break;

            case PAGE_GUEST:
                guest_mode();
                break;

            case PAGE_STATS:
                show_user_stats();
                printf("\nPress Enter to return...");
                getchar();
                getchar();
                navigate_to(PAGE_GAME_MODES);
                break;

            case PAGE_RULES:
                show_rules();
                navigate_to(PAGE_GAME_MODES);
                break;
        }
    }
    
    return 0;
}
