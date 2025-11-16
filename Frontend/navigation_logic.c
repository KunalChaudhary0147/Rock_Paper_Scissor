#include <stdio.h>

// Page definitions
#define PAGE_LOGIN 0
#define PAGE_REGISTER 1
#define PAGE_GAME_MODES 2
#define PAGE_PLAYING 3
#define PAGE_STATS 4
#define PAGE_RULES 5
#define PAGE_GUEST 6

static int current_page = PAGE_LOGIN;
static int previous_page = PAGE_LOGIN;

void navigate_to(int page) {
    previous_page = current_page;
    current_page = page;
}

int get_current_page() {
    return current_page;
}

int get_previous_page() {
    return previous_page;
}

void go_back() {
    int temp = current_page;
    current_page = previous_page;
    previous_page = temp;
}

const char* get_page_name(int page) {
    switch(page) {
        case PAGE_LOGIN: return "Login";
        case PAGE_REGISTER: return "Register";
        case PAGE_GAME_MODES: return "Game Modes";
        case PAGE_PLAYING: return "In Game";
        case PAGE_STATS: return "Statistics";
        case PAGE_RULES: return "Rules";
        case PAGE_GUEST: return "Guest Mode";
        default: return "Unknown";
    }
}

void show_navigation_header() {
    printf("\n+----------------------------------------+\n");
    printf("|          Current Page: %-16s|\n", get_page_name(current_page));
    printf("+----------------------------------------+\n");
}
