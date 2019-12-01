#ifndef XML_CONFIG_H
#define XML_CONFIG_H

#ifndef __CINT__
#include "RapidXmlWrapper.h"
#endif

// STD
#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include <sstream>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <memory>
#include <utility>         // std::pair, std::make_pair
#include <stdexcept>      // std::out_of_range

#include <sys/stat.h>

// Roobarb
#include "Logger.h"
#include "Utils.h"
	// Interfaces
	#include "IObject.h"




namespace jdb {

	class XmlString;

	class XmlConfig : public IObject
	{
	protected:

		// data - map of node path to string representation
		config_map data;
		vector<string> orderedKeys;

		
		// map<string, string> iterator
		typedef config_map::iterator map_it_type;
		// map<string, string> const iterator
		typedef config_map::const_iterator const_map_it_type;

		//Filename of the config file
		string filename;

		//The delimiter used for attributes - Default is ":"
		char attrDelim;

		//The delimiter used for paths - Default is "."
		char pathDelim;

		// The delimeter used for equality - Default is '='
		char equalDelim;

		// The delimeter used for maps - Default is "::"
		string mapDelim;

		//The delimeter for index open - Default is "["
		string indexOpenDelim;
		
		//The delimeter for index open - Default is "]"
		string indexCloseDelim;

		// Current node for relative path finding
		string currentNode = "";
		string lastNode = "";

		// a builting nodepath string for passing around
		string cat = "";

		// shared_ptr<XmlString> xStr = nullptr;
	public:
		virtual const char* classname() const { return "XmlConfig";}
		static const string declarationV1;

		/* Creates an XmlConfig from the given xml file
		 *@filename The file containg valid xml to load
		 *
		 * Loads the file, parses its content and makes its data available
		 */
		XmlConfig( string filename );

		/* Default Ctor, empty config
		 *
		 */
		XmlConfig();

		/* Dtor, closes resources
		 *
		 */
		~XmlConfig();

		/* Copy Ctor
		 *
		 * Copies maps to new object, it is now an effective copy of the original config
		 */
		XmlConfig( const XmlConfig &rhs);

		/* Sets the default values for delimeters etc.
		 *
		 */
		void setDefaults();

		/* Loads a file into the current config object
		 *
		 * @Filename 	path to XML file
		 */
		void loadFile( string filename );
		void loadFile( string filename, map<string, string> overrides );

		void loadXmlString( string xml );
		void loadXmlString( string xml, map<string, string> overrides );

		/* Gets the filename of the current XML file loaded into memory
		 *
		 * @return 	name of current file
		 */
		string getFilename() const { return filename; }

		bool isAttribute( string _in ) const;

		void at( string np ){
			cat = np;
		}

		string at() {
			return cat;
		}

		/**
		 * Sets the current node so that relative node paths can be used
		 * @nodePath 	Path to Node - this will be prepended the nodePath passed in to lookups
		 *
		 * @return 		Current Node before changing
		 */
		string cn( string nodePath = "" ){
			if ( currentNode == nodePath ) return lastNode;
			string lastNode = currentNode;
			currentNode = nodePath;
			return lastNode;
		}

		string back(){
			string tmp = currentNode;
			currentNode = lastNode;
			lastNode = tmp;
			return currentNode;
		}

		/* Same as getString(...) but with the [] operator.
		 * @nodePath See getString(...)
		 * @returns The underlying xml data at nodePath as a string
		 */
		string operator[]( string nodePath ) const; 

		template <typename T>
		const T operator[]( string path ) const{
			stringstream sstr;
			sstr << getString( path );
			T result;
			sstr >> result;
			return result;
		}

		template <typename T>
		T get( string path ) const {
			stringstream sstr;
			sstr << getString( path );
			T result;
			sstr >> result;
			return result;
		}

		template <typename T>
		T get( string path, T dv ) const {
			if ( !exists( path ) ){
				return dv;
			}
			stringstream sstr;
			sstr << getString( path );
			T result;
			sstr >> result;
			return result;
		}



		/* Set operator
		 * Usage:
		 * 	config[ full.node.path:to_attrib ] = "value_as_string"
		 */
		// TODO: [] assignment for adding nodes ?
		// string & operator[]( string nodePath );
		
