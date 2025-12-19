#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define MAX_USERS 100
#define LOGIN_FILE "Data/users.txt"
#define STATS_FILE "Data/statistics.txt"
#define ROCK 1
#define PAPER 2
#define SCISSORS 3

typedef struct {
    char username[50];
    char password[50];
    int games_played;
    int wins;
    int losses;
    int draws;
} User;

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *main_stack;
    User users[MAX_USERS];
    int num_users;
    User *current_user;
    int guest_games;
    int player_score;
    int computer_score;
    int games_in_match;
    int player_choice;
    int last_computer_choice;
    gboolean show_password;
    GtkWidget *user_button;
    GtkWidget *start_page_container;
    int player_rounds_won;
    int computer_rounds_won;
    int current_round;
    char previous_page[50];
    char current_game_mode[50];
    int survival_streak;
    int custom_rounds_total;
    char player1_name[50];
    char player2_name[50];
    int is_local_multiplayer;
    GtkWidget *guest_menu_title;
    int player1_choice;
    int player2_choice;
    int local_game_step;
    int p1_rounds_won;
    int p2_rounds_won;
} AppData;

AppData app_data = {0};
GtkWidget *vs_survival_btn = NULL;
GtkWidget *vs_custom_btn = NULL;

void load_users() {
    FILE *file = fopen(LOGIN_FILE, "r");
    if (file == NULL) return;

    app_data.num_users = 0;
    while (app_data.num_users < MAX_USERS &&
           fscanf(file, "%s %s %d %d %d %d\n",
                  app_data.users[app_data.num_users].username,
                  app_data.users[app_data.num_users].password,
                  &app_data.users[app_data.num_users].games_played,
                  &app_data.users[app_data.num_users].wins,
                  &app_data.users[app_data.num_users].losses,
                  &app_data.users[app_data.num_users].draws) == 6) {
        app_data.num_users++;
    }
    fclose(file);
}

void save_users() {
    FILE *file = fopen(LOGIN_FILE, "w");
    if (file == NULL) return;

    for (int i = 0; i < app_data.num_users; i++) {
        fprintf(file, "%s %s %d %d %d %d\n",
                app_data.users[i].username,
                app_data.users[i].password,
                app_data.users[i].games_played,
                app_data.users[i].wins,
                app_data.users[i].losses,
                app_data.users[i].draws);
    }
    fclose(file);
}

int register_user(const char *username, const char *password) {
    for (int i = 0; i < app_data.num_users; i++) {
        if (strcmp(app_data.users[i].username, username) == 0) {
            return 0;
        }
    }

    if (app_data.num_users < MAX_USERS) {
        strcpy(app_data.users[app_data.num_users].username, username);
        strcpy(app_data.users[app_data.num_users].password, password);
        app_data.users[app_data.num_users].games_played = 0;
        app_data.users[app_data.num_users].wins = 0;
        app_data.users[app_data.num_users].losses = 0;
        app_data.users[app_data.num_users].draws = 0;
        app_data.num_users++;
        save_users();
        return 1;
    }
    return -1;
}

int validate_password(const char *password) {
    int length = strlen(password);
    gboolean has_upper = FALSE, has_lower = FALSE, has_digit = FALSE, has_special = FALSE;

    if (length < 8) return 0;

    for (int i = 0; i < length; i++) {
        if (isupper(password[i])) has_upper = TRUE;
        if (islower(password[i])) has_lower = TRUE;
        if (isdigit(password[i])) has_digit = TRUE;
        if (!isalnum(password[i])) has_special = TRUE;
    }

    return has_upper && has_lower && has_digit && has_special;
}

int login(const char *username, const char *password) {
    for (int i = 0; i < app_data.num_users; i++) {
        if (strcmp(app_data.users[i].username, username) == 0 &&
            strcmp(app_data.users[i].password, password) == 0) {
            app_data.current_user = &app_data.users[i];
            return 1;
        }
    }
    return 0;
}

int get_computer_choice() {
    return (rand() % 3) + 1;
}

int play_round(int player_choice) {
    int computer_choice = get_computer_choice();
    app_data.last_computer_choice = computer_choice;

    if (player_choice == computer_choice) {
        return 0;
    }

    if ((player_choice == ROCK && computer_choice == SCISSORS) ||
        (player_choice == PAPER && computer_choice == ROCK) ||
        (player_choice == SCISSORS && computer_choice == PAPER)) {
        app_data.player_score++;
        return 1;
    }

    app_data.computer_score++;
    return -1;
}

const char* get_choice_name(int choice) {
    switch(choice) {
        case ROCK: return "ROCK";
        case PAPER: return "PAPER";
        case SCISSORS: return "SCISSORS";
        default: return "UNKNOWN";
    }
}

void setup_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: #764ba2; }"
        ".header-bar { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }"
        ".start-page { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }"
        ".btn-primary { background-color: #f8f9fa; color: black; border: 1px solid #cccccc; border-radius: 8px; font-weight: bold; font-size: 14px; }"
        ".btn-primary:hover { background-color: #e9ecef; }"
        ".btn-secondary { background-color: #f8f9fa; color: black; border: 1px solid #cccccc; border-radius: 8px; font-weight: bold; font-size: 14px; }"
        ".btn-secondary:hover { background-color: #e9ecef; }"
        ".btn-large { padding: 15px 40px; font-size: 15px; font-weight: bold; }"
        ".btn-game { background-color: #f8f9fa; color: black; border: 1px solid #cccccc; padding: 20px 40px; font-size: 16px; font-weight: bold; border-radius: 8px; }"
        ".btn-game:hover { background-color: #e9ecef; }"
        ".btn-choice { background-color: #f8f9fa; color: black; border: 2px solid #cccccc; padding: 20px 40px; font-size: 16px; font-weight: bold; border-radius: 8px; }"
        ".btn-choice:hover { background-color: #e9ecef; }"
        ".btn-choice-selected { background-color: #667eea; color: white; border: 2px solid #667eea; }"
        ".entry { border-radius: 6px; padding: 12px; border: 2px solid #e0e0e0; }"
        ".entry:focus { border-color: #667eea; }"
        ".error-text { color: #d32f2f; font-weight: bold; font-size: 12px; }"
        ".success-text { color: #388e3c; font-weight: bold; }"
        ".title-main { font-size: 42px; font-weight: bold; color: white; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }"
        ".title { font-size: 32px; font-weight: bold; color: #000000; margin-bottom: 20px; }"
        ".label-title { font-size: 16px; font-weight: bold; color: #000000; }"
        ".label-subtitle { font-size: 15px; color: #000000; font-weight: bold; }"
        ".score-display { font-size: 28px; font-weight: bold; color: #000000; }"
        ".stats-label { color: #000000; font-size: 14px; font-weight: bold; }"
        ".result-label { color: #000000; font-size: 18px; font-weight: bold; }"
        ".round-label { color: #000000; font-size: 20px; font-weight: bold; }"
        ".computer-choice-label { color: #667eea; font-size: 18px; font-weight: bold; }"
        ".card { background-color: white; border-radius: 12px; padding: 24px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }"
        ".user-btn-circular { background-color: #667eea; color: white; border: 2px solid white; font-weight: bold; border-radius: 50%; min-width: 50px; min-height: 50px; font-size: 20px; padding: 0; }"
        ".user-btn-circular:hover { background-color: #5568d3; }"
        ".user-btn { background-color: #f8f9fa; color: black; border: 1px solid #cccccc; font-weight: bold; border-radius: 6px; padding: 10px 20px; }"
        ".user-btn:hover { background-color: #e9ecef; }";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void dialog_response(GtkDialog *dialog, int response_id, gpointer user_data) {
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void show_dialog(GtkWidget *parent, const char *title, const char *message) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title,
                                                     GTK_WINDOW(parent),
                                                     GTK_DIALOG_MODAL,
                                                     "OK",
                                                     GTK_RESPONSE_OK,
                                                     NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *label = gtk_label_new(message);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_container_add(GTK_CONTAINER(content), label);

    g_signal_connect(dialog, "response", G_CALLBACK(dialog_response), NULL);
    gtk_widget_show_all(dialog);
}

GtkWidget *create_header_bar() {
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(header, 20);
    gtk_widget_set_margin_end(header, 20);
    gtk_widget_set_margin_top(header, 15);
    gtk_widget_set_margin_bottom(header, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header-bar");

    GtkWidget *spacer = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(header), spacer, TRUE, TRUE, 0);

    app_data.user_button = gtk_button_new_with_label("👤");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(app_data.user_button),
        "user-btn-circular"
    );

    /* disable by default */
    gtk_widget_set_sensitive(app_data.user_button, FALSE);

    gtk_box_pack_end(GTK_BOX(header), app_data.user_button, FALSE, FALSE, 0);

    return header;
}

