#include "state.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);
snake_t *init_snake(size_t);

/* Task 1 */
game_state_t *create_default_state() {
  game_state_t *game_state_ptr = malloc(sizeof(game_state_t));
  if (game_state_ptr == NULL) {
    return NULL;
  }

  const unsigned int num_of_snakes = 1;
  const unsigned int num_rows = 18;
  game_state_ptr->num_rows = num_rows;
  game_state_ptr->num_snakes = num_of_snakes;

  snake_t *snake_ptr = init_snake(num_of_snakes);
  if (snake_ptr == NULL) {
    free(game_state_ptr);
    return NULL;
  }
  game_state_ptr->snakes = snake_ptr;

  char **board_ptr = malloc(num_rows * sizeof(char *));
  if (board_ptr == NULL) {
    free(game_state_ptr);
    free(snake_ptr);
    return NULL;
  }
  const size_t num_columns = 20;
  for (int i = 0; i < num_rows; i++) {
    char *row_ptr = malloc(sizeof(char) * (num_columns + 1));
    if (row_ptr == NULL) {
      // TODO: how to free all malloced memory? especially all rows
      return NULL;
    }
    if (i == 0 || i == num_rows - 1) {
      strcpy(row_ptr, "####################");
    } else if (i == 2) {
      strcpy(row_ptr, "# d>D    *         #");
    } else {
      strcpy(row_ptr, "#                  #");
    }
    *(board_ptr + i) = row_ptr;
  }
  game_state_ptr->board = board_ptr;
  return game_state_ptr;
}

// TODO: This is a bad implementation because we accecpt a param of num of
// snakes but only initialize the first one, but that is ok currently
snake_t *init_snake(size_t num_of_snakes) {

  snake_t *snake_ptr = malloc(sizeof(snake_t) * num_of_snakes);
  if (snake_ptr == NULL) {
    return NULL;
  }
  snake_ptr->live = true;
  snake_ptr->head_row = 2;
  snake_ptr->head_col = 4;
  snake_ptr->tail_col = 2;
  snake_ptr->tail_row = 2;
  return snake_ptr;
}

