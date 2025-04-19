/*
* Name: Gurbaaz Singh Cheema
* Date: March 27 2025
*/

#ifndef VCHelpers_H
#define VCHelpers_H

#include "VCParser.h"
#include <ctype.h>

// A case insensitive implementation of strcmp
int strCaseIgnoreCmp (const char * str1, const char * str2);

// A function to check if our content line ends with CRLF, return 0 (missing CRLF) or 1 (CRLF present)
int checkForCRLF (char * stringToCheck);

// A function to handle optionalProperties and Birthday and Anniversary, returns a VCardErrorCode (OK for valid parsing and anything else for invalid)
VCardErrorCode optionalPropertiesHandler (char * stringFromFile, Card **obj, FILE * fileToRead, char * fileName, int * endFound);

// A function to count the occurrences of a character in a string
int countCharOccurrences(char * string, char character);

// A function to validate a single property, returns a VCardErrorCode depending on the validation of the property
VCardErrorCode validateProperty(const Property * prop);

// A function to validate a single property name (is it part of rcf 6350 section 6.1-6.9.3?), returns a VCardErrorCode
VCardErrorCode validatePropertyName (const Property * prop);

// A function to validate a single property's value list (making sure each entry in the list is a string, not NULL)
VCardErrorCode validatePropertyValues (const List * values);

// A function to validate a single property's parameter list (making sure name and value is not NULL or empty string)
VCardErrorCode validateParameter (const Property * prop);

#endif
