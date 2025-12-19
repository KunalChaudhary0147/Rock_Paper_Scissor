import customtkinter as ctk
import random
import json
import os
from datetime import datetime


# ---------------- CONFIG ----------------
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("green")

ROCK, PAPER, SCISSORS = 1, 2, 3


# ---------------- STATE ----------------
class State:
    def __init__(self):
        self.guest_matches_played = 0
        self.guest_max_matches = 2
        self.player_score = 0
        self.computer_score = 0
        self.selected = None
        self.current_user = None
        self.survival_streak = 0
        self.player1_name = ""
        self.player2_name = ""


state = State()


# ---------------- DATA ----------------
DATA_DIR = "Data"
USERS_FILE = os.path.join(DATA_DIR, "users.txt")
STATS_FILE = os.path.join(DATA_DIR, "statistics.txt")


def load_users():
    users = []
    if os.path.exists(USERS_FILE):
        with open(USERS_FILE, "r") as f:
            for line in f:
                line = line.strip()
                if line:
                    users.append(json.loads(line))
    return users


def save_users(users):
    with open(USERS_FILE, "w") as f:
        for user in users:
            f.write(json.dumps(user) + "\n")


def load_stats():
    stats = []
    if os.path.exists(STATS_FILE):
        with open(STATS_FILE, "r") as f:
            for line in f:
                line = line.strip()
                if line:
                    if line.startswith("{"):
                        stats.append(json.loads(line))
                    else:
                        # old format, skip or parse
                        pass
    return stats


def save_stat(stat):
    with open(STATS_FILE, "a") as f:
        f.write(json.dumps(stat) + "\n")


def get_user_stats(username):
    users = load_users()
    for user in users:
        if user["username"] == username:
            return user
    return None


def update_user(username, updates):
    users = load_users()
    for user in users:
        if user["username"] == username:
            user.update(updates)
            break
    save_users(users)


def get_last_5_matches(username):
    stats = load_stats()
    user_stats = [s for s in stats if s.get("user") == username]
    return user_stats[-5:]


# ---------------- APP ----------------
class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Rock Paper Scissors")
        self.geometry("1100x800")
        self.minsize(900, 700)

        self.container = ctk.CTkFrame(self)
        self.container.pack(fill="both", expand=True)

        self.frames = {}
        for Page in (
            LandingPage,
            RegisterPage,
            LoginPage,
            GuestMenuPage,
            RulesPage,
            QuickMatchPage,
            MainMenuPage,
            AccountPage,
            StatisticsPage,
            ComputerPlayPage,
            SurvivalMatchPage,
            CustomMatchPage,
            LocalMultiPage,
            LocalGameModePage,
        ):
            page = Page(self.container, self)
            self.frames[Page] = page
            page.place(relwidth=1, relheight=1)

        self.show(LandingPage)

    def show(self, page):
        self.frames[page].tkraise()


# ---------------- LANDING ----------------
class LandingPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="lightblue")

        ctk.CTkLabel(
            self,
            text="ROCK  PAPER  SCISSORS",
            font=("Arial Black", 44),
            text_color="black"
        ).pack(pady=120)

        self.login_btn = ctk.CTkButton(
            self,
            text="LOGIN",
            height=60,
            command=lambda: app.show(LoginPage)
        )
        self.login_btn.pack(pady=20)

        self.register_btn = ctk.CTkButton(
            self,
            text="REGISTER",
            height=60,
            command=lambda: app.show(RegisterPage)
        )
        self.register_btn.pack(pady=20)

        self.guest_btn = ctk.CTkButton(
            self,
            text="PLAY AS GUEST",
            height=60,
            command=lambda: app.show(GuestMenuPage)
        )
        self.guest_btn.pack(side="bottom", pady=40)

    def tkraise(self, aboveThis=None):
        if state.guest_matches_played >= state.guest_max_matches:
            self.guest_btn.configure(
                state="disabled",
                text="FREE TRIALS FINISHED"
            )
        super().tkraise(aboveThis)


