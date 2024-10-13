#include "global.h"

enum SlotState {
	SS_DONE,
	SS_ROLL,
	SS_REQ
};

#define SLOTS_SYMBOLS 6
#define SLOTS_BOMB (-1)
#define SLOTS_WIN (-2)

// TODO: Too forgiving
struct Slot {
	SlotState state = SS_ROLL;
	int results[3];
	int locked_results = 0;
	int cash = 100;
	int bet = -1;
	int lastwin = 0;
	bool first_game = true;
	float animtime = 0;
	float tmod = 1;
	int lastbet = 0;
	int turns = 0;
	int playing = 0;

	int GetNewRandom() const {
		bool exists = true;
		int r;
		while (exists) {
			exists = false;
			r = GetRandomValue(0, SLOTS_SYMBOLS - 1);
			for (int i = 0; i < locked_results; i++) {
				if (results[i] == r) {
					exists = true;
				}
			}
		}
		return r;
	}

	bool ShouldQuit() const {
		bool bomb = false;
		if (results[0] == SLOTS_BOMB || results[1] == SLOTS_BOMB || results[2] == SLOTS_BOMB)
			bomb = true;
		int wins = (results[0] == SLOTS_WIN) + (results[1] == SLOTS_WIN) + (results[2] == SLOTS_WIN);

		if (bomb)
			return GetRandomValue(0, 1) == 0;
		if (wins == 3)
			return true;
		return false;
	}

	int GetMatches() const {
		int matches = 1;
		if (results[0] == results[1] || results[1] == results[2] || results[0] == results[2])
			matches = 2;
		if (results[0] == results[1] && results[0] == results[2])
			matches = 3;
		return matches;
	}

	bool HasBomb() const {
		bool bomb = false;
		if (results[0] == SLOTS_BOMB || results[1] == SLOTS_BOMB || results[2] == SLOTS_BOMB)
			bomb = true;
		return bomb;
	}

	int GetWins() const {
		int wins = (results[0] == SLOTS_WIN) + (results[1] == SLOTS_WIN) + (results[2] == SLOTS_WIN);
		return wins;
	}

	float GetRewardMult() const {
		int matches = 1;
		if (results[0] == results[1] || results[1] == results[2] || results[0] == results[2])
			matches = 2;
		if (results[0] == results[1] && results[0] == results[2])
			matches = 3;
		
		bool bomb = false;
		if (results[0] == SLOTS_BOMB || results[1] == SLOTS_BOMB || results[2] == SLOTS_BOMB)
			bomb = true;

		int wins = (results[0] == SLOTS_WIN) + (results[1] == SLOTS_WIN) + (results[2] == SLOTS_WIN);

		if (bomb)
			return 0;
		if (wins == 3)
			return 2.f;
		if (wins == 2)
			return 1.7f;
		if (wins == 1)
			return 1.4f;
		if (matches == 3)
			return 1.1f;
		if (matches == 2)
			return 0.7f;
		return 0.4f;
	}

	float GetHappiness() const {
		int matches = 1;
		if (results[0] == results[1] || results[1] == results[2] || results[0] == results[2])
			matches = 2;
		if (results[0] == results[1] && results[0] == results[2])
			matches = 3;

		bool bomb = false;
		if (results[0] == SLOTS_BOMB || results[1] == SLOTS_BOMB || results[2] == SLOTS_BOMB)
			bomb = true;

		int wins = (results[0] == SLOTS_WIN) + (results[1] == SLOTS_WIN) + (results[2] == SLOTS_WIN);

		if (bomb)
			return -0.6;
		if (wins == 3)
			return 0.7f;
		if (wins == 2)
			return 0.5f;
		if (wins == 1)
			return 0.4f;
		if (matches == 3)
			return 0.23f;
		if (matches == 2)
			return -0.04f;
		return -0.3f;
	}

	void Randomize() {
		for (int i = locked_results; i < 3; i++) {
			results[i] = GetRandomValue(0, SLOTS_SYMBOLS - 1);
		}
	}

	void Reset(float happiness, int admod) {
		*this = {};
		float mod = admod + happiness * 3;
		cash = 100 + mod * 15;
		float r = ((float)rand() / (float)RAND_MAX);
		tmod = 1 + (r - 0.5f) * 0.2f;
		playing = 5 + r * 3;
	}
};

bool MouseOnSlots(int i) {
	float dist = 80 * i;
	Vector2 m = GetMousePosition();
	return CheckCollisionPointRec(m, { dist, 0, 80, 80 });
}

bool MouseOnSpecificSlot(int i, int j) {
	float dist = 80 * i;
	float sdist = 30 * j;
	Vector2 m = GetMousePosition();
	return CheckCollisionPointRec(m, { dist + sdist - 10, 0, 30, 25 });
}