void on_user_menu_logout(GtkWidget *button, gpointer user_data) {
    app_data.current_user = NULL;
    app_data.guest_games = 0;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "start");
}

void lambda_callback_with_param(GtkWidget *widget, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), (const char *)user_data);
}

void on_user_menu_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *menu = gtk_menu_new();

    if (app_data.current_user) {
        GtkWidget *account_item = gtk_menu_item_new_with_label("Account");
        GtkWidget *stats_item = gtk_menu_item_new_with_label("View Statistics");
        GtkWidget *logout_item = gtk_menu_item_new_with_label("Logout");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), account_item);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), stats_item);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), logout_item);

        g_signal_connect(account_item, "activate", G_CALLBACK(lambda_callback_with_param), "account");
        g_signal_connect(logout_item, "activate", G_CALLBACK(on_user_menu_logout), NULL);
        g_signal_connect(stats_item, "activate", G_CALLBACK(lambda_callback_with_param), "statistics");

        gtk_widget_show_all(menu);
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    } else {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "login");
    }
}

void on_toggle_password(GtkWidget *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry"));
    app_data.show_password = !app_data.show_password;
    gtk_entry_set_visibility(entry, app_data.show_password);
}

GtkWidget *create_password_field_with_toggle() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_style_context_add_class(gtk_widget_get_style_context(password_entry), "entry");
    gtk_box_pack_start(GTK_BOX(box), password_entry, TRUE, TRUE, 0);

    GtkWidget *toggle_button = gtk_button_new_with_label("👁");
    gtk_widget_set_size_request(toggle_button, 45, -1);
    g_object_set_data(G_OBJECT(toggle_button), "entry", password_entry);
    g_signal_connect(toggle_button, "clicked", G_CALLBACK(on_toggle_password), NULL);
    gtk_box_pack_end(GTK_BOX(box), toggle_button, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(box), "entry", password_entry);
    return box;
}

void on_login_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *username_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "username"));
    GtkEntry *password_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "password"));
    GtkLabel *error_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "error"));

    const char *username = gtk_entry_get_text(username_entry);
    const char *password = gtk_entry_get_text(password_entry);

    if (login(username, password)) {
        gtk_label_set_text(error_label, "");

        /* enable user menu */
        gtk_widget_set_sensitive(app_data.user_button, TRUE);

        /* enable VS COMPUTER modes */
        if (vs_survival_btn)
            gtk_widget_set_sensitive(vs_survival_btn, TRUE);
        if (vs_custom_btn)
            gtk_widget_set_sensitive(vs_custom_btn, TRUE);

        gtk_stack_set_visible_child_name(
            GTK_STACK(app_data.main_stack), "main_menu");
    }

    else {
        gtk_label_set_text(error_label, "Wrong username or password");
    }
}

void on_register_btn_clicked(GtkWidget *button, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "register");
}

void on_login_btn_clicked(GtkWidget *button, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "login");
}

void on_guest_play_clicked(GtkWidget *button, gpointer user_data) {
    if (app_data.guest_games < 2) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "guest_menu");
    }
}

void on_register_submit_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *username_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "username"));
    GtkEntry *password_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "password"));

    const char *username = gtk_entry_get_text(username_entry);
    const char *password = gtk_entry_get_text(password_entry);

    if (strlen(username) < 3) {
        show_dialog(app_data.window, "Error", "Username must be at least 3 characters");
        return;
    }

    if (!validate_password(password)) {
        show_dialog(app_data.window, "Invalid Password",
                   "Password must contain:\n\nAt least 8 characters\n1 uppercase letter\n1 lowercase letter\n1 number\n1 special character");
        return;
    }

    if (register_user(username, password)) {
        show_dialog(app_data.window, "Success", "Registration successful! Please login.");
        gtk_entry_set_text(username_entry, "");
        gtk_entry_set_text(password_entry, "");
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "login");
    } else {
        show_dialog(app_data.window, "Error", "Username already exists");
    }
}

void on_back_clicked(GtkWidget *button, gpointer user_data) {
    const char *page = (const char *)user_data;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), page);
}

void on_account_update_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *username_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "username"));
    GtkEntry *password_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "password"));

    const char *new_username = gtk_entry_get_text(username_entry);
    const char *new_password = gtk_entry_get_text(password_entry);

    if (strlen(new_username) < 3) {
        show_dialog(app_data.window, "Error", "Username must be at least 3 characters");
        return;
    }

    if (strlen(new_password) > 0 && !validate_password(new_password)) {
        show_dialog(app_data.window, "Invalid Password",
                   "Password must contain:\n\nAt least 8 characters\n1 uppercase letter\n1 lowercase letter\n1 number\n1 special character");
        return;
    }

    for (int i = 0; i < app_data.num_users; i++) {
        if (strcmp(app_data.users[i].username, new_username) == 0 &&
            &app_data.users[i] != app_data.current_user) {
            show_dialog(app_data.window, "Error", "Username already exists");
            return;
        }
    }

    strcpy(app_data.current_user->username, new_username);
    if (strlen(new_password) > 0) {
        strcpy(app_data.current_user->password, new_password);
    }

    save_users();
    show_dialog(app_data.window, "Success", "Account updated successfully!");
}

GtkWidget *create_account_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "main_menu");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 450, -1);

    GtkWidget *title = gtk_label_new("Account Settings");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *username_label = gtk_label_new("Username");
    gtk_widget_set_halign(username_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(username_label), "label-title");

    GtkWidget *username_entry = gtk_entry_new();
    if (app_data.current_user) {
        gtk_entry_set_text(GTK_ENTRY(username_entry), app_data.current_user->username);
    }
    gtk_style_context_add_class(gtk_widget_get_style_context(username_entry), "entry");

    GtkWidget *password_label = gtk_label_new("New Password (leave empty to keep current)");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(password_label), "label-title");

    GtkWidget *password_box = create_password_field_with_toggle();

    GtkWidget *update_btn = gtk_button_new_with_label("UPDATE ACCOUNT");
    gtk_style_context_add_class(gtk_widget_get_style_context(update_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(update_btn), "btn-large");
    g_object_set_data(G_OBJECT(update_btn), "username", username_entry);
    g_object_set_data(G_OBJECT(update_btn), "password", GTK_ENTRY(g_object_get_data(G_OBJECT(password_box), "entry")));
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_account_update_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), username_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), password_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), password_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), update_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

GtkWidget *create_start_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_box), "start-page");

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_widget_set_margin_top(content, 80);
    gtk_widget_set_margin_bottom(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);

    GtkWidget *title = gtk_label_new("ROCK PAPER SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title-main");

    GtkWidget *subtitle = gtk_label_new("Test Your Strategy");
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "label-subtitle");

    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_set_homogeneous(GTK_BOX(buttons_box), TRUE);
    gtk_widget_set_margin_top(buttons_box, 40);
    gtk_widget_set_margin_start(buttons_box, 40);
    gtk_widget_set_margin_end(buttons_box, 40);

    GtkWidget *login_btn = gtk_button_new_with_label("LOGIN");
    gtk_style_context_add_class(gtk_widget_get_style_context(login_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(login_btn), "btn-large");
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_btn_clicked), NULL);

    GtkWidget *register_btn = gtk_button_new_with_label("REGISTER");
    gtk_style_context_add_class(gtk_widget_get_style_context(register_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(register_btn), "btn-large");
    g_signal_connect(register_btn, "clicked", G_CALLBACK(on_register_btn_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(buttons_box), login_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons_box), register_btn, TRUE, TRUE, 0);

    GtkWidget *guest_btn = gtk_button_new_with_label("PLAY AS GUEST (2 Matches)");
    gtk_style_context_add_class(gtk_widget_get_style_context(guest_btn), "btn-game");
    gtk_widget_set_margin_top(guest_btn, 30);
    gtk_widget_set_margin_start(guest_btn, 60);
    gtk_widget_set_margin_end(guest_btn, 60);
    g_signal_connect(guest_btn, "clicked", G_CALLBACK(on_guest_play_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), buttons_box, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(content), guest_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    app_data.start_page_container = main_box;
    return main_box;
}

GtkWidget *create_login_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 400, -1);

    GtkWidget *title = gtk_label_new("Welcome Back");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");
    gtk_style_context_add_class(gtk_widget_get_style_context(username_entry), "entry");

    GtkWidget *password_box = create_password_field_with_toggle();

    GtkWidget *error_label = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(error_label), "error-text");

    GtkWidget *login_btn = gtk_button_new_with_label("LOGIN");
    gtk_style_context_add_class(gtk_widget_get_style_context(login_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(login_btn), "btn-large");
    g_object_set_data(G_OBJECT(login_btn), "username", username_entry);
    g_object_set_data(G_OBJECT(login_btn), "password", GTK_ENTRY(g_object_get_data(G_OBJECT(password_box), "entry")));
    g_object_set_data(G_OBJECT(login_btn), "error", error_label);
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_clicked), NULL);

    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-large");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "start");

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), password_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), error_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), login_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), back_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

