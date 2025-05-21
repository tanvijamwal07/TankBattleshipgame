#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

// terminal colors
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define RESET "\033[0m"

// Game settings
#define WIDTH 30
#define HEIGHT 15
#define MAX_BULLETS 5
#define MAX_ENEMIES 6
#define MAX_PLAYER_NAME 20
#define MAX_HIGH_SCORES 10  // Increased to show more high scores
#define HIGH_SCORE_FILE "high_scores.txt"

// Game board and objects
char board[HEIGHT][WIDTH];
int player_x, player_y;
int player_lives;
int score;
char player_name[MAX_PLAYER_NAME];

//Bullet structure
typedef struct {
    int x;
    int y;
    int active;
} Bullet;

//enemy structure
typedef struct {
    int x;
    int y;
    int active;
} Enemy;

// High score node for linked list
typedef struct ScoreNode {
    char name[MAX_PLAYER_NAME];
    int score;
    struct ScoreNode* next;
} ScoreNode;

// Linked list head pointer
ScoreNode* high_score_list = NULL;
int num_high_scores = 0;

// Game objects
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];

// Terminal settings
struct termios original_terminal;

// Function prototypes
void setup_terminal();
void reset_terminal();
void clear_screen();
void initialize_game();
void show_welcome();
void get_player_name();
void play_game();
void update_game();
void render_game();
void move_player(char direction);
void fire_bullet();
void create_enemy();
void check_collisions();
void show_game_over();
void show_game_over_menu();
void load_high_scores();
void save_high_scores();
void add_high_score(const char* name, int score);
void display_high_scores();
int is_high_score(int score);
void display_about_developers();
int min(int a, int b); // Added min function prototype

// Linked list functions
ScoreNode* create_score_node(const char* name, int score);
void free_score_list();
ScoreNode* binary_search_score(int target_score);
ScoreNode* find_player_entry(const char* name);

// Set up terminal for game input
void setup_terminal() {
    tcgetattr(STDIN_FILENO, &original_terminal);
    struct termios raw = original_terminal;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Reset terminal to original state
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal);
}

// Clear the screen
void clear_screen() {
    printf("\033[2J\033[H");
}

// Initialize the game state
void initialize_game() {
    // Set up the player
    player_x = WIDTH / 2;
    player_y = HEIGHT - 2;
    player_lives = 3;  // Back to 3 lives as requested
    score = 0;

    // Clear bullets and enemies
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }

    // Create all enemies (MAX_ENEMIES)
    for (int i = 0; i < MAX_ENEMIES; i++) {
        create_enemy();
    }
}

// Show welcome menu and get player choice
void show_welcome() {
    clear_screen();
    printf("\n\n");
    printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
    printf("    %s║       TANK BATTLESHIP         ║%s\n", YELLOW, RESET);
    printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

    printf("    %sMAIN MENU:%s\n\n", GREEN, RESET);
    printf("    %s1.%s Start Game\n", CYAN, RESET);
    printf("    %s2.%s View High Scores\n", CYAN, RESET);
    printf("    %s3.%s About Developers\n", CYAN, RESET);
    printf("    %s4.%s Quit\n\n", CYAN, RESET);

    printf("    %sEnter your choice (1-4): %s", MAGENTA, RESET);

    char choice;
    int valid_input = 0;

    while (!valid_input) {
        if (read(STDIN_FILENO, &choice, 1) > 0) {
            if (choice >= '1' && choice <= '4') {
                valid_input = 1;
            } else {
                // Show invalid input message
                clear_screen();
                printf("\n\n");
                printf("    %s╔═══════════════════════════════╗%s\n", RED, RESET);
                printf("    %s║         INVALID INPUT         ║%s\n", RED, RESET);
                printf("    %s╚═══════════════════════════════╝%s\n\n", RED, RESET);
                printf("    %sInvalid Input dalta h Gadhee common sense use krleeee....!!!%s\n\n", MAGENTA, RESET);

                // Wait for 3 seconds
                sleep(3);

                // welcome menu
                clear_screen();
                printf("\n\n");
                printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
                printf("    %s║     TANK BATTLESHIP           ║%s\n", YELLOW, RESET);
                printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

                printf("    %sMAIN MENU:%s\n\n", GREEN, RESET);
                printf("    %s1.%s Start Game\n", CYAN, RESET);
                printf("    %s2.%s View High Scores\n", CYAN, RESET);
                printf("    %s3.%s About Developers\n", CYAN, RESET);
                printf("    %s4.%s Quit\n\n", CYAN, RESET);

                printf("    %sEnter your choice (1-4): %s", MAGENTA, RESET);
            }
        }
        usleep(100000); // Wait a bit before checking again
    }

    switch (choice) {
        case '1':
            get_player_name();
            play_game();
            break;
        case '2':
            display_high_scores();
            show_welcome(); // Return to welcome screen after showing high scores
            break;
        case '3':
            display_about_developers();
            show_welcome(); // Return to welcome screen after showing developer info
            break;
        case '4':
            reset_terminal();
            clear_screen();
            printf("\nThanks for playing!\n\n");
            exit(0);
            break;
    }
}

