#include "tetris.h"

#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80
#define ESC 27
#define BX 5
#define BY 1
#define BW 10
#define BH 20

void DrawScreen();
void DrawBoard();
BOOL ProcessKey();
void PrintBrick(BOOL Show);
int GetAround(int x, int y, int b, int r);
BOOL MoveDown();
void TestFull();

typedef struct Point {
	int x; // 기준점으로부터의 x 길이
	int y; // 기준점으로부터의 y 길이
} Point;

// 7개의 블럭 마다 4개의 회전 모양에 있는 각각의 4개의 좌표를 가지고 있다(?)
// 첫 번째 첨자가 벽돌의 번호, 두 번째 첨자가 회전 번호이며
// 각 모양별로 4개의 새끼 벽돌 좌표를 가지는데 세 번째 첨자가 이 좌표들의 일련 번호이다
Point Shape[][4][4] = {
	{ { 0,0,1,0,2,0,-1,0 },{ 0,0,0,1,0,-1,0,-2 },{ 0,0,1,0,2,0,-1,0 },{ 0,0,0,1,0,-1,0,-2 } },
	{ { 0,0,1,0,0,1,1,1 },{ 0,0,1,0,0,1,1,1 },{ 0,0,1,0,0,1,1,1 },{ 0,0,1,0,0,1,1,1 } },
	{ { 0,0,-1,0,0,-1,1,-1 },{ 0,0,0,1,-1,0,-1,-1 },{ 0,0,-1,0,0,-1,1,-1 },{ 0,0,0,1,-1,0,-1,-1 } },
	{ { 0,0,-1,-1,0,-1,1,0 },{ 0,0,-1,0,-1,1,0,-1 },{ 0,0,-1,-1,0,-1,1,0 },{ 0,0,-1,0,-1,1,0,-1 } },
	{ { 0,0,-1,0,1,0,-1,-1 },{ 0,0,0,-1,0,1,-1,1 },{ 0,0,-1,0,1,0,1,1 },{ 0,0,0,-1,0,1,1,-1 } },
	{ { 0,0,1,0,-1,0,1,-1 },{ 0,0,0,1,0,-1,-1,-1 },{ 0,0,1,0,-1,0,-1,1 },{ 0,0,0,-1,0,1,1,1 } },
	{ { 0,0,-1,0,1,0,0,1 },{ 0,0,0,-1,0,1,1,0 },{ 0,0,-1,0,1,0,0,-1 },{ 0,0,-1,0,0,-1,0,1 } },
};

enum { EMPTY, BRICK, WALL };
char *arTile[] = { ". ","■","□" }; // 수에 해당하는 모양
int board[BW + 2][BH + 2]; // 수로 표시된 게임판 배열
int nx, ny; // 브릭 기준점의 현재 위치
int brick; // 현재 브릭
int rot; // 현재 회전

void main()
{
	int nFrame, nStay;
	int x, y;

	setcursortype(NOCURSOR); // 커서가 보이지 않는다. 
	randomize(); // 난수 발생기 초기화
	clrscr(); // 화면을 깨끗하게 지운다. (clear screen)
	
	// 네개의 모서리 중 하나라도 맞으면 wall, 아니면 empty
	for (x = 0; x<BW + 2; x++) {
		for (y = 0; y<BH + 2; y++) {
			board[x][y] = (y == 0 || y == BH + 1 || x == 0 || x == BW + 1) ? WALL : EMPTY;
		}
	}
	DrawScreen();
	nFrame = 20;

	for (; 1;) {
		brick = random(sizeof(Shape) / sizeof(Shape[0])); // 7개 중에서 랜덤하게 하나를 골라 배출.
		nx = BW / 2;
		ny = 3;
		rot = 0;
		PrintBrick(TRUE);

		if (GetAround(nx, ny, brick, rot) != EMPTY) break;
		nStay = nFrame;
		for (; 2;) {
			if (--nStay == 0) {
				nStay = nFrame;
				if (MoveDown()) break;
			}
			if (ProcessKey()) break;
			delay(1000 / 2000);
		}
	}

	// 게임 오버 후
	clrscr();
	gotoxy(30, 12); puts("G A M E  O V E R");
	setcursortype(NORMALCURSOR);
}