# ---------------- LOGIN ----------------
class LoginPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.password_show = False

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(LandingPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="LOGIN",
            font=("Arial Black", 32)
        ).pack(pady=40)

        # Username frame
        username_frame = ctk.CTkFrame(self, fg_color="transparent")
        username_frame.pack(pady=10)

        self.username_entry = ctk.CTkEntry(
            username_frame,
            placeholder_text="Username",
            width=300
        )
        self.username_entry.pack()

        # Password frame
        password_frame = ctk.CTkFrame(self, fg_color="transparent")
        password_frame.pack(pady=10)

        self.password_entry = ctk.CTkEntry(
            password_frame,
            placeholder_text="Password",
            show="*",
            width=260
        )
        self.password_entry.pack(side="left")

        self.eye_btn = ctk.CTkButton(
            password_frame,
            text="👁️",
            width=40,
            command=self.toggle_password
        )
        self.eye_btn.pack(side="left", padx=(5, 0))

        self.error_label = ctk.CTkLabel(self, text="", text_color="red")
        self.error_label.pack(pady=10)

        ctk.CTkButton(
            self,
            text="LOGIN",
            height=50,
            command=self.login
        ).pack(pady=20)

    def toggle_password(self):
        if self.password_show:
            self.password_entry.configure(show="*")
            self.eye_btn.configure(text="👁️")
            self.password_show = False
        else:
            self.password_entry.configure(show="")
            self.eye_btn.configure(text="🙈")
            self.password_show = True

    def login(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()

        users = load_users()
        for user in users:
            if user["username"] == username and user["password"] == password:
                state.current_user = username
                self.app.frames[MainMenuPage].welcome_label.configure(text=f"Welcome, {username}!")
                self.app.show(MainMenuPage)
                return

        self.error_label.configure(text="Invalid username or password")


# ---------------- REGISTER ----------------
class RegisterPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.password_show = False

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(LandingPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="REGISTER",
            font=("Arial Black", 32)
        ).pack(pady=40)

        # Username frame
        username_frame = ctk.CTkFrame(self, fg_color="transparent")
        username_frame.pack(pady=10)

        self.username_entry = ctk.CTkEntry(
            username_frame,
            placeholder_text="Username (at least 3 characters)",
            width=300
        )
        self.username_entry.pack()

        # Password frame
        password_frame = ctk.CTkFrame(self, fg_color="transparent")
        password_frame.pack(pady=10)

        self.password_entry = ctk.CTkEntry(
            password_frame,
            placeholder_text="Password",
            show="*",
            width=260
        )
        self.password_entry.pack(side="left")

        self.eye_btn = ctk.CTkButton(
            password_frame,
            text="👁️",
            width=40,
            command=self.toggle_password
        )
        self.eye_btn.pack(side="left", padx=(5, 0))

        # Password requirements
        req_frame = ctk.CTkFrame(self, fg_color="transparent")
        req_frame.pack(pady=5)
        ctk.CTkLabel(req_frame, text="Password must contain:", font=("Arial", 12)).pack(anchor="w")
        ctk.CTkLabel(req_frame, text="• At least 8 characters", font=("Arial", 10)).pack(anchor="w")
        ctk.CTkLabel(req_frame, text="• 1 uppercase letter", font=("Arial", 10)).pack(anchor="w")
        ctk.CTkLabel(req_frame, text="• 1 lowercase letter", font=("Arial", 10)).pack(anchor="w")
        ctk.CTkLabel(req_frame, text="• 1 number", font=("Arial", 10)).pack(anchor="w")
        ctk.CTkLabel(req_frame, text="• 1 special character", font=("Arial", 10)).pack(anchor="w")

        self.confirm_password_entry = ctk.CTkEntry(
            self,
            placeholder_text="Confirm Password",
            show="*",
            width=300
        )
        self.confirm_password_entry.pack(pady=10)

        self.error_label = ctk.CTkLabel(self, text="", text_color="red")
        self.error_label.pack(pady=10)

        ctk.CTkButton(
            self,
            text="REGISTER",
            height=50,
            command=self.register
        ).pack(pady=20)

    def toggle_password(self):
        if self.password_show:
            self.password_entry.configure(show="*")
            self.eye_btn.configure(text="👁️")
            self.password_show = False
        else:
            self.password_entry.configure(show="")
            self.eye_btn.configure(text="🙈")
            self.password_show = True

    def register(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get()
        confirm = self.confirm_password_entry.get()

        if len(username) < 3:
            self.error_label.configure(text="Username must be at least 3 characters")
            return

        if not self.is_valid_password(password):
            self.error_label.configure(text="Password does not meet requirements")
            return

        if password != confirm:
            self.error_label.configure(text="Passwords do not match")
            return

        users = load_users()
        if any(u["username"] == username for u in users):
            self.error_label.configure(text="Username already exists")
            return

        new_user = {
            "username": username,
            "password": password,
            "played": 0,
            "wins": 0,
            "losses": 0,
            "draws": 0,
            "max_streak": 0
        }
        users.append(new_user)
        save_users(users)

        self.error_label.configure(text="Registration successful! Please login.", text_color="green")
        # Clear fields
        self.username_entry.delete(0, "end")
        self.password_entry.delete(0, "end")
        self.confirm_password_entry.delete(0, "end")

    def is_valid_password(self, password):
        if len(password) < 8:
            return False
        has_upper = any(c.isupper() for c in password)
        has_lower = any(c.islower() for c in password)
        has_digit = any(c.isdigit() for c in password)
        has_special = any(not c.isalnum() for c in password)
        return has_upper and has_lower and has_digit and has_special


# ---------------- GUEST MENU ----------------
class GuestMenuPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="lightblue")

        top = ctk.CTkFrame(self, fg_color="transparent")
        top.pack(fill="x", pady=10)

        ctk.CTkButton(
            top, text="← Back",
            width=100,
            command=lambda: app.show(LandingPage)
        ).pack(side="left", padx=20)

        self.info_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.info_frame.pack(pady=40)

        self.info_emoji = ctk.CTkLabel(
            self.info_frame,
            text="🎮",
            font=("Segoe UI Emoji", 55)
        )
        self.info_emoji.pack(side="left", padx=(0, 10), pady=(0, 13))

        self.info_text = ctk.CTkLabel(
            self.info_frame,
            font=("Arial Black", 35)
        )
        self.info_text.pack(side="left")

        ctk.CTkButton(
            self,
            text="QUICK MATCH (Best of 3)",
            height=60,
            command=lambda: app.show(QuickMatchPage)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="VIEW RULES",
            height=50,
            command=lambda: app.show(RulesPage)
        ).pack(pady=10)

    def tkraise(self, aboveThis=None):
        remaining = state.guest_max_matches - state.guest_matches_played
        self.info_text.configure(
            text=f"Free matches remaining: {remaining} / {state.guest_max_matches}"
        )

        if remaining == 1:
            color = "#f1c40f"
        elif remaining == 0:
            color = "#e74c3c"
        else:
            color = "#2ecc71"

        self.info_emoji.configure(text_color=color)
        self.info_text.configure(text_color=color)

        super().tkraise(aboveThis)


# ---------------- MAIN MENU ----------------
class MainMenuPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="gray10")
        self.app = app
        self.dropdown_visible = False
        self.dropdown_frame = None

        top = ctk.CTkFrame(self, fg_color="transparent")
        top.pack(fill="x", pady=10)

        self.user_btn = ctk.CTkButton(
            top, text="👤",
            width=50,
            command=self.toggle_dropdown
        )
        self.user_btn.pack(side="right", padx=20)

        self.welcome_label = ctk.CTkLabel(
            self,
            text=f"Welcome, {state.current_user or 'Guest'}!",
            font=("Arial Black", 28),
            text_color="cyan"
        )
        self.welcome_label.pack(pady=40)

        ctk.CTkButton(
            self,
            text="PLAY VS COMPUTER",
            height=60,
            fg_color="green",
            command=lambda: app.show(ComputerPlayPage)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="LOCAL MULTIPLAYER",
            height=60,
            fg_color="blue",
            command=lambda: app.show(LocalMultiPage)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="VIEW RULES",
            height=50,
            fg_color="orange",
            command=lambda: app.show(RulesPage)
        ).pack(pady=20)

    def toggle_dropdown(self):
        if self.dropdown_visible:
            self.hide_dropdown()
        else:
            self.show_dropdown()

    def show_dropdown(self):
        if self.dropdown_frame:
            self.dropdown_frame.destroy()

        self.dropdown_frame = ctk.CTkFrame(self, width=150, height=120)
        self.dropdown_frame.place(relx=0.9, rely=0.05, anchor="ne")

        ctk.CTkButton(
            self.dropdown_frame,
            text="Account",
            command=self.go_account
        ).pack(fill="x", pady=2)

        ctk.CTkButton(
            self.dropdown_frame,
            text="View Statistics",
            command=self.go_statistics
        ).pack(fill="x", pady=2)

        ctk.CTkButton(
            self.dropdown_frame,
            text="Logout",
            command=self.logout
        ).pack(fill="x", pady=2)

        self.dropdown_visible = True

    def hide_dropdown(self):
        if self.dropdown_frame:
            self.dropdown_frame.destroy()
            self.dropdown_frame = None
        self.dropdown_visible = False

    def go_account(self):
        self.hide_dropdown()
        self.app.show(AccountPage)

    def go_statistics(self):
        self.hide_dropdown()
        self.app.show(StatisticsPage)

    def logout(self):
        self.hide_dropdown()
        state.current_user = None
        self.welcome_label.configure(text="Welcome, Guest!")
        self.app.show(LandingPage)