// Get player name
void get_player_name() {
    clear_screen();
    reset_terminal(); // Temporarily reset terminal to normal mode for name input
    tcflush(STDIN_FILENO, TCIFLUSH);
    
    printf("\n\n    %sEnter your name: %s", YELLOW, RESET);
    fgets(player_name, MAX_PLAYER_NAME, stdin);
    player_name[strcspn(player_name, "\n")] = 0;

    // If empty name, use "Player"
    if (strlen(player_name) == 0) {
        strcpy(player_name, "Player");
    }

    setup_terminal(); 
}

// game controls
void show_controls() {
    clear_screen();
    printf("\n\n");
    printf("    %sWelcome, %s!%s\n", GREEN, player_name, RESET);
    printf("    %sThis is the game instructions page..!!%s\n\n", MAGENTA, RESET);
    printf("    %s===== GAME CONTROLS =====%s\n\n", YELLOW, RESET);
    printf("    %sA%s - Move Left\n", GREEN, RESET);
    printf("    %sD%s - Move Right\n", GREEN, RESET);
    printf("    %sF%s - Fire\n", GREEN, RESET);
    printf("    %sM%s - Return to Main Menu\n", GREEN, RESET);
    printf("    %sQ%s - Quit Game\n\n", GREEN, RESET);
    printf("    %sGame Rules:%s\n", CYAN, RESET);
    printf("    - You have 3 lives\n");
    printf("    - Destroy enemy tanks to score points\n");
    printf("    - Lose a life when enemies reach the bottom\n");
    printf("    - Game ends when you lose all lives\n\n");
    printf("    %sGood Luck...!!%s\n\n", MAGENTA, RESET);
    printf("    %sPress any key to start the game...%s\n", YELLOW, RESET);

    // Wait for key press
    char c;
    while (read(STDIN_FILENO, &c, 1) <= 0) {
        usleep(100000);
    }
}

// Main game loop
void play_game() {
    show_controls();
    initialize_game();
    int quit = 0;

    while (!quit) {
        // Check for input
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            c = tolower(c);
            if (c == 'a' || c == 'd') {
                move_player(c);
            } else if (c == 'f') {
                fire_bullet();
            } else if (c == 'm') {
                show_welcome();
                return;
            } else if (c == 'q') {
                quit = 1;
            } else if (c != '\n' && c != '\r') {
             
                printf("\n\n");
                printf("    %sInvalid Input dalta h Gadhee common sense use krleeee....!!!%s\n", MAGENTA, RESET);
            
            }
        }

        update_game();
        render_game();

        // Check if game is over
        if (player_lives <= 0) {
            show_game_over();
            break;
        }

        usleep(200000); // Sleep for 200ms
    }
}

// Update game
void update_game() {
    // Move bullets up
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y--;
            if (bullets[i].y <= 0) {
                bullets[i].active = 0;
            }
        }
    }

    // Move enemies down
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (rand() % 15 == 0) {
                enemies[i].y++;

                // Check if enemy reached bottom
                if (enemies[i].y >= HEIGHT - 1) {
                    enemies[i].active = 0;
                    player_lives--;
                    // Create a new enemy to replace the one that reached the bottom
                    create_enemy();
                }
            }
        }
    }

    // Create new enemy if existing eliminated
    int active_count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            active_count++;
        }
    }

    // Keep always max enemies
    while (active_count < MAX_ENEMIES) {
        create_enemy();
        active_count++;
    }

    check_collisions();
}