		/* Gets xml node or attribute data as a string
		   @nodePath The path to the desired node from the root node. See examples below.
		   @def **Default** = "" The value to return if the node or attribute is not found

		   For an xml block like
		   ``` xml
			<root>
				<data>
					<category>
						<cut1 name="cut1" />
					</category>
				</data>
			</root>
		   ```
		   The node "<cut1 />" can be accessed using 
		   the nodePath = "data.category.cut1" and its 
		   attribute can be accessed using the nodePath =
		   "data.category.cut1:name"
		   
		   @return The underlying xml data at nodePath as a string
		 */
		string getRawString( string nodePath, string def = "" ) const;

		string getString( string nodePath, string def = "" ) const;

		string getString( string prefix, vector<string> paths, string def = "" ) const;



		/* Gets an String processed via XmlString
		 *
		 * XmlString processes the raw string and looks for {tokens}
		 * If such a token is found in the string then it is replace
		 * first by ENV variables matching the TOKEN or 
		 */
		string getXString( string nodePath, string def = "" ) const;

		vector<string> getStringVector( string nodePath ) const;
		/* Gets a vector of strings from a comma delimeter list
		 * @nodePath Path to node. See getString.
		 * Gets a vector from a comma delimeted list. 
		 * For instance, if a node contains "1, 3, 9, 7, 16" then the vector whould contain 5 elements
		 *
		 * @return A vector of strings or an empty vector if the node DNE
		 */
		vector<string> getStringVector( string nodePath, string defaultVal, int defaultLength ) const;
		
		// pass a default vector in as an initializer list etc.
		vector<string> getStringVector( string nodePath, vector<string> defaultVals ) const;

		/* Gets a node or attribute as integer data
		 * @nodePath Path to node. See getString(...)
		 * @def Default value if the endpoint DNE or conversion to int fails
		 *
		 * Uses atoi(...) to convert string data to builtin type int
		 * @return The underlying xml data at nodePath as an builtin type integer
		 */
		int getInt( string nodePath, int def = 0 ) const;

		/* Gets a vector of integers from a comma delimeted list
		 * @nodePath See getString(...)
		 * See getStringVector(...).
		 * Converts a comma separated list into a vector of int types. 
		 * Uses atoi(...) for string to int conversion.
		 * @return vector of integers, one for each item in the comma delimeted list
		 */
		vector<int> getIntVector( string nodePath, int defaultVal = 0, int defaultLength = 0 ) const;

		/* Gets a map<string, string> from a config node 
		 * @nodePath See getString(...)
		 * Converts a node like :
		 * ``` xml
		 * <Map>
		 * 		from : to,
		 * 		alpha : beta
		 * </Map>
		 * ```
		 * Into a map where map[ "from" ] = "to" and map[ "alpha" ] = "beta".
		 * 
		 */
		map<string, string> getStringMap( string nodePath ) const;
		
		/* TODO : add docs
		 *
		 */
		map<int, int> getIntMap( string nodePath ) const;

		map<float, float> getFloatMap( string nodePath ) const;


		/* Gets a node or attribute as double type
		 * @nodePath Path to node. See getString(...)
		 * @def Default value if the endpoint DNE or conversion to double fails
		 *
		 * Uses atof(...) to convert string data to builtin type double
		 * @return The underlying xml data at nodePath as an builtin type double
		 */
		double getDouble( string nodePath, double def = 0 ) const;

		/* Gets a vector of doubles from a comma delimeted list
		 * @nodePath See getString(...)
		 * See getStringVector(...).
		 * Converts a comma separated list into a vector of double types. 
		 * Uses atof(...) for string to double conversion.
		 * @return vector of doubles, one for each item in the comma delimeted list
		 */
		vector<double> getDoubleVector( string nodePath, double defaultVal = 0, int defaultLength = 0 ) const;
		vector<float> getFloatVector( string nodePath, float defaultVal = 0, int defaultLength = 0 ) const;

		/* Gets a node or attribute as foat type
		 * @nodePath Path to node. See getString(...)
		 * @def Default value if the endpoint DNE or conversion to foat fails
		 *
		 * Uses atof(...) and cast to convert string data to builtin type foat
		 * @return The underlying xml data at nodePath as an builtin type foat
		 */
		float getFloat( string nodePath, float def = 0 ) const;
		