# ---------------- ACCOUNT ----------------
class AccountPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="gray12")
        self.app = app
        self.password_show = False

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(MainMenuPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="ACCOUNT",
            font=("Arial Black", 32)
        ).pack(pady=20)

        main_frame = ctk.CTkFrame(self, fg_color="transparent")
        main_frame.pack(pady=20)

        # Left side: current info
        left_frame = ctk.CTkFrame(main_frame)
        left_frame.pack(side="left", padx=20)

        ctk.CTkLabel(left_frame, text="Current Account", font=("Arial Black", 20)).pack(pady=10)

        self.current_username_entry = ctk.CTkEntry(
            left_frame,
            fg_color="white",
            text_color="black",
            state="disabled"
        )
        self.current_username_entry.pack(pady=5)

        self.current_password_entry = ctk.CTkEntry(
            left_frame,
            fg_color="white",
            text_color="black",
            state="disabled"
        )
        self.current_password_entry.pack(pady=5)

        # Bind hover to change cursor
        self.current_username_entry.bind("<Enter>", lambda e: self.current_username_entry.configure(cursor="no"))
        self.current_username_entry.bind("<Leave>", lambda e: self.current_username_entry.configure(cursor=""))

        self.current_password_entry.bind("<Enter>", lambda e: self.current_password_entry.configure(cursor="no"))
        self.current_password_entry.bind("<Leave>", lambda e: self.current_password_entry.configure(cursor=""))

        # Populate current info
        user = get_user_stats(state.current_user)
        if user:
            self.current_username_entry.insert(0, user.get("username", ""))
            self.current_password_entry.insert(0, user.get("password", ""))

        # Right side: update
        right_frame = ctk.CTkFrame(main_frame)
        right_frame.pack(side="right", padx=20)

        ctk.CTkLabel(right_frame, text="Update Account", font=("Arial Black", 20)).pack(pady=10)
        ctk.CTkLabel(right_frame, text="Leave blank to keep current", font=("Arial", 12)).pack(pady=5)

        # Username frame
        username_frame = ctk.CTkFrame(right_frame, fg_color="transparent")
        username_frame.pack(pady=5)

        self.new_username_entry = ctk.CTkEntry(
            username_frame,
            placeholder_text="New Username (at least 3 chars)",
            width=300
        )
        self.new_username_entry.pack()

        # Password frame
        password_frame = ctk.CTkFrame(right_frame, fg_color="transparent")
        password_frame.pack(pady=5)

        self.new_password_entry = ctk.CTkEntry(
            password_frame,
            placeholder_text="New Password",
            show="*",
            width=260
        )
        self.new_password_entry.pack(side="left")

        self.eye_btn = ctk.CTkButton(
            password_frame,
            text="👁️",
            width=40,
            command=self.toggle_password
        )
        self.eye_btn.pack(side="left", padx=(5, 0))

        self.error_label = ctk.CTkLabel(right_frame, text="", text_color="red")
        self.error_label.pack(pady=5)

        ctk.CTkButton(
            right_frame,
            text="SUBMIT",
            command=self.update_account
        ).pack(pady=10)

    def toggle_password(self):
        if self.password_show:
            self.new_password_entry.configure(show="*")
            self.eye_btn.configure(text="👁️")
            self.password_show = False
        else:
            self.new_password_entry.configure(show="")
            self.eye_btn.configure(text="🙈")
            self.password_show = True

    def tkraise(self, aboveThis=None):
        user = get_user_stats(state.current_user)
        if user:
            self.current_username_entry.configure(state="normal")
            self.current_username_entry.delete(0, "end")
            self.current_username_entry.insert(0, user["username"])
            self.current_username_entry.configure(state="disabled")

            self.current_password_entry.configure(state="normal")
            self.current_password_entry.delete(0, "end")
            self.current_password_entry.insert(0, user["password"])
            self.current_password_entry.configure(state="disabled")
        super().tkraise(aboveThis)

    def update_account(self):
        new_username = self.new_username_entry.get().strip()
        new_password = self.new_password_entry.get().strip()

        user = get_user_stats(state.current_user)
        if not user:
            return

        updates = {}
        error = ""

        if new_username:
            if len(new_username) < 3:
                error = "Username must be at least 3 characters"
            else:
                # Check uniqueness
                users = load_users()
                if any(u["username"] == new_username and u["username"] != state.current_user for u in users):
                    error = "Username already taken"
                else:
                    updates["username"] = new_username
                    state.current_user = new_username

        if new_password:
            if not self.is_valid_password(new_password):
                error = "Password does not meet requirements"
            else:
                updates["password"] = new_password

        if error:
            self.error_label.configure(text=error)
        else:
            if updates:
                update_user(user["username"], updates)
                self.error_label.configure(text="Account updated successfully")
            else:
                self.error_label.configure(text="No changes made")
            self.new_username_entry.delete(0, "end")
            self.new_password_entry.delete(0, "end")

    def is_valid_password(self, password):
        if len(password) < 8:
            return False
        has_upper = any(c.isupper() for c in password)
        has_lower = any(c.islower() for c in password)
        has_digit = any(c.isdigit() for c in password)
        has_special = any(not c.isalnum() for c in password)
        return has_upper and has_lower and has_digit and has_special


# ---------------- STATISTICS ----------------
class StatisticsPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="gray8")
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(MainMenuPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="STATISTICS",
            font=("Arial Black", 32)
        ).pack(pady=20)

        self.stats_frame = ctk.CTkFrame(self)
        self.stats_frame.pack(pady=20, padx=20, fill="both", expand=True)

    def tkraise(self, aboveThis=None):
        for w in self.stats_frame.winfo_children():
            w.destroy()

        user = get_user_stats(state.current_user)
        if user:
            ctk.CTkLabel(
                self.stats_frame,
                text=f"Total Matches: {user.get('played', 0)}",
                font=("Arial", 20)
            ).pack(pady=5)

            ctk.CTkLabel(
                self.stats_frame,
                text=f"Total Wins: {user.get('wins', 0)}",
                font=("Arial", 20)
            ).pack(pady=5)

            ctk.CTkLabel(
                self.stats_frame,
                text=f"Total Losses: {user.get('losses', 0)}",
                font=("Arial", 20)
            ).pack(pady=5)

            ctk.CTkLabel(
                self.stats_frame,
                text=f"Max Streak: {user.get('max_streak', 0)}",
                font=("Arial", 20)
            ).pack(pady=5)

            ctk.CTkLabel(
                self.stats_frame,
                text="Last 5 Matches:",
                font=("Arial Black", 18)
            ).pack(pady=10)

            last_matches = get_last_5_matches(state.current_user)
            for match in reversed(last_matches):
                result = match.get("result", "UNKNOWN")
                opponent = match.get("opponent", "Unknown")
                ctk.CTkLabel(
                    self.stats_frame,
                    text=f"vs {opponent}: {result}",
                    font=("Arial", 16)
                ).pack(pady=2)

        super().tkraise(aboveThis)


# ---------------- RULES ----------------
class RulesPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent, fg_color="gray20")

        self.back_command = lambda: app.show(GuestMenuPage) if state.current_user is None else app.show(MainMenuPage)

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=self.back_command
        ).pack(anchor="nw", padx=20, pady=15)

        self.icon_label = ctk.CTkLabel(
            self,
            text="🪨",
            font=("Segoe UI Emoji", 90)
        )
        self.icon_label.pack(pady=(20, 10))

        ctk.CTkLabel(
            self,
            text="RULES",
            font=("Arial Black", 26)
        ).pack(pady=(0, 20))

        rules_box = ctk.CTkFrame(self, fg_color="transparent")
        rules_box.pack(pady=10)

        self.make_rule(rules_box, "🪨", "ROCK", "CRUSHES", "✂️", "SCISSORS")
        self.make_rule(rules_box, "✂️", "SCISSORS", "CUTS", "📄", "PAPER")
        self.make_rule(rules_box, "📄", "PAPER", "COVERS", "🪨", "ROCK")

        self.icons = ["🪨", "📄", "✂️"]
        self.anim_index = 0
        self.animate_icon()

    def animate_icon(self):
        self.icon_label.configure(text=self.icons[self.anim_index])
        self.anim_index = (self.anim_index + 1) % 3
        self.after(1400, self.animate_icon)

    def make_rule(self, parent, le, lt, act, re, rt):
        row = ctk.CTkFrame(parent, fg_color="transparent")
        row.pack(pady=14)

        ctk.CTkLabel(
            row, text=f"{le}  {lt}",
            font=("Segoe UI Emoji", 28),
            width=220, anchor="e"
        ).pack(side="left", padx=10)

        ctk.CTkLabel(
            row, text=act,
            font=("Arial Black", 22),
            width=140, text_color="#bbbbbb"
        ).pack(side="left", padx=10)

        ctk.CTkLabel(
            row, text=f"{re}  {rt}",
            font=("Segoe UI Emoji", 28),
            width=220, anchor="w"
        ).pack(side="left", padx=10)


