/*
 * MazeSolver v1.0.0
 *
 * Created by: Berkay Çubuk <berkay@berkaycubuk.com>
 */
#include "raylib.h"
#include <unistd.h>
#include <stdio.h>
#include "rlgl.h"
#include "raymath.h"
#include <time.h>
#include <stdlib.h>

#define MAZE_ROWS 80
#define MAZE_COLS 80
#define GRID_SQUARE_SIZE 40

int solved = 0;

typedef struct {
	int isStart;
	int isFinish;
	int isVisited;
	int rightWall;
	int downWall;
	int leftWall;
	int topWall;
} Cell;

void solver_stack_dehasher(int hash, int* x, int* y) {
	if (hash == -1) {
		return;
	}

	*x = hash % MAZE_ROWS;
	*y = (hash - *x) / MAZE_ROWS;
}

int solver_stack_hasher(int x, int y) {
	return (y * MAZE_ROWS) + x;
}

void shuffle(int array[4][2]) {
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
	
        int temp[2];

	temp[0] = array[i][0];
	temp[1] = array[i][1];

        array[i][0] = array[j][0];
        array[i][1] = array[j][1];

        array[j][0] = temp[0];
        array[j][1] = temp[1];
    }
}

// Depth First Search
void dfs_solver(
	Cell *maze[MAZE_ROWS][MAZE_COLS],
	int solver_stack[MAZE_ROWS * MAZE_COLS],
	int* solver_stack_cursor,
	int* x,
	int* y
) {
	if (*x < 0 || *x > MAZE_COLS || *y < 0 || *y > MAZE_ROWS) {
		// out of boundaries
		printf("Out of boundaries\n");
		return;
	}

	printf("X: %d, Y: %d, Cursor: %d\n", *x, *y, *solver_stack_cursor);

	// add current position to the solver_stack
	solver_stack[*solver_stack_cursor] = solver_stack_hasher(*x, *y);
	*solver_stack_cursor += 1;

	if (maze[*y][*x]->isFinish == 1) { // found the finish
		printf("Found the finish!");

		solved = 1;

		return;
	}

	// mark current position as visited
	maze[*y][*x]->isVisited = 1;
	int moved = 0;

	int directions[4][2] = {
		{-1, 0}, // left
		{0, 1},  // down
		{1, 0},  // right
		{0, -1}  // up
	};

	shuffle(directions);

	for (int i = 0; i < 4; i++) {
		int newX = *x + directions[i][0];
		int newY = *y + directions[i][1];

		if (
			newX < MAZE_COLS && newX >= 0 &&
			newY < MAZE_ROWS && newY >= 0 &&
			maze[newY][newX]->isVisited == 0
		) {
			int valid = 1;
			if (directions[i][0] > 0 && maze[*y][*x]->rightWall == 1) { // going right
				valid = 0;
			} else if (directions[i][0] < 0 && maze[*y][*x]->leftWall == 1) { // going left
				valid = 0;
			}

			if (directions[i][1] > 0 && maze[*y][*x]->downWall == 1) { // going down
				valid = 0;
			} else if (directions[i][1] < 0 && maze[*y][*x]->topWall == 1) { // going up
				valid = 0;
			}

			if (valid == 1) {
				*x = newX;
				*y = newY;
				moved = 1;
				break;
			}
		}
	}

	if (moved == 0) {
		printf("unable to move\n");
		// pop the last record from solver_stack
		*solver_stack_cursor -= 1;
		solver_stack[*solver_stack_cursor] = -1;
		
		// move to the position before
		if (*solver_stack_cursor >= 1 && solver_stack[*solver_stack_cursor - 1] != -1) {
			printf("backtracking\n");
			solver_stack_dehasher(solver_stack[*solver_stack_cursor -1], x, y);
			*solver_stack_cursor -= 1;
		}
	}
}

