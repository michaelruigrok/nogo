#include <stdbool.h>

/* a struct containing all the variables related to the game's state */
struct GameState {
    char p1type; //whether or not player 1 is a [c]omputer or a [h]uman
    char p2type; //whether or not player 2 is a [c]omputer or a [h]uman
    short height; //the number of vertical squares on the board
    short width; //the number of horizontal squares on the board
    char nextPlayer; //X or O
    bool started; //false iff the game is being initialised

    int nextMoveOY; //the next move of player 'O', on the Y axis
    int nextMoveOX; //the next move of player 'O', on the X axis
    int moveCountO; //the number of moves player 'O' has made
    int nextMoveXY; //the next move of player 'X', on the Y axis
    int nextMoveXX; //the next move of player 'X', on the X axis
    int moveCountX; //the number of moves player 'X' has made

    char** board; //a 2D array, containing the stone values of each square

    /* For each square, stringIDs stores  a number is identifying which string 
     * a stone is part of, or zero if it is not in a string with any other 
     * stone.
     */
    int** stringIds;
    int stringIdCount; //the number of non-zero ID's that exist

};

void quit(int exitValue);
void arg_parse(struct GameState* game, int argc, char** argv);
void next_player(struct GameState* game);
void load_file(struct GameState* game, char* filename);
void save_game(struct GameState* game, char* filename);
void parse_first_line(struct GameState* game, char* args);
int next_tok_arg(void);
bool in_size_bounds(int width, int height);

char* get_input(struct GameState* game, char* input);

void init_game_variables(struct GameState* game);
char** init_board(struct GameState* game);
bool on_grid_x(struct GameState* game, int x);
bool on_grid_y(struct GameState* game, int y);
bool check_for_captures(struct GameState* game, short row, short column);
bool update_row(struct GameState* game, short row, char* values);
bool place_stone(struct GameState* game, short row, short column);
int* get_adjacent_string_ids(struct GameState* game, short row, short column);
int add_string_to_array(struct GameState* game, int* stones, 
        int smallestId, char currentStone, short row, short column);
bool update_strings(struct GameState* game, short row, short column);
int next_adjacent_string(int* strings);
void replace_int_max(struct GameState* game, short row, short column,
        int new);
void draw_board(struct GameState* game);
char get_stone(struct GameState* game, short row, short column);
bool nearby_liberties(struct GameState* game, short row, short column);
bool nearby_opposing_stones(struct GameState* game, short row, short column);
bool square_empty(struct GameState* game, short row, short column);
bool stone_opposing(struct GameState* game, short row, short column);

void generate_cpu_move(struct GameState* game, int initialRow, 
        int initialColumn, int* counter, int* nextMoveY, int* nextMoveX, 
        int factor);
void next_cpu_move(struct GameState* game);