# ---------------- QUICK MATCH ----------------
class QuickMatchPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        # best of 3 logic
        self.rounds_played = 0          # only non-draw rounds
        self.max_rounds = 3
        self.wins_needed = 2

        self.build_ui()

    def build_ui(self):
        self.header = ctk.CTkLabel(
            self,
            text=self.round_text(),
            font=("Arial Black", 32)
        )
        self.header.pack(pady=20)

        self.score = ctk.CTkLabel(self, font=("Arial", 22))
        self.score.pack(pady=10)
        self.update_score()

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,
                font=("Segoe UI Emoji", 38),
                command=lambda v=val: self.select(v)
            )
            b.pack(side="left", padx=12)
            self.buttons[val] = b

        ctk.CTkButton(
            self, text="PLAY ROUND", height=50,
            command=self.play_round
        ).pack(pady=20)

        ctk.CTkButton(
            self, text="FINISH MATCH",
            fg_color="red",
            command=self.finish_match
        ).pack(side="bottom", pady=30)

    def round_text(self):
        return f"ROUND {self.rounds_played + 1} OF {self.max_rounds}"

    def select(self, val):
        state.selected = val
        for v, b in self.buttons.items():
            if v == val:
                b.configure(
                    fg_color="#2ecc71",
                    border_width=2,
                    border_color="#a8f0c6"
                )
            else:
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )

    def play_round(self):
        if state.selected is None:
            return

        for w in self.winfo_children():
            w.place_forget()
            w.pack_forget()

        self.animate_rps_sequence()

    # ---------------- ANIMATION ----------------
    def animate_rps_sequence(self):
        font = ("Arial Black", 110)

        self.lbl_rock = ctk.CTkLabel(self, text="ROCK", font=font)
        self.lbl_paper = ctk.CTkLabel(self, text="PAPER", font=font)
        self.lbl_scissors = ctk.CTkLabel(self, text="SCISSORS", font=font)

        self.lbl_rock.place(relx=0.5, y=60, anchor="center")
        self.after(800, self.show_paper)

    def show_paper(self):
        self.lbl_paper.place(relx=0.5, rely=0.5, anchor="center")
        self.after(800, self.show_scissors)

    def show_scissors(self):
        self.lbl_scissors.place(relx=0.5, rely=0.9, anchor="center")
        self.after(800, self.clear_rps)

    def clear_rps(self):
        self.lbl_rock.destroy()
        self.lbl_paper.destroy()
        self.lbl_scissors.destroy()
        self.show_shoot()

    def show_shoot(self):
        self.shoot_size = 30
        self.shoot_lbl = ctk.CTkLabel(
            self, text="🪨 📄 ✂️",
            font=("Arial Black", self.shoot_size)
        )
        self.shoot_lbl.place(relx=0.5, rely=0.5, anchor="center")
        self.zoom_shoot()

    def zoom_shoot(self):
        self.shoot_size += 6
        self.shoot_lbl.configure(font=("Arial Black", self.shoot_size))

        if self.shoot_size > 180:
            self.shoot_lbl.destroy()
            self.after(300, self.resolve_round)
        else:
            self.after(30, self.zoom_shoot)

    # ---------------- GAME LOGIC ----------------
    def resolve_round(self):
        comp = random.randint(1, 3)
        names = {ROCK: "ROCK", PAPER: "PAPER", SCISSORS: "SCISSORS"}

        # determine result
        if state.selected == comp:
            result = "DRAW 🤝"
            counted = False
        elif (state.selected - comp) % 3 == 1:
            state.player_score += 1
            self.rounds_played += 1
            result = "YOU WON THE ROUND 🎉"
            counted = True
        else:
            state.computer_score += 1
            self.rounds_played += 1
            result = "YOU LOST THE ROUND 😞"
            counted = True

        box = ctk.CTkFrame(self)
        box.place(relx=0.5, rely=0.5, anchor="center")

        ctk.CTkLabel(box, text=f"You selected: {names[state.selected]}", font=("Arial Black", 26)).pack(pady=6)
        ctk.CTkLabel(box, text=f"Computer selected: {names[comp]}", font=("Arial Black", 26)).pack(pady=6)
        ctk.CTkLabel(box, text=result, font=("Arial Black", 30)).pack(pady=14)

        # check match end
        if state.player_score == self.wins_needed or state.computer_score == self.wins_needed:
            self.after(4000, lambda: self.end_match(box))
        else:
            self.after(4000, lambda: self.reset_after_round(box))

    def reset_after_round(self, box):
        box.destroy()
        state.selected = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def update_score(self):
        if state.current_user:
            self.score.configure(
                text=f"{state.current_user} {state.player_score}  |  Computer {state.computer_score}"
            )
        else:
            self.score.configure(
                text=f"Guest {state.player_score}  |  Computer {state.computer_score}"
            )

    def end_match(self, box):
        box.destroy()
        self.finish_match()

    def finish_match(self):
        if state.current_user:
            # Update stats
            user = get_user_stats(state.current_user)
            if user:
                if state.player_score > state.computer_score:
                    result = "WIN"
                    update_user(state.current_user, {
                        "wins": user.get("wins", 0) + 1
                    })
                elif state.player_score < state.computer_score:
                    result = "LOSS"
                    update_user(state.current_user, {
                        "losses": user.get("losses", 0) + 1
                    })
                else:
                    result = "DRAW"

                update_user(state.current_user, {
                    "played": user.get("played", 0) + 1
                })

                save_stat({
                    "user": state.current_user,
                    "result": result,
                    "opponent": "Computer",
                    "timestamp": datetime.now().isoformat()
                })

            self.app.show(MainMenuPage)
        else:
            state.guest_matches_played += 1
            state.player_score = 0
            state.computer_score = 0
            state.selected = None
            self.app.show(LandingPage)


# ---------------- COMPUTER PLAY ----------------
class ComputerPlayPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(MainMenuPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="PLAY VS COMPUTER",
            font=("Arial Black", 32)
        ).pack(pady=40)

        ctk.CTkButton(
            self,
            text="QUICK MATCH (Best of 3)",
            height=60,
            command=lambda: app.show(QuickMatchPage)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="SURVIVAL MATCH",
            height=60,
            command=lambda: app.show(SurvivalMatchPage)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="CUSTOM MATCH",
            height=60,
            command=lambda: app.show(CustomMatchPage)
        ).pack(pady=20)


