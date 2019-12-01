#ifndef LOGGER_H
#define LOGGER_H 

#define JDB_LOG_LEVEL_ALL 	60
#define JDB_LOG_LEVEL_DEBUG 50
#define JDB_LOG_LEVEL_TRACE 40
#define JDB_LOG_LEVEL_INFO 	30
#define JDB_LOG_LEVEL_WARN 	20
#define JDB_LOG_LEVEL_ERROR 10

#ifndef JDB_LOG_LEVEL
	#define JDB_LOG_LEVEL JDB_LOG_LEVEL_ALL
#endif

// Allows the macros to use variable number of args
// so ERROR( "message" ) just outputs a message
// but ERROR( "tag", "message" ) outputs [tag.__func__] message
#define GET_MACRO(_1,_2,NAME,...) NAME
// #define ERROR(...) GET_MACRO(__VA_ARGS__, LOG_E_TAG, LOG_E )(__VA_ARGS__)
// #define WARN(...) GET_MACRO(__VA_ARGS__, LOG_W_TAG, LOG_W )(__VA_ARGS__)

#if JDB_LOG_LEVEL<JDB_LOG_LEVEL_ERROR
	#define ERROR(...) 0;
#else
	#define ERROR(...) {GET_MACRO(__VA_ARGS__, LOG_E_TAG, LOG_E )(__VA_ARGS__)};
#endif

#if JDB_LOG_LEVEL<JDB_LOG_LEVEL_WARN
	#define WARN(...) 0;
#else
	#define WARN(...) {GET_MACRO(__VA_ARGS__, LOG_W_TAG, LOG_W )(__VA_ARGS__)};
#endif

#if JDB_LOG_LEVEL<JDB_LOG_LEVEL_INFO
	#define INFO(...) 0;
#else
	#define INFO(...) {GET_MACRO(__VA_ARGS__, LOG_I_TAG, LOG_I )(__VA_ARGS__)};
#endif
 
#if JDB_LOG_LEVEL<JDB_LOG_LEVEL_TRACE
	#define TRACE(...) 0;
#else
	#define TRACE(...) {GET_MACRO(__VA_ARGS__, LOG_T_TAG, LOG_T )(__VA_ARGS__)};
#endif

#if JDB_LOG_LEVEL<JDB_LOG_LEVEL_DEBUG
	#define DEBUG(...) 0;
#else
	#define DEBUG(...) {GET_MACRO(__VA_ARGS__, LOG_D_TAG, LOG_D )(__VA_ARGS__)};
#endif


#define ERRORC(x) ERROR( classname(), x)
#define WARNC(x) WARN( classname(), x)
#define INFOC(x) INFO( classname(), x)
#define TRACEC(x) TRACE( classname(), x)
#define DEBUGC(x) DEBUG( classname(), x)


#include <iostream>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>
#include <time.h>
using namespace std;

#include "ANSIColors.h"


namespace jdb {

	/* An all purpose logging utility with log level functionality. 
	 *
	 * Meant to be used in modular projects. Multpile Logger instances can be used with different log levels.
	 */
	class Logger{


	protected:

		/**
		 * For removing output seamlessly
		 * Not the most efficient but I like the way it allows the system to behave
		 */
		class NullBuffer : public std::streambuf
		{
		public:
		  int overflow(int c) { return c; }
		};
		class NullStream : public std::ostream {
		public:
			NullStream() : std::ostream(&sb) {}
			private: 
				NullBuffer sb;
		};


	public:

		// Log everything ( Debug, Trace, Info, Warnings, Error )
		static const int llAll 		= 60;
		// Log Debug messages and below ( Debug, Trace, Info, Warnings, Error )
		static const int llDebug 	= 50;
		// Log Trace messages and below ( Trace, Info, Warning, Error )
		static const int llTrace 	= 40;
		// Log Info messages and below ( Info, Warning, Error )
		static const int llInfo		= 30;
		// Log Warning messages and below ( Warning, Error )
		static const int llWarn 	= 20;
		// Log Error messages and below ( Error )
		static const int llError 	= 10;	
		// Log nothing
		static const int llNone 	= 1;

		// The default log level at compile time
		static const int llDefault	= llNone;
		// The global log level - used by any newly created log IF not overridden
		static int llGlobal;

		static bool timeStamp;
		static string timeFormat;

		/*
		 * Gets the global log level for all Logger instances
		 */
		static int getGlobalLogLevel();
		/*
		 * Sets the global log level used by Logger instances
		 * @param ll Log level
		 */
		static void setGlobalLogLevel( int ll );
        static void setGlobalLogLevel( string ll );