GtkWidget *create_register_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 40);
    gtk_widget_set_margin_start(content, 60);
    gtk_widget_set_margin_end(content, 60);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 450, -1);

    GtkWidget *title = gtk_label_new("Create Your Account");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *username_label = gtk_label_new("Username (at least 3 characters)");
    gtk_widget_set_halign(username_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(username_label), "label-title");

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Choose your username");
    gtk_style_context_add_class(gtk_widget_get_style_context(username_entry), "entry");

    GtkWidget *password_label = gtk_label_new("Password Requirements");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(password_label), "label-title");

    GtkWidget *password_box = create_password_field_with_toggle();

    const char *requirements = "At least 8 characters\n"
                              "1 uppercase letter (A-Z)\n"
                              "1 lowercase letter (a-z)\n"
                              "1 number (0-9)\n"
                              "1 special character";

    GtkWidget *req_label = gtk_label_new(requirements);
    gtk_label_set_line_wrap(GTK_LABEL(req_label), TRUE);
    gtk_widget_set_halign(req_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(req_label), "label-subtitle");

    GtkWidget *register_btn = gtk_button_new_with_label("CREATE ACCOUNT");
    gtk_style_context_add_class(gtk_widget_get_style_context(register_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(register_btn), "btn-large");
    g_object_set_data(G_OBJECT(register_btn), "username", username_entry);
    g_object_set_data(G_OBJECT(register_btn), "password", GTK_ENTRY(g_object_get_data(G_OBJECT(password_box), "entry")));
    g_signal_connect(register_btn, "clicked", G_CALLBACK(on_register_submit_clicked), NULL);

    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-large");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "start");

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), username_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), password_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), password_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), req_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), register_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), back_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_mode_quick_match_clicked(GtkWidget *button, gpointer user_data) {
    strcpy(app_data.current_game_mode, "Quick Match (Best of 3)");
    app_data.player_score = 0;
    app_data.computer_score = 0;
    app_data.games_in_match = 0;
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;
    app_data.is_local_multiplayer = 0;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_vs_computer");
}

void on_mode_survival_clicked(GtkWidget *button, gpointer user_data) {
    strcpy(app_data.current_game_mode, "Survival Mode");
    app_data.player_score = 0;
    app_data.computer_score = 0;
    app_data.survival_streak = 0;
    app_data.is_local_multiplayer = 0;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_survival");
}

void on_mode_custom_clicked(GtkWidget *button, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "custom_rounds_input");
}

void on_local_multiplayer_clicked(GtkWidget *button, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "local_multiplayer_setup");
}

GtkWidget *create_vs_computer_mode_select_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "main_menu");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 25);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);

    GtkWidget *title = gtk_label_new("Select Game Mode");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *quick_btn = gtk_button_new_with_label("Quick Match (Best of 3)");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_btn), "btn-game");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_btn), "btn-large");
    g_signal_connect(quick_btn, "clicked", G_CALLBACK(on_mode_quick_match_clicked), NULL);

    /* Survival Mode — created always */
    vs_survival_btn = gtk_button_new_with_label(
        "Survival Mode (Play until you lose)"
    );
    gtk_style_context_add_class(
        gtk_widget_get_style_context(vs_survival_btn), "btn-game");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(vs_survival_btn), "btn-large");
    g_signal_connect(
        vs_survival_btn, "clicked",
        G_CALLBACK(on_mode_survival_clicked), NULL);

    /* Custom Match — created always */
    vs_custom_btn = gtk_button_new_with_label(
        "Custom Match (Choose number of rounds)"
    );
    gtk_style_context_add_class(
        gtk_widget_get_style_context(vs_custom_btn), "btn-game");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(vs_custom_btn), "btn-large");
    g_signal_connect(
        vs_custom_btn, "clicked",
        G_CALLBACK(on_mode_custom_clicked), NULL);

    /* guest = disabled */
    gtk_widget_set_sensitive(vs_survival_btn, FALSE);
    gtk_widget_set_sensitive(vs_custom_btn, FALSE);


    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), quick_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), vs_survival_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), vs_custom_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_view_rules_clicked(GtkWidget *button, gpointer user_data) {
    const char *current_page = gtk_stack_get_visible_child_name(GTK_STACK(app_data.main_stack));
    strncpy(app_data.previous_page, current_page, sizeof(app_data.previous_page) - 1);
    app_data.previous_page[sizeof(app_data.previous_page) - 1] = '\0';
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "rules");
}

void on_guest_play_computer_clicked(GtkWidget *button, gpointer user_data) {
    strcpy(app_data.current_game_mode, "Quick Match (Best of 3)");
    app_data.player_score = 0;
    app_data.computer_score = 0;
    app_data.games_in_match = 0;
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;
    app_data.is_local_multiplayer = 0;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_vs_computer");
}

GtkWidget *create_guest_menu_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);

    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-large");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "start");
    gtk_widget_set_halign(back_btn, GTK_ALIGN_START);
    gtk_widget_set_size_request(back_btn, 120, -1);

    char games_left[50];
    snprintf(games_left, sizeof(games_left), "Matches Remaining: %d/2", 2 - app_data.guest_games);

    GtkWidget *title = gtk_label_new(games_left);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");
    app_data.guest_menu_title = title;

    GtkWidget *play_btn = gtk_button_new_with_label("PLAY VS COMPUTER");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_btn), "btn-game");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_btn), "btn-large");
    g_signal_connect(play_btn, "clicked", G_CALLBACK(on_guest_play_computer_clicked), NULL);

    GtkWidget *rules_btn = gtk_button_new_with_label("VIEW RULES");
    gtk_style_context_add_class(gtk_widget_get_style_context(rules_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(rules_btn), "btn-large");
    g_signal_connect(rules_btn, "clicked", G_CALLBACK(on_view_rules_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), play_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), rules_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

GtkWidget *create_main_menu_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);

    char welcome[150];
    if (app_data.current_user) {
        snprintf(welcome, sizeof(welcome), "Welcome, %s!", app_data.current_user->username);
    } else {
        strcpy(welcome, "Main Menu");
    }

    GtkWidget *title = gtk_label_new(welcome);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *play_btn = gtk_button_new_with_label("PLAY VS COMPUTER");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_btn), "btn-game");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_btn), "btn-large");
    g_signal_connect(play_btn, "clicked", G_CALLBACK(lambda_callback_with_param), "vs_computer_mode_select");

    GtkWidget *local_btn = gtk_button_new_with_label("LOCAL MULTIPLAYER");
    gtk_style_context_add_class(gtk_widget_get_style_context(local_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(local_btn), "btn-large");
    g_signal_connect(local_btn, "clicked", G_CALLBACK(on_local_multiplayer_clicked), NULL);

    GtkWidget *rules_btn = gtk_button_new_with_label("VIEW RULES");
    gtk_style_context_add_class(gtk_widget_get_style_context(rules_btn), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(rules_btn), "btn-large");
    g_signal_connect(rules_btn, "clicked", G_CALLBACK(on_view_rules_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), play_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), local_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), rules_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void update_button_selection(GtkWidget *rock_btn, GtkWidget *paper_btn, GtkWidget *scissors_btn, int selected) {
    GtkStyleContext *rock_ctx = gtk_widget_get_style_context(rock_btn);
    GtkStyleContext *paper_ctx = gtk_widget_get_style_context(paper_btn);
    GtkStyleContext *scissors_ctx = gtk_widget_get_style_context(scissors_btn);

    gtk_style_context_remove_class(rock_ctx, "btn-choice-selected");
    gtk_style_context_remove_class(paper_ctx, "btn-choice-selected");
    gtk_style_context_remove_class(scissors_ctx, "btn-choice-selected");

    if (selected == ROCK) {
        gtk_style_context_add_class(rock_ctx, "btn-choice-selected");
    } else if (selected == PAPER) {
        gtk_style_context_add_class(paper_ctx, "btn-choice-selected");
    } else if (selected == SCISSORS) {
        gtk_style_context_add_class(scissors_ctx, "btn-choice-selected");
    }
}

void on_rock_clicked(GtkWidget *button, gpointer user_data) {
    app_data.player_choice = ROCK;
    GtkWidget **buttons = (GtkWidget **)user_data;
    update_button_selection(buttons[0], buttons[1], buttons[2], ROCK);
}

