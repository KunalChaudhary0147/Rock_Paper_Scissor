// game_mode_logic.c
// Full implementations for game modes moved from main.c
#include <stdio.h>
#include <stdlib.h>

// Page constants (match values used elsewhere)
#define PAGE_LOGIN 0
#define PAGE_REGISTER 1
#define PAGE_GAME_MODES 2
#define PAGE_PLAYING 3
#define PAGE_STATS 4
#define PAGE_RULES 5
#define PAGE_GUEST 6

// Forward declarations for functions provided by other modules (no headers used)
typedef struct User User;
extern void clear_screen();
extern void initialize_game();
extern int play_round(int player_choice);
extern void show_score();
extern void update_user_stats(int result);
extern void navigate_to(int page);
extern void show_navigation_header();
extern void show_rules();
extern void show_user_stats();
extern User* get_current_user();

void show_game_options() {
    printf("\nChoose your move:\n");
    printf("1. Rock\n");
    printf("2. Paper\n");
    printf("3. Scissors\n");
    printf("4. Return to menu\n");
    printf("Enter your choice (1-4): ");
}

void show_game_modes() {
    show_navigation_header();
    printf("\n1. Local Multiplayer\n");
    printf("2. VS Computer\n");
    printf("3. View Statistics\n");
    printf("4. Game Rules\n");
    printf("5. Back to Menu\n");
    printf("Enter your choice (1-5): ");
}

void show_vs_computer_menu() {
    show_navigation_header();
    printf("\n--- VS COMPUTER ---\n");
    printf("1. Quick Match (Best of 3)\n");
    printf("2. Survival Mode (Play until you lose)\n");
    printf("3. Custom Match (Choose number of rounds)\n");
    printf("4. Back to Menu\n");
    printf("Enter your choice (1-4): ");
}

void show_local_multiplayer_menu() {
    show_navigation_header();
    printf("\n--- LOCAL MULTIPLAYER ---\n");
    printf("1. Quick Match (Best of 3)\n");
    printf("2. Custom Match (Choose number of rounds)\n");
    printf("3. Back to Menu\n");
    printf("Enter your choice (1-3): ");
}

void play_best_of_three() {
    int player_wins = 0, computer_wins = 0;
    int rounds_played = 0;
    initialize_game();
    
    while (rounds_played < 3 && player_wins < 2 && computer_wins < 2) {
        clear_screen();
        printf("\nBest of 3 - Round %d\n", rounds_played + 1);
        printf("Player wins: %d | Computer wins: %d\n\n", player_wins, computer_wins);
        show_game_options();
        
        int choice;
        scanf("%d", &choice);
        
        if (choice >= 1 && choice <= 3) {
            int result = play_round(choice);
            show_score();
            update_user_stats(result);
            rounds_played++;
            if (result == 1) player_wins++;
            else if (result == -1) computer_wins++;
            printf("\nPress Enter to continue...");
            getchar();
            getchar();
        } else {
            printf("Invalid choice! Press Enter to try again...");
            getchar();
            getchar();
        }
    }
    
    clear_screen();
    printf("\nGame Over!\n");
    printf("Final Score - Player: %d | Computer: %d\n", player_wins, computer_wins);
    if (player_wins > computer_wins) printf("You won the match!\n");
    else printf("Computer won the match!\n");
    printf("\nPress Enter to continue...");
    getchar();
}

void play_survival_mode() {
    int streak = 0;
    initialize_game();
    
    while (1) {
        clear_screen();
        printf("\nSurvival Mode - Current Streak: %d\n", streak);
        show_game_options();
        
        int choice;
        scanf("%d", &choice);
        
        if (choice >= 1 && choice <= 3) {
            int result = play_round(choice);
            show_score();
            update_user_stats(result);
            
            if (result == 1) {
                streak++;
                printf("\nStreak continues! Current streak: %d", streak);
            } else {
                printf("\nGame Over! Final streak: %d", streak);
                break;
            }
            printf("\nPress Enter to continue...");
            getchar();
            getchar();
        } else if (choice == 4) {
            break;
        } else {
            printf("Invalid choice! Press Enter to try again...");
            getchar();
            getchar();
        }
    }
}