		static void setGlobalColor( bool state = true );
		static bool getGlobalColor( );
		static bool showColors;

		static void showTimeStamp( bool show = true );
		static void setTimeFormat( string fmt = "%Y-%m-%d %H:%M:%S" );

		//Singelton Instance
		static Logger log;


		/*
		 * Creates a Logger with 
		 * @param ll 	Log Level ( llAll, llDebug, ... )
		 * @param cs 	ClassSpace for prefixing messages
		 * @param oss	Pointer to an output stream to use
		 * 
		 */
		Logger( int ll, string cs, ostream* oss ){

			setLogLevel( ll );
			cSpace = cs;
			this->os = oss;
		} 
		
		/*
		 * Default constructor creates a Logger with the global log level and no class space prefix
		 * @param ll 	LogLevel
		 * @param cs 	ClassSpace prefix for messages 
		 */
		Logger( int ll = Logger::getGlobalLogLevel(), string cs = "" ){
			setLogLevel( ll );
			cSpace = cs;
			os = &cout;
		}

		/*
		 * Get the Log Level of this Logger instance
		 * @return the current int value LogLevel
		 */
		int getLogLevel() { return logLevel; }
		/*
		 * Set the LogLevel for this Logger instance
		 * @param ll 	LogLevel ( llAll, llDebug, ... )
		 */
		void setLogLevel( int ll ) { 
			
			logLevel = ll; 
			
		}
		/*
		 * Set the LogLevel for this Logger instance
		 * @param ll 	LogLEvel as string 
		 */
		void setLogLevel( string ll ) { 
			
			logLevel = logLevelFromString( ll ); 
			
		}

		/*
		 * Get Class Space prefix
		 * @return ClassSpace prefix string
		 */
		string getClassSpace() { return cSpace; }

		/*
		 * Set the ClassSpace Prefix
		 * @param cs 	ClassSpace Prefix String - usually the name of the class
		 */
		void setClassSpace(string cs ) { cSpace = cs; }

		/*
		 * Warning level ( and below ) log messages
		 * @param functionName 	The name of the calling function to prepend to output
		 * @param showPrefix 	Show the LogLevel, classSpace, and Function name
		 * @return 	The output stream for writing. If the LogLevel does not permit this
		 *              level of output then a NullStream is returned.
		 */
		ostream & warn( string functionName = "", bool showPrefix=true ){
			if ( logLevel < llWarn )
				return ns;

			if ( showPrefix )
				preMessage( "Warning", functionName );
			
			return (*os);
		}
		


		/*
		 * Error level ( and below ) log messages
		 * @param functionName 	The name of the calling function to prepend to output
		 * @param showPrefix 	Show the LogLevel, classSpace, and Function name
		 * @return 	The output stream for writing. If the LogLevel does not permit this
		 *              level of output then a NullStream is returned.
		 */
		ostream & error( string functionName = "", bool showPrefix=true ){
			if ( logLevel < llError )
				return ns;

			if ( showPrefix )
				preMessage( "ERROR", functionName );
			
			return (*os);
		}
		

		/*
		 * Info level ( and below ) log messages
		 * @param functionName 	The name of the calling function to prepend to output
		 * @param showPrefix 	Show the LogLevel, classSpace, and Function name
		 * @return 	The output stream for writing. If the LogLevel does not permit this
		 *              level of output then a NullStream is returned.
		 */
		inline ostream & info( string functionName = "", bool showPrefix = true ){
			if ( logLevel < llInfo )
				return ns;

			if ( showPrefix )
				preMessage( "Info", functionName );
			
			return (*os);
		}
		

		/*
		 * Trace level ( and below ) log messages
		 * @param functionName 	The name of the calling function to prepend to output
		 * @param showPrefix 	Show the LogLevel, classSpace, and Function name
		 * @return 	The output stream for writing. If the LogLevel does not permit this
		 *              level of output then a NullStream is returned.
		 */
		ostream & trace( string functionName = "", bool showPrefix=true ){
			if ( logLevel < llTrace )
				return ns;

			if ( showPrefix )
				preMessage( "Trace", functionName );
			
			return (*os);
		}
		