void on_paper_clicked(GtkWidget *button, gpointer user_data) {
    app_data.player_choice = PAPER;
    GtkWidget **buttons = (GtkWidget **)user_data;
    update_button_selection(buttons[0], buttons[1], buttons[2], PAPER);
}

void on_scissors_clicked(GtkWidget *button, gpointer user_data) {
    app_data.player_choice = SCISSORS;
    GtkWidget **buttons = (GtkWidget **)user_data;
    update_button_selection(buttons[0], buttons[1], buttons[2], SCISSORS);
}

void on_play_game_clicked(GtkWidget *button, gpointer user_data) {
    if (app_data.player_choice == 0) {
        show_dialog(app_data.window, "Error", "Please select Rock, Paper, or Scissors");
        return;
    }

    if (app_data.player_rounds_won >= 2 || app_data.computer_rounds_won >= 2) {
        show_dialog(app_data.window, "Match Over", "This match is already finished. Click FINISH MATCH to continue.");
        app_data.player_choice = 0;
        return;
    }

    int result = play_round(app_data.player_choice);
    app_data.games_in_match++;

    GtkWidget *result_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "result"));
    GtkWidget *score_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "score"));
    GtkWidget *round_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "round"));
    GtkWidget *computer_choice_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "computer_choice"));

    char computer_text[100];
    snprintf(computer_text, sizeof(computer_text), "Computer chose: %s", get_choice_name(app_data.last_computer_choice));
    gtk_label_set_text(GTK_LABEL(computer_choice_label), computer_text);

    char result_text[100];
    if (result == 1) {
        app_data.player_rounds_won++;
        snprintf(result_text, sizeof(result_text), "You Win Round %d!", app_data.current_round);
    } else if (result == -1) {
        app_data.computer_rounds_won++;
        snprintf(result_text, sizeof(result_text), "Computer Wins Round %d!", app_data.current_round);
    } else {
        snprintf(result_text, sizeof(result_text), "Round %d is a Draw!", app_data.current_round);
    }

    gtk_label_set_text(GTK_LABEL(result_label), result_text);

    char score_text[100];
    snprintf(score_text, sizeof(score_text), "Rounds Won - You: %d | Computer: %d",
             app_data.player_rounds_won, app_data.computer_rounds_won);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    if (app_data.player_rounds_won >= 2) {
        gtk_label_set_text(GTK_LABEL(round_label), "MATCH OVER - YOU WIN!");
        gtk_label_set_text(GTK_LABEL(result_label), "Congratulations! You won the match!");
    } else if (app_data.computer_rounds_won >= 2) {
        gtk_label_set_text(GTK_LABEL(round_label), "MATCH OVER - COMPUTER WINS!");
        gtk_label_set_text(GTK_LABEL(result_label), "Computer won the match!");
    } else {
        if (result != 0)
            app_data.current_round++;

        char round_text[50];
        snprintf(round_text, sizeof(round_text), "Round %d of 3", app_data.current_round);
        gtk_label_set_text(GTK_LABEL(round_label), round_text);
    }

    app_data.player_choice = 0;
    GtkWidget **buttons = (GtkWidget **)g_object_get_data(G_OBJECT(button), "buttons");
    update_button_selection(buttons[0], buttons[1], buttons[2], 0);
}

void on_finish_match_clicked(GtkWidget *button, gpointer user_data) {
    if (app_data.current_user) {
        if (app_data.player_rounds_won > app_data.computer_rounds_won) {
            app_data.current_user->wins++;
        } else if (app_data.player_rounds_won < app_data.computer_rounds_won) {
            app_data.current_user->losses++;
        } else {
            app_data.current_user->draws++;
        }
        app_data.current_user->games_played++;
        save_users();
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "main_menu");
    } else {
        app_data.guest_games++;
        if (app_data.guest_games >= 2) {
            show_dialog(app_data.window, "Guest Limit Reached",
                       "You've played your 2 free matches!\nPlease login or register to continue playing.");
            gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "start");
        } else {
            char games_left[50];
            snprintf(games_left, sizeof(games_left), "Matches Remaining: %d/2", 2 - app_data.guest_games);
            gtk_label_set_text(GTK_LABEL(app_data.guest_menu_title), games_left);
            gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "guest_menu");
        }
    }
}

GtkWidget *create_game_vs_computer_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    const char *back_page = app_data.current_user ? "vs_computer_mode_select" : "guest_menu";
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), (gpointer)back_page);
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 30);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_widget_set_margin_start(content, 30);
    gtk_widget_set_margin_end(content, 30);

    GtkWidget *title = gtk_label_new("Best of 3");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *round_label = gtk_label_new("Round 1 of 3");
    gtk_style_context_add_class(gtk_widget_get_style_context(round_label), "round-label");

    GtkWidget *computer_choice_label = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(computer_choice_label), "computer-choice-label");

    GtkWidget *score_label = gtk_label_new("Rounds Won - You: 0 | Computer: 0");
    gtk_style_context_add_class(gtk_widget_get_style_context(score_label), "score-display");

    GtkWidget *instructions = gtk_label_new("Select Your Move:");
    gtk_widget_set_halign(instructions, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(instructions), "label-title");

    GtkWidget *choices_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_set_homogeneous(GTK_BOX(choices_box), TRUE);

    GtkWidget *rock_button = gtk_button_new_with_label("ROCK");
    gtk_style_context_add_class(gtk_widget_get_style_context(rock_button), "btn-choice");

    GtkWidget *paper_button = gtk_button_new_with_label("PAPER");
    gtk_style_context_add_class(gtk_widget_get_style_context(paper_button), "btn-choice");

    GtkWidget *scissors_button = gtk_button_new_with_label("SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(scissors_button), "btn-choice");

    static GtkWidget *buttons[3];
    buttons[0] = rock_button;
    buttons[1] = paper_button;
    buttons[2] = scissors_button;

    g_signal_connect(rock_button, "clicked", G_CALLBACK(on_rock_clicked), buttons);
    g_signal_connect(paper_button, "clicked", G_CALLBACK(on_paper_clicked), buttons);
    g_signal_connect(scissors_button, "clicked", G_CALLBACK(on_scissors_clicked), buttons);

    gtk_box_pack_start(GTK_BOX(choices_box), rock_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), paper_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), scissors_button, TRUE, TRUE, 0);

    GtkWidget *result_label = gtk_label_new("Select a move to play");
    gtk_style_context_add_class(gtk_widget_get_style_context(result_label), "result-label");

    GtkWidget *play_button = gtk_button_new_with_label("PLAY ROUND");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-large");
    g_object_set_data(G_OBJECT(play_button), "result", result_label);
    g_object_set_data(G_OBJECT(play_button), "score", score_label);
    g_object_set_data(G_OBJECT(play_button), "round", round_label);
    g_object_set_data(G_OBJECT(play_button), "computer_choice", computer_choice_label);
    g_object_set_data(G_OBJECT(play_button), "buttons", buttons);
    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_game_clicked), NULL);

    GtkWidget *finish_button = gtk_button_new_with_label("FINISH MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-large");
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_finish_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), round_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), computer_choice_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), score_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), instructions, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), choices_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), result_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), finish_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_play_survival_clicked(GtkWidget *button, gpointer user_data) {
    if (app_data.player_choice == 0) {
        show_dialog(app_data.window, "Error", "Please select Rock, Paper, or Scissors");
        return;
    }

    int result = play_round(app_data.player_choice);

    GtkWidget *result_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "result"));
    GtkWidget *streak_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "streak"));
    GtkWidget *computer_choice_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "computer_choice"));

    char computer_text[100];
    snprintf(computer_text, sizeof(computer_text), "Computer chose: %s", get_choice_name(app_data.last_computer_choice));
    gtk_label_set_text(GTK_LABEL(computer_choice_label), computer_text);

    char result_text[100];
    if (result == 1) {
        app_data.survival_streak++;
        snprintf(result_text, sizeof(result_text), "You Win! Streak continues!");
    } else if (result == -1) {
        snprintf(result_text, sizeof(result_text), "You Lost! Game Over!");
        if (app_data.current_user) {
            app_data.current_user->losses++;
            app_data.current_user->games_played++;
            save_users();
        }
    } else {
        snprintf(result_text, sizeof(result_text), "Draw! Streak continues!");
    }

    gtk_label_set_text(GTK_LABEL(result_label), result_text);

    char streak_text[100];
    snprintf(streak_text, sizeof(streak_text), "Current Streak: %d", app_data.survival_streak);
    gtk_label_set_text(GTK_LABEL(streak_label), streak_text);

    if (result == -1) {
        show_dialog(app_data.window, "Game Over", "You lost! Your final streak was: " + app_data.survival_streak);
    }

    app_data.player_choice = 0;
    GtkWidget **buttons = (GtkWidget **)g_object_get_data(G_OBJECT(button), "buttons");
    update_button_selection(buttons[0], buttons[1], buttons[2], 0);
}

