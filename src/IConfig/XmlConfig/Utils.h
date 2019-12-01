#ifndef JDB_UTILS_H
#define JDB_UTILS_H 

#include <string>
#include <iostream>

// for timing functions
#include <time.h>  

// for ttys detection
#include "unistd.h"

// for round
#include <math.h>
#include <vector>

// for logging in the framework
#include "Logger.h"

using namespace std;


namespace jdb{

	/* Converts an integer to a string
	 * @i integer to convert
	 * @len the maximum length of string
	 *
	 * Requires c++11 or newer
	 * 
	 * @return the string representation of the value
	 */
	std::string ts( int i, int len = -1);
	/* Converts a double to a string
	 * @d double to convert
	 * @len the maximum length of string
	 * 
	 * @return the string representation of the value
	 */
	std::string ts( double d, int len = -1);
	/* Converts a float to a string
	 * @f float to convert
	 * @len the maximum length of string
	 * 
	 * @return the string representation of the value
	 */
	std::string ts( float f, int len  = -1);
	/* Converts an unsigned integer to a string
	 * @u unsigned int to convert
	 * @len the maximum length of string
	 * 
	 * @return the string representation of the value
	 */
	std::string ts( unsigned int u, int len  = -1);

	/* Converts a size_t to a string
	 * @u unsigned int to convert
	 * @len the maximum length of string
	 * 
	 * @return the string representation of the value
	 */
	//É¾³ý
	//std::string ts( size_t u, int len  = -1);

	/* Removes trailing zeros
	 * @in string representation of a number
	 * 
	 * @return The input string with trailing zeros removed
	 */
	std::string trimTrailingZeros( string in );
	
	/* Converts a double to a string and trims trailing zeros
	 * @d double to convert
	 *
	 * Converts a float (or double) to string and trims trailing zeros
	 * 
	 * @return the string representation of the value
	 */
	std::string dts( double d );
	std::string dtes( double d, std::string p = "p" );

	/* Used to properly pluralize words
	 * @count 		the countable value
	 * @fSingle 	the singular form
	 * @fPlural 	the plural form
	 * 
	 * Example :
	 * ``` c++
	 * cout << "Hello " << plural( numOfPeople, "person", "people" ) << endl;
	 * ```
	 * Will return 
	 * 'Hello person' if numOfPeople == 1
	 * and
	 * 'Hello people' otherwise
	 * 
	 * @return 		the proper pluralization based on count
	 */
	std::string  plural( double count, std::string fSingle, std::string fPlural );

	 /* Converts vector to string
	  * @v 		vector to convert
	  *
	  * Converts vector to string
	  *
	  * @return string representation of vector
	  */
	std::string vts( vector<int> v );

	 /* Converts vector to string
	  * @v 		vector to convert
	  *
	  * Converts vector to string
	  *
	  * @return string representation of vector
	  */
	std::string vts( vector<double> v );

	/* Converts vector to string
	  * @v 		vector to convert
	  *
	  * Converts vector to string
	  *
	  * @return string representation of vector
	  */
	std::string vts( vector<float> v );

	/* Converts vector to string
	  * @v 		vector to convert
	  *
	  * Converts vector to string
	  *
	  * @return string representation of vector
	  */
	std::string vts( vector<string> v );

	/* Converts bool to string
	  * @b 		bool to convert
	  *
	  * Converts bool to string
	  *
	  * @return string representation of bool
	  */
	std::string bts( bool b );

	std::string indentation( int count, std::string tab="\t" );

	std::string quote( std::string str );
	std::string quote( int str );
	std::string quote( double str );
	std::string quote( float str );


	/* Task Timer
	 * Utility Class for timing tasks and reporting
	 */
	class TaskTimer{
	public:
		// start time of task
		clock_t startTime;
		// set the start time to now
		void start( );
		/* Return the amount of time elapsed since start() was called
		 *
		 * @return Time past in seconds as double
		 */
		double elapsed( );
		/* Return the elapsed time as a string
		 * Formats the string to be human readable
		 * 
		 * @return The amount of elapsed time
		 */
		string elapsedTime();
	};
	
	/* Plain text progress bar that only updates when it needs to
	 * @i the current step in a loop
	 * @nevents the maximum number of steps
	 * @textWidth The maximum width of the bar in characters
	 * @elapsedTime shows the elapsed time if positive
	 * 
	 */
	void progressBar( int i, int nevents, int textWidth = 60, double elapsedTime = -1, bool skipNonTTY = true );


	/* Task Progress
	 * Integrates the task timer and progress bars into a single utility
	 */
	class TaskProgress{
	protected:
		// Maximum number of steps
		int max;
		// width of the progress bar
		int barWidth;
		// Show the title of the task
		bool showTitle;
		// Show the elapsed time
		bool showElapsed;
		// Title of the task
		string title;
		// Task Timer
		TaskTimer tt;

	public:
		/* Default Constructor
		 * 
		 */
		TaskProgress();
		~TaskProgress(){}

		/* Creates a TaskProgress
		 * @title Title of the Task
		 * @max Maximum number of steps
		 * @width The width of the progress bar
		 * @sTitle Show th title
		 * @sElapse Show elapsed time
		 * 
		 */
		TaskProgress( string title, int max = 100, int width = 60, bool sTitle = true, bool sElapse = true );

		/* Called inside a loop to show the progress of the current task
		 * @i The step in the current loop
		 */
		void showProgress( int i );

	};

}
using namespace jdb;


#endif