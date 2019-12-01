#include "XmlConfig.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.h"

int main( int argc, char** argv ){

	loguru::init(argc, argv);
	loguru::g_stderr_verbosity = 8;
	
	Logger::setGlobalLogLevel( 40 );
	
	
		XmlConfig xfg( "./tests/main.xml");

		Logger::setGlobalLogLevel( 0 );
		xfg.toXmlFile( "out.xml" );
		xfg.dumpToFile( "out.yaml" );
		
	

	// tests the bool template specialization
	cout << bts( xfg.get<bool>( "ccbar.EventLoop:progress" ) ) << endl;

	return 0;
}