// Render the game to the screen
void render_game() {
    clear_screen();

    // Clear the board
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                board[y][x] = '#';
            } else {
                board[y][x] = ' ';
            }
        }
    }

    // Place player
    board[player_y][player_x] = 'A';

    // Place bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            board[bullets[i].y][bullets[i].x] = '*';
        }
    }

    // Place enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            board[enemies[i].y][enemies[i].x] = 'M';
        }
    }

    // Draw the board
    printf("\n");
    for (int y = 0; y < HEIGHT; y++) {
        printf("    ");
        for (int x = 0; x < WIDTH; x++) {
            char c = board[y][x];
            if (c == '#') {
                printf("%s%c%s", BLUE, c, RESET);
            } else if (c == 'A') {
                printf("%s%c%s", GREEN, c, RESET);
            } else if (c == 'M') {
                printf("%s%c%s", RED, c, RESET);
            } else if (c == '*') {
                printf("%s%c%s", YELLOW, c, RESET);
            } else {
                printf("%c", c);
            }
        }
        printf("\n");
    }

    // Show player info
    printf("\n    %sPlayer: %s%s", GREEN, player_name, RESET);
    printf("    %sLives: %s", GREEN, RESET);
    for (int i = 0; i < player_lives; i++) {
        printf("%s♥ %s", RED, RESET);
    }

    printf("    %sScore: %d%s\n", YELLOW, score, RESET);
    printf("\n    %sControls: A=Left D=Right F=Fire M=Menu Q=Quit%s\n", BLUE, RESET);
}

// Move the player left or right
void move_player(char direction) {
    if (direction == 'a' && player_x > 1) {
        player_x--;
    } else if (direction == 'd' && player_x < WIDTH - 2) {
        player_x++;
    }
}

// Fire a bullet from player position
void fire_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player_x;
            bullets[i].y = player_y - 1;
            bullets[i].active = 1;
            break;
        }
    }
}

// Create a new enemy at the top of the screen
void create_enemy() {
    // Count active enemies
    int active_count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            active_count++;
        }
    }

    // If already max enemies, don't create
    if (active_count >= MAX_ENEMIES) {
        return;
    }

    // Find an inactive slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            // Choose a random x-position that is not free
            int new_x;
            int valid_position = 0;
            int attempts = 0;

            // Retry if alredy there is an enemy
            while (!valid_position && attempts < 10) {
                valid_position = 1;
                new_x = 1 + rand() % (WIDTH - 2);

                // Check if this position is already occupied by another enemy
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active && enemies[j].x == new_x && enemies[j].y <= 2) {
                        valid_position = 0;
                        break;
                    }
                }
                attempts++;
            }

            enemies[i].x = new_x;
            enemies[i].y = 1;
            enemies[i].active = 1;
            break;
        }
    }
}

// Check for collisions between bullets and enemies
void check_collisions() {
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].active) {
                    if (bullets[b].x == enemies[e].x && bullets[b].y == enemies[e].y) {
                        // Collision detected
                        bullets[b].active = 0;
                        enemies[e].active = 0;
                        score++;

                        // Create a new enemy to replace the destroyed one
                        create_enemy();
                    }
                }
            }
        }
    }
}

// Show game over screen
void show_game_over() {
    clear_screen();
    printf("\n\n");
    printf("    %s╔═══════════════════════════════╗%s\n", RED, RESET);
    printf("    %s║         GAME OVER             ║%s\n", RED, RESET);
    printf("    %s╚═══════════════════════════════╝%s\n\n", RED, RESET);
    printf("    %sPlayer: %s%s\n", GREEN, player_name, RESET);
    printf("    %sFinal Score: %d%s\n\n", YELLOW, score, RESET);

    // Check if player already has an entry
    ScoreNode* existing = find_player_entry(player_name);

    if (existing != NULL) {
        // Player already has an entry, check if this score is better
        if (score > existing->score) {
            printf("    %s★ CONGRATULATIONS! NEW PERSONAL BEST! ★%s\n\n", MAGENTA, RESET);
            add_high_score(player_name, score);
            save_high_scores();
        } else {
            printf("    %sYour best score is still %d%s\n\n", YELLOW, existing->score, RESET);
        }
    } else if (is_high_score(score)) {
        // New high score entry
        printf("    %s★ CONGRATULATIONS! NEW HIGH SCORE! ★%s\n\n", MAGENTA, RESET);
        add_high_score(player_name, score);
        save_high_scores();
    }

    printf("    %sPress any key to continue...%s\n", GREEN, RESET);

    // Pause for a moment
    sleep(1);

    // Wait for key press
    char c;
    while (read(STDIN_FILENO, &c, 1) <= 0) {
        usleep(100000);
    }

    // Show game over menu
    show_game_over_menu();
}