void play_custom_match() {
    int rounds;
    printf("\nEnter number of rounds (1-10): ");
    scanf("%d", &rounds);

    if (rounds < 1 || rounds > 10) {
        printf("Invalid number of rounds! Using 3 rounds instead.\n");
        rounds = 3;
    }

    int player_wins = 0, computer_wins = 0;
    initialize_game();

    for (int i = 0; i < rounds; i++) {
        clear_screen();
        printf("\nRound %d/%d\n", i + 1, rounds);
        printf("Player wins: %d | Computer wins: %d\n\n", player_wins, computer_wins);
        show_game_options();

        int choice;
        scanf("%d", &choice);

        if (choice >= 1 && choice <= 3) {
            int result = play_round(choice);
            show_score();
            update_user_stats(result);
            if (result == 1) player_wins++;
            else if (result == -1) computer_wins++;
            printf("\nPress Enter to continue...");
            getchar();
            getchar();
        } else {
            printf("Invalid choice! Press Enter to try again...");
            getchar();
            getchar();
            i--; // Repeat this round
        }
    }

    clear_screen();
    printf("\nGame Over!\n");
    printf("Final Score - Player: %d | Computer: %d\n", player_wins, computer_wins);
    if (player_wins > computer_wins) printf("You won the match!\n");
    else if (player_wins < computer_wins) printf("Computer won the match!\n");
    else printf("It's a tie!\n");
    printf("\nPress Enter to continue...");
    getchar();
}

int get_player_choice_local(const char* player_name) {
    printf("\n%s, choose your move:\n", player_name);
    printf("1. Rock\n");
    printf("2. Paper\n");
    printf("3. Scissors\n");
    printf("Enter your choice (1-3): ");
    int choice;
    scanf("%d", &choice);
    getchar();
    return choice;
}

void show_local_result(const char* p1_name, int p1_choice, const char* p2_name, int p2_choice, int p1_wins, int p2_wins) {
    printf("\n%s's choice: ", p1_name);
    switch(p1_choice) {
        case 1: printf("Rock"); break;
        case 2: printf("Paper"); break;
        case 3: printf("Scissors"); break;
    }

    printf("\n%s's choice: ", p2_name);
    switch(p2_choice) {
        case 1: printf("Rock"); break;
        case 2: printf("Paper"); break;
        case 3: printf("Scissors"); break;
    }
    printf("\n");

    if (p1_choice == p2_choice) {
        printf("It's a draw!\n");
    } else if ((p1_choice == 1 && p2_choice == 3) ||
               (p1_choice == 2 && p2_choice == 1) ||
               (p1_choice == 3 && p2_choice == 2)) {
        printf("%s wins this round!\n", p1_name);
    } else {
        printf("%s wins this round!\n", p2_name);
    }

    printf("\nScore - %s: %d | %s: %d\n", p1_name, p1_wins, p2_name, p2_wins);
}

void play_local_best_of_three(const char* p1_name, const char* p2_name) {
    int p1_wins = 0, p2_wins = 0;
    int rounds_played = 0;

    while (rounds_played < 3 && p1_wins < 2 && p2_wins < 2) {
        clear_screen();
        printf("\nBest of 3 - Round %d\n", rounds_played + 1);
        printf("%s wins: %d | %s wins: %d\n\n", p1_name, p1_wins, p2_name, p2_wins);

        int p1_choice = get_player_choice_local(p1_name);

        if (p1_choice >= 1 && p1_choice <= 3) {
            clear_screen();
            printf("\nBest of 3 - Round %d\n", rounds_played + 1);
            printf("%s wins: %d | %s wins: %d\n\n", p1_name, p1_wins, p2_name, p2_wins);

            int p2_choice = get_player_choice_local(p2_name);

            if (p2_choice >= 1 && p2_choice <= 3) {
                if (p1_choice == p2_choice) {
                    // Draw
                } else if ((p1_choice == 1 && p2_choice == 3) ||
                          (p1_choice == 2 && p2_choice == 1) ||
                          (p1_choice == 3 && p2_choice == 2)) {
                    p1_wins++;
                } else {
                    p2_wins++;
                }

                show_local_result(p1_name, p1_choice, p2_name, p2_choice, p1_wins, p2_wins);
                rounds_played++;
                printf("\nPress Enter to continue...");
                getchar();
            } else {
                printf("Invalid choice! Press Enter to try again...");
                getchar();
            }
        } else {
            printf("Invalid choice! Press Enter to try again...");
            getchar();
        }
    }

    clear_screen();
    printf("\nGame Over!\n");
    printf("Final Score - %s: %d | %s: %d\n", p1_name, p1_wins, p2_name, p2_wins);
    if (p1_wins > p2_wins) printf("%s won the match!\n", p1_name);
    else printf("%s won the match!\n", p2_name);
    printf("\nPress Enter to continue...");
    getchar();
}

