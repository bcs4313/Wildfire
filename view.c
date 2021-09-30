/// Responsible for taking a matrix of Tree structs and showing them to the user, in print mode or display mode.
/// @author Cody Smith bcs4313@cs.rit.edu

#include <stdio.h> // used for print functions
#include "display.h" // cursor control
#include "wildfire.h" // project header file 

// definitions of how tree states are displayed by ASCII characters
#define C_TREE 'Y'
#define C_BURNING '*'
#define C_BURNED '.'
#define C_ZAPPED '!'

// var settings to display on the view
static int setting_initial_burn;
static int setting_prop_neighbor;
static int setting_probability_catch;
static int setting_density;
static int setting_print_count;
static int setting_size;
static int setting_lightning;

// variables that the view specifically listens to. Updated by the spreader notify_view function.
static int current_changes = 0;
static int cumulative_changes = 0;
static int cumulative_steps = 0;

/// initialize the view by passing settings from the main method to this view file.
/// each param matches to the setting found in wildfire.c
void view_init(int s_initial_burn, int s_size, int s_prop_neighbor, int s_probability_catch, int s_density,
	       	int s_print_count, int s_lightning)
{
	setting_initial_burn = s_initial_burn;
	setting_size = s_size;
	setting_prop_neighbor = s_prop_neighbor;
	setting_probability_catch = s_probability_catch;
	setting_density = s_density;
	setting_print_count = s_print_count;
	setting_size = s_size;
	setting_lightning = s_lightning;
}

/// update some statistics about the grid to print.
/// this method modifies the values of current_changes,
/// cumulative_changes and cumulative_steps.
/// @param change_count number of changes recorded by spreader.c
void notify_view(int change_count)
{
	current_changes = change_count;
	cumulative_changes += change_count;
	cumulative_steps += 1;
}

/// Display statistics about the current simulation.
/// These statistics include what settings were
/// entered, changes, cumulative changes, and 
/// cumulative steps.
static void display_stats()
{
	// Line 1 -- Settings only
	int size = setting_size;
	float pCatch = (float)setting_probability_catch / 100;
	float density = (float)setting_density / 100;
	float initial_burn = (float)setting_initial_burn / 100;
	float prop_neighbor = (float)setting_prop_neighbor / 100;
	if(setting_print_count == -5)
	{
		set_cur_pos(setting_size+1, 0);
	}
	printf("size %i, pCatch %f, density %f, pBurning %f pNeighbor %f"
			"\ncycle %i, changes %i, cumulative changes %i, pLightning %i\n",
		       	size, pCatch, density, initial_burn, prop_neighbor,
			cumulative_steps, current_changes, cumulative_changes, setting_lightning);
}

/// Print the grid in print mode, which only uses printf functions
/// Goes through each slot in the grid, checking the state of the cell
/// and prints a character depending on what state it was.
/// @param grid the current state of the grid in the simulation
static void display_print_mode(Tree grid[setting_size][setting_size])
{
	for(int y = 0; y < setting_size; y++)
	{
		for(int x = 0; x < setting_size; x++)
		{
			Tree tree = grid[y][x];
			switch(tree.state)
			{
				case(EMPTY):
					printf(" ");
					break;
				case(TREE):
					printf("%c", C_TREE);
					break;
				case(BURNING):
					printf("%c", C_BURNING);
					break;
				case(BURNED):
					printf("%c", C_BURNED);
					break;
				case(ZAPPED):
					printf("%c", C_ZAPPED);
					break;
			}
		}
		printf("\n");
	}
	display_stats();
}

/// Print the grid in display mode, using cursor logic.
/// We loop through each initial grid cell and check for its state,
/// a character is printed out depending on the result of that state.
/// @param grid the current state of the grid in the simulation
static void display_overlay_mode(Tree grid[setting_size][setting_size])
{
	clear();
	set_cur_pos(1,1);
	for(int y = 0; y < setting_size; y++)
	{
		for(int x = 0; x < setting_size; x++)
		{
			set_cur_pos(y + 1, x + 1);
			Tree tree = grid[y][x];
			switch(tree.state)
			{
				case(EMPTY):
					put(' ');
					break;
				case(TREE):
					put(C_TREE);
					break;
				case(BURNING):
					put(C_BURNING);
					break;
				case(BURNED):
					put(C_BURNED);
					break;
				case(ZAPPED):
					put(C_ZAPPED);
					break;
			}
		}
	}
	printf("\n");
	set_cur_pos(setting_size+1,0);
	display_stats();
}

/// Updates the view by either calling print mode or display mode methods.
/// Generally used for viewing by wildfire.c
/// @param grid the current state of the grid in the simulation.
void updateView(Tree grid[setting_size][setting_size])
{
	if(setting_print_count != -5)
	{
		display_print_mode(grid);
	}
	else
	{
		display_overlay_mode(grid);
	}
}
