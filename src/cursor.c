//****************************************************************************
// Program: cursor
// Version: 1.5
// Date:    1991-06-12
// Author:  Rohin Gosling
//
// Description:
//
//   DOS text-mode cursor shape utility. Sets the text-mode cursor to a full
//   block ( "cursor -block" ) or the default underline shape
//   ( "cursor -line" ), then exits. The new shape persists at the prompt.
//
//   The shape is sized adaptively: the character cell height is read via
//   INT 10h ( AH=11h, AL=30h ), and the cursor scan lines are computed from
//   it.
//
//     - block = 0 .. bottom
//     - line  = ( bottom - 1 ) .. bottom
//
//   So the result is correct on CGA, EGA, and VGA ( and in DOSBox ).
//
//   Switches accept a '-' or '/' prefix and are case-insensitive: -block / -b
//   and -line / -l. Anything else ( help, no arguments, the wrong argument
//   count, or an unrecognised switch ) prints the usage screen. Silent on
//   success; exit code 0 = applied, 1 = usage / error.
//
//****************************************************************************

#include <dos.h>        // union REGS, int86 ()
#include <stdio.h>      // printf ()
#include <string.h>     // stricmp () -- Borland extension, not ISO C.

#define VIDEO 0x10      // INT 10h -- BIOS video services.

// Code Page 437 (DOS OEM) single-line box-drawing characters.

#define BOX_TOP_LEFT      0xDA
#define BOX_TOP_RIGHT     0xBF
#define BOX_BOTTOM_LEFT   0xC0
#define BOX_BOTTOM_RIGHT  0xD9
#define BOX_HORIZONTAL    0xC4
#define BOX_VERTICAL      0xB3

//----------------------------------------------------------------------------
// Type: CURSOR_COMMAND
//
// Description:
//
//   The action selected by the command line.
//
//----------------------------------------------------------------------------

typedef enum
{
	CURSOR_COMMAND_BLOCK,       // Set a full block cursor.
	CURSOR_COMMAND_LINE,        // Set the default underline ( line ) cursor.
	CURSOR_COMMAND_USAGE        // Show the usage screen ( help / error ).
}
CURSOR_COMMAND;


//----------------------------------------------------------------------------
// Function: draw_box
//
// Description:
//
//   Draw a single-line CP437 box-drawing rectangle around a label, with one
//   space of padding on each side of the label text. The box width adapts to
//   the label length.
//
// Arguments:
//
//   - label : The text to display inside the box.
//
// Returns:
//
//   - None.
//
//----------------------------------------------------------------------------

void draw_box ( const char *label )
{
	int inner_width = ( int ) strlen ( label ) + 2;     // One space each side.
	int i;

	// Top border.

	putchar ( BOX_TOP_LEFT );

	for ( i = 0; i < inner_width; i++ )
	{
		putchar ( BOX_HORIZONTAL );
	}

	putchar ( BOX_TOP_RIGHT );
	putchar ( '\n' );

	// Label row, padded with one space on each side of the text.

	printf ( "%c %s %c\n", BOX_VERTICAL, label, BOX_VERTICAL );

	// Bottom border.

	putchar ( BOX_BOTTOM_LEFT );

	for ( i = 0; i < inner_width; i++ )
	{
		putchar ( BOX_HORIZONTAL );
	}

	putchar ( BOX_BOTTOM_RIGHT );
	putchar ( '\n' );
}


//----------------------------------------------------------------------------
// Function: print_usage
//
// Description:
//
//   Print the usage screen to standard output.
//
// Arguments:
//
//   - None.
//
// Returns:
//
//   - None.
//
//----------------------------------------------------------------------------

void print_usage ( void )
{
	draw_box ( "Cursor (version 1.5)" );
	printf   ( "\n" );
	printf   ( "Usage:\n" );
	printf   ( "  cursor -block    Set a full block cursor.\n" );
	printf   ( "  cursor -line     Set the default underline (line) cursor.\n\n" );
	printf   ( "Switches may use - or / and are case-insensitive (-b, -l also accepted).\n" );
}


//----------------------------------------------------------------------------
// Function: parse_arguments
//
// Description:
//
//   Map the command-line arguments to a CURSOR_COMMAND. Exactly one switch
//   argument is expected, prefixed with '-' or '/' and matched
//   case-insensitively. 
//
//   Help, no arguments, the wrong argument count, and any unrecognised switch
//   all resolve to CURSOR_COMMAND_USAGE.
//
// Arguments:
//
//   - argc : Argument count.
//   - argv : Argument vector.
//
// Returns:
//
//   - The selected CURSOR_COMMAND.
//
//----------------------------------------------------------------------------

CURSOR_COMMAND parse_arguments ( int argc, char *argv [] )
{
	char *switch_text;

	// Exactly one argument is required.

	if ( argc != 2 )
	{
		return CURSOR_COMMAND_USAGE;
	}

	// The argument must begin with a '-' or '/' switch prefix.

	if ( ( argv [ 1 ] [ 0 ] != '-' ) && ( argv [ 1 ] [ 0 ] != '/' ) )
	{
		return CURSOR_COMMAND_USAGE;
	}

	// Compare the text after the prefix, case-insensitively.

	switch_text = argv [ 1 ] + 1;

	if ( ( stricmp ( switch_text, "block" ) == 0 ) || ( stricmp ( switch_text, "b" ) == 0 ) )
	{
		return CURSOR_COMMAND_BLOCK;
	}

	if ( ( stricmp ( switch_text, "line" ) == 0 ) || ( stricmp ( switch_text, "l" ) == 0 ) )
	{
		return CURSOR_COMMAND_LINE;
	}

	// Anything else -- including help ( "?", "h", "help" ) and unknown
	// switches -- shows the usage screen.

	return CURSOR_COMMAND_USAGE;
}