/* Task 2 */
void free_state(game_state_t *state) {
  free(state->snakes);
  for (int i = 0; i < state->num_rows; i++) {
    free(*(state->board + i));
  }
  free(state->board);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  if (state == NULL) {
    return;
  }
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", *(state->board + i));
    fprintf(fp, "\n");
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  char array[] = {'w', 'a', 's', 'd'};
  for (int i = 0; i < 4; i++) {
    if (array[i] == c) {
      return true;
    }
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  char array[] = {'W', 'A', 'S', 'D', 'x'};
  for (int i = 0; i < 5; i++) {
    if (array[i] == c) {
      return true;
    }
  }
  return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  char array[] = {'<', 'v', '>', '^'};
  for (int i = 0; i < 4; i++) {
    if (array[i] == c) {
      return true;
    }
  }
  return is_head(c) || is_tail(c);
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  char bodys[] = {'^', '<', 'v', '>'};
  char tails[] = {'w', 'a', 's', 'd'};
  int i = 0;
  for (; i < 4; i++) {
    if (c == bodys[i]) {
      break;
    }
  }
  return tails[i];
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  char heads[] = {'W', 'A', 'S', 'D'};
  char bodys[] = {'^', '<', 'v', '>'};
  int i = 0;
  for (; i < 4; i++) {
    if (c == heads[i]) {
      break;
    }
  }
  return bodys[i];
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  switch (c) {
  case 'v':
  case 's':
  case 'S':
    return cur_row + 1;
  case '^':
  case 'w':
  case 'W':
    return cur_row - 1;
  default:
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  switch (c) {
  case '>':
  case 'd':
  case 'D':
    return cur_col + 1;
  case '<':
  case 'a':
  case 'A':
    return cur_col - 1;
  default:
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function
  snake_t *cur_snake_ptr = state->snakes + snum;
  char head_char =
      state->board[cur_snake_ptr->head_row][cur_snake_ptr->head_col];
  unsigned int next_row = get_next_row(cur_snake_ptr->head_row, head_char);
  unsigned int next_col = get_next_col(cur_snake_ptr->head_col, head_char);
  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  if (state == NULL) {
    return;
  }
  // update the state borad
  snake_t *snake_ptr = state->snakes + snum;
  unsigned int head_row = snake_ptr->head_row;
  unsigned int head_col = snake_ptr->head_col;
  char head_char = state->board[head_row][head_col];
  state->board[head_row][head_col] = head_to_body(head_char);

  unsigned int next_row = get_next_row(snake_ptr->head_row, head_char);
  unsigned int next_col = get_next_col(snake_ptr->head_col, head_char);
  state->board[next_row][next_col] = head_char;

  // update the snake head
  snake_ptr->head_col = next_col;
  snake_ptr->head_row = next_row;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  if (state == NULL) {
    return;
  }
  // update the state borad
  snake_t *snake_ptr = state->snakes + snum;
  unsigned int tail_row = snake_ptr->tail_row;
  unsigned int tail_col = snake_ptr->tail_col;
  char tail_char = state->board[tail_row][tail_col];
  state->board[tail_row][tail_col] = ' ';

  unsigned int next_row = get_next_row(snake_ptr->tail_row, tail_char);
  unsigned int next_col = get_next_col(snake_ptr->tail_col, tail_char);
  state->board[next_row][next_col] =
      body_to_tail(state->board[next_row][next_col]);

  // update the snake tail
  snake_ptr->tail_col = next_col;
  snake_ptr->tail_row = next_row;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  for (int i = 0; i < state->num_snakes; i++) {
    char next = next_square(state, i);
    if (next == '#' || is_snake(next)) {
      // this snake is dead
      snake_t *snake_ptr = state->snakes + i;
      snake_ptr->live = false;
      state->board[snake_ptr->head_row][snake_ptr->head_col] = 'x';
    } else if (next == '*') {
      update_head(state, i);
      add_food(state);
    } else {
      update_head(state, i);
      update_tail(state, i);
    }
  }
  return;
}

#define INIT_LINE_NUM 5

typedef struct {
  size_t line_num;
  unsigned int *line_lengths;
} line_stat;

line_stat load_line_length(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("fopen error");
    exit(EXIT_FAILURE);
  }

  unsigned int* line_lengths = calloc(INIT_LINE_NUM, sizeof(unsigned int));
  if (line_lengths == NULL) {
    perror("line_lengths malloc error");
    exit(EXIT_FAILURE);
  }
  
  line_stat* lsp = malloc(sizeof(line_stat));
  if (lsp == NULL) {
    perror("line_stat malloc error");
    exit(EXIT_FAILURE);
  }
  
  lsp->line_num = 0;
  lsp->line_lengths = line_lengths;

  char c;
  int curr_line_length = 0;
  int max_line = INIT_LINE_NUM; 
  while ((c = fgetc(fp)) != EOF) {
    if (c == '\n') {
      if (lsp->line_num >= (max_line - 1)) {
        max_line += 5;
        lsp->line_lengths = realloc(lsp->line_lengths, max_line * sizeof(unsigned int));
        if (lsp->line_lengths == NULL) {
          perror("realloc error");
          exit(EXIT_FAILURE);
        } 
      }

      lsp->line_lengths[lsp->line_num++] = curr_line_length;
      curr_line_length = 0;
    } else {
      curr_line_length++;
    }
  }
  fclose(fp);
  return *lsp;
}


/* Task 5 */
game_state_t *load_board(char *filename) {
  // TODO: Implement this function.
  game_state_t *game_state_ptr = malloc(sizeof(game_state_t));
  if (game_state_ptr == NULL) {
    return NULL;
  }

  unsigned int LINE_NUM = 1000;
  unsigned int line_length_array[LINE_NUM];
  line_stat ls = load_line_length(filename);

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    return NULL;
  }

  char** board = malloc(sizeof(char*) * ls.line_num);
  if (board == NULL) {
    return NULL;
  }
  for (int i = 0; i < ls.line_num; i++) {
    char* line = malloc(*(ls.line_lengths + i) + 1);
    if (line == NULL) {
      perror("line malloc error");
      exit(EXIT_FAILURE);
    }
    fgets(line, *(ls.line_lengths + i) + 1, fp);
    *(board + i) = line;
    // consume the '\n'
    fgetc(fp);
  }


  game_state_ptr->board = board;
  game_state_ptr->num_rows = ls.line_num;

  game_state_ptr->num_snakes = 0;
  game_state_ptr->snakes = NULL;

  fclose(fp);
  return game_state_ptr;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  return NULL;
}