		/* Gets a node or attribute as bool type
		 * @nodePath Path to node. See getString(...)
		 * @def Default value if the endpoint DNE or conversion to bool fails
		 *
		 * Can be string of any case "true", or "false"
		 * Uses atoi(...)  to convert string data to builtin type bool
		 * @return The underlying xml data at nodePath as an builtin type bool
		 */
		bool getBool( string nodePath, bool def = false ) const;

		/* Determine whether a node exists in the xml or not
		 * @nodePath Path to node. See getString(...)
		 *
		 * Searches the xml structure for the given node.
		 * @return **True** - node or attribute is found. **False** otherwise
		 */
		bool exists( string nodePath ) const;

		string oneOf( vector<string> _paths );
		string oneOf( string _p1, string _p2, string _p3 = "", string _p4 = "" );

		/* Lists the children of a node
		 * @nodePath Path to node. See getString(...)
		 * @depth The number of levels to search for children
		 * @attrs Show Attributes of node and children or not
		 * 
		 * Gets a list of paths to all children of a given node. 
		 * Very useful for automating tasks, building lots of objects, etc.
		 *
		 * @return Vector of strings containg paths to each node or attribute
		 */
		vector<string> childrenOf( string nodePath, int depth = -1, bool attrs = false ) const;

		/* Lists the children of a node selecting only a given tag
		 * @nodePath Path to node. See getString(...)
		 * @tagName Filters children of the given tagName
		 * @depth depth to search - not yet implemented
		 * 
		 * Gets a list of paths to all children of a given node of the given tagName type. 
		 * Very useful for automating tasks, building lots of objects, etc.
		 *
		 * 	Just a special case of the more general childrenOf(...)
		 * 
		 * @return Vector of strings containg paths to each node or attribute
		 */
		vector<string> childrenOf( string nodePath, string tagName, int depth = -1) const;

		/* Get the attributes of a node
		 * @nodePath Path to node. See getString(...)
		 * 
		 * One path is added for each attribute of the given
		 * node or an empty list if no attributes 
		 * are present. Use attributeName( attributePath ) to 
		 * get only the attribute name from the full path.
		 *
		 * @return Vector of strings containg paths to each attribute
		 */
		vector<string> attributesOf( string nodePath ) const;

		/* TODO : docs
		 *
		 */
		map<string, string> attributesMap( string nodePath ) const;

		/* Find nodes based on search criteria
		 * @nodePath Path to node. See getString(...). May also 
		 * contain conditional parts. See description.
		 *
		 * Can be used to find nodes matching a conditions   
		 * 
		 * **Case 1 )** Queries like : "group.node" returns a list 
		 * of paths to all nodes of the form "group.node[0..N]"   
		 * 
		 * **Case 2 )** Queries like : "group.node:name" returns a 
		 * list of paths to all nodes of the form "group.node[0..N]" 
		 * when they contain a "*name*" attribute   
		 * 
		 * **Case 3 )** Queries like : "group.node:name=value" 
		 * returns a list of paths to all nodes of the 
		 * form "group.node[0..N]" when they contain a "*name*" 
		 * attribute and it equals "*value*"
		 *
		 * @return A Vector of strings. One for each path to a 
		 * node or attribute matching the query
		 */
		vector<string> getNodes( string nodePath ) const;
		vector<string> query( string _qs ) const;
		string q( string _qs ) const {
			vector<string> qrs = query( _qs );
			if ( qrs.size() >= 1 )
				return qrs[0];
			return "";
		}

		/*Splits a string into pieces by the delimeter character
		 *@s Input string to split
		 *@delim delimeting character
		 *
		 *@return A vector of strings, one for each piece after 
		 *splitting on the delimeting character. If the delimeting 
		 *character is not found a zero length Vector is returned.
		 */
		vector<string> split(const string &s, char delim) const ;
		vector<string> split(string &s, string delim) const ;

