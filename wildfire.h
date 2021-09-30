/// wildfire.h -- used by spreader.c, wildfire.c, view.c 
/// @author Cody Smith (bcs4313@cs.rit.edu)

#ifndef wildfire
#define wildfire

// setting declarations
static int setting_size;

/// This enumeration represents all the potential states of a cell in our
/// simulation's grid.
/// EMPTY = no tree has ever existed here.
/// TREE = an unharmed but susceptible tree is in this slot
/// BURNING = a tree has caught fire! The tree is dying and causing other trees to burn.
/// BURNED = the tree is dead. It cannot be burnt anymore.
/// ZAPPED = the tree was just struck by lightning. It will be in the BURNING state next step.
typedef enum states{EMPTY, TREE, BURNING, BURNED, ZAPPED} enum_state;

/// Struct that represents an existing tree in a cell.
/// @param state an enum value that indicates whether a tree cell is empty, healthy, burning, or burned
/// @param vitality represents the tree's current health. When a tree is burning its vitality goes down
/// every step. Once vitality hits 0 the tree is 'burned'. Default value for vitality is 3.
typedef struct trees
{
	enum_state state;
	int vitality;
} Tree;

/// view.c methods
void view_init(int s_initial_burn, int s_size, int s_prop_neighbor,
	       	int s_probability_catch, int s_density, int s_print_count, int s_lightning); // initialize setting vars needed in spreader.c 
void updateView(Tree grid[setting_size][setting_size]); // show the user the current grid in display mode or print mode
void notify_view(int change_count); // notify view of changes in the grid's state. For printed vars only

// spreader.c methods
void spreader_init(int s_size, int s_prop_neighbor, int s_probability_catch,
	       	int s_lightning); // initialize setting vars needed in spreader.c
int update(Tree grid[setting_size][setting_size]); // go through the tree matrix of the simulation and determine which will change state.
#endif

