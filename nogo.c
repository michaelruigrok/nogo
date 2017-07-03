#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "nogo.h"

/*
 * print the string ID of each square
 */
void list_ids(struct GameState* game) {
    for (short row = 0; row < game->height; row++) {
        for (short column = 0; column < game->width; column++) {
            printf("%d, ", game->stringIds[row][column]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {

    struct GameState gameState;
    gameState.started = false;

    arg_parse(&gameState, argc, argv);
    draw_board(&gameState);
    gameState.started = true;

    char* input = malloc(sizeof(char) * 71); //max input length is 70

    while (true) {

        if (!get_input(&gameState, input)) {
            continue; //no valid input, continue
        }

        if (input[0] == '\n') {
            continue; //no input, continue
        } else if (input[0] == 'w') {
            //save game, input's value after w is a file
            save_game(&gameState, input + 1); 
            continue;
        } else if (input[0] == '~') {
            list_ids(&gameState);
            continue;
        }

        //get first argument as an integer integer using strtok
        int row = strtol(strtok(input, " "), NULL, 10);
        int column = next_tok_arg(); //get next argument as integer with strtok

        if (!place_stone(&gameState, row, column)) {
            continue; //invalid stone, continue
        }

        draw_board(&gameState);

        if (update_strings(&gameState, row, column)) {
            return 0; // a stone was captured, end the game
        }

        next_player(&gameState);
    }
    quit(6);
} 

/*
 * prints an error message corresponding to the given value,
 * and exits using that value as an exit status
 */
void quit(int exitValue) {
    switch (exitValue) {
        case 1:
            fprintf(stderr, "Usage: nogo p1type p2type [height width | "
                    "filename]\n");
            exit(1);
        case 2:
            fprintf(stderr, "Invalid player type\n");
            exit(2);
        case 3:
            fprintf(stderr, "Invalid board dimension\n");
            exit(3);
        case 4:
            fprintf(stderr, "Unable to open file\n");
            exit(4);
        case 5:
            fprintf(stderr, "Incorrect file contents\n");
            exit(5);
        case 6:
            fprintf(stderr, "End of input from user\n");
            exit(6);
    }
}

/*
 * changes the next player value to the next player
 */
void next_player(struct GameState* game) {

    if (game->nextPlayer == 'X') {
        game->nextPlayer = 'O';
    } else {
        game->nextPlayer = 'X';
    }
}

/* 
 * Parses the arguments given to main into the game struct.
 *
 * If arguments are incorrect, this function make call exit()
 * in the form of quit()
 *
 */
void arg_parse(struct GameState* game, int runtimeArgCount,
        char** runtimeArgs) {

    //check for correct number of arguments
    if (runtimeArgCount != 4 && runtimeArgCount != 5) {
        quit(1);
    }

    //the first and second arg are p1's type and p2's type respectively
    char p1type = *runtimeArgs[1], p2type = *runtimeArgs[2];

    //check player type arguments are equal to only 'h' or 'c'
    if (strlen(runtimeArgs[1]) > 1 || strlen(runtimeArgs[2]) > 1 ||
            (p1type != 'h' && p1type != 'c') ||
            (p2type != 'h' && p2type != 'c')) {
        quit(2);
    }
    game->p1type = p1type;
    game->p2type = p2type;

    int height, width;
    char* validIntCheck; //to see if strtol returns valid numbers
    height = strtol(runtimeArgs[3], &validIntCheck, 10);

    //check if height and width are non-numbers or are outside bounds
    if (*validIntCheck) {
        //height is non-numerical, try and load it as a file
        load_file(game, runtimeArgs[3]);

        //check each stone for adjacent stones and find appropriate string IDs
        game->stringIdCount = 0;
        for (short row = 0; row < game->height; row++) {
            for (short column = 0; column < game->width; column++) {
                if (get_stone(game, row, column) && 
                        get_stone(game, row, column) != '.') {
                    update_strings(game, row, column);
                }
            }
        }
    } else if (runtimeArgCount != 5) {
        quit(1); //no file was found and no width argument was found
    } else if (!(width = strtol(runtimeArgs[4], &validIntCheck, 10)) ||
            *validIntCheck || !in_size_bounds(height, width)) {
        quit(3); //width is invalid
    } else {
        //apply valid values to game state
        game->height = height;
        game->width = width;
        init_game_variables(game);
        init_board(game);
    }
}

/*
 * returns whether or not the given width and height are reasonable
 */
bool in_size_bounds(int height, int width) {
    return (height >= 4 && height <= 1000 && width >= 4 && width <= 1000);
}

/*
 * returns next appropriate value from strtok after strtok has been initialised
 */
int next_tok_arg(void) {

    char* validIntCheck;
    char* nextArg;
    if (!(nextArg = strtok(NULL, " "))) {
        //if the next argument isn't anything, return 0, 
        //which is always out of range
        return 0;
    }
    //otherwise convert to an integer and return
    int next = strtol(nextArg, &validIntCheck, 10);

    return next;
}

/*
 * loads the contents of a saved game file into the  game state
 */
void load_file(struct GameState* game, char* filename) {

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        quit(4);
    }

    //parse first line of environment variables
    char args[70]; // 70 comfortably covers the longest possible arguments
    fgets(args, 70, file);

    //is there anything there at all?
    if (!args[0]) {
        quit(5);
    }

    parse_first_line(game, args);
    init_board(game);

    for (int i = 0; i < game->height; i++) {
        if (!fgets(args, game->width + 2, file) ||
                !update_row(game, i, args)) {
            quit(5);
        }
    }
    fclose(file);

}

/*
 * saves the current game state as a file
 */
void save_game(struct GameState* game, char* filename) {

    //replace the next line character with null
    int i = 0;
    while (filename[i] != '\n') {
        i++;
    }
    filename[i] = '\0';

    FILE* file = fopen(filename, "w");

    fprintf(file, "%d %d %d %d %d %d %d %d %d\n", game->height, game->width, 
            (game->nextPlayer == 'X'), game->nextMoveOY, game->nextMoveOX,
            game->moveCountO, game->nextMoveXY, game->nextMoveXX,
            game->moveCountX);

    //print each line of the game's board to file
    for (int i = 0; i < game->height; i++) {
        fprintf(file, "%s\n", game->board[i]);
    }
    fclose(file);

}

/* 
 * loads environment variables from a given string
 */
void parse_first_line(struct GameState* game, char* args) {
    int nextArg;

    //height and width, get arguments and convert both to integers
    int height = strtol(strtok(args, " "), NULL, 10);
    int width = strtol(strtok(NULL, " "), NULL, 10);
    if (!in_size_bounds(height, width)) {
        quit(5);
    }
    game->height = height;
    game->width = width;

    //next player (either 1 or 0)
    nextArg = next_tok_arg();
    if (!nextArg) {
        game->nextPlayer = 'O';
    } else if (nextArg == 1) {
        game->nextPlayer = 'X';
    } else {
        quit(5);
    }

    //O's next co-ordinates to attempt
    //Make sure both are within the grid bounds
    if (!on_grid_y(game, game->nextMoveOY = next_tok_arg()) || 
            !on_grid_x(game, game->nextMoveOX = next_tok_arg())) {
        quit(5);
    }

    //the number of moves O has made
    nextArg = next_tok_arg();
    if (nextArg > height * width / 2 || nextArg < 0) {
        quit(5); //the number of moves is to small or large for the given board
    }
    game->moveCountO = nextArg;

    //X's next co-ordinates to attempt
    //Make sure both are within the grid bounds
    if (!on_grid_y(game, game->nextMoveXY = next_tok_arg()) || 
            !on_grid_x(game, game->nextMoveXX = next_tok_arg())) {
        quit(5);
    }

    //the number of moves X has made
    nextArg = next_tok_arg();
    if (nextArg > height * width / 2 || nextArg < 0) {
        quit(5); //the number of moves is to small or large for the given board
    }
    game->moveCountX = nextArg;
}

/*
 * get the appropriate input for the player types
 */
char* get_input(struct GameState* game, char* input) {

    if (game->nextPlayer == 'X' && game->p2type == 'c') {
        if (get_stone(game, game->nextMoveXY, game->nextMoveXX) == '.') {
            //if p2's next given move's square is free, the move is valid
            sprintf(input, "%d %d\n", game->nextMoveXY, game->nextMoveXX);
            printf("Player %c: %s", game->nextPlayer, input);
        } else {
            input = 0;
        }
        next_cpu_move(game);

    } else if (game->nextPlayer == 'O' && game->p1type == 'c') {
        if (get_stone(game, game->nextMoveOY, game->nextMoveOX) == '.') {
            //if p1's next given move's square is free, the move is valid
            sprintf(input, "%d %d\n", game->nextMoveOY, game->nextMoveOX);
            printf("Player %c: %s", game->nextPlayer, input);
        } else {
            input = 0;
        }
        next_cpu_move(game);

    } else {
        printf("Player %c> ", game->nextPlayer); 

        if (!fgets(input, 72, stdin)) {
            quit(6);
        }
        if (feof(stdin)) {
            quit(6);
        } else if (strlen(input) > 70) {
            input = "0";
            return input;
        }
    }
    return input;
}


/*
 * initialise new game variables as their default values
 */
void init_game_variables(struct GameState* game) {
    game->nextPlayer = 'O';
    game->nextMoveOY = 1;
    game->nextMoveOX = 4 % game->width;
    game->moveCountO = 0;
    game->nextMoveXY = 2;
    game->nextMoveXX = 10 % game->width;
    game->moveCountX = 0;
    game->stringIdCount = 0;

}


/*
 * Initialise the go board for the given height and width,
 * and an equivalent two dimensional array for each squares string Id.
 * Populate each with '.'s and 0's respectively
 *
 */
char** init_board(struct GameState* game) {

    int width = game->width;
    int height = game->height;

    //initialise the board, fill with '.'
    char** board = malloc(sizeof(char*) * height);
    short column;
    for (short row = 0; row < game->height; row++) {
        board[row] = malloc(sizeof(char) * width + 1); //extra element for '\0'
        for (column = 0; column < width; column++) {
            board[row][column] = '.';
        }
        board[row][column] = '\0';
    }

    //initialise the string ID array, fill with 0
    int** stringIds = malloc(sizeof(int*) * height);
    for (short row = 0; row < game->height; row++) {
        stringIds[row] = malloc(sizeof(int) * width);
        for (column = 0; column < width; column++) {
            stringIds[row][column] = 0;
        }
    }

    game->board = board;
    game->stringIds = stringIds;
    return board;

}

/*
 * check to see if x is a valid column on the board
 */
bool on_grid_x(struct GameState* game, int x) {
    return (x < game->width && x >= 0);
}

/*
 * check to see if y is a valid row on the board
 */
bool on_grid_y(struct GameState* game, int y) {
    return (y < game->height && y >= 0);
}

/*
 * returns the value of a stone, or null if it does not exists
 */
char get_stone(struct GameState* game, short row, short column) {
    if (!on_grid_y(game, row) || !on_grid_x(game, column)) {
        return '\0';
    }
    return game->board[row][column];
}

/*
 * return true iff an adjacent square is empty
 */
bool check_liberties(struct GameState* game, short row, short column) {
    return square_empty(game, row, column + 1) ||
            square_empty(game, row, column - 1) ||
            square_empty(game, row + 1, column) ||
            square_empty(game, row - 1, column);
}

/*
 * returns true if there is the opposite stone in the surrounding area
 */
bool nearby_opposing_stones(struct GameState* game, short row, short column) {
    return stone_opposing(game, row, column + 1) ||
            stone_opposing(game, row, column - 1) ||
            stone_opposing(game, row + 1, column) ||
            stone_opposing(game, row - 1, column);
}

/*
 * return true iff a square is empty
 */
bool square_empty(struct GameState* game, short row, short column) {
    return (get_stone(game, row, column) == '.');
}

/*
 * return true iff a square contains a stone opposite to the current player
 */
bool stone_opposing(struct GameState* game, short row, short column) {
    if (game->nextPlayer == 'X') {
        return (get_stone(game, row, column) == 'O');
    } else if (game->nextPlayer == 'O') {
        return (get_stone(game, row, column) == 'X');
    }
    return 0;
}

/*
 * inserts square values into the given board row
 *
 * returns true iff successful
 */
bool update_row(struct GameState* game, short row, char* values) {
    for (short int i = 0; i < game->width; i++) {
        if (values[i] != 'X' && values[i] != 'O' && values[i] != '.') {
            return false; //invalid value, return false
        }
        game->board[row][i] = values[i];
    }
    return strlen(values) == game->width + 1;
}

/*
 * place the next player's stone on a square if it's empty
 *
 * returns true iff successful
 */
bool place_stone(struct GameState* game, short row, short column) {
    if (get_stone(game, row, column) != '.') {
        return false;
    }

    game->board[row][column] = game->nextPlayer;
    return true;
}

/*
 * For a given stone, combine any adjacent strings to form a single string,
 * with the lowest string ID of the previous strings. If necessary, create a
 * new string with a new string ID
 */
bool update_strings(struct GameState* game, short row, short column) {
    int** stringIds = game->stringIds;

    int* oldIds = get_adjacent_string_ids(game, row, column);

    int oldIdCount = next_adjacent_string(oldIds); //the number of new IDs
    if (oldIdCount == 0) {
        //no adjacent stones exists
        return check_for_captures(game, row, column);
    }

    //the last ID in the list is the smallest, give the given stone its value
    int newId = oldIds[oldIdCount - 1];
    stringIds[row][column] = newId;

    //Any adjacent stones that have a string ID of INT_MAX have no string,
    //give them the new string ID.
    replace_int_max(game, row, column - 1, newId);
    replace_int_max(game, row, column + 1, newId);
    replace_int_max(game, row - 1, column, newId);
    replace_int_max(game, row + 1, column, newId);

    //return if there was only 1 string ID, no strings have to be re-ID'd
    if (oldIdCount == 1) {
        return check_for_captures(game, row, column);
    }
    oldIds[oldIdCount - 1] = 0;

    //iterate over the board, replacing any stringIds in oldIDs with the new 1

    int bigger; //the number of oldIDs the current stringID is bigger than
    for (short row = 0; row < game->height; row++) {
        for (short column = 0; column < game->width; column++) {
            bigger = 0;
            for (int* id = oldIds; *id > 0 && id < (id + 4); id++) {
                if (*id == stringIds[row][column]) {
                    stringIds[row][column] = newId; //replace old id with new
                } else if (stringIds[row][column] > *id) {
                    bigger++; //if the id is bigger than old id, increment
                }
            }
            stringIds[row][column] -= bigger;
        }
    }
    game->stringIdCount -= oldIdCount; //remove all old IDs from the id count
    //free(oldIds);
    return check_for_captures(game, row, column);
}

/*
 * returns first unused element in the string of adjacent squares,
 * or 4 if they are all used
 */
int next_adjacent_string(int* strings) {
    for (int i = 0; i < 4; i++) {
        if (strings[i] == 0) {
            return i;
        }
    }
    return 4;

}

/*
 * replaces an INT_MAX value in a given square with the given value
 */
void replace_int_max(struct GameState* game, short row, short column,
        int new) {
    if (get_stone(game, row, column) && 
            game->stringIds[row][column] == INT_MAX) {
        game->stringIds[row][column] = new;
    }

}

/*
 * returns an array containing the string ids of all equivalently valued
 * adjacent strings, with the smallest Id at the end
 *
 * all unused elements are 0
 */
int* get_adjacent_string_ids(struct GameState* game, short row, short column) {

    char currentStone = get_stone(game, row, column);
    int* oldIds = malloc(sizeof(int) * 4); //there are up to 4 adjacent stones

    for (int i = 0; i < 4; i++) {
        oldIds[i] = 0; //initialise

    }
    int smallestId = INT_MAX;

    //for each adjacent square, add its string ID to the array
    smallestId = add_string_to_array(game, oldIds, smallestId, currentStone,
            row, column + 1);
    smallestId = add_string_to_array(game, oldIds, smallestId, currentStone,
            row, column - 1);
    smallestId = add_string_to_array(game, oldIds, smallestId, currentStone, 
            row + 1, column);
    smallestId = add_string_to_array(game, oldIds, smallestId, currentStone, 
            row - 1, column);

    //if smallestID is still INT_MAX, no strings were found
    if (smallestId == INT_MAX) {
        //if the last ID is INT_MAX, stones without strings were found
        if (oldIds[3] == 0) {
            return oldIds; //no stones were found
        }
        //only IDs of 0 were found, generate a new ID for them
        oldIds[3] = 0;
        smallestId = ++(game->stringIdCount);
    }

    //Append the smallest ID to the end
    oldIds[next_adjacent_string(oldIds)] = smallestId;
    return oldIds;

}

/*
 * Add the stringId of a given square, or smallestID to an array, whichever is
 * larger. Return the other value.
 *
 * smallestID is the smallest ID of an adjacent stone found so far by 
 * get_adjacent_string_ids(), and currentStone is the stone value being 
 * compared to
 */
int add_string_to_array(struct GameState* game, int* stones, 
        int smallestId, char currentStone, short row, short column) {

    int** stringIds = game->stringIds;
    int tempId; //the ID of the stone being currently evaluated

    //ensure the stones are equivalent
    if (currentStone == get_stone(game, row, column)) {
        tempId = stringIds[row][column];
        if (tempId == 0) {
            /* Stone is not part of string. Make the ID either:
               - the smallest (i.e. its intended value),
               - the same as another adjacent stone (so it will be changed), or
               - INT_MAX (which will be checked for manually)
             */
            stringIds[row][column] = smallestId;
            stones[3] = INT_MAX; //set flag, check for surrounding INT_MAX's

        } else if (smallestId == INT_MAX) {
            smallestId = tempId; //No strings have been found yet

        } else if (smallestId > tempId) {
            stones[next_adjacent_string(stones)] = smallestId;
            smallestId = tempId;

        } else if (smallestId < tempId) {
            //if the ID is already in the list, don't add it again
            for (int* id = stones; *id != 0; id++) {
                if (tempId == *id) {
                    return smallestId;
                }
            }
            stones[next_adjacent_string(stones)] = tempId;
        }
    }

    return smallestId;

}

/*
 * Iterate over the entire board, checking for captured strings.
 * Return true iff a string has been captured
 *
 * The meat of this function operated within two loops that frequently
 * have the potential to return, as well as frequently iterate over
 * and modifying an array as part of such checks.
 * As such, it would be unnecessarily messy to separate very specialised checks
 * into separate functions.
 *
 */
bool check_for_captures(struct GameState* game, short row, short column) {

    //if the game hasn't started yet, or there isn't anything to be captured
    if (!game->started || !nearby_opposing_stones(game, row, column)) {
        return false;
    }

    short width = game->width, height = game->height;
    
    //for every string, keep track of whether or not it's been captured
    bool* captured = malloc(sizeof(bool) * (game->stringIdCount));
    char losingStone = 0;

    for (int string = 0; string < game->stringIdCount; string++) {
        //initialise as captured, no liberties have been found
        captured[string] = true;
    }

    //for each square...
    for (short row = 0; row < height; row++) {
        for (short column = 0; column < width; column++) {

            if (square_empty(game, row, column)) {
                continue;
            }

            //If it is a solitary stone with no liberties, mark as captured
            if (game->stringIds[row][column] == 0) { 
                if (!check_liberties(game, row, column)) {
                    if ((losingStone = get_stone(game, row, column)) !=
                            game->nextPlayer) {
                        /* If the captured stone belongs to the opponent, the 
                        game ends immediately. Otherwise, keep checking to
                        see if the current player has captured the enemy */
                        free(captured);
                        printf("Player %c wins\n", game->nextPlayer);
                        return true;
                    }
                }
            } else if (check_liberties(game, row, column)) {
                //if the stone has liberties, mark its string as not captured
                captured[game->stringIds[row][column] - 1] = false;
            }
        }
    }

    for (int string = 0; string < game->stringIdCount; string++) {
        if (captured[string]) {
            //find the value of the stones featuring the string id
            for (short row = 0; row < height; row++) {
                for (short column = 0; column < width; column++) {
                    if (game->stringIds[row][column] - 1 == string && 
                            (losingStone = get_stone(game, row, column)) != 
                            game->nextPlayer) {
                        /* If the captured stone belongs to the opponent, the 
                        game ends immediately. Otherwise, keep checking to
                        see if the current player has captured the enemy */
                        free(captured);
                        printf("Player %c wins\n", game->nextPlayer);
                        return true;
                    }
                }
            }
        }
    }

    //If a losing stone exists, but the current player hasn't claimed a
    //victory, he has doomed himself to defeat.
    if (losingStone) {
        free(captured);
        if (game->nextPlayer == 'X') {
            printf("Player O wins\n");
        } else {
            printf("Player X wins\n");
        }
        return true;
    }
    free(captured);
    return false;
}

/*
 * prints the current board state to standard output
 */
void draw_board(struct GameState* game) {

    //top border
    printf("/");
    for(int i = 1; printf("-"), i < game->width; i++);
    printf("\\\n");

    for (short row = 0; row < game->height; row++) {
        printf("|%s|\n", game->board[row]);
    }

    //bottom border
    printf("\\");
    for(int i = 1; printf("-"), i < game->width; i++);
    printf("/\n");
} 


/*
 * update the next move values appropriately for the next player
 */
void next_cpu_move(struct GameState* game) {
    int initialRow, initialColumn;
    int* counter, *nextMoveX, *nextMoveY;
    int factor;

    //apply player-specific variables for move generation algorithm
    if (game->nextPlayer == 'X') {
        counter = &(game->moveCountX);

        initialRow = 2;
        initialColumn = 10;
        nextMoveX = &game->nextMoveXX;
        nextMoveY = &game->nextMoveXY;
        factor = 17;
    } else {
        counter = &(game->moveCountO);

        initialRow = 1;
        initialColumn = 4;
        nextMoveX = &game->nextMoveOX;
        nextMoveY = &game->nextMoveOY;
        factor = 29;
    }

    do {
        generate_cpu_move(game, initialRow, initialColumn, counter, nextMoveY, 
                nextMoveX, factor);
    } while (!square_empty(game, *nextMoveY, *nextMoveX));

}


/*
 * Generate the a game move for the given parameters
 */
void generate_cpu_move(struct GameState* game, int initialRow, 
        int initialColumn, int* counter, int* nextMoveY, int* nextMoveX, 
        int factor) {

    int temp;
    (*counter)++;

    switch (*counter % 5) {
        case 0:
            temp = (initialRow * game->width + initialColumn);
            temp = (temp + *counter / 5 * factor) % 1000003;
            *nextMoveY = (temp / game->width);
            *nextMoveX = (temp % game->width);
            break;
        case 1:
            (*nextMoveX)++;
            (*nextMoveY)++;
            break;
        case 2:
            (*nextMoveX)++;
            (*nextMoveY) += 2;
            break;
        case 3:
            (*nextMoveY)++;
            break;
        case 4:
            (*nextMoveX)++;

    }

    //ensure next moves are on the same grid
    *nextMoveY %= game->height;
    *nextMoveX %= game->width;
}
