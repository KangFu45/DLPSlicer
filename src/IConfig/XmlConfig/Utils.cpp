#include "Utils.h"


namespace jdb{

	/* Converts an integer to a string
	 * @i integer to convert
	 * @len the maximum length of string
	 * @return the string representation of the value
	 */
	std::string ts( int i, int len ){
		if ( len <= -1 )
			return (to_string( (long long int) i));
		else
			return (to_string( (long long int) i)).substr( 0, len );
	}
	/* Converts a double to a string
	 * @d double to convert
	 * @len the maximum length of string
	 * @return the string representation of the value
	 */
	std::string ts( double d, int len ){
		if ( len <= -1 )
			return to_string( (long double) d);
		else 
			return (to_string( (long double) d)).substr( 0, len );
	}
	/* Converts a float to a string
	 * @f float to convert
	 * @len the maximum length of string
	 * @return the string representation of the value
	 */
	std::string ts( float f, int len ){
		return ts( (double) f, len );
	}
	/* Converts an unsigned integer to a string
	 * @u unsigned int to convert
	 * @len the maximum length of string
	 * @return the string representation of the value
	 */
	std::string ts( unsigned int u, int len ){
		if ( len <= -1 )
			return to_string( (long long unsigned int) u);
		else 
			return (to_string( (long long unsigned int) u)).substr( 0, len );
	}

	/* Converts a size_t to a string
	 * @u unsigned int to convert
	 * @len the maximum length of string
	 * @return the string representation of the value
	 */
	//std::string ts( size_t u, int len ){
	//	if ( len <= -1 )
	//		return to_string( u);
	//	else 
	//		return (to_string(u)).substr( 0, len );
	//}

	/* Removes trailing zeros
	 * @in string representation of a number
	 * @return The input string with trailing zeros removed
	 */
	std::string trimTrailingZeros( string in ) {
		string r = in;
		while ( r[ r.size() - 1 ] == '0' && r.size() > 1 && r[ r.size() - 2 ] != '.' ){
			r = r.substr( 0, r.size() - 1 );
		}
		return r;
	}
	/* Converts a double to a string and trims trailing zeros
	 * @d double to convert
	 * @return the string representation of the value
	 */
	std::string dts( double d ){
		return trimTrailingZeros( ts( d ) );
	}

	std::string dtes( double d, string p ){
		std::string s = dts( d );
		std::size_t pos = s.find( "." );
		if ( pos != std::string::npos ){
			return s.replace( pos, 1, p );
		}
		return s;
	}


	std::string plural( double count, std::string fSingle, std::string fPlural ){
		if ( count == 1.0 )
			return fSingle;
		else 
			return fPlural;
		return fPlural;
	}


	std::string vts( vector<int> v ){
	 	string str = "";
	 	string delim = "";
	 	for ( int i : v ){
	 		str += delim + ts(i);
	 		delim=", ";
	 	}
	 	return str;
	 }

	 std::string vts( vector<double> v ){
	 	string str = "";
	 	string delim = "";
	 	for ( double i : v ){
	 		str += delim + dts(i);
	 		delim=", ";
	 	}
	 	return str;
	 }
	std::string vts( vector<float> v ){
		string str = "";
		string delim = "";
		for ( double i : v ){
			str += delim + dts(i);
			delim=", ";
		}
		return str;
	}

	std::string vts( vector<string> v ){
		string str = "";
		string delim = "";
		for ( string i : v ){
			str += delim + i;
			delim=", ";
		}
		return str;
	}


	 std::string bts( bool b ){
	 	if ( b )
	 		return "true";
	 	return "false";
	 }


	std::string indentation( int count, std::string tab ) {
		string is = "";
		for ( int i = 0; i < count; i++ )
			is += tab;
		return is;
	}

	std::string quote( std::string str ){
		return "\"" + str + "\"";
	}
	std::string quote( int i ){
		return quote( ts(i) );
	}
	std::string quote( double d ){
		return quote( ts(d) );
	}
	std::string quote( float f ){
		return quote( ts(f) );
	}