GtkWidget *create_game_survival_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "vs_computer_mode_select");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 30);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_widget_set_margin_start(content, 30);
    gtk_widget_set_margin_end(content, 30);

    GtkWidget *title = gtk_label_new("Survival Mode");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *streak_label = gtk_label_new("Current Streak: 0");
    gtk_style_context_add_class(gtk_widget_get_style_context(streak_label), "score-display");

    GtkWidget *computer_choice_label = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(computer_choice_label), "computer-choice-label");

    GtkWidget *instructions = gtk_label_new("Select Your Move:");
    gtk_widget_set_halign(instructions, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(instructions), "label-title");

    GtkWidget *choices_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_set_homogeneous(GTK_BOX(choices_box), TRUE);

    GtkWidget *rock_button = gtk_button_new_with_label("ROCK");
    gtk_style_context_add_class(gtk_widget_get_style_context(rock_button), "btn-choice");

    GtkWidget *paper_button = gtk_button_new_with_label("PAPER");
    gtk_style_context_add_class(gtk_widget_get_style_context(paper_button), "btn-choice");

    GtkWidget *scissors_button = gtk_button_new_with_label("SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(scissors_button), "btn-choice");

    static GtkWidget *buttons_surv[3];
    buttons_surv[0] = rock_button;
    buttons_surv[1] = paper_button;
    buttons_surv[2] = scissors_button;

    g_signal_connect(rock_button, "clicked", G_CALLBACK(on_rock_clicked), buttons_surv);
    g_signal_connect(paper_button, "clicked", G_CALLBACK(on_paper_clicked), buttons_surv);
    g_signal_connect(scissors_button, "clicked", G_CALLBACK(on_scissors_clicked), buttons_surv);

    gtk_box_pack_start(GTK_BOX(choices_box), rock_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), paper_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), scissors_button, TRUE, TRUE, 0);

    GtkWidget *result_label = gtk_label_new("Select a move to play");
    gtk_style_context_add_class(gtk_widget_get_style_context(result_label), "result-label");

    GtkWidget *play_button = gtk_button_new_with_label("PLAY ROUND");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-large");
    g_object_set_data(G_OBJECT(play_button), "result", result_label);
    g_object_set_data(G_OBJECT(play_button), "streak", streak_label);
    g_object_set_data(G_OBJECT(play_button), "computer_choice", computer_choice_label);
    g_object_set_data(G_OBJECT(play_button), "buttons", buttons_surv);
    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_survival_clicked), NULL);

    GtkWidget *finish_button = gtk_button_new_with_label("END GAME");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-large");
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_back_clicked), "main_menu");

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), streak_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), computer_choice_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), instructions, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), choices_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), result_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), finish_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_start_custom_match_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *rounds_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "rounds"));
    const char *rounds_text = gtk_entry_get_text(rounds_entry);
    int rounds = atoi(rounds_text);

    if (rounds < 1 || rounds > 10) {
        show_dialog(app_data.window, "Invalid Input", "Defaulting to 3 rounds. Please enter a number between 1 and 10.");
        rounds=3;
    }

    app_data.custom_rounds_total = rounds;
    app_data.player_score = 0;
    app_data.computer_score = 0;
    app_data.games_in_match = 0;
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;
    app_data.is_local_multiplayer = 0;
    strcpy(app_data.current_game_mode, "Custom Match");

    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_custom");
}

GtkWidget *create_custom_rounds_input_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "vs_computer_mode_select");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 400, -1);

    GtkWidget *title = gtk_label_new("Custom Match");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *label = gtk_label_new("Enter number of rounds (1-10):");
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "label-title");
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    GtkWidget *rounds_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(rounds_entry), "e.g., 5");
    gtk_style_context_add_class(gtk_widget_get_style_context(rounds_entry), "entry");

    GtkWidget *start_btn = gtk_button_new_with_label("START MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-large");
    g_object_set_data(G_OBJECT(start_btn), "rounds", rounds_entry);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(on_start_custom_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), rounds_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), start_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_play_custom_clicked(GtkWidget *button, gpointer user_data);

GtkWidget *create_game_custom_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "vs_computer_mode_select");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 30);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_widget_set_margin_start(content, 30);
    gtk_widget_set_margin_end(content, 30);

    GtkWidget *title = gtk_label_new("Custom Match");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *round_label = gtk_label_new("Round 1");
    gtk_style_context_add_class(gtk_widget_get_style_context(round_label), "round-label");

    GtkWidget *computer_choice_label = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(computer_choice_label), "computer-choice-label");

    GtkWidget *score_label = gtk_label_new("You: 0 | Computer: 0");
    gtk_style_context_add_class(gtk_widget_get_style_context(score_label), "score-display");

    GtkWidget *instructions = gtk_label_new("Select Your Move:");
    gtk_widget_set_halign(instructions, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(instructions), "label-title");

    GtkWidget *choices_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_set_homogeneous(GTK_BOX(choices_box), TRUE);

    GtkWidget *rock_button = gtk_button_new_with_label("ROCK");
    gtk_style_context_add_class(gtk_widget_get_style_context(rock_button), "btn-choice");

    GtkWidget *paper_button = gtk_button_new_with_label("PAPER");
    gtk_style_context_add_class(gtk_widget_get_style_context(paper_button), "btn-choice");

    GtkWidget *scissors_button = gtk_button_new_with_label("SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(scissors_button), "btn-choice");

    static GtkWidget *buttons_custom[3];
    buttons_custom[0] = rock_button;
    buttons_custom[1] = paper_button;
    buttons_custom[2] = scissors_button;

    g_signal_connect(rock_button, "clicked", G_CALLBACK(on_rock_clicked), buttons_custom);
    g_signal_connect(paper_button, "clicked", G_CALLBACK(on_paper_clicked), buttons_custom);
    g_signal_connect(scissors_button, "clicked", G_CALLBACK(on_scissors_clicked), buttons_custom);

    gtk_box_pack_start(GTK_BOX(choices_box), rock_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), paper_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), scissors_button, TRUE, TRUE, 0);

    GtkWidget *result_label = gtk_label_new("Select a move to play");
    gtk_style_context_add_class(gtk_widget_get_style_context(result_label), "result-label");

    GtkWidget *play_button = gtk_button_new_with_label("PLAY ROUND");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(play_button), "btn-large");
    g_object_set_data(G_OBJECT(play_button), "result", result_label);
    g_object_set_data(G_OBJECT(play_button), "score", score_label);
    g_object_set_data(G_OBJECT(play_button), "round", round_label);
    g_object_set_data(G_OBJECT(play_button), "computer_choice", computer_choice_label);
    g_object_set_data(G_OBJECT(play_button), "buttons", buttons_custom);
    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_custom_clicked), NULL);

    GtkWidget *finish_button = gtk_button_new_with_label("FINISH MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-large");
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_finish_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), round_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), computer_choice_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), score_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), instructions, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), choices_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), result_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), finish_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_play_custom_clicked(GtkWidget *button, gpointer user_data) {
    if (app_data.player_choice == 0) {
        show_dialog(app_data.window, "Error", "Please select Rock, Paper, or Scissors");
        return;
    }

    if (app_data.current_round > app_data.custom_rounds_total) {
        show_dialog(app_data.window, "Match Over", "This match is already finished. Click FINISH MATCH to continue.");
        app_data.player_choice = 0;
        return;
    }

    int result = play_round(app_data.player_choice);
    app_data.games_in_match++;

    GtkWidget *result_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "result"));
    GtkWidget *score_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "score"));
    GtkWidget *round_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "round"));
    GtkWidget *computer_choice_label = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "computer_choice"));

    char computer_text[100];
    snprintf(computer_text, sizeof(computer_text), "Computer chose: %s", get_choice_name(app_data.last_computer_choice));
    gtk_label_set_text(GTK_LABEL(computer_choice_label), computer_text);

    char result_text[100];
    if (result == 1) {
        app_data.player_rounds_won++;
        snprintf(result_text, sizeof(result_text), "You Win Round %d!", app_data.current_round);
    } else if (result == -1) {
        app_data.computer_rounds_won++;
        snprintf(result_text, sizeof(result_text), "Computer Wins Round %d!", app_data.current_round);
    } else {
        snprintf(result_text, sizeof(result_text), "Round %d is a Draw!", app_data.current_round);
    }

    gtk_label_set_text(GTK_LABEL(result_label), result_text);

    char score_text[100];
    snprintf(score_text, sizeof(score_text), "You: %d | Computer: %d",
             app_data.player_rounds_won, app_data.computer_rounds_won);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    if (app_data.current_round >= app_data.custom_rounds_total) {
        gtk_label_set_text(GTK_LABEL(round_label), "MATCH OVER");
        if (app_data.player_rounds_won > app_data.computer_rounds_won) {
            gtk_label_set_text(GTK_LABEL(result_label), "Congratulations! You won the match!");
        } else if (app_data.player_rounds_won < app_data.computer_rounds_won) {
            gtk_label_set_text(GTK_LABEL(result_label), "Computer won the match!");
        } else {
            gtk_label_set_text(GTK_LABEL(result_label), "It's a tie!");
        }
    } else {
        app_data.current_round++;
        char round_text[50];
        snprintf(round_text, sizeof(round_text), "Round %d of %d", app_data.current_round, app_data.custom_rounds_total);
        gtk_label_set_text(GTK_LABEL(round_label), round_text);
    }

    app_data.player_choice = 0;
    GtkWidget **buttons = (GtkWidget **)g_object_get_data(G_OBJECT(button), "buttons");
    update_button_selection(buttons[0], buttons[1], buttons[2], 0);
}