# ---------------- SURVIVAL MATCH ----------------
class SurvivalMatchPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.streak = 0

        self.build_ui()

    def build_ui(self):
        self.header = ctk.CTkLabel(
            self,
            text=f"SURVIVAL MODE - Streak: {self.streak}",
            font=("Arial Black", 32)
        )
        self.header.pack(pady=20)

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,
                font=("Segoe UI Emoji", 38),
                command=lambda v=val: self.select(v)
            )
            b.pack(side="left", padx=12)
            self.buttons[val] = b

        ctk.CTkButton(
            self, text="PLAY ROUND", height=50,
            command=self.play_round
        ).pack(pady=20)

        ctk.CTkButton(
            self, text="FINISH MATCH",
            fg_color="red",
            command=self.finish_match
        ).pack(side="bottom", pady=30)

    def select(self, val):
        state.selected = val
        for v, b in self.buttons.items():
            if v == val:
                b.configure(
                    fg_color="#2ecc71",
                    border_width=2,
                    border_color="#a8f0c6"
                )
            else:
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )

    def play_round(self):
        if state.selected is None:
            return

        for w in self.winfo_children():
            w.place_forget()
            w.pack_forget()

        self.animate_rps_sequence()

    def animate_rps_sequence(self):
        font = ("Arial Black", 110)

        self.lbl_rock = ctk.CTkLabel(self, text="ROCK", font=font)
        self.lbl_paper = ctk.CTkLabel(self, text="PAPER", font=font)
        self.lbl_scissors = ctk.CTkLabel(self, text="SCISSORS", font=font)

        self.lbl_rock.place(relx=0.5, y=60, anchor="center")
        self.after(800, self.show_paper)

    def show_paper(self):
        self.lbl_paper.place(relx=0.5, rely=0.5, anchor="center")
        self.after(800, self.show_scissors)

    def show_scissors(self):
        self.lbl_scissors.place(relx=0.5, rely=0.9, anchor="center")
        self.after(800, self.clear_rps)

    def clear_rps(self):
        self.lbl_rock.destroy()
        self.lbl_paper.destroy()
        self.lbl_scissors.destroy()
        self.show_shoot()

    def show_shoot(self):
        self.shoot_size = 30
        self.shoot_lbl = ctk.CTkLabel(
            self, text="🪨 📄 ✂️",
            font=("Arial Black", self.shoot_size)
        )
        self.shoot_lbl.place(relx=0.5, rely=0.5, anchor="center")
        self.zoom_shoot()

    def zoom_shoot(self):
        self.shoot_size += 6
        self.shoot_lbl.configure(font=("Arial Black", self.shoot_size))

        if self.shoot_size > 180:
            self.shoot_lbl.destroy()
            self.after(300, self.resolve_round)
        else:
            self.after(30, self.zoom_shoot)

    def resolve_round(self):
        comp = random.randint(1, 3)
        names = {ROCK: "ROCK", PAPER: "PAPER", SCISSORS: "SCISSORS"}

        if state.selected == comp:
            result = "DRAW 🤝"
            self.streak += 0  # no change
        elif (state.selected - comp) % 3 == 1:
            result = "YOU WON THE ROUND 🎉"
            self.streak += 1
        else:
            result = "YOU LOST THE ROUND 😞"
            # end match
            self.after(4000, lambda: self.end_match())
            return

        box = ctk.CTkFrame(self)
        box.place(relx=0.5, rely=0.5, anchor="center")

        ctk.CTkLabel(box, text=f"You selected: {names[state.selected]}", font=("Arial Black", 26)).pack(pady=6)
        ctk.CTkLabel(box, text=f"Computer selected: {names[comp]}", font=("Arial Black", 26)).pack(pady=6)
        ctk.CTkLabel(box, text=result, font=("Arial Black", 30)).pack(pady=14)

        self.after(4000, lambda: self.reset_after_round(box))

    def reset_after_round(self, box):
        box.destroy()
        state.selected = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def end_match(self):
        # Update max_streak
        user = get_user_stats(state.current_user)
        if user and self.streak > user.get("max_streak", 0):
            update_user(state.current_user, {"max_streak": self.streak})

        # Save stat
        save_stat({
            "user": state.current_user,
            "result": "LOSS",
            "opponent": "Computer",
            "timestamp": datetime.now().isoformat()
        })

        # Update played and losses
        update_user(state.current_user, {
            "played": user.get("played", 0) + 1,
            "losses": user.get("losses", 0) + 1
        })

        self.app.show(MainMenuPage)

    def finish_match(self):
        # If user finishes early, don't count as loss
        self.app.show(MainMenuPage)


# ---------------- CUSTOM MATCH ----------------
class CustomMatchPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(ComputerPlayPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="CUSTOM MATCH",
            font=("Arial Black", 32)
        ).pack(pady=40)

        self.rounds_entry = ctk.CTkEntry(
            self,
            placeholder_text="Number of rounds (1-10)",
            width=300
        )
        self.rounds_entry.pack(pady=20)

        self.error_label = ctk.CTkLabel(self, text="", text_color="red")
        self.error_label.pack(pady=10)

        ctk.CTkButton(
            self,
            text="START MATCH",
            height=50,
            command=self.start_match
        ).pack(pady=20)

    def start_match(self):
        try:
            rounds = int(self.rounds_entry.get().strip())
            if 1 <= rounds <= 10:
                # Create a custom match page with the rounds
                self.app.frames[CustomGamePage] = CustomGamePage(self.app.container, self.app, rounds)
                self.app.frames[CustomGamePage].place(relwidth=1, relheight=1)
                self.app.show(CustomGamePage)
            else:
                self.error_label.configure(text="Number of rounds must be between 1 and 10")
        except ValueError:
            self.error_label.configure(text="Please enter a valid number")