bool MouseOnSpecificSlot(int i, int j, int y) {
	float dist = 80 * i;
	float sdist = 30 * j;
	float ydist = 30 * y;
	Vector2 m = GetMousePosition();
	return CheckCollisionPointRec(m, { dist + sdist - 10, ydist, 30, 25 });
}

bool MousePressSpecificSlot(int i, int j) {
	return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && MouseOnSpecificSlot(i, j);
}

bool MousePressSpecificSlot(int i, int j, int y) {
	return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && MouseOnSpecificSlot(i, j, y);
}

enum TutorialState {
	TS_BET,
	TS_TWO,
	TS_THREE,
	TS_BOMB,
	TS_WIN,
	TS_MONEY,
	TS_RESPECT,
	TS_PURCHASE,
	TS_OVER
};

const char *tutoriallines[] = {
	"Hello! Do you see that slot machine? There's someone playing at it right now! Try to make them get three different numbers.",
	"Can you make them get two of the same number?",
	"How about three?",
	"The ':(' is a 'bomb'. If they get one, they'll always lose all of their bet.",
	"The '!' is a 'payout'. This will always give them at least a 100% payout, and more if there's a payout match.",
	"Money (above this text) is a finite resource. If you run out, the game is over. It is now enabled!",
	"People's happiness (on the right) is also a finite resource. The more people win, the more they'll want to spend and come back. Run out and you lose.",
	"You can press [m] or [n] to buy things! Multiple people playing at once reduces passive happiness drain. Ads increase spending.",
	""
};

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	Slot slots[6] = { {} };
	int slotsi = 1;
	int bank = 500;
	float happiness = 0.5f;
	int admod = 1;
	TutorialState ts = TS_BET;
	bool buffer = false;
	bool cash = true;

	PlaySound(SND_START);
	DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_N) && ts >= TS_PURCHASE) {
			if (slotsi < 6 && bank >= 550) {
				slots[slotsi].Reset(happiness, admod);
				slotsi++;
				bank -= 500;
				if (ts == TS_PURCHASE) {
					ts = TS_OVER;
				}
				buffer = false;
				cash = true;
			}
			else if (bank < 500) {
				cash = false;
				buffer = false;
			}
			else if (bank < 550) {
				buffer = true;
				cash = true;
			}
		}
		if (IsKeyPressed(KEY_M) && ts >= TS_PURCHASE) {
			if (bank >= 200) {
				admod++;
				bank -= 150;
				if (ts == TS_PURCHASE) {
					ts = TS_OVER;
				}
				buffer = false;
				cash = true;
			}
			else if (bank < 150) {
				cash = false;
				buffer = false;
			}
			else {
				buffer = true;
				cash = true;
			}
		}

		slotsi = Clamp(slotsi, 1, 6);
		for (int i = 0; i < slotsi; i++) {
			Slot &slot = slots[i];
			slot.animtime += GetFrameTime();
			if (slot.state == SS_REQ) {
				if (MouseOnSlots(i)) {
					if (MousePressSpecificSlot(i, 0)) {
						if (slot.locked_results == 0)
							slot.results[slot.locked_results] = slot.GetNewRandom();
						else
							slot.results[slot.locked_results] = slot.results[0];
						slot.locked_results++;
					}
					else if (MousePressSpecificSlot(i, 1) && slot.locked_results >= 1) {
						if (slot.locked_results == 1)
							slot.results[slot.locked_results] = slot.GetNewRandom();
						else
							slot.results[slot.locked_results] = slot.results[1];
						slot.locked_results++;
					}
					else if (MousePressSpecificSlot(i, 2) && slot.locked_results >= 2) {
						if (slot.locked_results == 2)
							slot.results[slot.locked_results] = slot.GetNewRandom();
						else
							slot.results[slot.locked_results] = slot.results[2];
						slot.locked_results++;
					}
					else if (MousePressSpecificSlot(i, 2, 2)) {
						slot.results[slot.locked_results++] = SLOTS_BOMB;
					}
					else if (MousePressSpecificSlot(i, 2, 1)) {
						slot.results[slot.locked_results++] = SLOTS_WIN;
					}
				}

				if (slot.locked_results >= 3) {
					float rm = slot.GetRewardMult();
					float hm = slot.GetHappiness();
					slot.state = SS_DONE;
					slot.animtime = 0;
					slot.lastwin = slot.bet * rm;
					slot.lastbet = slot.bet;
					if (ts >= TS_RESPECT)
						happiness += 0.13f * hm;
					slot.first_game = false;
					slot.cash -= slot.bet;
					slot.cash += slot.lastwin;
					if (ts >= TS_MONEY) {
						bank += slot.bet;
						bank -= slot.lastwin;
					}

					if (ts == TS_BET) {
						if (slot.GetMatches() == 1) {
							ts = TS_TWO;
						}
					}
					else if (ts == TS_TWO) {
						if (slot.GetMatches() == 2) {
							ts = TS_THREE;
						}
					}
					else if (ts == TS_THREE) {
						if (slot.GetMatches() == 3) {
							ts = TS_BOMB;
						}
					}
					else if (ts == TS_BOMB) {
						if (slot.HasBomb()) {
							ts = TS_WIN;
						}
					}
					else if (ts == TS_WIN) {
						if (slot.GetWins() > 0) {
							ts = TS_MONEY;
						}
					}
					else if (ts == TS_MONEY) {
						ts = TS_RESPECT;
					}
					else if (ts == TS_RESPECT) {
						ts = TS_PURCHASE;
					}
				}
				slot.Randomize();
				if (ts == TS_RESPECT)
					happiness -= (GetFrameTime() * 0.01f * 0.4f * slot.tmod) / (slotsi * 1.5f);
			}
			else if (slot.state == SS_DONE) {
				if (slot.animtime >= 2.1f * slot.tmod) {
					slot.turns++;
					if (slot.cash > 0 && slot.turns <= slot.playing) {
						slot.state = SS_ROLL;
						slot.locked_results = 0;
						slot.bet = -1;
						slot.animtime = 0;
					}
					else {
						slot.Reset(happiness, admod);
					}
				}
			}
			else if (slot.state == SS_ROLL) {
				if (slot.bet < 0) {
					float mod = admod + happiness * 3;
					int fifth = slot.cash / 3; // dw about it

					slot.bet = Max(5, GetRandomValue(fifth - mod * 3, fifth + mod * 4));
					if (slot.cash < slot.bet)
						slot.bet = slot.cash;
				}
				else {
					if (slot.animtime >= 0.f * slot.tmod) {
						slot.state = SS_REQ;
						slot.animtime = 0;
					}
					slot.Randomize();
				}
			}
		}

		happiness = Clamp(happiness, 0, 1);
		
		assert(happiness > 0);
		assert(bank >= 0);

		BeginDrawing();

		ClearBackground(BLACK);

		for (int i = 0; i < slotsi; i++) {
			int distance = 80 * i;
			Slot &slot = slots[i];
			if (MouseOnSlots(i))
				DrawRectangle(distance, 0, 80, 80, DARKGRAY);
			//if (slot.state == SS_REQ)
			//	DrawRectangle(distance, 5, 10, 10, RED);
			DrawText(" !", distance + 2 * 30, 1 * 30, 20, GREEN);
			DrawText(" :(", distance + 2 * 30, 2 * 30, 20, RED);
			for (int j = 0; j < 3; j++) {
				DrawText((slot.results[j] == SLOTS_BOMB ? ":(" : slot.results[j] == SLOTS_WIN ? "!" : TextFormat("%d", slot.results[j])), j * 30 + distance, 0, 20, slot.results[j] == SLOTS_BOMB ? RED : slot.results[j] == SLOTS_WIN ? GREEN : (slot.locked_results == j && MouseOnSpecificSlot(i, j)) ? YELLOW : (slot.locked_results < j) ? LIGHTGRAY : (MouseOnSpecificSlot(i, j) && j < slot.locked_results && slot.locked_results < 3) ? PURPLE : WHITE);
			}
			if (slot.first_game)
				DrawText("new person!", distance, 38 + 25, 10, GREEN);

			DrawText(TextFormat("cash: %d\nbet: %d\nlast:", slot.cash, slot.bet), distance, 20, 10, GRAY);
			int lw = slot.lastwin - slot.lastbet;
			DrawText(TextFormat("%+d", lw), distance + 25, 20 + 20 + 10, 10, lw == 0 ? GRAY : lw > 0 ? GREEN : RED);
		}

		DrawText(TextFormat("Happiness: %.1f%%\nBank: %d", happiness * 100, bank), 0, 80, 10, YELLOW);

		DrawText(TextFormat("$%d", bank), 10, 600 - 55 - 10 - 30, 30, cash ? GREEN : RED);
		if (buffer) {
			DrawText("You need a buffer of at least $50 to make a purchase!", 10, 600 - 55 - 10 - 30 - 20, 20, RED);
		}

		DrawLine(800 - 20, 20, 800 - 20, 600 - 60, WHITE);
		DrawLine(800 - 25, 20, 800 - 15, 20, WHITE);
		DrawLine(800 - 25, 600 - 60, 800 - 15, 600 - 60, WHITE);

		float a = ((20 + 20) * happiness) + ((600 - 60 - 20) * (1 - happiness));
		DrawCircle(800 - 20, a, 20, WHITE);
		DrawText(":)", 800 - 25, a - 10, 20, BLACK);

		if (ts != TS_OVER) {
			DrawText(tutoriallines[ts], 10, 600 - 55, 10, YELLOW);
		}

		DrawKeybindBar("", "[n] $500 machine [m] $150 advert");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:
	SaveGlobState();

	return restart;
}