void on_start_local_match_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *p1_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "player1"));
    GtkEntry *p2_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "player2"));

    const char *p1_name = gtk_entry_get_text(p1_entry);
    const char *p2_name = gtk_entry_get_text(p2_entry);

    if (strlen(p1_name) < 1 || strlen(p2_name) < 1) {
        show_dialog(app_data.window, "Invalid Input", "Please enter both player names");
        return;
    }

    strncpy(app_data.player1_name, p1_name, sizeof(app_data.player1_name) - 1);
    strncpy(app_data.player2_name, p2_name, sizeof(app_data.player2_name) - 1);
    app_data.is_local_multiplayer = 1;
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;

    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "local_multiplayer_mode_select");
}

void on_local_quick_match_clicked(GtkWidget *button, gpointer user_data) {
    strcpy(app_data.current_game_mode, "Local Multiplayer - Quick Match");
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;
    app_data.local_game_step = 0;
    app_data.player1_choice = 0;
    app_data.player2_choice = 0;
    app_data.p1_rounds_won = 0;
    app_data.p2_rounds_won = 0;
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_local_quick");
}

void on_local_custom_match_clicked(GtkWidget *button, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "local_custom_rounds_input");
}

void on_start_local_custom_match_clicked(GtkWidget *button, gpointer user_data) {
    GtkEntry *rounds_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "rounds"));
    const char *rounds_text = gtk_entry_get_text(rounds_entry);
    int rounds = atoi(rounds_text);

    if (rounds < 1 || rounds > 10) {
        rounds = 3;
    }

    app_data.custom_rounds_total = rounds;
    app_data.player_rounds_won = 0;
    app_data.computer_rounds_won = 0;
    app_data.current_round = 1;
    app_data.local_game_step = 0;
    app_data.player1_choice = 0;
    app_data.player2_choice = 0;
    app_data.p1_rounds_won = 0;
    app_data.p2_rounds_won = 0;
    strcpy(app_data.current_game_mode, "Local Multiplayer - Custom Match");

    gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "game_local_custom");
}

GtkWidget *create_local_multiplayer_setup_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "main_menu");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 400, -1);

    GtkWidget *title = gtk_label_new("Local Multiplayer Setup");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *p1_label = gtk_label_new("Player 1 Name:");
    gtk_style_context_add_class(gtk_widget_get_style_context(p1_label), "label-title");
    gtk_widget_set_halign(p1_label, GTK_ALIGN_START);

    GtkWidget *p1_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(p1_entry), "Enter Player 1 name");
    gtk_style_context_add_class(gtk_widget_get_style_context(p1_entry), "entry");

    GtkWidget *p2_label = gtk_label_new("Player 2 Name:");
    gtk_style_context_add_class(gtk_widget_get_style_context(p2_label), "label-title");
    gtk_widget_set_halign(p2_label, GTK_ALIGN_START);

    GtkWidget *p2_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(p2_entry), "Enter Player 2 name");
    gtk_style_context_add_class(gtk_widget_get_style_context(p2_entry), "entry");

    GtkWidget *start_btn = gtk_button_new_with_label("START MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-large");
    g_object_set_data(G_OBJECT(start_btn), "player1", p1_entry);
    g_object_set_data(G_OBJECT(start_btn), "player2", p2_entry);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(on_start_local_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), p1_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), p1_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), p2_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), p2_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), start_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

GtkWidget *create_local_multiplayer_mode_select_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "local_multiplayer_setup");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 25);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);

    char title_text[150];
    snprintf(title_text, sizeof(title_text), "%s vs %s - Select Game Mode", app_data.player1_name, app_data.player2_name);
    GtkWidget *title = gtk_label_new(title_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *quick_btn = gtk_button_new_with_label("Quick Match (Best of 3)");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_btn), "btn-game");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_btn), "btn-large");
    g_signal_connect(quick_btn, "clicked", G_CALLBACK(on_local_quick_match_clicked), NULL);

    GtkWidget *custom_btn = gtk_button_new_with_label("Custom Match (Choose number of rounds)");
    gtk_style_context_add_class(gtk_widget_get_style_context(custom_btn), "btn-game");
    gtk_style_context_add_class(gtk_widget_get_style_context(custom_btn), "btn-large");
    g_signal_connect(custom_btn, "clicked", G_CALLBACK(on_local_custom_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), quick_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), custom_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

GtkWidget *create_local_custom_rounds_input_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "local_multiplayer_mode_select");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 60);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 400, -1);

    GtkWidget *title = gtk_label_new("Custom Match - Enter Rounds");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *label = gtk_label_new("Enter number of rounds (1-10):");
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "label-title");
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    GtkWidget *rounds_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(rounds_entry), "e.g., 5");
    gtk_style_context_add_class(gtk_widget_get_style_context(rounds_entry), "entry");

    GtkWidget *start_btn = gtk_button_new_with_label("START MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-primary");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_btn), "btn-large");
    g_object_set_data(G_OBJECT(start_btn), "rounds", rounds_entry);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(on_start_local_custom_match_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), rounds_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), start_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_local_choice_clicked(GtkWidget *button, gpointer user_data) {
    int choice = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "choice"));
    GtkLabel *result_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "result"));
    GtkLabel *score_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "score"));
    GtkLabel *round_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "round"));

    if (app_data.local_game_step == 0) {
        app_data.player1_choice = choice;
        app_data.local_game_step = 1;

        char instruction_text[150];
        snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player2_name);
        gtk_label_set_text(result_label, instruction_text);
    } else if (app_data.local_game_step == 1) {
        app_data.player2_choice = choice;

        int p1_choice = app_data.player1_choice;
        int p2_choice = app_data.player2_choice;

        char result_text[300];
        snprintf(result_text, sizeof(result_text), "%s chose %s\n%s chose %s\n\n",
                 app_data.player1_name, get_choice_name(p1_choice),
                 app_data.player2_name, get_choice_name(p2_choice));

        if (p1_choice == p2_choice) {
            strcat(result_text, "It's a draw!");
        } else if ((p1_choice == ROCK && p2_choice == SCISSORS) ||
                   (p1_choice == PAPER && p2_choice == ROCK) ||
                   (p1_choice == SCISSORS && p2_choice == PAPER)) {
            strcat(result_text, app_data.player1_name);
            strcat(result_text, " wins this round!");
            app_data.p1_rounds_won++;
        } else {
            strcat(result_text, app_data.player2_name);
            strcat(result_text, " wins this round!");
            app_data.p2_rounds_won++;
        }

        gtk_label_set_text(result_label, result_text);

        char score_text[150];
        snprintf(score_text, sizeof(score_text), "%s: %d | %s: %d",
                 app_data.player1_name, app_data.p1_rounds_won,
                 app_data.player2_name, app_data.p2_rounds_won);
        gtk_label_set_text(score_label, score_text);

        app_data.current_round++;
        app_data.local_game_step = 2;

        if (app_data.p1_rounds_won >= 2 || app_data.p2_rounds_won >= 2 || app_data.current_round > 3) {
            char final_text[200];
            if (app_data.p1_rounds_won > app_data.p2_rounds_won) {
                snprintf(final_text, sizeof(final_text), "Match Over!\n%s WINS!", app_data.player1_name);
            } else if (app_data.p2_rounds_won > app_data.p1_rounds_won) {
                snprintf(final_text, sizeof(final_text), "Match Over!\n%s WINS!", app_data.player2_name);
            } else {
                snprintf(final_text, sizeof(final_text), "Match Over!\nIt's a TIE!");
            }

            char full_result[400];
            snprintf(full_result, sizeof(full_result), "%s\n\n%s", result_text, final_text);
            gtk_label_set_text(result_label, full_result);
        } else {
            char round_text[50];
            snprintf(round_text, sizeof(round_text), "Round %d of 3", app_data.current_round);
            gtk_label_set_text(round_label, round_text);

            GTimer *timer = g_timer_new();
            while (g_timer_elapsed(timer, NULL) < 2.0) {
                while (gtk_events_pending()) {
                    gtk_main_iteration();
                }
            }
            g_timer_destroy(timer);

            app_data.local_game_step = 0;
            char instruction_text[150];
            snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player1_name);
            gtk_label_set_text(result_label, instruction_text);
        }
    }
}