# ---------------- CUSTOM GAME ----------------
class CustomGamePage(ctk.CTkFrame):
    def __init__(self, parent, app, max_rounds):
        super().__init__(parent)
        self.app = app
        self.max_rounds = max_rounds
        self.rounds_played = 0
        self.wins_needed = (max_rounds // 2) + 1

        self.build_ui()

    def build_ui(self):
        self.header = ctk.CTkLabel(
            self,
            text=self.round_text(),
            font=("Arial Black", 32)
        )
        self.header.pack(pady=20)

        self.score = ctk.CTkLabel(self, font=("Arial", 22))
        self.score.pack(pady=10)
        self.update_score()

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,
                font=("Segoe UI Emoji", 38),
                command=lambda v=val: self.select(v)
            )
            b.pack(side="left", padx=12)
            self.buttons[val] = b

        ctk.CTkButton(
            self, text="PLAY ROUND", height=50,
            command=self.play_round
        ).pack(pady=20)

        ctk.CTkButton(
            self, text="FINISH MATCH",
            fg_color="red",
            command=self.finish_match
        ).pack(side="bottom", pady=30)

    def round_text(self):
        return f"ROUND {self.rounds_played + 1} OF {self.max_rounds}"

    def select(self, val):
        state.selected = val
        for v, b in self.buttons.items():
            if v == val:
                b.configure(
                    fg_color="#2ecc71",
                    border_width=2,
                    border_color="#a8f0c6"
                )
            else:
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )

    def play_round(self):
        if state.selected is None:
            return

        for w in self.winfo_children():
            w.place_forget()
            w.pack_forget()

        self.animate_rps_sequence()

    def animate_rps_sequence(self):
        font = ("Arial Black", 110)

        self.lbl_rock = ctk.CTkLabel(self, text="ROCK", font=font)
        self.lbl_paper = ctk.CTkLabel(self, text="PAPER", font=font)
        self.lbl_scissors = ctk.CTkLabel(self, text="SCISSORS", font=font)

        self.lbl_rock.place(relx=0.5, y=60, anchor="center")
        self.after(800, self.show_paper)

    def show_paper(self):
        self.lbl_paper.place(relx=0.5, rely=0.5, anchor="center")
        self.after(800, self.show_scissors)

    def show_scissors(self):
        self.lbl_scissors.place(relx=0.5, rely=0.9, anchor="center")
        self.after(800, self.clear_rps)

    def clear_rps(self):
        self.lbl_rock.destroy()
        self.lbl_paper.destroy()
        self.lbl_scissors.destroy()
        self.show_shoot()

    def show_shoot(self):
        self.shoot_size = 30
        self.shoot_lbl = ctk.CTkLabel(
            self, text="🪨 📄 ✂️",
            font=("Arial Black", self.shoot_size)
        )
        self.shoot_lbl.place(relx=0.5, rely=0.5, anchor="center")
        self.zoom_shoot()

    def zoom_shoot(self):
        self.shoot_size += 6
        self.shoot_lbl.configure(font=("Arial Black", self.shoot_size))

        if self.shoot_size > 180:
            self.shoot_lbl.destroy()
            self.after(300, self.resolve_round)
        else:
            self.after(30, self.zoom_shoot)

    def resolve_round(self):
        comp = random.randint(1, 3)
        names = {ROCK: "ROCK", PAPER: "PAPER", SCISSORS: "SCISSORS"}

        # determine result
        if state.selected == comp:
            result = "DRAW 🤝"
            counted = False
        elif (state.selected - comp) % 3 == 1:
            state.player_score += 1
            self.rounds_played += 1
            result = "YOU WON THE ROUND 🎉"
            counted = True
        else:
            state.computer_score += 1
            self.rounds_played += 1
            result = "YOU LOST THE ROUND 😞"
            counted = True

        if result == "YOU LOST THE ROUND 😞":
            # Show direct result for loss
            self.result_label1 = ctk.CTkLabel(self, text=f"You selected: {names[state.selected]}", font=("Arial Black", 26))
            self.result_label1.place(relx=0.5, rely=0.3, anchor="center")

            self.result_label2 = ctk.CTkLabel(self, text=f"Computer selected: {names[comp]}", font=("Arial Black", 26))
            self.result_label2.place(relx=0.5, rely=0.5, anchor="center")

            self.result_label3 = ctk.CTkLabel(self, text="You lose the match!", font=("Arial Black", 30))
            self.result_label3.place(relx=0.5, rely=0.7, anchor="center")

            self.after(5000, self.end_survival_match)
        else:
            box = ctk.CTkFrame(self)
            box.place(relx=0.5, rely=0.5, anchor="center")

            ctk.CTkLabel(box, text=f"You selected: {names[state.selected]}", font=("Arial Black", 26)).pack(pady=6)
            ctk.CTkLabel(box, text=f"Computer selected: {names[comp]}", font=("Arial Black", 26)).pack(pady=6)
            ctk.CTkLabel(box, text=result, font=("Arial Black", 30)).pack(pady=14)

            # check match end
            if state.player_score == self.wins_needed or state.computer_score == self.wins_needed or self.rounds_played == self.max_rounds:
                self.after(4000, lambda: self.end_match(box))
            else:
                self.after(4000, lambda: self.reset_after_round(box))

    def reset_after_round(self, box):
        box.destroy()
        state.selected = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def update_score(self):
        self.score.configure(
            text=f"{state.current_user} {state.player_score}  |  Computer {state.computer_score}"
        )

    def end_match(self, box):
        box.destroy()
        self.finish_match()

    def finish_match(self):
        # Update stats
        user = get_user_stats(state.current_user)
        if user:
            if state.player_score > state.computer_score:
                result = "WIN"
                update_user(state.current_user, {
                    "wins": user.get("wins", 0) + 1
                })
            elif state.player_score < state.computer_score:
                result = "LOSS"
                update_user(state.current_user, {
                    "losses": user.get("losses", 0) + 1
                })
            else:
                result = "DRAW"

            update_user(state.current_user, {
                "played": user.get("played", 0) + 1
            })

            save_stat({
                "user": state.current_user,
                "result": result,
                "opponent": "Computer",
                "timestamp": datetime.now().isoformat()
            })

        state.player_score = 0
        state.computer_score = 0
        state.selected = None
        self.app.show(MainMenuPage)

    def end_survival_match(self):
        self.result_label1.destroy()
        self.result_label2.destroy()
        self.result_label3.destroy()
        self.finish_survival_match()

    def finish_survival_match(self):
        # Update stats for loss
        user = get_user_stats(state.current_user)
        if user:
            result = "LOSS"
            update_user(state.current_user, {
                "losses": user.get("losses", 0) + 1
            })

            update_user(state.current_user, {
                "played": user.get("played", 0) + 1
            })

            save_stat({
                "user": state.current_user,
                "result": result,
                "opponent": "Computer",
                "timestamp": datetime.now().isoformat()
            })

        state.player_score = 0
        state.computer_score = 0
        state.selected = None
        self.app.show(ComputerPlayPage)


# ---------------- LOCAL MULTIPLAYER ----------------
class LocalMultiPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(MainMenuPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="LOCAL MULTIPLAYER",
            font=("Arial Black", 32)
        ).pack(pady=40)

        self.player1_entry = ctk.CTkEntry(
            self,
            placeholder_text="Player 1 Name",
            width=300
        )
        self.player1_entry.pack(pady=10)

        self.player2_entry = ctk.CTkEntry(
            self,
            placeholder_text="Player 2 Name",
            width=300
        )
        self.player2_entry.pack(pady=10)

        self.error_label = ctk.CTkLabel(self, text="", text_color="red")
        self.error_label.pack(pady=10)

        ctk.CTkButton(
            self,
            text="ENTER",
            height=50,
            command=self.enter_names
        ).pack(pady=20)

    def enter_names(self):
        p1 = self.player1_entry.get().strip()
        p2 = self.player2_entry.get().strip()

        if not p1 or not p2:
            self.error_label.configure(text="Please enter both names")
            return

        if p1 == p2:
            self.error_label.configure(text="Names must be different")
            return

        # Store names in state
        state.player1_name = p1
        state.player2_name = p2

        self.app.show(LocalGameModePage)


# ---------------- LOCAL GAME MODE ----------------
class LocalGameModePage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(LocalMultiPage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="CHOOSE GAME MODE",
            font=("Arial Black", 32)
        ).pack(pady=40)

        ctk.CTkButton(
            self,
            text="QUICK MATCH (Best of 3)",
            height=60,
            command=lambda: self.start_quick_match()
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="CUSTOM MATCH",
            height=60,
            command=lambda: self.start_custom_match()
        ).pack(pady=20)

    def start_quick_match(self):
        self.app.frames[LocalQuickMatchPage] = LocalQuickMatchPage(self.app.container, self.app)
        self.app.frames[LocalQuickMatchPage].place(relwidth=1, relheight=1)
        self.app.show(LocalQuickMatchPage)

    def start_custom_match(self):
        self.app.frames[LocalCustomMatchPage] = LocalCustomMatchPage(self.app.container, self.app)
        self.app.frames[LocalCustomMatchPage].place(relwidth=1, relheight=1)
        self.app.show(LocalCustomMatchPage)


