import customtkinter as ctk
import random

# ---------------- CONFIG ----------------
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("dark-blue")

ROCK, PAPER, SCISSORS = 1, 2, 3


# ---------------- STATE ----------------
class State:
    def __init__(self):
        self.guest_matches_played = 0
        self.guest_max_matches = 2
        self.player_score = 0
        self.computer_score = 0
        self.selected = None


state = State()


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
            GuestMenuPage,
            RulesPage,
            QuickMatchPage,
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
        super().__init__(parent)

        ctk.CTkLabel(
            self,
            text="ROCK  PAPER  SCISSORS",
            font=("Arial Black", 44)
        ).pack(pady=120)

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


# ---------------- GUEST MENU ----------------
class GuestMenuPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)

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


# ---------------- RULES ----------------
class RulesPage(ctk.CTkFrame):
    def __init__(self, parent, app):
        super().__init__(parent)

        ctk.CTkButton(
            self, text="← Back",
            width=100,
            command=lambda: app.show(GuestMenuPage)
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
        self.build_ui()

    def build_ui(self):
        ctk.CTkLabel(self, text="BEST OF 3", font=("Arial Black", 32)).pack(pady=20)

        self.score = ctk.CTkLabel(self, font=("Arial", 22))
        self.score.pack(pady=10)
        self.update_score()

        self.buttons = {}
        btn_frame = ctk.CTkFrame(self)
        btn_frame.pack(pady=30)

        # ✅ Properly centered emoji buttons
        for txt, val in [("🪨", ROCK), ("📄", PAPER), ("✂️", SCISSORS)]:
            b = ctk.CTkButton(
                btn_frame,
                text=txt,
                width=150,
                height=90,                     # taller button
                font=("Segoe UI Emoji", 38),   # slightly smaller font
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

        win = (state.selected - comp) % 3 == 1

        if win:
            state.player_score += 1
            result = "YOU WON THE ROUND 🎉"
        elif state.selected == comp:
            result = "DRAW 🤝"
        else:
            state.computer_score += 1
            result = "YOU LOST THE ROUND 😞"

        box = ctk.CTkFrame(self)
        box.place(relx=0.5, rely=0.5, anchor="center")

        ctk.CTkLabel(box, text=f"You selected: {names[state.selected]}", font=("Arial Black", 26)).pack(pady=8)
        ctk.CTkLabel(box, text=f"Computer selected: {names[comp]}", font=("Arial Black", 26)).pack(pady=8)
        ctk.CTkLabel(box, text=result, font=("Arial Black", 30)).pack(pady=16)

        self.after(5000, lambda: self.reset_after_round(box))

    def reset_after_round(self, box):
        box.destroy()
        state.selected = None

        for w in self.winfo_children():
            w.destroy()

        self.build_ui()

    def update_score(self):
        self.score.configure(
            text=f"Guest {state.player_score}  |  Computer {state.computer_score}"
        )

    def finish_match(self):
        state.guest_matches_played += 1
        state.player_score = 0
        state.computer_score = 0
        state.selected = None
        self.app.show(LandingPage)


# ---------------- RUN ----------------
if __name__ == "__main__":
    App().mainloop()
