
#include "utils.h"

/// @brief Get a random double from 0 to 1 [0;1]
/// @return
double get_random()
{
    unsigned int seed = (unsigned int)time(NULL);
    double r = (double)rand_r(&seed) / (double)RAND_MAX;
    return r;
}

/// @brief Get a random integer bounded from lower bound lBound to upper bound  uBound
/// @param seed the reference of the seed used in the pseudo random number generator
/// @param lBound the lower bound of the range
/// @param uBound the upper bound of the range
/// @return a random integer bounded in range [lBound;uBound]
int get_bound_random(unsigned int *seed, unsigned int lBound, unsigned int uBound)
{
    int r = (int)rand_r(seed) % (uBound - lBound + 1) + lBound;
    return r;
}

/// @brief Function which reads a specified file containing various sudoku puzzles, identified by their unique hash
/// and writes the puzzle's grid into a 2D integer line array
/// @param filename the specified file with the corresponding scheme:
///      12 bytes of SHA1 hash of the digits string (for randomising order)
///      81 bytes of puzzle digits
///       4 bytes of rating (nn.n)
///       3 bytes of white-space (including the linefeed);
///     100 bytes total
/// @param sudoku_dimension the dimension of the sudoku in the file
/// @param puzzle_hash the specific sudoku puzzle to read
/// @return 
int **read_sudoku_file(char *filename, size_t sudoku_dimension, char *puzzle_hash)
{
    int fd;
    int pos = 0;

    char c;                     // current character
    char *line_buffer;          // buffer of read characters
    size_t bytes_read;          // numbers of characters read
    off_t size = LINE_SIZE + 1; // the size of each line

    // The elements to get from each line of the given file
    double difficulty;
    char hash[HASH_SIZE + 1];
    char puzzle[PUZZLE_SIZE + 1];

    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        perror("Problem encountered when opening puzzle file");
        exit(EXIT_FAILURE);
    }

    if ((line_buffer = (char *)malloc(sizeof(char) * size)) == NULL)
    {
        fprintf(stderr, "ERROR: Out of memory!!!\n");
        exit(EXIT_FAILURE);
    }

    // This loop iterates through each character in the line of size LINE_SIZE (100 bytes),
    // get the hash, puzzle and difficulty form the line and start again at the next line

    lseek(fd, 0, SEEK_SET); // make suze to start at the start of the file
    while ((bytes_read = read(fd, &c, sizeof(char))) > 0)
    {
        if (c == '\n') // end of current sudoku grid entry
        {
            line_buffer[pos] = '\0';

            printf("%s\n", line_buffer);
            if (sscanf(line_buffer, "%s %s %lf", hash, puzzle, &difficulty) != 3)
            {
                fprintf(stderr, "ERROR: invalid input line %s|\n|", line_buffer);
                break;
            }
#if _DEBUG_
            printf("Sudoku file %s | %s | %f\n", hash, puzzle, difficulty);
#endif
            // if the specific puzzle hash is given, then retrieve it from given file
            if (puzzle_hash != (char *)NULL)
            {
                if (strcmp(hash, puzzle_hash) == 0)
                {
                    break;
                }
            }

            pos = 0;
            line_buffer[pos] = '\0'; // empty the line buffer
        }
        else
        {
            line_buffer[pos++] = c;
        }
    }
    // free the line buffer and close the file handler
    free(line_buffer);
    close(fd);

    int **puzzle_grid;
    int lines = SUDOKU_SIZE, cols = SUDOKU_SIZE;

    if ((puzzle_grid = (int **)malloc(sizeof(int *) * lines)) == NULL)
    {
        fprintf(stderr, "ERROR: Out of memory!!\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < lines; i++)
        puzzle_grid[i] = (int *)malloc(sizeof(int) * cols);

    int line = 0;
    for (int i = 0; i < PUZZLE_SIZE; i++)
    {
        if (i != 0 && i % SUDOKU_SIZE == 0)
            line++;
        puzzle_grid[line][i % SUDOKU_SIZE] = puzzle[i] - '0';
    }

    return puzzle_grid;
}

/// @brief Prints a sudoku grid from a 2D array of pointers, assuming the 2D grid is structured with lines and columns
/// @param sudoku_grid
void print_sudoku_pointers(int ***sudoku_grid)
{
    int i, j;
    printf("╭-------+-------+-------╮\n");
    for (i = 0; i < SUDOKU_SIZE; i++)
    {
        for (j = 0; j < SUDOKU_SIZE; j++)
        {
            if (j == 0)
                printf("| ");
            printf("%d ", *sudoku_grid[i][j]);
            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf("├-------+-------+-------┤\n");
    }
}

/// @brief Prints a sudoku grid from a 2D array, assuming the 2D grid is structured with lines and columns
/// @param sudoku_grid
void print_sudoku(int **sudoku_grid)
{
    int i, j;
    printf("╭-------+-------+-------╮\n");
    for (i = 0; i < SUDOKU_SIZE; i++)
    {
        for (j = 0; j < SUDOKU_SIZE; j++)
        {
            if (j == 0)
                printf("| ");
            printf("%d ", sudoku_grid[i][j]);
            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf("├-------+-------+-------┤\n");
    }
}

/// @brief Utility function to append the results obtained from the algorithm to a file
/// @param filename the file in question, is created if doesn't exist yet. We assume it corresponds to the sudoku hash
/// @param score the value returned from the cost function in the sudoku solving algorithm
/// @param n_try the current try number
/// @param date
void sudoku_write_stats(char * filename, int score, int n_try, char * date) {
    char file[FILE_SIZE];
    snprintf(file, FILE_SIZE, "%s%s-%s.txt", "./data/", filename, date);

    FILE *fp = fopen(file, "a+");

    if(!fp) fp = fopen(file, "w+");
    if(!fp) {
        fprintf(stderr, "Can't open file for statistics of sudoku [%s]\n", filename);
        return;
    }

    fprintf(fp, "%d %d\n", n_try, score);

    fclose(fp);
}

/// @brief Utility function to print the debug outputinfo  of the sudoku algorithm to the specified file
/// @param filename the specified file
/// @param info the debug info to send to tthe file 
/// @param date the current file execution timestamp (h:m:s)
void sudoku_debug_output(char * filename, char * info, char * date) {
    char file[FILE_SIZE];
    snprintf(file, FILE_SIZE, "%s%s-%s.txt", "./debug/", filename, date);

    FILE *fp = fopen(file, "a+");

    if(!fp) fp = fopen(file, "w+");
    if(!fp) {
        fprintf(stderr, "Can't open file for statistics of sudoku [%s]\n", filename);
        return;
    }

    fprintf(fp, "> [%s]\n", info);

    fclose(fp);
}