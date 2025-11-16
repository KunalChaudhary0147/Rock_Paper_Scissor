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
    printf("\n1. Quick Match (Best of 3)\n");
    printf("2. Survival Mode (Play until you lose)\n");
    printf("3. Custom Match (Choose number of rounds)\n");
    printf("4. View Statistics\n");
    printf("5. Game Rules\n");
    printf("6. Back to Menu\n");
    printf("Enter your choice (1-6): ");
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

void play_game() {
    int playing = 1;
    
    while (playing) {
        clear_screen();
        show_game_modes();
        
        int choice;
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                play_best_of_three();
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
                navigate_to(PAGE_STATS);
                show_user_stats();
                printf("\nPress Enter to return...");
                getchar();
                getchar();
                navigate_to(PAGE_GAME_MODES);
                break;
            case 5:
                navigate_to(PAGE_RULES);
                show_rules();
                navigate_to(PAGE_GAME_MODES);
                break;
            case 6:
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