		/*
		 * Debug level ( and below ) log messages
		 * @param functionName 	The name of the calling function to prepend to output
		 * @param showPrefix 	Show the LogLevel, classSpace, and Function name
		 * @return 	The output stream for writing. If the LogLevel does not permit this
		 *              level of output then a NullStream is returned.
		 */
		ostream & debug( string functionName = "", bool showPrefix=true ){
			if ( logLevel < llDebug )
				return ns;

			if ( showPrefix )
				preMessage( "Debug", functionName );
			
			return (*os);
		}

		
		/*
		 * Converts a Human Readable string to the corresponding LogLevel
		 * @param ll 	String log level 
		 */
		static int logLevelFromString( string ll ){
			// push to lower case for comparison
			string str2 = ll;

			//É¾³ý
			//for ( unsigned long int i = 0; i < str2.length(); i++ ){
			//	str2[ i ] = std::tolower( str2[ i ] );
			//}

			if ( "debug" == str2 )
				return llDebug;
			else if ( "trace" == str2 )
				return llTrace;
			else if ( "info" == str2 )
				return llInfo;
			else if ( "warning" == str2  )
				return llWarn;
			else if ( "error" == str2 )
				return llError;
			else if ( "all" == str2 )
				return llAll;
			else if ( "none" == str2 )
				return llNone;
			return llAll;

		}

		void summary() {

			int w1 = 10;
			info() << std::left << std::setw(w1) << "# of ERRORS : " << counts[ "error" ] << endl;
			info() << std::left << std::setw(w1) << "# of Warnings : " << counts[ "warning" ] << endl;
			info() << std::left << std::setw(w1) << "# of Info : " << counts[ "info" ] << endl;
		}


	protected:
		
		int logLevel;
		map< string, int > counts;

		void preMessage( string level = "", string functionName = "" ){

			string tag = level;
			// force level to lower case
			transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

			string coloredLevel = level;
			if ( showColors && logLevelFromString( tag ) == llError )
				coloredLevel = ANSIColors::color( level, "red" );
				// coloredLevel = "\033[1;31m" + level + "\033[0;m";
			else if ( showColors && logLevelFromString( tag ) == llWarn )
				coloredLevel = ANSIColors::color( level, "yellow" );
				// coloredLevel = "\033[1;33m" + level + "\033[0;m";

			if (timeStamp){

			}

			int w1 = 8;

			string context = "[" + cSpace + "." + functionName + "] ";
			if ( cSpace.length() < 2 && functionName.length() >= 2 )
				context = "[" + functionName + "] ";
			else if ( cSpace.length() < 2 && functionName.length() < 2 )
				context = " ";


			(*os) << std::left;

			if (timeStamp){
				char buff[20];
				struct tm *sTm;

				time_t now = time (0);
				gmtime_s(sTm, &now);

				strftime (buff, sizeof(buff), timeFormat.c_str(), sTm);
				(*os) << buff << " ";
			} else {
			}


			(*os) << std::left << std::setw(w1) << coloredLevel << " :";
			(*os) << " " << context;

			// if ( cSpace.length() >= 2 && functionName.length() >= 2 )
			// 	(*os) << std::left << std::setw(w1) << coloredLevel << " : " << "[" << cSpace << "." << functionName << "] ";
			// else if (cSpace.length() < 2 && functionName.length() >= 2)
			// 	(*os) << std::left << std::setw(w1) << coloredLevel << " : " << "[" << functionName << "] ";
			// else if ( coloredLevel.length() >= 1 )
			// 	(*os) << "" << std::left << std::setw(w1) << coloredLevel << " : ";

			
			if ( !counts[ tag ] )
				counts[ tag ] = 0;
			counts[ tag ]++;
		}

		// The output stream used for printing messages
		std::ostream* os;
		// The Null Stream used to swallow messages
		NullStream ns;
		// The "class space" ie the context
		string cSpace;
		
#ifdef __CINT__
		ClassDef( jdb::Logger, 1 )
#endif

	};


}


#define LOG_D(x) Logger::log.debug( __func__ ) << x << endl;
#define LOG_T(x) Logger::log.trace( __func__ ) << x << endl;
#define LOG_I(x)  Logger::log.info( __func__ ) << x << endl;
#define LOG_W(x)  Logger::log.warn( __func__ ) << x << endl;
#define LOG_E(x) Logger::log.error( __func__ ) << x << endl;

// TODO: same here


#define LOG_D_TAG( tag, x ) Logger::log.setClassSpace( tag ); Logger::log.debug( __func__ ) << x << endl; 
#define LOG_T_TAG( tag, x ) Logger::log.setClassSpace( tag ); Logger::log.trace( __func__ ) << x << endl; 
#define LOG_I_TAG( tag, x ) Logger::log.setClassSpace( tag ); Logger::log.info( __func__ ) << x << endl; 
#define LOG_W_TAG( tag, x ) Logger::log.setClassSpace( tag ); Logger::log.warn( __func__ ) << x << endl; 
#define LOG_E_TAG( tag, x ) Logger::log.setClassSpace( tag ); Logger::log.error( __func__ ) << x << endl;



#endif