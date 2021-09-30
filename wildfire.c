/// Serves as the core of the program, Parsing arguments, initializing a grid and running a simulation with debug info.
/// @author Cody Smith (bcs4313@cs.rit.edu)

#define _DEFAULT_SOURCE // needed for usleep to work

#include <stdio.h> // needed for many basic operations
#include <getopt.h> // used to process command line arguments in any order
#include <stdlib.h> // used for random generation and seeding
#include <unistd.h> // used for sleep functions in display mode
#include "wildfire.h" // enums, structs, and methods are defined in here to be shared.

// Default Settings for the simulation (constant)
#define DEFAULT_BURN 10
#define DEFAULT_PROB_CATCH 30
#define DEFAULT_DENSITY 50
#define DEFAULT_PROP_NEIGHBOR 25
#define DEFAULT_PRINT_COUNT -5
#define DEFAULT_SIZE 10
#define DEFAULT_LIGHTNING 0

// settings used for the entire program. Could potentially be overridden by program arguments
static int setting_initial_burn = DEFAULT_BURN; // percentage of initially burning trees
static int setting_probability_catch = DEFAULT_PROB_CATCH; // chance of tree to catch fire when exceeding neighbor threshold
static int setting_density = DEFAULT_DENSITY; // percentage of trees to initially populate in grid
static int setting_prop_neighbor = DEFAULT_PROP_NEIGHBOR; // percentage of neighbors needed to catch fire to a tree
static int setting_print_count = DEFAULT_PRINT_COUNT; // number of print steps for print mode or -5 for display view
static int setting_size = DEFAULT_SIZE; // size of initial grid (n x n) min = 5, max = 40
static int setting_lightning = DEFAULT_LIGHTNING; // chance for lightning to strike a random part of the grid, setting a tree on fire.

/// Initialize our grid by generating a 2d array of tree structs
/// taking into mind the settings that were passed into our static arguments.
/// initialization has 3 phases.
/// phase 1: initialize trees in all cells and set them to the TREE state
/// phase 2: randomly pick trees in the grid and set their state to EMPTY
/// phase 3: set random trees on fire according to the initial_burn setting, ignoring empty cells
/// @param grid the grid that is used in our current simulation
/// @return starting grid of tree cells adhering to the requested settings of our program.
static void initialize_grid(Tree grid[setting_size][setting_size])
{	
	// generation phase 1
	for(int y = 0; y < setting_size; y++)
	{
		for(int x = 0; x < setting_size; x++)
		{
			Tree tree;
			tree.state = TREE;
			tree.vitality = 3;
			grid[y][x] = tree;
		}
	}	

	// generation phase 2
	float empty_proportion = (float)(100 - setting_density) / 100.0;
	int empty_slots = (setting_size * setting_size) * empty_proportion;
	while(empty_slots != 0)
	{
	        int y = rand() % setting_size;
		int x = rand() % setting_size;
		Tree tree = grid[y][x];

		// check state before update
		if(tree.state == TREE)
		{
			tree.state = EMPTY;
			grid[y][x] = tree;
			empty_slots --;
		}	
	}

	// generation phase 3
	float burn_proportion = (float)(setting_initial_burn) / 100.0;
	empty_slots = (setting_size * setting_size) * empty_proportion;
	int trees_left = (setting_size * setting_size) - empty_slots;
	int burn_slots = (float)trees_left * burn_proportion;

	while(burn_slots != 0)
	{
		int y = rand() % setting_size;
		int x = rand() % setting_size;
		Tree tree = grid[y][x];

		// check state before update
		if(tree.state == TREE)
		{
			tree.state = BURNING;
			grid[y][x] = tree;
			burn_slots --;
		}
	}
}

// prints to standard output how to use the program. Triggered by the -H flag in parse_args.
void print_usage()
{
	printf("\nusage: wildfire [options]"
			"\nBy default, the simulation runs in overlay display mode"
			"\nThe -pn option makes the simulation run in print mode for up to N states/"
			"\n"
			"\nSimulation Configuration Options:"
			"\n-H # View Simulation options and quit."
			"\n-bN # proportion of trees that are already burning. 0 < N < 101."
			"\n-cN # probability that a tree will catch fire. 0 < n < 101."
			"\n-dN density: the proportion of trees in the grid. 0 < N < 101."
			"\n-nN # proportion of neighbors that influence a tree catching fire. -1 < N < 101."
			"\n-pN # number of states to print before quitting. -1 < N < ..."
			"\n-sN # simulation grid size. 4 < N < 41."
			"\n-Ln # chance per state for a lightning bolt to strike a random node, setting it on fire. -1 < N < 101"
			"\n");
}

