/// Responsible for taking a grid and determing how it changes in every time step. It also notifies view.c with info 
/// regarding changes to individual trees.
/// @author Cody Smith (bcs4313.cs.rit.edu)

#include <stdio.h> // needed for many basic operations
#include <stdlib.h> // used for random generation and seeding
#include "wildfire.h" // enums, structs, and methods are shared in this file

// settings inherited from wildfire.c
static int setting_size = -1;
static int setting_probability_catch = -1;
static int setting_prop_neighbor = -1;
static int setting_lightning = -1;

/// The init function initializes this c file
/// by inputting specific settings from main. They are
/// needed for the spread function to work properly.
/// parameters are static setting vars from wildfire.c
void spreader_init(int s_size, int s_prop_neighbor, int s_probability_catch, int s_lightning)
{
	srand(43);
	setting_size = s_size;
	setting_probability_catch = s_probability_catch;
	setting_prop_neighbor = s_prop_neighbor;
	setting_lightning = s_lightning;
}

/// Simple evaluation used by the spread function on
/// a trees neighbors. Determines if a tree is
/// burning.
/// @param tree the tree neighbor to be checked.
/// @return is this current tree burning?
static int burn_eval(Tree tree)
{
	if(tree.state == BURNING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// Simple evaluation that declares whether a certain 
// node contains a living tree neighbor.
// @param tree the tree struct to be evaluated.
// @return if the tree is alive.
static int neighbor_eval(Tree tree)
{
	if(tree.state != EMPTY && tree.state != BURNED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/// Return a tree node with its state and vitality depending on
/// its state, neighbors, settings, and random number generation. 
/// Trees in EMPTY / BURNED state: do nothing
/// Trees in BURNING state: reduce vitality by 1 unless it is zero, if zero 
/// then change it to the BURNED state.
/// Trees in TREE state:
/// Neighbors from all 8 cardinal directions are checked to
/// see if they are burning, and are added to the burning total.
/// If this total exceeds the amount needed to catch fire, a 
/// random roll occurs to set the tree to the BURNING state.
///
/// @param grid the 2d array of trees that the tree is located in
/// @param y y location on the grid
/// @param x x location on the grid
/// @return an updated version of the tree node targeted
static Tree spread(Tree grid[setting_size][setting_size], int y, int x)
{
	// Block for trees that aren't in the TREE state
	Tree tree = grid[y][x];
	if(tree.state != TREE)
	{
		if(tree.state == EMPTY || tree.state == BURNED)
		{
			return tree;
		}
		if(tree.state == BURNING)
		{
			if(tree.vitality == 0)
			{
				tree.state = BURNED;
				return tree;
			}
			else
			{
				tree.vitality --;
				return tree;
			}
		}
	}
	// Counts the amount of living tree niehgbors around this node
	// used for neighbors_needed calculation.
	int neighbor_count = 0;

	// counts the amount of neighbors burning around the tree
	int burning_neighbors = 0;

	// we check each neighbor to see if the position is valid before
	// checking its state.
	int s = setting_size; 

	// NW
	if((y - 1) >= 0 && (x - 1) >= 0)
	{
		Tree node = grid[y - 1][x - 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// N
	if((y - 1) >= 0)
	{
		Tree node = grid[y - 1][x];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// NE
	if((y - 1) >= 0 && (x + 1) < s)
	{
		Tree node = grid[y - 1][x + 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// W
	if((x - 1) >= 0)
	{
		Tree node = grid[y][x - 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// E
	if((x + 1) < s)
	{
		Tree node = grid[y][x + 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// SW 
	if((y + 1) < s && (x - 1) >= 0)
	{
		Tree node = grid[y + 1][x - 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// S
	if((y + 1) < s)
	{
		Tree node = grid[y + 1][x];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}
	// SE
	if((y + 1) < s && (x + 1) < s)
	{
		Tree node = grid[y + 1][x + 1];
		burning_neighbors += burn_eval(node);
		neighbor_count += neighbor_eval(node);
	}

	// Block for all trees in the TREE state
	// BURNING neighbor count needed for this tree to catch fire
	int neighbors_needed = neighbor_count * ((float)setting_prop_neighbor / 100);

	// now to determine by our rules if this tree BURNS or not
	if(burning_neighbors > neighbors_needed || tree.state == ZAPPED)
	{
		int random_num = rand() % 100;
		
		// random check
		if(random_num <= setting_probability_catch)
		{
			tree.state = BURNING;
			return tree;
		}
		else
		{
			return tree;
		}
	}
	return tree;
}

/// Run a chance to strike lightning in a random area.
/// If the chance passes set that trees state to ZAPPED.
/// @param grid the grid in the simulation to potentially strike with lightning
/// @return if the lightning strike was successful or not.
static int strike_lightning(Tree grid[setting_size][setting_size])
{
	int strike_roll = rand() % 100;
	if(strike_roll < setting_lightning)
	{
		// time to strike
		int rand_y = rand() % setting_size;
		int rand_x = rand() % setting_size;

		Tree tree = grid[rand_y][rand_x];
		if(tree.state == TREE || tree.state == BURNING)
		{
			tree.state = ZAPPED;
			tree.vitality = 3;
			grid[rand_y][rand_x] = tree;
			return 1;
		}
	}
	return 0;
}

/// Update method goes through each individual tree node and
/// applies the spread method on it. Updates to the grid only
/// apply at THE END of these modifications, to avoid concurrent
/// spread method issues.
//
// Note: This method also calls notify_view to give the change
// count found to view.c
//
/// @param grid the 2d Tree Array of the simulation to update
/// @returns if trees are still burning or not (bool int). Used for display mode.
int update(Tree grid[setting_size][setting_size])
{
	// initial state of the grid to read
	Tree grid_init[setting_size][setting_size];

	// copy grid state to grid_init for evaluation
	for(int y = 0; y < setting_size; y++)
	{
		for(int x = 0; x < setting_size; x++)
		{
			grid_init[y][x] = grid[y][x];
		}
	}
		
	// checks if a BURNING tree exists in the grid
	int burning_status = 0;

	// tracks the amount of changes done to the grid in this step
	int changes = 0;
	
	// run spread method on ALL nodes
	// make the spread model interpret the pre-image,
	// and update the post image
	for(int y = 0; y < setting_size; y++)
	{
		for(int x = 0; x < setting_size; x++)
		{
			Tree old_tree = grid_init[y][x]; // used for change detection
			Tree tree = spread(grid_init, y, x);
			grid[y][x] = tree;
			if(tree.state == BURNING || tree.state == ZAPPED)
			{
				burning_status = 1;
				if(old_tree.state == TREE) // change detection 1
				{
					changes ++;
				}
			}

			// change detection 2
			if(tree.state == BURNED && old_tree.state == BURNING)
			{
				changes ++;
			}
		}
	}
	int strike = strike_lightning(grid);
	if(strike == 1)
	{
		burning_status = 1;
	}
	// notify view on changes made in step
	notify_view(changes);
	return burning_status;
}