	/**
	 * Plain text progress bar that only updates when it needs to
	 * @i 			the current step in a loop
	 * @nevents 	the maximum number of steps
	 * @textWidth 	The maximum width of the bar in characters
	 * @elapsedTime shows the elapsed time if positive
	 * 
	 */
	void progressBar( int i, int nevents, int textWidth, double elapsedTime, bool skipNonTTY ){
		
		bool tty = _isatty( _fileno(stdout) );
		string crc = "\r";
		if ( !tty )
			crc = "\n";

		// for output
		double progress =  ((double)i / (double)nevents);
		if ( i == nevents - 1)
    		progress = 1.001;


    	int res = (int)((double)nevents / (double) textWidth);
    	if ( res == 0 || res == 1)
    		res = i;

    	// decide when to update
    	if ( i == 0 || (i % res ) == 0 || i == nevents - 1  ){
			// skip for non interactive output
			// if (!isatty(fileno(stdout)) && progress <= 1 && skipNonTTY )
			// 	return;

			double per = progress  * 100;
			per = round( per );

			
			cout << "[";
	    	for ( int ip = 0; ip < textWidth; ip ++ ){
	    		if ( ip < round( (progress * (double)textWidth) ) )
	    			cout << "=";
	    		else 
	    			cout << " ";
	    	}
	    	if ( true ){ 
	    		if ( per >= 10 )
		 	   		cout << "]" << per << "%" ;
				else 
					cout << "] " << per << "%" ;

				if ( elapsedTime >= 0 ){
					
					int nDig = 3;
					
					if ( elapsedTime >= 1 )
						nDig = ceil( log10( elapsedTime ) ) + 2;

					if ( elapsedTime < 60 )
						cout << " : " << ts(elapsedTime, nDig) << " s";
					else {
						int nh = (int)elapsedTime / 3600;
						int nm = (int)elapsedTime / 60;
						nm = nm % 60;
						double ns = elapsedTime - ((int)elapsedTime / 60) * 60.0;
						nDig = ceil( log10( ns ) ) + 2;
						
						if ( nh > 0 )
							cout << " : " << nh << "h " << nm << "m " << ts( ns, nDig ) << "s";
						else 
							cout << " : " << nm << "m " << ts( ns, nDig ) << "s";
					}
				}
				cout << crc;
				std::cout.flush();
				if (progress > 1 && tty ) 
					cout << "[" << endl;
			}
			//  else {
			// 	if ( per >= 10 )
			// 		cout << "]" << per << "%" << "\n";
			// 	else 
			// 		cout << "] " << per << "%" << "\n";
			// }
		}
	}



	// set the start time to now
	void TaskTimer::start( ) { startTime = clock(); }
	/* Return the amount of time elapsed since start() was called
	 * @return Time past in seconds as double
	 */
	double TaskTimer::elapsed( ) { return ( (clock() - startTime) / (double)CLOCKS_PER_SEC ); }
	/* Return the elapsed time as a string
	 * Formats the string to be human readable
	 * @return The amount of elapsed time
	 */
	string TaskTimer::elapsedTime() { 
		int nDig = 3;
		double et = elapsed();
		if ( et >= 1 )
			nDig = ceil( log10( et ) ) + 2;

		return  ts(et, nDig) + " sec";
	}


	
	/* Default Constructor
	 * 
	 */
	TaskProgress::TaskProgress(){

	}

	/* Creates a TaskProgress
	 * @title Title of the Task
	 * @max Maximum number of steps
	 * @width The width of the progress bar
	 * @sTitle Show th title
	 * @sElapse Show elapsed time
	 * 
	 */
	TaskProgress::TaskProgress( string title, int max, int width, bool sTitle, bool sElapse ){
		this->title = title;
		this->max = max;
		barWidth = width;
		showTitle = sTitle;
		showElapsed = sElapse;
		title = "";
	}

	/* Called inside a loop to show the progress of the current task
	 * @i The step in the current loop
	 */
	void TaskProgress::showProgress( int i ){
		if ( 0 == i){
			tt.start();
			if ( showTitle )
				cout << "Running Task : " << title << endl;
		}

		if ( showElapsed )
			progressBar( i, max, barWidth, tt.elapsed() );
		else 
			progressBar( i, max, barWidth );
	}


}