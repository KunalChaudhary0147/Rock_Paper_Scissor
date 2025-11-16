#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// 1 = Rock, 2 = Paper, 3 = Scissors
#define ROCK 1
#define PAPER 2
#define SCISSORS 3

static int player_score = 0;
static int computer_score = 0;
static int games_played = 0;

int get_computer_choice() {
    return (rand() % 3) + 1;
}

void initialize_game() {
    srand((unsigned int)time(NULL));
    player_score = 0;
    computer_score = 0;
    games_played = 0;
}

int play_round(int player_choice) {
    int computer_choice = get_computer_choice();
    
    printf("\nYour choice: ");
    switch(player_choice) {
        case ROCK: printf("Rock"); break;
        case PAPER: printf("Paper"); break;
        case SCISSORS: printf("Scissors"); break;
    }
    
    printf("\nComputer's choice: ");
    switch(computer_choice) {
        case ROCK: printf("Rock"); break;
        case PAPER: printf("Paper"); break;
        case SCISSORS: printf("Scissors"); break;
    }
    printf("\n");

    // Draw
    if (player_choice == computer_choice) {
        printf("It's a draw!\n");
        return 0;
    }
    
    // Player wins
    if ((player_choice == ROCK && computer_choice == SCISSORS) ||
        (player_choice == PAPER && computer_choice == ROCK) ||
        (player_choice == SCISSORS && computer_choice == PAPER)) {
        printf("You win this round!\n");
        player_score++;
        return 1;
    }
    
    // Computer wins
    printf("Computer wins this round!\n");
    computer_score++;
    return -1;
}

void show_score() {
    printf("\nScore - You: %d Computer: %d\n", player_score, computer_score);
}

int get_player_score() {
    return player_score;
}

int get_computer_score() {
    return computer_score;
}