		/*Trims characters off the front and back of strings
		 *@str Input string to trim
		 *@whitespace A string containing characters to trim. **Default** = " \t\n".
		 */
		std::string trim(const std::string& str, const std::string& whitespace = " \t\n") const;

		
		/* Get just the tag name from a full path
		 * @nodePath Path to node. See getString(...)
		 *
		 * Extracts the name of the final node (tag name )
		 * For instnce:
		 * The node path "category.sub.a" yields "a"
		 *
		 * @return the tag name of the node pointed to by nodePath
		 */
		string tagName( string nodePath, int atDepth = -1 ) const;

		/* Get the path to the parent of this node
		 * @nodePath Path to node. See getString(...)
		 *
		 * Extracts the path to the parent of this node
		 * For instnce:
		 * The node path "category.sub.a" yields "category.sub"
		 *
		 * @return the path to the parent of given node
		 */
		string pathToParent( string nodePath ) const;
		string pathToDepth( string nodePath, int depth ) const;


		/* Gets the base path from a node path
		 * @nodePath Path to node. See getString(...)
		 *
		 * Given 'Test.Node...' or '...Test.Node:bleh'
		 * gives 'Test.Node'
		 *
		 * @return the path to the base node
		 */
		string basePath( string nodePath, bool keepAttribute = false ) const;

		/* Joins two paths correctly
		 *
		 * @paths 	list of string type paths to join
		 * 1) 	Given 'A.B' and 'C.D'
		 * 		Gives : 'A.B.C.D'
		 * 
		 * 2) 	Given 'A.B..' and 'C.D.'
		 * 		Gives : 'A.B.C.D'
		 * 3) 	Given 'A.B..' and 'C.D' and '.E:attribute'
		 * 		Gives : 'A.B.C.D.E:attribute'
		 *
		 * @return properly joined full path
		 */
		string join( std::initializer_list<string> paths ) const{
			vector<string> vpaths;
			for ( string p : paths ){
				vpaths.push_back( p );
			}
			return join( vpaths );
		}

		/* Convenience method for joining path string
		 *
		 * @return fully joined path
		 */
		string join( string a, string b, string c="", string d="", string e="", string f="" ) const {
			return join( {a, b, c, d, e, f} );
		}

		string join( vector<string> paths ) const;

		/* Determines the depth of a node
		 * @nodePath Path to Node
		 *
		 * Calulates the depth of the node from the root node
		 * @return the depth of the given node. Children of the root node have depth 0
		 */
		int depthOf( string nodePath ) const;

		/* Calculates the depth of a node relative to another node
		 * @nodePath Path to node of interest
		 * @relativeTo The node whose depth is considered 0
		 */
		int depthOf( string nodePath, string relativeTo ) const {
			return depthOf( nodePath ) - depthOf( relativeTo );
		}

		string report( string nodePath = "" ) const;

		void applyOverrides( map< string, string > over );
		void set( string nodePath, string value );


		string indentation( int count, string tab="\t" ) const {
			string is = "";
			for ( int i = 0; i < count; i++ )
				is += tab;
			return is;
		}

		string toXml( string nodePath ="", int tabCount = 0, string tab="\t", string nl = "\r\n" ) const;

		void toXmlFile( string filename ) const;

		string dump() const;
		void dumpToFile( string filename ) const;

		void add( string nodePath, string value="" );

		// Sanatizes node paths
		string sanitize( string nodePath ) const;

		// Strips the FINAL index if there are multiple
		string stripIndex( string _in ) const{
			string::size_type index = _in.find_last_of( indexOpenDelim[0] );
			if ( string::npos != index ){
				return _in.substr( 0, index );
			}
			return _in;
		}

		string addIndex( string _in, int index = 0 ) const {
			// return basePath( _in ) + indexOpenDelim[0] + ts(index) + indexCloseDelim[0];
			return _in + indexOpenDelim[0] + ts(index) + indexCloseDelim[0];
		}

		string stripAttribute( string _in ) const{
			
			string::size_type index = _in.find( attrDelim );
			if ( string::npos != index ){
				return _in.substr( 0, index );
			} 
			return _in;
		}