# ---------------- LOCAL QUICK MATCH ----------------
class LocalQuickMatchPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        self.rounds_played = 0
        self.max_rounds = 3
        self.wins_needed = 2
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None

        self.build_ui()

    def build_ui(self):
        self.header = ctk.CTkLabel(
            self,
            text=f"ROUND {self.rounds_played + 1} OF {self.max_rounds}",
            font=("Arial Black", 32)
        )
        self.header.pack(pady=20)

        self.score = ctk.CTkLabel(self, font=("Arial", 22))
        self.score.pack(pady=10)
        self.update_score()

        self.player_label = ctk.CTkLabel(
            self,
            text=f"{state.player1_name if self.current_player == 1 else state.player2_name}, choose:",
            font=("Arial Black", 28)
        )
        self.player_label.pack(pady=20)

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,
                font=("Segoe UI Emoji", 38),
                command=lambda v=val: self.select(v)
            )
            b.pack(side="left", padx=12)
            self.buttons[val] = b

        self.play_btn = ctk.CTkButton(
            self, text="PLAY ROUND", height=50,
            command=self.play_round, state="disabled"
        )
        self.play_btn.pack(pady=20)

        ctk.CTkButton(
            self, text="FINISH MATCH",
            fg_color="red",
            command=self.finish_match
        ).pack(side="bottom", pady=30)

    def select(self, val):
        if self.current_player == 1:
            self.player1_choice = val
        else:
            self.player2_choice = val

        for v, b in self.buttons.items():
            if v == val:
                b.configure(
                    fg_color="#2ecc71",
                    border_width=2,
                    border_color="#a8f0c6"
                )
            else:
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )

        self.play_btn.configure(state="normal")

    def play_round(self):
        if self.current_player == 1 and self.player1_choice is None:
            return
        if self.current_player == 2 and self.player2_choice is None:
            return

        if self.current_player == 1:
            self.current_player = 2
            self.player_label.configure(text=f"{state.player2_name}, choose:")
            self.play_btn.configure(state="disabled")
            # Reset buttons
            for b in self.buttons.values():
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )
        else:
            # Both have chosen, animate
            self.animate_rps_sequence()

    def show_result(self):
        names = {ROCK: "ROCK", PAPER: "PAPER", SCISSORS: "SCISSORS"}

        p1 = self.player1_choice
        p2 = self.player2_choice

        if p1 == p2:
            result = "DRAW 🤝"
        elif (p1 - p2) % 3 == 1:
            state.player_score += 1
            self.rounds_played += 1
            result = f"{state.player1_name} WON THE ROUND 🎉"
        else:
            state.computer_score += 1
            self.rounds_played += 1
            result = f"{state.player2_name} WON THE ROUND 🎉"

        self.result_label1 = ctk.CTkLabel(self, text=f"{state.player1_name} selected: {names[p1]}", font=("Arial Black", 26))
        self.result_label1.place(relx=0.5, rely=0.3, anchor="center")

        self.result_label2 = ctk.CTkLabel(self, text=f"{state.player2_name} selected: {names[p2]}", font=("Arial Black", 26))
        self.result_label2.place(relx=0.5, rely=0.5, anchor="center")

        self.result_label3 = ctk.CTkLabel(self, text=result, font=("Arial Black", 30))
        self.result_label3.place(relx=0.5, rely=0.7, anchor="center")

        if state.player_score == self.wins_needed or state.computer_score == self.wins_needed or self.rounds_played == self.max_rounds:
            self.after(5000, self.end_match)
        else:
            self.after(5000, self.rebuild_ui)

    def rebuild_ui(self):
        self.result_label1.destroy()
        self.result_label2.destroy()
        self.result_label3.destroy()
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None
        self.build_ui()

    def reset_after_round(self, box):
        box.destroy()
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def update_score(self):
        self.score.configure(
            text=f"{state.player1_name} {state.player_score}  |  {state.player2_name} {state.computer_score}"
        )

    def end_match(self):
        # Save match result if logged in
        if state.current_user:
            if state.player_score > state.computer_score:
                result = "WIN"
            elif state.player_score < state.computer_score:
                result = "LOSS"
            else:
                result = "DRAW"
            save_stat({
                "user": state.current_user,
                "result": result,
                "opponent": f"Local Player ({state.player2_name})",
                "timestamp": datetime.now().isoformat()
            })
        # No stats update for local multiplayer
        state.player_score = 0
        state.computer_score = 0
        self.app.show(MainMenuPage)

    def finish_match(self):
        state.player_score = 0
        state.computer_score = 0
        self.app.show(MainMenuPage)

    # ---------------- ANIMATION ----------------
    def animate_rps_sequence(self):
        # Clear the entire screen
        for w in self.winfo_children():
            w.destroy()

        font = ("Arial Black", 110)

        self.lbl_rock = ctk.CTkLabel(self, text="ROCK", font=font)
        self.lbl_paper = ctk.CTkLabel(self, text="PAPER", font=font)
        self.lbl_scissors = ctk.CTkLabel(self, text="SCISSORS", font=font)

        self.lbl_rock.place(relx=0.5, y=90, anchor="center")
        self.update()
        self.after(800, self.show_paper)

    def show_paper(self):
        self.lbl_paper.place(relx=0.5, rely=0.5, anchor="center")
        self.update()
        self.after(800, self.show_scissors)

    def show_scissors(self):
        self.lbl_scissors.place(relx=0.5, rely=0.9, anchor="center")
        self.update()
        self.after(800, self.clear_rps)

    def clear_rps(self):
        self.lbl_rock.destroy()
        self.lbl_paper.destroy()
        self.lbl_scissors.destroy()
        self.show_shoot()

    def show_shoot(self):
        self.shoot_size = 30
        self.shoot_lbl = ctk.CTkLabel(
            self, text="🪨 📄 ✂️",
            font=("Arial Black", self.shoot_size)
        )
        self.shoot_lbl.place(relx=0.5, rely=0.5, anchor="center")
        self.zoom_shoot()

    def zoom_shoot(self):
        self.shoot_size += 6
        self.shoot_lbl.configure(font=("Arial Black", self.shoot_size))

        if self.shoot_size > 180:
            self.shoot_lbl.destroy()
            self.after(300, self.show_result)
        else:
            self.after(30, self.zoom_shoot)


