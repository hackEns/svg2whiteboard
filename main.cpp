// tutorial demo program
#include <iostream>
#include "tinyxml.h"
#include <string>
#include <sstream>
#include <algorithm>

float xmin = 10000, xmax = -10000, ymin = 10000, ymax = -10000;

bool up(float x, float y) {
    if (x>370 || y>90 || x<-370 || y<-90) return false;
    if (x<xmin) xmin=x;
    if (x>xmax) xmax=x;
    if (y<ymin) ymin=y;
    if (y>ymax) ymax=y;
    return true;
}

bool up(std::string s) {
    const char* cs1 = s.c_str();
    const char* cs2 = strchr(cs1,',');
    bool keep = true;
    if (cs2!=NULL) {
        keep = keep && up(atof(cs1),atof(cs2+1));
    }
    return keep;
}

float x(std::string coords) {
    return atof(coords.c_str());
}

float y(std::string s) {
    const char* cs1 = s.c_str();
    const char* cs2 = strchr(cs1,',');
    if (cs2!=NULL) {
        return atof(cs2+1);
    }
    return 0;
}

float cursorX=0, cursorY=0;

void parsePath(std::string data) {
    const char* keywords[] = {"m", "l", "h", "v", "c", "s", "q", "t", "a", "z"};
    const int nkeywords = 10;
    std::stringstream ss;
    ss << data;
    std::string token;

    std::string acc;
    std::string keyword;
    ss >> keyword;
    while (ss >> token) {
        std::string tokencopy = token;
        std::transform(tokencopy.begin(), tokencopy.end(), tokencopy.begin(), ::tolower);
        bool iskeyword=false;
        for (int i=0; i<nkeywords; i++) {
            iskeyword = iskeyword || (tokencopy == std::string(keywords[i]));
        }
        if (iskeyword) {
            if (keyword==std::string("m")) {
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords) {
                    if (up(coords))
                        std::cout<<"moveTo("<<coords<<");"<<std::endl;
                    cursorX = x(coords);
                    cursorY = y(coords);
                }
            }
            else if (keyword==std::string("C")) {  //Curve absolute
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords, data>>coords, data>>coords) {
                    if (up(coords))
                        std::cout<<"lineTo("<<coords<<");"<<std::endl;
                    cursorX = x(coords);
                    cursorY = y(coords);
                }
            }
            else if (keyword==std::string("c")) {  //Curve relative
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords, data>>coords, data>>coords) {
                    if (up(coords))
                        std::cout<<"lineToRel("<<coords<<");"<<std::endl;
                }
            }

            keyword = token;
            acc = std::string();
        }
        else {
            acc += std::string(" ")+token;
        }
    }
}

std::string get_g(TiXmlElement* pElement) {
	if ( !pElement ) return 0;
	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	while (pAttrib) {
		if (pAttrib->Name() == std::string("d")) {
            return std::string(pAttrib->Value());
        }
		pAttrib=pAttrib->Next();
	}
	return std::string();
}

void dump_to_stdout( TiXmlNode* pParent, unsigned int indent = 0 ) {
	if ( !pParent ) return;

	TiXmlNode* pChild;

	int t = pParent->Type();
	switch ( t ) {
	  case TiXmlNode::TINYXML_ELEMENT:
		if (pParent->Value() == std::string("path")) {
			std::string g = get_g(pParent->ToElement());
            parsePath(g);
		}
		break;
	  default:
		break;
	}

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())  {
		dump_to_stdout( pChild );
	}
}

// load the named file and dump its structure to STDOUT
void dump_to_stdout(const char* pFilename) {
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
    std::cout<<"void draw() {"<<std::endl;
	if (loadOkay) {
		dump_to_stdout( &doc ); // defined later in the tutorial
	}
    std::cout<<"}"<<std::endl;
    std::cout<<"float xmin = "<<xmin<<";"<<std::endl;
    std::cout<<"float xmax = "<<xmax<<";"<<std::endl;
    std::cout<<"float ymin = "<<ymin<<";"<<std::endl;
    std::cout<<"float ymax = "<<ymax<<";"<<std::endl;
}

// ----------------------------------------------------------------------
// main() for printing files named on the command line
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
	for (int i=1; i<argc; i++) {
		dump_to_stdout(argv[i]);
	}
	return 0;
}