// 벽과 안쪽을 만든다. 그리고 설명도 해줌.
void DrawScreen()
{
	int x, y;

	for (x = 0; x<BW + 2; x++) {
		for (y = 0; y<BH + 2; y++) {
			gotoxy(BX + x * 2, BY + y);
			puts(arTile[board[x][y]]);
		}
	}

	for (x = 0; x<BW + 2; x++) {
		for (y = 0; y<BH + 2; y++) {
			gotoxy(50 + BX + x * 2, BY + y);
			printf("%d", board[x][y]);
		}
	}

	//gotoxy(50, 3); puts("Tetris Ver 1.0");
	//gotoxy(50, 5); puts("좌우:이동, 위:회전, 아래:내림");
	//gotoxy(50, 6); puts("공백:전부 내림");
}

// 안쪽 가로 10칸, 세로 20칸의 공간에 arTile[board[x][y]]에 
// 해당하는 모양들을 BX BY칸 떨어진 곳에 놓는다
void DrawBoard()
{
	int x, y;

	for (x = 1; x<BW + 1; x++) {
		for (y = 1; y<BH + 1; y++) {
			gotoxy(BX + x * 2, BY + y);
			puts(arTile[board[x][y]]);
		}
	}
}

BOOL ProcessKey()
{
	int ch, trot;

	if (kbhit()) {
		ch = getch();
		if (ch == 0xE0 || ch == 0) {
			ch = getch();
			switch (ch) {
			case LEFT:
				if (GetAround(nx - 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE);
					nx--;
					PrintBrick(TRUE);
				}
				break;
			case RIGHT:
				if (GetAround(nx + 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE);
					nx++;
					PrintBrick(TRUE);
				}
				break;
			case UP:
				trot = (rot == 3 ? 0 : rot + 1);
				if (GetAround(nx, ny, brick, trot) == EMPTY) {
					PrintBrick(FALSE);
					rot = trot;
					PrintBrick(TRUE);
				}
				break;
			case DOWN:
				if (MoveDown()) {
					return TRUE;
				}
				break;
			}
		}
		else {
			switch (ch) {
			case ' ':
				while (MoveDown() == FALSE) { ; }
				return TRUE;
			}
		}
	}
	return FALSE;
}

// brick= 현재 브릭, rot = 회전모양, i = 일련번호
// 현재 상태의 브릭을 출력한다.
void PrintBrick(BOOL Show)
{
	int i;

	// 각각의 새끼 브릭에 대해 반복
	for (i = 0; i<4; i++) {
		gotoxy(BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		puts(arTile[Show ? BRICK : EMPTY]);
	}

	for (i = 0; i<4; i++) {
		gotoxy(50+BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		printf("%d",Show ? BRICK : EMPTY);
	}
}

int GetAround(int x, int y, int b, int r)
{
	int i, k = EMPTY;

	for (i = 0; i<4; i++) {
		k = max(k, board[x + Shape[b][r][i].x][y + Shape[b][r][i].y]);
	}
	return k;
}

BOOL MoveDown()
{
	if (GetAround(nx, ny + 1, brick, rot) != EMPTY) {
		TestFull();
		return TRUE;
	}
	PrintBrick(FALSE);
	ny++;
	PrintBrick(TRUE);
	return FALSE;
}

void TestFull()
{
	int i, x, y, ty;

	for (i = 0; i<4; i++) {
		board[nx + Shape[brick][rot][i].x][ny + Shape[brick][rot][i].y] = BRICK;
	}

	for (y = 1; y<BH + 1; y++) {
		for (x = 1; x<BW + 1; x++) {
			if (board[x][y] != BRICK) break;
		}
		if (x == BW + 1) {
			for (ty = y; ty>1; ty--) {
				for (x = 1; x<BW + 1; x++) {
					board[x][ty] = board[x][ty - 1];
				}
			}
			DrawBoard();
			delay(200);
		}
	}
}