void on_play_local_quick_clicked(GtkWidget *button, gpointer user_data);

GtkWidget *create_game_local_quick_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "local_multiplayer_mode_select");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 30);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_widget_set_margin_start(content, 30);
    gtk_widget_set_margin_end(content, 30);

    GtkWidget *title = gtk_label_new("Local Multiplayer - Best of 3");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    GtkWidget *round_label = gtk_label_new("Round 1 of 3");
    gtk_style_context_add_class(gtk_widget_get_style_context(round_label), "round-label");

    char score_text[150];
    snprintf(score_text, sizeof(score_text), "%s: 0 | %s: 0", app_data.player1_name, app_data.player2_name);
    GtkWidget *score_label = gtk_label_new(score_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(score_label), "score-display");

    char instruction_text[150];
    snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player1_name);
    GtkWidget *result_label = gtk_label_new(instruction_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(result_label), "result-label");

    GtkWidget *choices_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(choices_box, GTK_ALIGN_CENTER);

    GtkWidget *rock_btn = gtk_button_new_with_label("ROCK");
    gtk_style_context_add_class(gtk_widget_get_style_context(rock_btn), "btn-choice");
    g_object_set_data(G_OBJECT(rock_btn), "choice", GINT_TO_POINTER(ROCK));
    g_object_set_data(G_OBJECT(rock_btn), "result", result_label);
    g_object_set_data(G_OBJECT(rock_btn), "score", score_label);
    g_object_set_data(G_OBJECT(rock_btn), "round", round_label);
    g_signal_connect(rock_btn, "clicked", G_CALLBACK(on_local_choice_clicked), NULL);

    GtkWidget *paper_btn = gtk_button_new_with_label("PAPER");
    gtk_style_context_add_class(gtk_widget_get_style_context(paper_btn), "btn-choice");
    g_object_set_data(G_OBJECT(paper_btn), "choice", GINT_TO_POINTER(PAPER));
    g_object_set_data(G_OBJECT(paper_btn), "result", result_label);
    g_object_set_data(G_OBJECT(paper_btn), "score", score_label);
    g_object_set_data(G_OBJECT(paper_btn), "round", round_label);
    g_signal_connect(paper_btn, "clicked", G_CALLBACK(on_local_choice_clicked), NULL);

    GtkWidget *scissors_btn = gtk_button_new_with_label("SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(scissors_btn), "btn-choice");
    g_object_set_data(G_OBJECT(scissors_btn), "choice", GINT_TO_POINTER(SCISSORS));
    g_object_set_data(G_OBJECT(scissors_btn), "result", result_label);
    g_object_set_data(G_OBJECT(scissors_btn), "score", score_label);
    g_object_set_data(G_OBJECT(scissors_btn), "round", round_label);
    g_signal_connect(scissors_btn, "clicked", G_CALLBACK(on_local_choice_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(choices_box), rock_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), paper_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), scissors_btn, FALSE, FALSE, 0);

    GtkWidget *finish_button = gtk_button_new_with_label("FINISH MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-large");
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_back_clicked), "main_menu");

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), round_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), score_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), result_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), choices_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), finish_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}



void on_play_local_quick_clicked(GtkWidget *button, gpointer user_data) {
    show_dialog(app_data.window, "Coming Soon", "Local multiplayer gameplay is coming soon!");
}

void on_local_custom_choice_clicked(GtkWidget *button, gpointer user_data);

GtkWidget *create_game_local_custom_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "local_custom_rounds_input");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 30);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_widget_set_margin_start(content, 30);
    gtk_widget_set_margin_end(content, 30);

    char title_text[150];
    snprintf(title_text, sizeof(title_text), "%s vs %s - Custom Match", app_data.player1_name, app_data.player2_name);
    GtkWidget *title = gtk_label_new(title_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    char round_text[50];
    snprintf(round_text, sizeof(round_text), "Round 1 of %d", app_data.custom_rounds_total);
    GtkWidget *round_label = gtk_label_new(round_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(round_label), "round-label");

    char score_text[150];
    snprintf(score_text, sizeof(score_text), "%s: 0 | %s: 0", app_data.player1_name, app_data.player2_name);
    GtkWidget *score_label = gtk_label_new(score_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(score_label), "score-display");

    char instruction_text[150];
    snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player1_name);
    GtkWidget *result_label = gtk_label_new(instruction_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(result_label), "result-label");

    GtkWidget *choices_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(choices_box, GTK_ALIGN_CENTER);

    GtkWidget *rock_btn = gtk_button_new_with_label("ROCK");
    gtk_style_context_add_class(gtk_widget_get_style_context(rock_btn), "btn-choice");
    g_object_set_data(G_OBJECT(rock_btn), "choice", GINT_TO_POINTER(ROCK));
    g_object_set_data(G_OBJECT(rock_btn), "result", result_label);
    g_object_set_data(G_OBJECT(rock_btn), "score", score_label);
    g_object_set_data(G_OBJECT(rock_btn), "round", round_label);
    g_signal_connect(rock_btn, "clicked", G_CALLBACK(on_local_custom_choice_clicked), NULL);

    GtkWidget *paper_btn = gtk_button_new_with_label("PAPER");
    gtk_style_context_add_class(gtk_widget_get_style_context(paper_btn), "btn-choice");
    g_object_set_data(G_OBJECT(paper_btn), "choice", GINT_TO_POINTER(PAPER));
    g_object_set_data(G_OBJECT(paper_btn), "result", result_label);
    g_object_set_data(G_OBJECT(paper_btn), "score", score_label);
    g_object_set_data(G_OBJECT(paper_btn), "round", round_label);
    g_signal_connect(paper_btn, "clicked", G_CALLBACK(on_local_custom_choice_clicked), NULL);

    GtkWidget *scissors_btn = gtk_button_new_with_label("SCISSORS");
    gtk_style_context_add_class(gtk_widget_get_style_context(scissors_btn), "btn-choice");
    g_object_set_data(G_OBJECT(scissors_btn), "choice", GINT_TO_POINTER(SCISSORS));
    g_object_set_data(G_OBJECT(scissors_btn), "result", result_label);
    g_object_set_data(G_OBJECT(scissors_btn), "score", score_label);
    g_object_set_data(G_OBJECT(scissors_btn), "round", round_label);
    g_signal_connect(scissors_btn, "clicked", G_CALLBACK(on_local_custom_choice_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(choices_box), rock_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), paper_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(choices_box), scissors_btn, FALSE, FALSE, 0);

    GtkWidget *finish_button = gtk_button_new_with_label("FINISH MATCH");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(finish_button), "btn-large");
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_back_clicked), "main_menu");

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), round_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), score_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), result_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), choices_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), finish_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_local_custom_choice_clicked(GtkWidget *button, gpointer user_data) {
    int choice = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "choice"));
    GtkLabel *result_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "result"));
    GtkLabel *score_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "score"));
    GtkLabel *round_label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "round"));

    if (app_data.local_game_step == 0) {
        app_data.player1_choice = choice;
        app_data.local_game_step = 1;

        char instruction_text[150];
        snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player2_name);
        gtk_label_set_text(result_label, instruction_text);
    } else if (app_data.local_game_step == 1) {
        app_data.player2_choice = choice;

        int p1_choice = app_data.player1_choice;
        int p2_choice = app_data.player2_choice;

        char result_text[300];
        snprintf(result_text, sizeof(result_text), "%s chose %s\n%s chose %s\n\n",
                 app_data.player1_name, get_choice_name(p1_choice),
                 app_data.player2_name, get_choice_name(p2_choice));

        if (p1_choice == p2_choice) {
            strcat(result_text, "It's a draw!");
        } else if ((p1_choice == ROCK && p2_choice == SCISSORS) ||
                   (p1_choice == PAPER && p2_choice == ROCK) ||
                   (p1_choice == SCISSORS && p2_choice == PAPER)) {
            strcat(result_text, app_data.player1_name);
            strcat(result_text, " wins this round!");
            app_data.p1_rounds_won++;
        } else {
            strcat(result_text, app_data.player2_name);
            strcat(result_text, " wins this round!");
            app_data.p2_rounds_won++;
        }

        gtk_label_set_text(result_label, result_text);

        char score_text[150];
        snprintf(score_text, sizeof(score_text), "%s: %d | %s: %d",
                 app_data.player1_name, app_data.p1_rounds_won,
                 app_data.player2_name, app_data.p2_rounds_won);
        gtk_label_set_text(score_label, score_text);

        app_data.current_round++;
        app_data.local_game_step = 2;

        if (app_data.current_round > app_data.custom_rounds_total) {
            char final_text[200];
            if (app_data.p1_rounds_won > app_data.p2_rounds_won) {
                snprintf(final_text, sizeof(final_text), "Match Over!\n%s WINS!", app_data.player1_name);
            } else if (app_data.p2_rounds_won > app_data.p1_rounds_won) {
                snprintf(final_text, sizeof(final_text), "Match Over!\n%s WINS!", app_data.player2_name);
            } else {
                snprintf(final_text, sizeof(final_text), "Match Over!\nIt's a TIE!");
            }

            char full_result[400];
            snprintf(full_result, sizeof(full_result), "%s\n\n%s", result_text, final_text);
            gtk_label_set_text(result_label, full_result);
        } else {
            char round_text[50];
            snprintf(round_text, sizeof(round_text), "Round %d of %d", app_data.current_round, app_data.custom_rounds_total);
            gtk_label_set_text(round_label, round_text);

            GTimer *timer = g_timer_new();
            while (g_timer_elapsed(timer, NULL) < 2.0) {
                while (gtk_events_pending()) {
                    gtk_main_iteration();
                }
            }
            g_timer_destroy(timer);

            app_data.local_game_step = 0;
            char instruction_text[150];
            snprintf(instruction_text, sizeof(instruction_text), "%s, make your choice:", app_data.player1_name);
            gtk_label_set_text(result_label, instruction_text);
        }
    }
}

