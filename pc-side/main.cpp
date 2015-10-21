#include <iostream>
#include "tinyxml.h"
#include <string>
#include <sstream>
#include <algorithm>

#include "serial.h"
#include <vector>

using namespace std;
using namespace boost;

struct coord {
	float x;
	float y;
	int type;
};


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

void parsePath(std::string data, std::vector<coord> &path_list) {
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
				coord p;
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords) {
                    if (up(coords))
                        std::cout<<"moveTo("<<coords<<");"<<std::endl;
                    p.x = x(coords);
                    p.y = y(coords);
					p.type = 0;
					path_list.push_back(p);
                }
            }
            else if (keyword==std::string("C")) {  //Curve absolute
				coord p;
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords, data>>coords, data>>coords) {
                    if (up(coords))
                        std::cout<<"lineTo("<<coords<<");"<<std::endl;
                    p.x = x(coords);
                    p.y = y(coords);
					p.type = 1;
					path_list.push_back(p);
                }
            }
            else if (keyword==std::string("c")) {  //Curve relative
                std::stringstream data;
                data<<acc;
                std::string coords;
                while (data >> coords, data>>coords, data>>coords) {
					coord p;
					p.x = x(coords);
					p.y = y(coords);
					p.type = 2;
                    if (up(coords))
						path_list.push_back(p);
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

void dump_to_stdout( TiXmlNode* pParent, std::vector<coord>&path_list) {
	if ( !pParent ) return;

	TiXmlNode* pChild;

	int t = pParent->Type();
	switch ( t ) {
	  case TiXmlNode::TINYXML_ELEMENT:
		if (pParent->Value() == std::string("path")) {
			std::string g = get_g(pParent->ToElement());
            parsePath(g, path_list);
		}
		break;
	  default:
		break;
	}

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())  {
		dump_to_stdout( pChild, path_list );
	}
}

// load the named file and dump its structure to STDOUT
void dump_to_stdout(const char* pFilename, std::vector<coord> &path_list) {
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay) {
		dump_to_stdout( &doc, path_list); // defined later in the tutorial
	} else {
		std::cout << "Couldn't load file " << pFilename << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::vector<coord> path_list;
	for (int i=2; i<argc; i++) {
		dump_to_stdout(argv[i], path_list);
	}
	cout << path_list.size() << endl;
	for(coord i: path_list) {
		cout << i.x << endl << i.y << i.type << endl;
	}
    try {

        SimpleSerial serial(argv[1],115200);

	
			int i = 0;
			for(coord co: path_list) {
				if(i > 1) {
					std::ostringstream ostrx;
					std::ostringstream ostry;

					  ostry << co.y;
					  ostrx << co.x;

					    std::string xvalue = ostrx.str();
					    std::string yvalue = ostry.str();
        			serial.writeString("P" + xvalue + ";" + yvalue + ";1\n");
        			cout << ("P" + xvalue + ";" + yvalue + ";1\n") << endl;
				}
					i++;
			}
		}

    catch(boost::system::system_error& e)
    {
        cout<<"Error: "<<e.what()<<endl;
        return 1;
    }
}