/// Parse the arguments placed into standard input for the program.
/// These arguments will update the general settings of our simulation
/// to the will of the user.
/// 
/// Also, parse_args checks to see if values are in the proper range. If not
/// it will return -1.
///
/// @param argc number of arguments put into standard input
/// @param argv the individual values of each argument put into standard input
/// @return 1 if parse_args requests a close of the program, 0 if not, -1 is a complete failure.
static int parse_args(int argc, char** argv)
{
	int option_index = 0; // location of var in argv
	while((option_index = getopt(argc, argv, "Hb:c:d:n:p:s:L:")) != -1)
	{
		switch(option_index)
		{
			case 'b':
				setting_initial_burn = atoi(optarg);
				if(setting_initial_burn <= 0 || setting_initial_burn >= 101)
				{
					printf("(-bN) proportion already burning. must be an integer in [1...100]\n");
					print_usage();
					return -1;
				}
				break;
			case 'c':
				setting_probability_catch = atoi(optarg);
				if(setting_probability_catch <= 0 || setting_probability_catch >= 101)
				{
					printf("(-cN) probability a tree will catch fire. must be an integer in [1...100].\n");
					print_usage(0);
					return -1;
				}
				break;
			case 'd':
				setting_density = atoi(optarg);
				if(setting_density <= 0 || setting_density >= 101)
				{
					printf("(-dN) density of trees in the grid must be an integer in [1...100].\n");
					print_usage();
					return -1;
				}
				break;
			case 'n':
				setting_prop_neighbor = atoi(optarg);
				if(setting_prop_neighbor <= -1 || setting_prop_neighbor >= 101)
				{
					printf("(-nN) neighbors influencing a tree catching fire must be an integer in [0...100].\n");
					print_usage();
					return -1;
				}
				break;
			case 'H':
				print_usage();
				return 1;
			case 'p':
				setting_print_count = atoi(optarg);
				if(setting_print_count <= -1 || setting_print_count >= 10001)
				{
					printf("(-pN) number of states to print. must be an integer in [0...10000].\n");
					print_usage();
					return -1;
				}
				break;
			case 's':
				setting_size = atoi(optarg);
				if(setting_size <= 4 || setting_size >= 41)
				{
					printf("(-sN) simulation grid size must be an integer in [5...40].\n");
					print_usage();
					return -1;
				}
				break;
			case 'L':
				setting_lightning = atoi(optarg);
				if(setting_lightning <= -1 || setting_lightning >= 101)
				{
					printf("(-LN simulation lightning strike chance must be an integer in [1...100])");
					print_usage();
					return -1;
				}
				break;
			case '?':
				printf("Failed to recognize one of the inputted flags. Check the following settings for validation.\n");
				print_usage();
				return -1;
		}
	}
	return 0;
}

/// Main method of the program.
/// The main method follows this sequence.
///
/// Step 1: calls srand to seed the random number generator with seed 43
/// Step 2: parses arguments with parse_args and quits if the method returns 1 or -1
/// Step 3: Initialize the grid with the initialize_grid method and declaration.
/// Step 4: Initialize spreader_init and view_init to transfer settings received from parse_args()
/// Step 5: For print mode, loop a set amount of times a updateView and update(<grid>) to change tree states
/// For display mode, loop until update returns an exit value and update(<grid>) to change tree states
/// @param argc argument count number
/// @param argv argument string values
int main(int argc, char** argv)
{
	srand(43); // seeds the random number generator
	
	// tracks exit status of parse_args
	int status = 0; 
	status = parse_args(argc, argv); // process arguments
	
	if(status == 1)
	{
		return EXIT_SUCCESS;
	}
	else if(status == -1)
	{
		return EXIT_FAILURE;
	}

	// Initialize the grid for our simulation
	Tree grid[setting_size][setting_size];
	initialize_grid(grid);
	
	// initialize other c files
	spreader_init(setting_size, setting_prop_neighbor, setting_probability_catch, setting_lightning);
	view_init(setting_initial_burn, setting_size, setting_prop_neighbor,
			setting_probability_catch, setting_density, setting_print_count, setting_lightning);

	// check for print mode
	if(setting_print_count != -5)
	{
		for(int i = 0; i <= setting_print_count; i++)
		{
			updateView(grid);
			update(grid);
		}
	}
	else // display mode
	{
		int status = 1; // exit status dependant on trees still burning
		while(status == 1)
		{
			updateView(grid);
			status = update(grid);
			usleep(750000); // delay for analysis
		}
		updateView(grid);
	}
	printf("Fires are out.\n");

	return EXIT_SUCCESS;
}