GtkWidget *create_statistics_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(top_bar, 20);
    gtk_widget_set_margin_top(top_bar, 20);
    GtkWidget *back_btn = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "btn-secondary");
    gtk_widget_set_size_request(back_btn, 100, 40);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), "main_menu");
    gtk_box_pack_start(GTK_BOX(top_bar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 60);
    gtk_widget_set_margin_start(content, 80);
    gtk_widget_set_margin_end(content, 80);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(content, 450, -1);

    GtkWidget *title = gtk_label_new("Your Statistics");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    if (app_data.current_user) {
        char stats_text[400];
        float win_rate = app_data.current_user->games_played > 0 ?
                        (float)app_data.current_user->wins / app_data.current_user->games_played * 100 : 0;

        snprintf(stats_text, sizeof(stats_text),
                "Total Matches: %d\n"
                "Total Wins: %d\n"
                "Total Losses: %d\n"
                "Total Draws: %d\n\n"
                "Win Rate: %.1f%%",
                app_data.current_user->games_played,
                app_data.current_user->wins,
                app_data.current_user->losses,
                app_data.current_user->draws,
                win_rate);

        GtkWidget *stats_label = gtk_label_new(stats_text);
        gtk_style_context_add_class(gtk_widget_get_style_context(stats_label), "stats-label");
        gtk_label_set_line_wrap(GTK_LABEL(stats_label), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(stats_label), PANGO_WRAP_WORD);

        gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(content), stats_label, FALSE, FALSE, 0);

        GtkWidget *history_title = gtk_label_new("\nRecent Game History:");
        gtk_style_context_add_class(gtk_widget_get_style_context(history_title), "label-title");
        gtk_widget_set_halign(history_title, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(content), history_title, FALSE, FALSE, 10);

        FILE *file = fopen(STATS_FILE, "r");
        if (file != NULL) {
            char line[256];
            char last_games[5][256];
            int count = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, app_data.current_user->username)) {
                    strcpy(last_games[count % 5], line);
                    count++;
                }
            }
            fclose(file);

            int games_to_show = count < 5 ? count : 5;
            for (int i = 0; i < games_to_show; i++) {
                int idx = (count - games_to_show + i) % 5;
                GtkWidget *history_label = gtk_label_new(last_games[idx]);
                gtk_style_context_add_class(gtk_widget_get_style_context(history_label), "stats-label");
                gtk_label_set_line_wrap(GTK_LABEL(history_label), TRUE);
                gtk_widget_set_halign(history_label, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(content), history_label, FALSE, FALSE, 0);
            }

            if (games_to_show == 0) {
                GtkWidget *no_history = gtk_label_new("No game history yet");
                gtk_style_context_add_class(gtk_widget_get_style_context(no_history), "stats-label");
                gtk_widget_set_halign(no_history, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(content), no_history, FALSE, FALSE, 0);
            }
        }
    } else {
        GtkWidget *label = gtk_label_new("You must login to view statistics");
        gtk_style_context_add_class(gtk_widget_get_style_context(label), "label-title");
        gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_rules_back_clicked(GtkWidget *button, gpointer user_data) {
    if (strlen(app_data.previous_page) > 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), app_data.previous_page);
    } else {
        gtk_stack_set_visible_child_name(GTK_STACK(app_data.main_stack), "main_menu");
    }
}

GtkWidget *create_rules_page() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = create_header_bar();
    g_object_set_data(G_OBJECT(app_data.user_button), "parent", main_box);
    g_signal_connect(app_data.user_button, "clicked", G_CALLBACK(on_user_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(content, 40);
    gtk_widget_set_margin_bottom(content, 40);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);

    GtkWidget *title = gtk_label_new("Game Rules");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "title");

    const char *rules_text = "ROCK crushes SCISSORS\n\n"
                            "SCISSORS cuts PAPER\n\n"
                            "PAPER covers ROCK\n\n\n"
                            "GUEST MODE: Play 2 best-of-3 matches before login is required\n\n"
                            "LOGGED IN: Track your stats and play unlimited games";

    GtkWidget *rules_label = gtk_label_new(rules_text);
    gtk_label_set_line_wrap(GTK_LABEL(rules_label), TRUE);
    gtk_label_set_justify(GTK_LABEL(rules_label), GTK_JUSTIFY_LEFT);
    gtk_style_context_add_class(gtk_widget_get_style_context(rules_label), "stats-label");

    GtkWidget *back_button = gtk_button_new_with_label("BACK");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_button), "btn-secondary");
    gtk_style_context_add_class(gtk_widget_get_style_context(back_button), "btn-large");
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_rules_back_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), rules_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), back_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), content, TRUE, TRUE, 0);

    return main_box;
}

void on_activate(GtkApplication *app, gpointer user_data) {
    setup_css();
    srand((unsigned int)time(NULL));
    load_users();

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Rock Paper Scissors");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 900);

    app_data.window = window;
    app_data.app = app;

    GtkWidget *stack = gtk_stack_new();
    app_data.main_stack = stack;

    gtk_stack_add_named(GTK_STACK(stack), create_start_page(), "start");
    gtk_stack_add_named(GTK_STACK(stack), create_login_page(), "login");
    gtk_stack_add_named(GTK_STACK(stack), create_register_page(), "register");
    gtk_stack_add_named(GTK_STACK(stack), create_guest_menu_page(), "guest_menu");
    gtk_stack_add_named(GTK_STACK(stack), create_main_menu_page(), "main_menu");
    gtk_stack_add_named(GTK_STACK(stack), create_vs_computer_mode_select_page(), "vs_computer_mode_select");
    gtk_stack_add_named(GTK_STACK(stack), create_game_vs_computer_page(), "game_vs_computer");
    gtk_stack_add_named(GTK_STACK(stack), create_game_survival_page(), "game_survival");
    gtk_stack_add_named(GTK_STACK(stack), create_custom_rounds_input_page(), "custom_rounds_input");
    gtk_stack_add_named(GTK_STACK(stack), create_game_custom_page(), "game_custom");
    gtk_stack_add_named(GTK_STACK(stack), create_local_multiplayer_setup_page(), "local_multiplayer_setup");
    gtk_stack_add_named(GTK_STACK(stack), create_local_multiplayer_mode_select_page(), "local_multiplayer_mode_select");
    gtk_stack_add_named(GTK_STACK(stack), create_local_custom_rounds_input_page(), "local_custom_rounds_input");
    gtk_stack_add_named(GTK_STACK(stack), create_game_local_quick_page(), "game_local_quick");
    gtk_stack_add_named(GTK_STACK(stack), create_game_local_custom_page(), "game_local_custom");
    gtk_stack_add_named(GTK_STACK(stack), create_statistics_page(), "statistics");
    gtk_stack_add_named(GTK_STACK(stack), create_rules_page(), "rules");
    gtk_stack_add_named(GTK_STACK(stack), create_account_page(), "account");

    gtk_stack_set_visible_child_name(GTK_STACK(stack), "start");

    gtk_container_add(GTK_CONTAINER(window), stack);
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.rockpaperscissors", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
