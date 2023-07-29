#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "state.h"

int main(int argc, char* argv[]) {
  char* in_filename = NULL;
  char* out_filename = NULL;
  game_state_t* state = NULL;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i < argc - 1) {
      in_filename = argv[i + 1];
      i++;
      continue;
    }
    if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
      out_filename = argv[i + 1];
      i++;
      continue;
    }
    fprintf(stderr, "Usage: %s [-i filename] [-o filename]\n", argv[0]);
    return 1;
  }

  // Do not modify anything above this line.

  /* Task 7 */

  // Read board from file, or create default board
  if (in_filename != NULL) {
    state = load_board(in_filename);
    if (state == NULL) {
      return -1;
    }
    initialize_snakes(state);
    // TODO: Load the board from in_filename
    // TODO: If the file doesn't exist, return -1
    // TODO: Then call initialize_snakes on the state you made
  } else {
    state = create_default_state();
    // TODO: Create default state
  }

  // TODO: Update state. Use the deterministic_food function
  // (already implemented in snake_utils.h) to add food.
  update_state(state, deterministic_food);

  // Write updated board to file or stdout
  if (out_filename != NULL) {
    // TODO: Save the board to out_filename
    FILE* fw = fopen(out_filename, "w");
    if (fw == NULL) {
      return -1;
    }
    print_board(state, fw);
  } else {
    print_board(state, stdout);
    // TODO: Print the board to stdout
  }
  free_state(state);

  // TODO: Free the state

  return 0;
}
