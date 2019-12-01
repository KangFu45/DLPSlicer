#ifndef RAPID_XML_WRAPPER_H
#define RAPID_XML_WRAPPER_H

#include "rapidXML.h"
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>
#include <map>
#include <unordered_map>
using namespace std;
using namespace rapidxml;

#include "Logger.h"
using namespace jdb;


typedef unordered_map<string, string> config_map;

class RapidXmlWrapper
{

protected:
	string configFile;
	string fname;
	char* doc_string = nullptr;

	bool good = false;

	xml_document<> doc;
public:
	RapidXmlWrapper(){
		//DEBUG(  "RapidXmlWrapper", "()" );
		pathDelim = '.';
		attrDelim = ':';
		indexOpenDelim = "[";
		indexCloseDelim = "]";
		equalDelim = '=';
	}
	RapidXmlWrapper( string filename ){

		//DEBUG( "RapidXmlWrapper", "( filename=" << filename << " )" )
		fname = filename;
		configFile = getFileContents( filename.c_str() );
		parseXmlString( configFile );

		pathDelim = '.';
		attrDelim = ':';
		indexOpenDelim = "[";
		indexCloseDelim = "]";
		equalDelim = '=';

	}
	~RapidXmlWrapper(){
		if ( doc_string != nullptr ){
			delete[] doc_string;
			doc_string = nullptr;
		}
	}

	void parseXmlString( string xml ){
		if ( doc_string != nullptr ){
			delete[] doc_string;
			doc_string = nullptr;
		}
		doc_string = new char[xml.size() + 1];  	// Create char buffer to store string copy
		strcpy (doc_string, xml.c_str());             		// Copy string into char buffer

		try {
			doc.parse<0>(doc_string);
			good = true;
		} catch ( exception &e ){
			cout << "Could not parse xml string : " << e.what() << endl;
		}

	}

	string getFileContents(const char *filename){
		ifstream in(filename, ios::in | ios::binary);

		if (in.good()){
			std::string contents;

			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());

			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());

			in.close();

			return(contents);
		}
		return "";
	}

	std::string trim(const std::string& str, const std::string& whitespace = " \t\n") {
		const std::size_t strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return ""; // no content

		const std::size_t strEnd = str.find_last_not_of(whitespace);
		const std::size_t strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	void makeMap( vector<string> *ordered_keys, config_map *data ){
		string context = "";
		xml_node<> * node = doc.first_node();
		map<string, size_t> index;
		makeMap( node, context, ordered_keys, data, index );
	}
	void makeMap( string context, vector<string> *ordered_keys, config_map *data ){
		xml_node<> * node = doc.first_node();
		map<string, size_t> index;
		makeMap( node, context, ordered_keys, data, index );
	}

	void makeMap( string context, vector<string> *ordered_keys, config_map *data, map<string, size_t> &index ){
		xml_node<> * node = doc.first_node();
		makeMap( node, context, ordered_keys, data, index );
	}

	void makeMap( xml_node<> * node, string context, vector<string> *ordered_keys, config_map *data ){
		map<string, size_t> index;
		makeMap( node, context, ordered_keys, data, index );
	}
	void makeMap( xml_node<> * node, string context, vector<string> *ordered_keys, config_map *data, map< string, size_t > &index ){

		if ( false == good ) {
			cerr << "RapidXmlWrapper :: ERROR -> doc root invalid" << endl;
			return;
		}
		stringstream sstr;
		for (xml_node<> *child = node->first_node(); child; child = child->next_sibling() ){
			string nodeName = child->name();
			if ( "" == nodeName )
				continue;

			if ( index.count( nodeName ) <= 0 )
				index[ nodeName ] = 0;

			sstr.str("");
			sstr << index[ nodeName ];
			string path =  context ;

			if ( "" != context )
				path += pathDelim + nodeName;
			else 
				path += nodeName;
			
			path += indexOpenDelim + sstr.str() + indexCloseDelim;

			index[ nodeName ]++;

			string nValue = "";
			if ( child->value() ){
				nValue = trim( std::string(child->value()) );
			} 
			
			(*data)[path] = nValue;
			(*ordered_keys).push_back( path );

			/**
			 * Get attributes
			 */
			for( const xml_attribute<>* a = child->first_attribute(); a ; a = a->next_attribute() ) {
				string aName = a->name();
				string aVal = a->value();
				if ( "" == aName )
					continue;

				string aPath = path + attrDelim + aName;
				(*data)[ aPath ] = aVal;
				(*ordered_keys).push_back( aPath );
			}

			makeMap( child, path, ordered_keys, data );
		}

	}// make map


	//pjdco{ "name" : "char attrDelim", "desc" : "The delimiter used for attributes - Default is \":\""}
	char attrDelim;

	//pjdco{ "name" : "char pathDelim", "desc" : "The delimiter used for paths - Default is \".\""}
	char pathDelim;

	//pjdco{ "name" : "char equalDelim", "desc" : ""}
	char equalDelim;

	//pjdco{ "name" : "indexOpenDelim", "desc" : "The delimeter for index open - Default is \"[\""}
	string indexOpenDelim;
	
	//pjdco{ "name" : "indexCloseDelim", "desc" : "The delimeter for index open - Default is \"]\""}
	string indexCloseDelim;
	
};



#endif