//----------------------------------------------------------------------------
// Function: read_cell_bottom_scan_line
//
// Description:
//
//   Return the bottom scan line of the current text cell ( cell_height - 1 ).
//   
//   The cell height is read from INT 10h ( AH=11h, AL=30h ), which returns it
//   in CX. On adapters that do not support the call ( pre-EGA ), fall back to
//   a fixed 8-line cell.
//
// Arguments:
//
//   - None.
//
// Returns:
//
//   - The bottom scan line index ( e.g. 15 on a VGA / DOSBox 16-line cell ).
//
//----------------------------------------------------------------------------

unsigned char read_cell_bottom_scan_line ( void )
{
	union REGS    regs;                 // BIOS-call register block.
	unsigned int  cell_height;          // Character cell height ( scan lines ).

	regs.h.ah = 0x11;                   // Character generator.
	regs.h.al = 0x30;                   // Get font / cell information.
	regs.h.bh = 0x00;                   // Font-pointer selector 0.

	// Call INT 10h to read the character generator information.

	int86 ( VIDEO, &regs, &regs );

	// The returned cell height ( scan lines per cell ) comes back in CX.

	cell_height = regs.x.cx;            // Scan lines per character cell.

	// Guard against an unsupported call ( a pre-EGA BIOS leaves CX undefined ).

	if ( ( cell_height < 2 ) || ( cell_height > 32 ) )
	{
		cell_height = 8;
	}

	// The bottom scan line is one less than the cell height.

	return ( unsigned char ) ( cell_height - 1 );
}


//----------------------------------------------------------------------------
// Function: set_cursor_shape
//
// Description:
//
//   Apply a cursor shape via INT 10h ( AH=01h ). The visibility / blink bits
//   of CH are left clear, so the cursor stays visible.
//
// Arguments:
//
//   - start_scan_line : Top scan line of the cursor.
//   - end_scan_line   : Bottom scan line of the cursor.
//
// Returns:
//
//   - None.
//
//----------------------------------------------------------------------------

void set_cursor_shape ( unsigned char start_scan_line, unsigned char end_scan_line )
{
	union REGS regs;                    // BIOS-call register block.

	regs.h.ah = 0x01;                   // Set cursor type.
	regs.h.ch = start_scan_line;        // Start ( top ) scan line.
	regs.h.cl = end_scan_line;          // End ( bottom ) scan line.

	// Call INT 10h to apply the new cursor shape.

	int86 ( VIDEO, &regs, &regs );
}


//----------------------------------------------------------------------------
// Function: set_block_cursor
//
// Description:
//
//   Set a full block cursor: scan line 0 to the cell's bottom scan line.
//
// Arguments:
//
//   - None.
//
// Returns:
//
//   - None.
//
//----------------------------------------------------------------------------

void set_block_cursor ( void )
{
	// Measure the cell so the block fills its full height.

	unsigned char bottom_scan_line = read_cell_bottom_scan_line ();

	// Block: scan line 0 down to the bottom of the cell.

	set_cursor_shape ( 0, bottom_scan_line );
}


//----------------------------------------------------------------------------
// Function: set_line_cursor
//
// Description:
//
//   Set an underline ( line ) cursor: the bottom one or two scan lines of the
//   cell. The top line is clamped so a degenerate single-line cell can never
//   produce start > end.
//
// Arguments:
//
//   - None.
//
// Returns:
//
//   - None.
//
//----------------------------------------------------------------------------

void set_line_cursor ( void )
{
	// Measure the cell; clamp the top line so start never exceeds end.

	unsigned char bottom_scan_line = read_cell_bottom_scan_line ();
	unsigned char top_scan_line    = ( bottom_scan_line > 0 ) ? ( unsigned char ) ( bottom_scan_line - 1 ) : 0;

	// Line: the bottom one or two scan lines of the cell.

	set_cursor_shape ( top_scan_line, bottom_scan_line );
}


//----------------------------------------------------------------------------
// Function: main
//
// Description:
//
//   Program entry point. Parse the command line and dispatch: set the block
//   or line cursor ( silent, exit 0 ), or print the usage screen ( exit 1 ).
//
// Arguments:
//
//   - argc : Argument count.
//   - argv : Argument vector.
//
// Returns:
//
//   - 0 when a cursor shape was applied; 1 for usage / error.
//
//----------------------------------------------------------------------------

int main ( int argc, char *argv [] )
{
	// Resolve the command line to a single cursor action.

	CURSOR_COMMAND command   = parse_arguments ( argc, argv );
	int            exit_code = 1;        // Default: usage / error.

	// Dispatch on the selected command.

	switch ( command )
	{
		case CURSOR_COMMAND_BLOCK:

			set_block_cursor ();
			exit_code = 0;
			break;

		case CURSOR_COMMAND_LINE:

			set_line_cursor ();
			exit_code = 0;
			break;

		case CURSOR_COMMAND_USAGE:
		default:

			print_usage ();
			break;
	}

	// 0 when a cursor shape was applied; 1 for usage / error.

	return exit_code;
}