// Show menu after game over
void show_game_over_menu() {
    clear_screen();
    printf("\n\n");
    printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
    printf("    %s║         GAME OVER             ║%s\n", YELLOW, RESET);
    printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

    printf("    %sWhat would you like to do?%s\n\n", GREEN, RESET);
    printf("    %s1.%s Play Again\n", CYAN, RESET);
    printf("    %s2.%s View High Scores\n", CYAN, RESET);
    printf("    %s3.%s Return to Main Menu\n", CYAN, RESET);
    printf("    %s4.%s Quit\n\n", CYAN, RESET);

    printf("    %sEnter your choice (1-4): %s", MAGENTA, RESET);

    // Get player choice
    char choice;
    int valid_input = 0;

    while (!valid_input) {
        if (read(STDIN_FILENO, &choice, 1) > 0) {
            if (choice >= '1' && choice <= '4') {
                valid_input = 1;
            } else {
                // Show invalid input message
                clear_screen();
                printf("\n\n");
                printf("    %s╔═══════════════════════════════╗%s\n", RED, RESET);
                printf("    %s║         INVALID INPUT         ║%s\n", RED, RESET);
                printf("    %s╚═══════════════════════════════╝%s\n\n", RED, RESET);
                printf("    %sInvalid Input dalta h Gadhee common sense use krleeee....!!!%s\n\n", MAGENTA, RESET);

                // Wait for 3 seconds
                sleep(3);

                // Show game over menu again
                clear_screen();
                printf("\n\n");
                printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
                printf("    %s║         GAME OVER             ║%s\n", YELLOW, RESET);
                printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

                printf("    %sWhat would you like to do?%s\n\n", GREEN, RESET);
                printf("    %s1.%s Play Again\n", CYAN, RESET);
                printf("    %s2.%s View High Scores\n", CYAN, RESET);
                printf("    %s3.%s Return to Main Menu\n", CYAN, RESET);
                printf("    %s4.%s Quit\n\n", CYAN, RESET);

                printf("    %sEnter your choice (1-4): %s", MAGENTA, RESET);
            }
        }
        usleep(100000);
    }

    switch (choice) {
        case '1': 
            play_game();
            break;
        case '2': 
            display_high_scores();
            show_game_over_menu(); 
            break;
        case '3': 
            show_welcome();
            break;
        case '4':
            reset_terminal();
            clear_screen();
            printf("\nThanks for playing!\n\n");
            exit(0);
            break;
    }
}

// Load high scores from file into linked list
void load_high_scores() {
    // Clear existing scores
    free_score_list();

    FILE* file = fopen(HIGH_SCORE_FILE, "r");
    if (!file) {
        // File doesn't exist.
        num_high_scores = 0;
        return;
    }

    // Read all scores into a temporary array first for sorting
    char names[MAX_HIGH_SCORES][MAX_PLAYER_NAME];
    int scores[MAX_HIGH_SCORES];
    int count = 0;

    while (count < MAX_HIGH_SCORES && 
           fscanf(file, "%s %d", names[count], &scores[count]) == 2) {
        count++;
    }

    fclose(file);

    // Sort scores in descending order (bubble sort)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                // Swap scores
                int temp_score = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp_score;

                // Swap names
                char temp_name[MAX_PLAYER_NAME];
                strcpy(temp_name, names[j]);
                strcpy(names[j], names[j + 1]);
                strcpy(names[j + 1], temp_name);
            }
        }
    }

    // Create linked list from sorted array
    for (int i = 0; i < count; i++) {
        ScoreNode* new_node = create_score_node(names[i], scores[i]);
        if (!new_node) continue;

        // Add to end of list (already sorted)
        if (high_score_list == NULL) {
            high_score_list = new_node;
        } else {
            ScoreNode* current = high_score_list;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_node;
        }

        num_high_scores++;
    }

    // Print debug message when scores are loaded
    printf("    %sLoaded %d high scores into linked list%s\n", GREEN, num_high_scores, RESET);
    usleep(500000); // Show message for 0.5 seconds
}

