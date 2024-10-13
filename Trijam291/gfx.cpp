#include "global.h"

void DrawKeybindBar(const char *left, const char *right, bool bg) {
	if (bg)
		DrawRectangle(0, 600 - 30, 800, 30, Fade(BLACK, 0.7f));
	DrawLine(0, 600 - 31, 800, 600 - 31, WHITE); // I dislike the number "31" here, but it is correct. Sad.
	DrawText(left, 10, 600 - 25, 20, WHITE);
	int rlen = MeasureText(right, 20);
	DrawText(right, 800 - 10 - rlen, 600 - 25, 20, WHITE);
}

void DoFadeOutAnimation() {
	int top = 0;

	while ((top += 80) <= 800) {
		float t = ((float)top) / 800.f;
		t *= t;
		t *= 800;

		BeginDrawing();

		DrawRectangle(0, 0, t, 600, BLUE);

		EndDrawing();
	}
}

void DoFadeInAnimation(int &top) {
	if (top < 800) {
		top += 60;
		float t = ((float)top) / 800.f;
		t *= t;
		t *= 800;

		DrawRectangle(t, 0, 800, 600, BLUE);
	}
}