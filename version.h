#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "11";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2014";
	static const char UBUNTU_VERSION_STYLE[] =  "14.06";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 0;
	static const long BUILD  = 70;
	static const long REVISION  = 437;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 75;
	#define RC_FILEVERSION 1,0,70,437
	#define RC_FILEVERSION_STRING "1, 0, 70, 437\0"
	static const char FULLVERSION_STRING [] = "1.0.70.437";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 70;
	

}
#endif //VERSION_H
