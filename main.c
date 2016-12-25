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
	int x; // ���������κ����� x ����
	int y; // ���������κ����� y ����
} Point;

// 7���� �� ���� 4���� ȸ�� ��翡 �ִ� ������ 4���� ��ǥ�� ������ �ִ�(?)
// ù ��° ÷�ڰ� ������ ��ȣ, �� ��° ÷�ڰ� ȸ�� ��ȣ�̸�
// �� ��纰�� 4���� ���� ���� ��ǥ�� �����µ� �� ��° ÷�ڰ� �� ��ǥ���� �Ϸ� ��ȣ�̴�
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
char *arTile[] = { ". ","��","��" }; // ���� �ش��ϴ� ���
int board[BW + 2][BH + 2]; // ���� ǥ�õ� ������ �迭
int nx, ny; // �긯 �������� ���� ��ġ
int brick; // ���� �긯
int rot; // ���� ȸ��

void main()
{
	int nFrame, nStay;
	int x, y;

	setcursortype(NOCURSOR); // Ŀ���� ������ �ʴ´�. 
	randomize(); // ���� �߻��� �ʱ�ȭ
	for (; 3;) {
		clrscr(); // ȭ���� �����ϰ� �����. (clear screen)
	
		// �װ��� �𼭸� �� �ϳ��� ������ wall, �ƴϸ� empty
		for (x = 0; x<BW + 2; x++) {
			for (y = 0; y<BH + 2; y++) {
				board[x][y] = (y == 0 || y == BH + 1 || x == 0 || x == BW + 1) ? WALL : EMPTY;
			}
		}
		DrawScreen();
		nFrame = 20;

		for (; 1;) {
			brick = random(sizeof(Shape) / sizeof(Shape[0])); // 7�� �߿��� �����ϰ� �ϳ��� ��� ����.
			nx = BW / 2; // �긯�� ó������ ������� ����߸�
			ny = 3;
			rot = 0;
			PrintBrick(TRUE);

			// ó�� �긯�� ������ �� ������ ���� ������ ���ӿ���(for�� Ż��)
			if (GetAround(nx, ny, brick, rot) != EMPTY) break;
		
			nStay = nFrame;
			for (; 2;) {
				if (--nStay == 0) {
					nStay = nFrame;
					// nStay�� nFrame���� 0���� �� ������ ��� �� ĭ �Ʒ��� �ڵ����� �̵�
					if (MoveDown()) break;
				}
				// gotoxy(0, 0); printf("%2d", nStay); // ������ ǥ��
				// Ű ó��, �ٴڿ� ������ for�� Ż��
				// ���ο� �긯�� ����
				if (ProcessKey()) break;
				delay(1000 / 20);
			}
		}

		// ���� ���� ��
		clrscr();
		gotoxy(30, 12); puts("G A M E  O V E R");
		gotoxy(25, 14); puts("�ٽ� �����Ϸ��� Y�� ��������");
		if (tolower(getch()) != 'y') break;
	}
	setcursortype(NORMALCURSOR);
}

// ���� ������ �����. �׸��� ���� ����.
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
	gotoxy(50, 5); puts("�¿�:�̵�, ��:ȸ��, �Ʒ�:����");
	gotoxy(50, 6); puts("����:���� ����");
}

// ���� ���� 10ĭ, ���� 20ĭ�� ������ arTile[board[x][y]]�� 
// �ش��ϴ� ������ BX BYĭ ������ ���� ���´�
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