# ---------------- LOCAL CUSTOM MATCH ----------------
class LocalCustomMatchPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(LocalGameModePage)
        ).pack(anchor="nw", padx=20, pady=15)

        ctk.CTkLabel(
            self,
            text="CUSTOM MATCH",
            font=("Arial Black", 32)
        ).pack(pady=40)

        self.rounds_entry = ctk.CTkEntry(
            self,
            placeholder_text="Number of rounds (1-10)",
            width=300
        )
        self.rounds_entry.pack(pady=20)

        self.error_label = ctk.CTkLabel(self, text="", text_color="red")
        self.error_label.pack(pady=10)

        ctk.CTkButton(
            self,
            text="START MATCH",
            height=50,
            command=self.start_match
        ).pack(pady=20)

    def start_match(self):
        try:
            rounds = int(self.rounds_entry.get().strip())
            if 1 <= rounds <= 10:
                self.app.frames[LocalCustomGamePage] = LocalCustomGamePage(self.app.container, self.app, rounds)
                self.app.frames[LocalCustomGamePage].place(relwidth=1, relheight=1)
                self.app.show(LocalCustomGamePage)
            else:
                self.error_label.configure(text="Number of rounds must be between 1 and 10")
        except ValueError:
            self.error_label.configure(text="Please enter a valid number")


# ---------------- LOCAL CUSTOM GAME ----------------
class LocalCustomGamePage(ctk.CTkFrame):
    def __init__(self, parent, app, max_rounds):
        super().__init__(parent)
        self.app = app
        self.max_rounds = max_rounds
        self.rounds_played = 0
        self.wins_needed = (max_rounds // 2) + 1
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None

        self.build_ui()

    def build_ui(self):
        self.header = ctk.CTkLabel(
            self,
            text=f"ROUND {self.rounds_played + 1} OF {self.max_rounds}",
            font=("Arial Black", 32)
        )
        self.header.pack(pady=20)

        self.score = ctk.CTkLabel(self, font=("Arial", 22))
        self.score.pack(pady=10)
        self.update_score()

        self.player_label = ctk.CTkLabel(
            self,
            text=f"{state.player1_name if self.current_player == 1 else state.player2_name}, choose:",
            font=("Arial Black", 28)
        )
        self.player_label.pack(pady=20)

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,
                font=("Segoe UI Emoji", 38),
                command=lambda v=val: self.select(v)
            )
            b.pack(side="left", padx=12)
            self.buttons[val] = b

        self.play_btn = ctk.CTkButton(
            self, text="PLAY ROUND", height=50,
            command=self.play_round, state="disabled"
        )
        self.play_btn.pack(pady=20)

        ctk.CTkButton(
            self, text="FINISH MATCH",
            fg_color="red",
            command=self.finish_match
        ).pack(side="bottom", pady=30)

    def select(self, val):
        if self.current_player == 1:
            self.player1_choice = val
        else:
            self.player2_choice = val

        for v, b in self.buttons.items():
            if v == val:
                b.configure(
                    fg_color="#2ecc71",
                    border_width=2,
                    border_color="#a8f0c6"
                )
            else:
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )

        self.play_btn.configure(state="normal")

    def play_round(self):
        if self.current_player == 1 and self.player1_choice is None:
            return
        if self.current_player == 2 and self.player2_choice is None:
            return

        if self.current_player == 1:
            self.current_player = 2
            self.player_label.configure(text=f"{state.player2_name}, choose:")
            self.play_btn.configure(state="disabled")
            # Reset buttons
            for b in self.buttons.values():
                b.configure(
                    fg_color="transparent",
                    border_width=0
                )
        else:
            # Both have chosen, animate
            self.animate_rps_sequence()

    def show_result(self):
        names = {ROCK: "ROCK", PAPER: "PAPER", SCISSORS: "SCISSORS"}

        p1 = self.player1_choice
        p2 = self.player2_choice

        if p1 == p2:
            result = "DRAW 🤝"
        elif (p1 - p2) % 3 == 1:
            state.player_score += 1
            self.rounds_played += 1
            result = f"{state.player1_name} WON THE ROUND 🎉"
        else:
            state.computer_score += 1
            self.rounds_played += 1
            result = f"{state.player2_name} WON THE ROUND 🎉"

        self.result_label1 = ctk.CTkLabel(self, text=f"{state.player1_name} selected: {names[p1]}", font=("Arial Black", 26))
        self.result_label1.place(relx=0.5, rely=0.3, anchor="center")

        self.result_label2 = ctk.CTkLabel(self, text=f"{state.player2_name} selected: {names[p2]}", font=("Arial Black", 26))
        self.result_label2.place(relx=0.5, rely=0.5, anchor="center")

        self.result_label3 = ctk.CTkLabel(self, text=result, font=("Arial Black", 30))
        self.result_label3.place(relx=0.5, rely=0.7, anchor="center")

        if state.player_score == self.wins_needed or state.computer_score == self.wins_needed or self.rounds_played == self.max_rounds:
            self.after(5000, self.end_match)
        else:
            self.after(5000, self.rebuild_ui)

    def rebuild_ui(self):
        self.result_label1.destroy()
        self.result_label2.destroy()
        self.result_label3.destroy()
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None
        self.build_ui()

    def reset_after_round(self, box):
        box.destroy()
        self.current_player = 1
        self.player1_choice = None
        self.player2_choice = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def update_score(self):
        self.score.configure(
            text=f"{state.player1_name} {state.player_score}  |  {state.player2_name} {state.computer_score}"
        )

    def end_match(self):
        # Save match result if logged in
        if state.current_user:
            if state.player_score > state.computer_score:
                result = "WIN"
            elif state.player_score < state.computer_score:
                result = "LOSS"
            else:
                result = "DRAW"
            save_stat({
                "user": state.current_user,
                "result": result,
                "opponent": f"Local Player ({state.player2_name})",
                "timestamp": datetime.now().isoformat()
            })
        state.player_score = 0
        state.computer_score = 0
        self.app.show(MainMenuPage)

    def finish_match(self):
        state.player_score = 0
        state.computer_score = 0
        self.app.show(MainMenuPage)

    # ---------------- ANIMATION ----------------
    def animate_rps_sequence(self):
        # Clear the entire screen
        for w in self.winfo_children():
            w.destroy()

        font = ("Arial Black", 110)

        self.lbl_rock = ctk.CTkLabel(self, text="ROCK", font=font)
        self.lbl_paper = ctk.CTkLabel(self, text="PAPER", font=font)
        self.lbl_scissors = ctk.CTkLabel(self, text="SCISSORS", font=font)

        self.lbl_rock.place(relx=0.5, y=90, anchor="center")
        self.after(800, self.show_paper)

    def show_paper(self):
        self.lbl_paper.place(relx=0.5, rely=0.5, anchor="center")
        self.after(800, self.show_scissors)

    def show_scissors(self):
        self.lbl_scissors.place(relx=0.5, rely=0.9, anchor="center")
        self.after(800, self.clear_rps)

    def clear_rps(self):
        self.lbl_rock.destroy()
        self.lbl_paper.destroy()
        self.lbl_scissors.destroy()
        self.show_shoot()

    def show_shoot(self):
        self.shoot_size = 30
        self.shoot_lbl = ctk.CTkLabel(
            self, text="🪨 📄 ✂️",
            font=("Arial Black", self.shoot_size)
        )
        self.shoot_lbl.place(relx=0.5, rely=0.5, anchor="center")
        self.zoom_shoot()

    def zoom_shoot(self):
        self.shoot_size += 6
        self.shoot_lbl.configure(font=("Arial Black", self.shoot_size))

        if self.shoot_size > 180:
            self.shoot_lbl.destroy()
            self.after(300, self.show_result)
        else:
            self.after(30, self.zoom_shoot)


# ---------------- RUN ----------------
if __name__ == "__main__":
    App().mainloop()