		/* Get the attribute name from a full path
		 *@nodePath Path to node. See getString(...)
		 *
		 * Extracts the attribute name from a full node path.
		 * For instance:
		 * The node path "category.sub.a:name" yields "name"
		 *
		 * @return The attribute name if the path contains one.
		 * Empty string otherwise
		 */
		string attributeName( string _in ) const{
			string::size_type index = _in.find( attrDelim );
			if ( string::npos != index ){
				return _in.substr( index+1 );
			} 
			return "";
		}


		// void include( XmlConfig otherXfg, string p = "", bool overwrite = false );

		config_map getDataMap(  ) const { return data; }
		vector<string> getOrderedKeys(  ) const { return orderedKeys; }
		// map<string, bool> getNodeExistMap(  ) const { return nodeExists; }
		// map<string, bool> getIsAttributeMap(  ) const { return isAttribute; } 

		// Adding content
		void addNode( string nodePath, string value="" );
		size_t deleteNode( string nodePath );
		void addAttribute( string nodePath, string value="" );
		bool deleteAttribute( string path );

		// void checkNewIncludes( string nodePath){
		// 	if ( unprocessedIncludes( nodePath ) ){
		// 		parseIncludes( nodePath );
		// 	}
		// }
		void include_xml( RapidXmlWrapper * rxw, string path, vector<string>::iterator );
		void include_xml( RapidXmlWrapper * rxw, string path );
		void include_xml( string xmlstr, string path );
		

	protected:

		size_t indexOf( string path ){
			return find( orderedKeys.begin(), orderedKeys.end(), path) - orderedKeys.begin();
		}

		vector<string>::iterator iteratorOf( string path ){
			return find( orderedKeys.begin(), orderedKeys.end(), path);
		}

		/* gets the index for a given tag up to that ordering in the ordered keys
		 *
		 * @path 	Path.To.Node
		 */
		size_t countTag( string path ){
			return countTag( path, 0, orderedKeys.size() );
		}

		size_t countTag( string path, size_t start, size_t stop ) const;

		/* Rewrites a key 
		 * Also checks for children and attributes, rewriting them if needed
		 *
		 */
		bool rewrite( string oldKey, string newKey );

		/* rewriteKey
		 *
		 * Rewrites a single key
		 */
		bool rewriteKey( string oldKey, string newKey );

		/* Increments tag indices
		 *
		 */
		void incrementTagIndex( string path, string tagname, int n, size_t start, size_t stop );

		void ensureLineage( string path );
		// A manual case lowing function
		string manualToLower( string str ) const;
		
		// Gets a vector from comma delimeted strings
		vector<string> vectorFromString( string data ) const;
		// Splits strings using the given delim character
		vector<string> &split(const string &s, char delim, vector<string> &elems) const;
		vector<string> &split(string &s, string delim, vector<string> &elems) const;

		// A special case version of split used for the map decoding
		// Allows string delimeter
		pair<string, string> stringToPair( string &s, string delim  ) const;

		string resolveFilename( string in ){
			struct stat buffer;
			bool exists = (stat (in.c_str(), &buffer) == 0);

			if (exists)
				return in;

			string basePath = pathFromFilename( filename );

			in = basePath + in;
			exists = (stat (in.c_str(), &buffer) == 0);
			if ( exists )
				return in;
			return "";
		}


		string pathFromFilename (const string& str) const {
			size_t found;
			found=str.find_last_of("/\\");
			//cout << " folder: " << str.substr(0,found+1) << endl;
			//cout << " file: " << str.substr(found+1) << endl;
			return str.substr(0,found+1);
		}

		// parseInclude nodes
		// @return 	number of includes that could not be resolved
		int parseIncludes( string nodePath = "" );

		int unprocessedIncludes( string nodePath = "" );
		void merge( vector<string> *_opaths, map<string, string> *_data, bool resolveConflicts = true  ){}
		bool conflictExists( map<string, string> *_data, string &shortestConflict );

		bool passConditional( string cond, string nodePath ) const;

		int numberOf( string _path );
		string incrementPath( string _in, int _n = 1);
		int pathIndex( string _in ) const;





		// Override content from node after include when used in conjunction with applyOverrides
		map<string, string> makeOverrideMap( string _nodePath );



#ifdef __CINT__
		ClassDef( jdb::XmlConfig, 1 )
#endif
	};

}


#endif