void play_local_custom_match(const char* p1_name, const char* p2_name) {
    int rounds;
    printf("\nEnter number of rounds (1-10): ");
    scanf("%d", &rounds);
    getchar();

    if (rounds < 1 || rounds > 10) {
        printf("Invalid number of rounds! Using 3 rounds instead.\n");
        rounds = 3;
    }

    int p1_wins = 0, p2_wins = 0;

    for (int i = 0; i < rounds; i++) {
        clear_screen();
        printf("\nRound %d/%d\n", i + 1, rounds);
        printf("%s wins: %d | %s wins: %d\n\n", p1_name, p1_wins, p2_name, p2_wins);

        int p1_choice = get_player_choice_local(p1_name);

        if (p1_choice >= 1 && p1_choice <= 3) {
            clear_screen();
            printf("\nRound %d/%d\n", i + 1, rounds);
            printf("%s wins: %d | %s wins: %d\n\n", p1_name, p1_wins, p2_name, p2_wins);

            int p2_choice = get_player_choice_local(p2_name);

            if (p2_choice >= 1 && p2_choice <= 3) {
                if (p1_choice == p2_choice) {
                    // Draw
                } else if ((p1_choice == 1 && p2_choice == 3) ||
                          (p1_choice == 2 && p2_choice == 1) ||
                          (p1_choice == 3 && p2_choice == 2)) {
                    p1_wins++;
                } else {
                    p2_wins++;
                }

                show_local_result(p1_name, p1_choice, p2_name, p2_choice, p1_wins, p2_wins);
                printf("\nPress Enter to continue...");
                getchar();
            } else {
                printf("Invalid choice! Press Enter to try again...");
                getchar();
                i--;
            }
        } else {
            printf("Invalid choice! Press Enter to try again...");
            getchar();
            i--;
        }
    }

    clear_screen();
    printf("\nGame Over!\n");
    printf("Final Score - %s: %d | %s: %d\n", p1_name, p1_wins, p2_name, p2_wins);
    if (p1_wins > p2_wins) printf("%s won the match!\n", p1_name);
    else if (p1_wins < p2_wins) printf("%s won the match!\n", p2_name);
    else printf("It's a tie!\n");
    printf("\nPress Enter to continue...");
    getchar();
}

void handle_vs_computer_mode() {
    int playing = 1;

    while (playing) {
        clear_screen();
        show_vs_computer_menu();

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                navigate_to(PAGE_PLAYING);
                play_best_of_three();
                navigate_to(PAGE_GAME_MODES);
                break;
            case 2:
                if (get_current_user() != NULL) {
                    navigate_to(PAGE_PLAYING);
                    play_survival_mode();
                    navigate_to(PAGE_GAME_MODES);
                } else {
                    printf("\nSurvival mode is only available for registered users!\n");
                    printf("Please login or register to access this mode.\n");
                    printf("\nPress Enter to continue...");
                    getchar();
                    getchar();
                }
                break;
            case 3:
                if (get_current_user() != NULL) {
                    navigate_to(PAGE_PLAYING);
                    play_custom_match();
                    navigate_to(PAGE_GAME_MODES);
                } else {
                    printf("\nCustom matches are only available for registered users!\n");
                    printf("Please login or register to access this mode.\n");
                    printf("\nPress Enter to continue...");
                    getchar();
                    getchar();
                }
                break;
            case 4:
                playing = 0;
                break;
            default:
                printf("Invalid choice! Press Enter to try again...");
                getchar();
                getchar();
        }
    }
}

void handle_local_multiplayer_mode() {
    char player1_name[50], player2_name[50];

    clear_screen();
    printf("\n--- LOCAL MULTIPLAYER SETUP ---\n");
    printf("Enter Player 1 name: ");
    scanf("%s", player1_name);
    getchar();

    printf("Enter Player 2 name: ");
    scanf("%s", player2_name);
    getchar();

    printf("\nPress Enter to continue...");
    getchar();

    int playing = 1;

    while (playing) {
        clear_screen();
        show_local_multiplayer_menu();

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                navigate_to(PAGE_PLAYING);
                play_local_best_of_three(player1_name, player2_name);
                navigate_to(PAGE_GAME_MODES);
                break;
            case 2:
                navigate_to(PAGE_PLAYING);
                play_local_custom_match(player1_name, player2_name);
                navigate_to(PAGE_GAME_MODES);
                break;
            case 3:
                playing = 0;
                break;
            default:
                printf("Invalid choice! Press Enter to try again...");
                getchar();
        }
    }
}

void play_game() {
    int playing = 1;

    while (playing) {
        clear_screen();
        show_game_modes();

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
                navigate_to(PAGE_STATS);
                show_user_stats();
                printf("\nPress Enter to return...");
                getchar();
                getchar();
                navigate_to(PAGE_GAME_MODES);
                break;
            case 4:
                navigate_to(PAGE_RULES);
                show_rules();
                navigate_to(PAGE_GAME_MODES);
                break;
            case 5:
                navigate_to(PAGE_LOGIN);
                playing = 0;
                break;
            default:
                printf("Invalid choice! Press Enter to try again...");
                getchar();
                getchar();
        }
    }
}