// Save high scores from linked list to file
void save_high_scores() {
    FILE* file = fopen(HIGH_SCORE_FILE, "w");
    if (!file) {
        // Can't write to file
        return;
    }

    // Write high scores from linked list
    ScoreNode* current = high_score_list;
    while (current != NULL) {
        fprintf(file, "%s %d\n", current->name, current->score);
        current = current->next;
    }

    fclose(file);

    // Debug message
    printf("    %sSaved %d high scores to file%s\n", GREEN, num_high_scores, RESET);
    usleep(500000); // Show message for 0.5 seconds
}

// Check if player already has an entry with the same name
ScoreNode* find_player_entry(const char* name) {
    ScoreNode* current = high_score_list;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// Add a new high score to the linked list
void add_high_score(const char* name, int score) {
    // Check if player already has an entry
    ScoreNode* existing = find_player_entry(name);

    if (existing != NULL) {
        // Player already has an entry
        if (score > existing->score) {
            // Update existing entry if new score is higher
            printf("    %sUpdated high score: %s %d → %d%s\n", 
                   CYAN, name, existing->score, score, RESET);
            existing->score = score;

            // Re-sort the list (bubble sort)
            ScoreNode* i, *j;
            ScoreNode dummy; // Dummy node to simplify sorting
            dummy.next = high_score_list;

            int swapped;
            do {
                swapped = 0;
                i = &dummy;

                while (i->next != NULL && i->next->next != NULL) {
                    j = i->next;
                    ScoreNode* next_node = j->next;

                    if (j->score < next_node->score) {
                        // Swap nodes
                        j->next = next_node->next;
                        next_node->next = j;
                        i->next = next_node;

                        swapped = 1;
                    }

                    i = i->next;
                }
            } while (swapped);

            high_score_list = dummy.next;
        } else {
            // No update needed
            printf("    %sYour current high score of %d is better than %d%s\n", 
                   YELLOW, existing->score, score, RESET);
        }

        usleep(1000000); // Show message for 1 second
        return;
    }

    // Create new node for a new player
    ScoreNode* new_node = create_score_node(name, score);
    if (!new_node) return; // Failed to allocate memory

    // Insert in sorted order (descending)
    if (high_score_list == NULL || score > high_score_list->score) {
        // Insert at beginning
        new_node->next = high_score_list;
        high_score_list = new_node;
        num_high_scores++;
    } else {
        // Find position to insert
        ScoreNode* current = high_score_list;
        while (current->next != NULL && score <= current->next->score) {
            current = current->next;
        }

        // Insert after current
        new_node->next = current->next;
        current->next = new_node;

        // Increment count if we haven't reached the maximum
        if (num_high_scores < MAX_HIGH_SCORES) {
            num_high_scores++;
        } else {
            // We're at maximum, need to remove the last node if we inserted elsewhere
            if (new_node->next != NULL) {
                // Find the node before the last one
                ScoreNode* temp = high_score_list;
                while (temp->next != NULL && temp->next->next != NULL) {
                    temp = temp->next;
                }
                // Remove the last node
                if (temp->next != NULL) {
                    free(temp->next);
                    temp->next = NULL;
                }
            }
        }
    }

    // Debug message for adding high score
    printf("    %sAdded new high score: %s - %d%s\n", CYAN, name, score, RESET);
    usleep(1000000); // Show message for 1 second
}

// Check if a score is a high score
int is_high_score(int score) {
    // Get the lowest high score (last node in list)
    ScoreNode* current = high_score_list;
    ScoreNode* lowest = NULL;

    while (current != NULL) {
        lowest = current;
        current = current->next;
    }

    // Check if we have less than MAX_HIGH_SCORES entries
    if (num_high_scores < MAX_HIGH_SCORES) {
        return 1;
    }

    // Check if score is higher than the lowest high score
    if (lowest != NULL && score > lowest->score) {
        return 1;
    }

    // Check if the player already has an entry with a lower score
    // (This will be handled in add_high_score)

    return 0;
}

// Display high scores
void display_high_scores() {
    clear_screen();
    printf("\n\n");
    printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
    printf("    %s║         TOP 3 SCORES          ║%s\n", YELLOW, RESET);
    printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

    // Show info about data structure
    printf("    %sHigh scores are stored in a linked list%s\n", BLUE, RESET);
    printf("    %sBinary search implemented for score lookup%s\n\n", BLUE, RESET);

    if (num_high_scores == 0) {
        printf("    %sNo high scores yet!%s\n\n", RED, RESET);
    } else {
        // Display only top 3 high scores from linked list
        ScoreNode* current = high_score_list;
        int i = 0;
        int max_display = 3; // Only show top 3

        while (current != NULL && i < max_display) {
            // Gold, silver, bronze for top 3
            const char* medal = "";
            const char* color = RESET;

            if (i == 0) {
                medal = "🥇 ";
                color = YELLOW;
            } else if (i == 1) {
                medal = "🥈 ";
                color = CYAN;
            } else if (i == 2) {
                medal = "🥉 ";
                color = RED;
            }

            printf("    %s%s%d. %s%-20s %d%s\n", 
                  color, medal, i+1, medal, current->name, current->score, RESET);

            current = current->next;
            i++;
        }

        // Show total number of scores in database
        printf("\n    %s(Showing top 3 of %d high scores)%s\n", MAGENTA, num_high_scores, RESET);
        printf("\n");
    }

    printf("    %sPress any key to continue...%s\n", GREEN, RESET);

    // Wait for key press
    char c;
    while (read(STDIN_FILENO, &c, 1) <= 0) {
        usleep(100000);
    }
}

// Display developers information
void display_about_developers() {
    clear_screen();
    printf("\n\n");
    printf("    %s╔═══════════════════════════════╗%s\n", YELLOW, RESET);
    printf("    %s║       ABOUT DEVELOPERS        ║%s\n", YELLOW, RESET);
    printf("    %s╚═══════════════════════════════╝%s\n\n", YELLOW, RESET);

    printf("    %sDevelopers:%s\n\n", GREEN, RESET);

    printf("    %sTanvi Jamwal%s\n", MAGENTA, RESET);
    printf("    24bcs084\n");
    printf("    B Tech CSE 1st year\n");
    printf("    Shri Mata Vaishno Devi University\n\n");

    printf("    %sHarsh Bardhan%s\n", MAGENTA, RESET);
    printf("    24bcs026\n");
    printf("    B Tech CSE 1st year\n");
    printf("    Shri Mata Vaishno Devi University\n\n");

    printf("    %sPress any key to continue...%s\n", GREEN, RESET);

    // Wait for key press
    char c;
    while (read(STDIN_FILENO, &c, 1) <= 0) {
        usleep(100000);
    }
}

// Utility function for min of two ints
int min(int a, int b) {
    return a < b ? a : b;
}

// Create a new score node for the linked list
ScoreNode* create_score_node(const char* name, int score) {
    ScoreNode* new_node = (ScoreNode*)malloc(sizeof(ScoreNode));
    if (new_node) {
        strcpy(new_node->name, name);
        new_node->score = score;
        new_node->next = NULL;
    }
    return new_node;
}

// Free all nodes in the linked list
void free_score_list() {
    ScoreNode* current = high_score_list;
    while (current != NULL) {
        ScoreNode* temp = current;
        current = current->next;
        free(temp);
    }
    high_score_list = NULL;
    num_high_scores = 0;
}

// Binary search to find score in the linked list (returns node if found, NULL if not found)
ScoreNode* binary_search_score(int target_score) {
    // Convert linked list to array for binary search
    if (high_score_list == NULL || num_high_scores == 0) {
        return NULL;
    }

    // Create an array of pointers to nodes
    ScoreNode** score_array = (ScoreNode**)malloc(num_high_scores * sizeof(ScoreNode*));
    if (!score_array) return NULL;

    // Fill array with pointers to nodes
    ScoreNode* current = high_score_list;
    int count = 0;
    while (current != NULL && count < num_high_scores) {
        score_array[count++] = current;
        current = current->next;
    }

    // Binary search on the array
    int left = 0;
    int right = count - 1;
    ScoreNode* result = NULL;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        // Check if target_score is present at mid
        if (score_array[mid]->score == target_score) {
            result = score_array[mid];
            break;
        }

        // If target_score is greater, ignore left half (scores are in descending order)
        if (score_array[mid]->score < target_score)
            right = mid - 1;
        // If target_score is smaller, ignore right half
        else
            left = mid + 1;
    }

    // Free the temporary array
    free(score_array);

    return result;
}

// Main function
int main() {
    // Initialize random number generator
    srand(time(NULL));

    // Load high scores
    load_high_scores();

    // Set up the terminal
    setup_terminal();

    // Start the game
    show_welcome();

    // Clean up
    reset_terminal();
    clear_screen();
    printf("\nThanks for playing!\n\n");

    // Free memory used by high score linked list
    free_score_list();

    return 0;
}