void generate_maze(Cell *maze[MAZE_ROWS][MAZE_COLS], int x, int y, int maze_stack[MAZE_ROWS * MAZE_COLS], int *maze_stack_cursor) {
	int valid = 0;
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			if (maze[i][j]->isVisited == 0) {
				valid = 1;
				break;
			}
		}
	}

	if (valid == 0) {
		printf("Looped through all the cells.\n");
		return;
	}

	maze[y][x]->isVisited = 1;

	maze_stack[*maze_stack_cursor] = solver_stack_hasher(x, y);
	*maze_stack_cursor += 1;

	int directions[4][2] = {
		{-1, 0}, // left
		{0, 1},  // down
		{1, 0},  // right
		{0, -1}  // up
	};

	shuffle(directions);

	int moved = 0;
	for (int i = 0; i < 4; i++) {
		int newX = x + directions[i][0];
		int newY = y + directions[i][1];

		if (newX >= 0 && newX < MAZE_COLS && newY >= 0 && newY < MAZE_ROWS && maze[newY][newX]->isVisited == 0) {
			moved = 1;

			if (x == newX) {
				if (newY > y) {
					maze[y][x]->downWall = 0;
					maze[newY][x]->topWall = 0;
				} else {
					maze[y][x]->topWall = 0;
					maze[newY][x]->downWall = 0;
				}
			} else if (newY == y) {
				if (newX > x) {
					maze[y][x]->rightWall = 0;
					maze[y][newX]->leftWall = 0;
				} else {
					maze[y][x]->leftWall = 0;
					maze[y][newX]->rightWall = 0;
				}
			} else {
				printf("\n---- NOOOO! ---\n");
				printf("x: %d, prevX: %d, y: %d, prevY: %d\n", x, newX, y, newY);
				printf("---- NOOOO! ---\n");
			}

			generate_maze(maze, newX, newY, maze_stack, maze_stack_cursor);
		}
	}

	if (moved == 0) {
		printf("unable to move\n");
		*maze_stack_cursor -= 1;
		maze_stack[*maze_stack_cursor] = -1;
		
		// move to the position before
		if (*maze_stack_cursor >= 1 && maze_stack[*maze_stack_cursor - 1] != -1) {
			printf("Cursor: %d\n", *maze_stack_cursor);
			int tempX = 0, tempY = 0;
			solver_stack_dehasher(maze_stack[*maze_stack_cursor - 1], &tempX, &tempY);
			*maze_stack_cursor -= 1;
			//maze_stack[*maze_stack_cursor] = -1;
			printf("X: %d, Y: %d, %d", tempX, tempY, maze_stack[*maze_stack_cursor]);
			generate_maze(maze, tempX, tempY, maze_stack, maze_stack_cursor);
		}
	}
}

void init_maze(Cell *maze[MAZE_ROWS][MAZE_COLS]) {
	// fill the maze with -1 and 0
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			maze[i][j] = (Cell *)malloc(sizeof(Cell));
			if (maze[i][j] == NULL) {
				printf("ERROR: memory allocation failed!\n");
				exit(-1);
			}
			maze[i][j]->isStart = 0;
			maze[i][j]->isFinish = 0;
			maze[i][j]->isVisited = 0;
			maze[i][j]->rightWall = 1;
			maze[i][j]->downWall = 1;
			maze[i][j]->leftWall = 1;
			maze[i][j]->topWall = 1;
		}
	}

	int maze_stack[MAZE_ROWS * MAZE_COLS];
	int maze_stack_cursor = 0;

	int end_x = MAZE_COLS - 1, end_y = MAZE_ROWS - 1;
	generate_maze(maze, 0, 0, maze_stack, &maze_stack_cursor);

	// mark the finish position
	maze[0][0]->isStart = 1;
	maze[end_y][end_x]->isFinish = 1;

	// clean
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			maze[i][j]->isVisited = 0;
		}
	}
}

void renderCell(Cell* cell, int x, int y) {
	if (cell->isVisited == 1) {
		DrawCircle(x + (GRID_SQUARE_SIZE / 2), y + (GRID_SQUARE_SIZE / 2), 6.0f, GRAY);
	}
	if (cell->isStart == 1) {
		DrawCircle(x + (GRID_SQUARE_SIZE / 2), y + (GRID_SQUARE_SIZE / 2), 10.0f, GREEN);
	}
	if (cell->isFinish == 1) {
		DrawCircle(x + (GRID_SQUARE_SIZE / 2), y + (GRID_SQUARE_SIZE / 2), 10.0f, RED);
	}
	if (cell->rightWall == 1) {
		DrawLine(x + GRID_SQUARE_SIZE, y, x + GRID_SQUARE_SIZE, y + GRID_SQUARE_SIZE, WHITE);
	}
	if (cell->leftWall == 1) {
		DrawLine(x, y, x, y + GRID_SQUARE_SIZE, WHITE);
	}
	if (cell->topWall == 1) {
		DrawLine(x, y, x + GRID_SQUARE_SIZE, y, WHITE);
	}
	if (cell->downWall == 1) {
		DrawLine(x, y + GRID_SQUARE_SIZE, x + GRID_SQUARE_SIZE, y + GRID_SQUARE_SIZE, WHITE);
	}
}