// Ű �Է��� ó���Ѵ�.
// �ٴڿ� ����� ��� TRUE, �ƴϸ� FALSE.
BOOL ProcessKey()
{
	int ch;
	int trot; // ������ ȸ�� ����

	if (kbhit()) { // Ű���尡 ������
		ch = getch(); // ���� �ϳ��� �޾ƿ�
		if (ch == 0xE0 || ch == 0) { // ����Ű ���� ���
			ch = getch();
			switch (ch) {
			case LEFT:
				// �� ĭ �������� �������� �� �ε����� �ʴ´ٸ�
				if (GetAround(nx - 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE); // �긯�� �����
					nx--; // ��ĭ ��������
					PrintBrick(TRUE); // �긯�� ����Ѵ�
				}
				break;
			case RIGHT:
				if (GetAround(nx + 1, ny, brick, rot) == EMPTY) {
					PrintBrick(FALSE); 
					nx++; // ��ĭ ����������
					PrintBrick(TRUE);
				}
				break;
			case UP:
				trot = (rot == 3 ? 0 : rot + 1);
				// ������ �ε����� �ʴ´ٸ�
				if (GetAround(nx, ny, brick, trot) == EMPTY) {
					PrintBrick(FALSE);
					rot = trot;
					PrintBrick(TRUE);
				}
				break;
			case DOWN:
				if (MoveDown()) {
					return TRUE; // �ٴڿ� ���� ���
				}
				break;
				
			}
		}
		else { // ����Ű ���� �ƴ� ���
			switch (tolower(ch)) {
			case ' ':
				// �ٴڿ� ���� �ʾ��� ��� ��� ��ĭ�� ������.
				// �׳� �� ������.
				while (MoveDown() == FALSE) { ; }
				return TRUE; // �ٴڿ� ����
			case ESC:
				exit(0);
			case 'p':
				clrscr();
				gotoxy(15, 10);
				puts("Tetris ��� ����. �ٽ� �����Ϸ��� �ƹ� Ű�� ��������.");
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

// brick= ���� �긯, rot = ȸ�����, i = �Ϸù�ȣ
// Show = TRUE: ���� ������ �긯�� ����Ѵ�.
// Show = FALSE: ���� ������ �긯�� �����.
void PrintBrick(BOOL Show)
{
	int i;

	// ������ ���� �긯�� ���� �ݺ�
	for (i = 0; i<4; i++) {
		gotoxy(BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		puts(arTile[Show ? BRICK : EMPTY]);
	}

	/*for (i = 0; i<4; i++) {
		gotoxy(50+BX + (Shape[brick][rot][i].x + nx) * 2, BY + Shape[brick][rot][i].y + ny);
		printf("%d",Show ? BRICK : EMPTY);
	}*/
}

// �� �Ǵ� �긯�� �ε����� �� ���� ��ȯ�Ѵ�.
// ���� �ֺ��� ������ �ִ��� �˻��Ͽ� ������ �̵� �� ȸ�� ���ɼ��� �����Ѵ�.
// x, y: ���� �������� x, y��ǥ(nx, ny), b: �긯(brick), r: ȸ��(rot)
int GetAround(int x, int y, int b, int r)
{
	int i, k = EMPTY;

	for (i = 0; i<4; i++) {
		k = max(k, board[x + Shape[b][r][i].x][y + Shape[b][r][i].y]);
	}
	return k;
}

// ������ ��ĭ �Ʒ��� �̵���Ų��.
// ���� �ٴڿ� ��Ҵٸ� TestFull �Լ��� ȣ���� �� TRUE�� �����Ѵ�.
BOOL MoveDown()
{
	// ���� �������� �� �ε����ٸ� (�� ������ �� ������)
	if (GetAround(nx, ny + 1, brick, rot) != EMPTY) {
		TestFull();
		return TRUE;
	}
	// �� �ε����� ���
	PrintBrick(FALSE);
	ny++;
	PrintBrick(TRUE);
	return FALSE;
}

// �������� �� ä���� ���� ã�� �����Ѵ�.
void TestFull()
{
	int i, x, y, ty;

	for (i = 0; i<4; i++) {
		board[nx + Shape[brick][rot][i].x][ny + Shape[brick][rot][i].y] = BRICK;
	}

	// ������ �ٿ� ���� �˻�
	for (y = 1; y<BH + 1; y++) {
		// �� ���� ��� �긯���� ä���� ������ ���������� x=BW+1
		for (x = 1; x<BW + 1; x++) {
			if (board[x][y] != BRICK) break;
		}
		if (x == BW + 1) { // �� ���� ��� �긯���� ä���� �ִ� ���
			for (ty = y; ty>1; ty--) { // �� ���ٺ��� �� �� ĭ�� �Ʒ��� ������
				for (x = 1; x<BW + 1; x++) {
					board[x][ty] = board[x][ty - 1];
				}
			}
			DrawBoard(); // ������Ʈ�� board�� ���� �������� �ٽ� �׸���
			delay(200);
		}
	}
}