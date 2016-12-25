#include "tetris.h"

#define TETRIS_VERSION "1.0.2"

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
	{ { 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 } },
	{ { 0,0,0,0,0,-1,1,0 },{ 0,0,0,0,-1,0,0,-1 },{ 0,0,0,0,0,1,-1,0 },{ 0,0,0,0,0,1,1,0 } },

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
	for (; 3;) {
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
			nx = BW / 2; // 브릭은 처음에는 가운데에서 떨어뜨림
			ny = 3;
			rot = 0;
			PrintBrick(TRUE);

			// 처음 브릭이 놓여야 할 공간에 뭐가 있으면 게임오버(for문 탈출)
			if (GetAround(nx, ny, brick, rot) != EMPTY) break;
		
			nStay = nFrame;
			for (; 2;) {
				if (--nStay == 0) {
					nStay = nFrame;
					// nStay가 nFrame에서 0까지 다 감소한 경우 한 칸 아래로 자동으로 이동
					if (MoveDown()) break;
				}
				// gotoxy(0, 0); printf("%2d", nStay); // 프레임 표시
				// 키 처리, 바닥에 닿으면 for문 탈출
				// 새로운 브릭을 생성
				if (ProcessKey()) break;
				delay(1000 / 20);
			}
		}

		// 게임 오버 후
		clrscr();
		gotoxy(30, 12); puts("G A M E  O V E R");
		gotoxy(25, 14); puts("다시 시작하려면 Y를 누르세요");
		if (tolower(getch()) != 'y') break;
	}
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

	/*for (x = 0; x<BW + 2; x++) {
		for (y = 0; y<BH + 2; y++) {
			gotoxy(50 + BX + x * 2, BY + y);
			printf("%d", board[x][y]);
		}
	}*/

	gotoxy(50, 3); puts("Tetris ver " TETRIS_VERSION);
	gotoxy(50, 5); puts("좌우:이동, 위:회전, 아래:내림");
	gotoxy(50, 6); puts("공백:전부 내림");
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

// 키 입력을 처리한다.
// 바닥에 닿았을 경우 TRUE, 아니면 FALSE.
BOOL ProcessKey()
{
	int ch;
	int trot; // 가상의 회전 상태

	if (kbhit()) { // 키보드가 눌리면
		ch = getch(); // 문자 하나를 받아옴
		if (ch == 0xE0 || ch == 0) { // 방향키 등의 경우
			ch = getch();
			switch (ch) {
			case LEFT:
				// 한 칸 왼쪽으로 움직였을 때 부딪히지 않는다면
				if (GetAround(nx - 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE); // 브릭을 지운다
					nx--; // 한칸 왼쪽으로
					PrintBrick(TRUE); // 브릭을 출력한다
				}
				break;
			case RIGHT:
				if (GetAround(nx + 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE); 
					nx++; // 한칸 오른쪽으로
					PrintBrick(TRUE);
				}
				break;
			case UP:
				trot = (rot == 3 ? 0 : rot + 1);
				// 돌려도 부딪히지 않는다면
				if (GetAround(nx, ny, brick, trot) == EMPTY) {
					PrintBrick(FALSE);
					rot = trot;
					PrintBrick(TRUE);
				}
				break;
			case DOWN:
				if (MoveDown()) {
					return TRUE; // 바닥에 닿은 경우
				}
				break;
				
			}
		}
		else { // 방향키 등이 아닌 경우
			switch (tolower(ch)) {
			case ' ':
				// 바닥에 닿지 않았을 경우 계속 한칸씩 내린다.
				// 그냥 쭉 내린다.
				while (MoveDown() == FALSE) { ; }
				return TRUE; // 바닥에 닿음
			case ESC:
				exit(0);
			case 'p':
				clrscr();
				gotoxy(15, 10);
				puts("Tetris 잠시 중지. 다시 시작하려면 아무 키나 누르세요.");
				getch();
				clrscr();
				DrawScreen();
				PrintBrick(TRUE);
				break;
			}
		}
	}
	return FALSE;
}

// brick= 현재 브릭, rot = 회전모양, i = 일련번호
// Show = TRUE: 현재 상태의 브릭을 출력한다.
// Show = FALSE: 현재 상태의 브릭을 지운다.
void PrintBrick(BOOL Show)
{
	int i;

	// 각각의 새끼 브릭에 대해 반복
	for (i = 0; i<4; i++) {
		gotoxy(BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		puts(arTile[Show ? BRICK : EMPTY]);
	}

	/*for (i = 0; i<4; i++) {
		gotoxy(50+BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		printf("%d",Show ? BRICK : EMPTY);
	}*/
}

// 벽 또는 브릭과 부딪히면 그 값을 반환한다.
// 벽돌 주변에 무엇이 있는지 검사하여 벽돌의 이동 및 회전 가능성을 조사한다.
// x, y: 현재 기준점의 x, y좌표(nx, ny), b: 브릭(brick), r: 회전(rot)
int GetAround(int x, int y, int b, int r)
{
	int i, k = EMPTY;

	for (i = 0; i<4; i++) {
		k = max(k, board[x + Shape[b][r][i].x][y + Shape[b][r][i].y]);
	}
	return k;
}

// 벽돌을 한칸 아래로 이동시킨다.
// 만약 바닥에 닿았다면 TestFull 함수를 호출한 후 TRUE를 리턴한다.
BOOL MoveDown()
{
	// 만약 내려갔을 때 부딪힌다면 (더 내려갈 수 없으면)
	if (GetAround(nx, ny + 1, brick, rot) != EMPTY) {
		TestFull();
		return TRUE;
	}
	// 안 부딪히는 경우
	PrintBrick(FALSE);
	ny++;
	PrintBrick(TRUE);
	return FALSE;
}

// 수평으로 다 채워진 줄을 찾아 삭제한다.
void TestFull()
{
	int i, x, y, ty;

	for (i = 0; i<4; i++) {
		board[nx + Shape[brick][rot][i].x][ny + Shape[brick][rot][i].y] = BRICK;
	}

	// 각각의 줄에 대해 검사
	for (y = 1; y<BH + 1; y++) {
		// 한 줄이 모두 브릭으로 채워져 있으면 마지막에는 x=BW+1
		for (x = 1; x<BW + 1; x++) {
			if (board[x][y] != BRICK) break;
		}
		if (x == BW + 1) { // 한 줄이 모두 브릭으로 채워져 있는 경우
			for (ty = y; ty>1; ty--) { // 그 윗줄부터 다 한 칸씩 아래로 내린다
				for (x = 1; x<BW + 1; x++) {
					board[x][ty] = board[x][ty - 1];
				}
			}
			DrawBoard(); // 업데이트된 board에 따라 게임판을 다시 그린다
			delay(200);
		}
	}
}