void renderSolution(int solution_stack[MAZE_ROWS * MAZE_COLS], int* solution_stack_cursor, int offsetX, int offsetY) {
	int x, y;
	for (int i = 1; i < *solution_stack_cursor; i++) {
		solver_stack_dehasher(solution_stack[i], &x, &y);

		x *= GRID_SQUARE_SIZE;
		x += offsetX + (GRID_SQUARE_SIZE / 2);
		y *= GRID_SQUARE_SIZE;
		y += offsetY + (GRID_SQUARE_SIZE / 2);

		int prevX, prevY;
		solver_stack_dehasher(solution_stack[i - 1], &prevX, &prevY);

		prevX *= GRID_SQUARE_SIZE;
		prevX += offsetX + (GRID_SQUARE_SIZE / 2);
		prevY *= GRID_SQUARE_SIZE;
		prevY += offsetY + (GRID_SQUARE_SIZE / 2);

		DrawLine(x, y, prevX, prevY, BLUE);
	}
}

int main(void) {
	const int screenWidth = 800;
	const int screenHeight = 450;
	int currentRow = 0;
	int currentCol = 0;

	int paused = 1;

	srand(time(NULL)); // Seed for randomness

	InitWindow(screenWidth, screenHeight, "MazeSolver");

	// camera
	Camera2D camera = { 0 };
	camera.zoom = 1.0f;

	// maze
	Cell *maze[MAZE_ROWS][MAZE_COLS];
	init_maze(maze);

	int solver_stack[MAZE_ROWS * MAZE_COLS];
	int solver_stack_cursor = 0;

	SetTargetFPS(60);

	while (!WindowShouldClose()) { // Detect window close button or ESC key
		if (IsKeyPressed(KEY_R)) {
			paused = 1;
			for (int i = 0; i < solver_stack_cursor; i++) {
				solver_stack[i] = -1;
			}
			solver_stack_cursor = 0;

			for (int i = 0; i < MAZE_ROWS; i++) {
				for (int j = 0; j < MAZE_COLS; j++) {
					maze[i][j]->isVisited = 0;
				}
			}

			currentRow = 0;
			currentCol = 0;

			paused = 0;
		} else if (IsKeyPressed(KEY_SPACE)) {
			if (paused == 0) paused = 1;
			else paused = 0;
		} else if (IsKeyPressed(KEY_G)) {
			paused = 1;
			for (int i = 0; i < solver_stack_cursor; i++) {
				solver_stack[i] = -1;
			}
			solver_stack_cursor = 0;

			for (int i = 0; i < MAZE_ROWS; i++) {
				for (int j = 0; j < MAZE_COLS; j++) {
					free(maze[i][j]);
				}
			}

			init_maze(maze);

			currentRow = 0;
			currentCol = 0;
			paused = 0;
			solved = 0;
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 delta = GetMouseDelta();
			delta = Vector2Scale(delta, -1.0f/camera.zoom);
			camera.target = Vector2Add(camera.target, delta);
		}

		float wheel = GetMouseWheelMove();
		if (wheel != 0) {
			Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

			camera.offset = GetMousePosition();

			camera.target = mouseWorldPos;

			float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
			if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
			camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
		}

		if (paused == 0 && solved == 0) dfs_solver(maze, solver_stack, &solver_stack_cursor, &currentCol, &currentRow);

		BeginDrawing();

			ClearBackground(BLACK);

			BeginMode2D(camera);

				for (int row = 0; row < MAZE_ROWS; row++) {
					for (int col = 0; col < MAZE_COLS; col++) {
						int x = (GRID_SQUARE_SIZE * col) + 50;
						int y = (GRID_SQUARE_SIZE * row) + 50;

						renderCell(maze[row][col], x, y);
					}
				}

				renderSolution(solver_stack, &solver_stack_cursor, 50, 50);

			EndMode2D();

			DrawRectangle(10, 10, 250, 150, Fade(BLUE, 0.5f));
			DrawRectangleLines(10, 10, 250, 150, BLUE);

			DrawText("Controls:", 20, 20, 10, WHITE);
			DrawText("SPACE: pause / play", 20, 40, 10, WHITE);
			DrawText("R: reset", 20, 60, 10, WHITE);
			DrawText("ESC: exit", 20, 80, 10, WHITE);
			DrawText("MazeSolver v1.0.0", 20, 120, 10, WHITE);
			DrawText("Created by Berkay Çubuk", 20, 140, 10, WHITE);

			if (paused == 1) {
				DrawText("PAUSED", screenWidth - 100, 20, 20, RED);
			}

		EndDrawing();
	}

	// free allocated memory
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			free(maze[i][j]);
		}
	}

	CloseWindow();

	return 0;
}
