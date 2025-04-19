/*
* Assignment 3
* 
* Name: Gurbaaz Singh Cheema
* Email: gcheem07@uoguelph.ca
* Student ID: 1278175
* Class: CIS2750 W25
* Date: March 27 2025
*/

#include "VCHelpers.h"

// A case insensitive implementation of strcmp
int strCaseIgnoreCmp (const char * str1, const char * str2) {
    // Declare and initialize all needed variables
    int lengthOfOne = strlen(str1);
    int lengthOfTwo = strlen(str2);
    char tempStrOne [lengthOfOne + 1];
    char tempStrTwo [lengthOfTwo + 1];
    strcpy(tempStrOne, str1);
    strcpy(tempStrTwo, str2);

    // Use a for loop to go through and convert each and every character of tempStrOne and tempStrTwo to lowercase using toLower
    for (int i = 0; i < lengthOfOne; i++) {
        tempStrOne[i] = tolower((unsigned char)tempStrOne[i]);
    }
    for (int i = 0; i < lengthOfTwo; i++) {
        tempStrTwo[i] = tolower((unsigned char)tempStrTwo[i]);
    }

    // Use strcmp to compare the two strings and return the result
    return strcmp(tempStrOne, tempStrTwo);
}

// A function to check if our content line ends with CRLF, return 0 (missing CRLF) or 1 (CRLF present)
int checkForCRLF (char * stringToCheck) {
    // Make sure that stringToCheck is not NULL
    if (stringToCheck == NULL) {
        return 0; // Return 0 in this case
    }

    // Declare and initialize all needed variables
    int lengthOfString = strlen(stringToCheck);

    // If string is empty, return back with 0
    if (lengthOfString == 0) {
        return 0;
    }
    // Make sure the length of the string is greater than 1 and that the last two characters are \r and \n
    else if (lengthOfString > 1 && stringToCheck[lengthOfString - 2] == '\r' && stringToCheck[lengthOfString - 1] == '\n') {
        return 1; // Return 1 if CRLF present
    }
    else {
        return 0; // Return 0 if CRLF missing
    }
}

// A function to handle optionalProperties and Birthday and Anniversary, returns a VCardErrorCode (OK for valid parsing and anything else for invalid)
VCardErrorCode optionalPropertiesHandler (char * stringFromFile, Card **obj, FILE * fileToRead, char * fileName, int * endFound) {
    // Declare and initialize all needed variables
    long currentFilePos = ftell(fileToRead);
    FILE * independentFile = fopen(fileName, "r");
    fseek(independentFile, currentFilePos, SEEK_SET);
    char * tokenString;
    char * value;
    Property * tempP;
    Parameter * tempParameter;
    tempP = (Property *)malloc(sizeof(Property));
    tempP->name = (char *)malloc(sizeof(char));
    tempP->group = (char *)malloc(sizeof(char));
    tempP->group[0] = '\0';  // Initialize the first character to null terminator
    tempP->values = initializeList(&valueToString, &deleteValue, &compareValues);
    tempP->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    int numOfSemiColons = 0;
    char anotherString[1000];
    int startIndex = 0;
    int counter = 0;
    char * valueString;
    char * groupString;
    char * nameString;
    char * parameterString;
    int semiColonsNeeded = 0;
    int equalFound = 0;
    int sizeOfName = 0;
    int sizeOfValue = 0;
    bool lineUnfolded = false;

    // Check if colon is present within the string
    if (strpbrk(stringFromFile, ":") != NULL && checkForCRLF(stringFromFile) == 1) {

        // Store the length of the tokenString in tempLength
        strcpy(anotherString, stringFromFile); // Make another copy of the string

        // Store the first token, which will be the property name
        tokenString = strtok(anotherString, ":");
        valueString = strtok(NULL, "\r\n");

        groupString = NULL;
        if (strpbrk(tokenString, ".") != NULL) {
            groupString = strtok(tokenString, ".");
            tokenString = strtok(NULL, "\0");
        }

        nameString = NULL;
        parameterString = NULL;
        if (strpbrk(tokenString, ";") != NULL) {
            nameString = strtok(tokenString, ";");
            parameterString = strtok(NULL, "\0");
        }
        else {
            nameString = tokenString;
        }
            
        // Check if we have found N property
        if ((strlen(nameString) == 1 && strCaseIgnoreCmp(nameString, "N") == 0) 
            || (strlen(nameString) == 3 && strCaseIgnoreCmp(nameString, "ADR") == 0)) {
            // Handle name
            if (nameString != NULL) {
                tempP->name = (char *)realloc(tempP->name, sizeof(char) * (strlen(nameString) + 1));
                strcpy(tempP->name,nameString);
            }
            else {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_PROP;
            }

            // Handle group
            if (groupString != NULL) {
                tempP->group = (char *)realloc(tempP->group, sizeof(char) * (strlen(groupString) + 1));
                strcpy(tempP->group, groupString);
            }

            // Handle parameters
            if (parameterString != NULL) {
                tokenString = parameterString;
                while ((tokenString = strtok(tokenString, ";")) != NULL) {
                    counter = 0;
                    sizeOfName = 0;
                    sizeOfValue = 0;
                    equalFound = 0;

                    // Calculate the size of the name and value parts
                    for (int i = 0; i < strlen(tokenString); i++) {
                        if (tokenString[i] == '=') {
                            equalFound = 1;
                            break;
                        }
                        sizeOfName++;
                    }
                    
                    if (equalFound == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_PROP;  // Invalid file, no '=' in the parameter
                    }

                    sizeOfValue = strlen(tokenString) - sizeOfName - 1; 
                    
                    // Make sure that both name and value sizes are valid
                    if (sizeOfName == 0 || sizeOfValue == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_PROP;  // Invalid file, either name or value is missing
                    }

                    char nameStr [sizeOfName + 1];
                    char valueStr [sizeOfValue + 1];

                    for (int i = 0; i < sizeOfName; i++) {
                        nameStr[i] = tokenString[i];
                    }
                    nameStr[sizeOfName] = '\0';

                    for (int i = sizeOfName + 1, j = 0; i < strlen(tokenString); i++, j++) {
                        valueStr[j] = tokenString[i];
                    }
                    valueStr[sizeOfValue] = '\0';

                    tempParameter = (Parameter *)malloc(sizeof(Parameter));

                    tempParameter->name = (char *)malloc(sizeof(char) * (strlen(nameStr) + 1));
                    tempParameter->value = (char *)malloc(sizeof(char) * (strlen(valueStr) + 1));

                    strcpy(tempParameter->name, nameStr);
                    strcpy(tempParameter->value, valueStr);

                    insertBack(tempP->parameters, tempParameter);

                    tokenString = NULL;
                }
            }

            if (valueString == NULL) {
                // Declare and initialize some temp variables
                char lineFoldingString [1000];
                char * cleanFStr;
                char anotherLFS [1000] = "";

                while (fgets(lineFoldingString, sizeof(lineFoldingString), independentFile) != NULL) {
                    if (strCaseIgnoreCmp(lineFoldingString, "END:VCARD\r\n") == 0) {
                        *endFound = 1;
                        break;
                    }
                    else {
                        if (lineFoldingString > 0 && (lineFoldingString[0] == ' ' || lineFoldingString[0] == '\t')) {
                            cleanFStr = strtok(lineFoldingString, "\r\n");
                            cleanFStr++;

                            if (cleanFStr != NULL) {
                                strcat(anotherLFS, cleanFStr);
                            }
                        }
                        else {
                            break;
                        }
                    }
                }

                if (strlen(anotherLFS) > 0) {
                    valueString = anotherLFS;
                    lineUnfolded = true;
                }
            }

            if (valueString != NULL) {
                // Declare and initialize some temp variables
                char lineFoldingString [1000];
                char * cleanFStr;

                if (lineUnfolded == false) {
                    while (fgets(lineFoldingString, sizeof(lineFoldingString), independentFile) != NULL) {
                        if (strCaseIgnoreCmp(lineFoldingString, "END:VCARD\r\n") == 0) {
                            *endFound = 1;
                            break;
                        }
                        else {
                            if (lineFoldingString > 0 && (lineFoldingString[0] == ' ' || lineFoldingString[0] == '\t')) {
                                cleanFStr = strtok(lineFoldingString, "\r\n");
                                cleanFStr++;
    
                                if (cleanFStr != NULL) {
                                    strcat(valueString, cleanFStr);
                                }
                            }
                            else {
                                break;
                            }
                        }
                    }
                }

                // Ensure there are 5 values (or 4 semicolons present in the string)
                numOfSemiColons = countCharOccurrences(valueString, ';');

                // Set semiColonsNeeded to 5 or 7 based on if name of property is N or ADR
                if (strCaseIgnoreCmp(nameString, "ADR") == 0) {
                    semiColonsNeeded = 6;
                }
                else {
                    semiColonsNeeded = 4;
                }

                if (numOfSemiColons == semiColonsNeeded && valueString != NULL) {
                    for (int i = 0; i < strlen(valueString); i++) {
                        if (valueString[i] == ';') {
                            for (int j = startIndex; j < i; j++) {
                                counter++;
                            }
                            char tempValue [counter + 1];
                            for (int j = startIndex, k = 0; j < i; j++, k++) {
                                tempValue[k] = valueString[j];
                            }
                            tempValue[counter] = '\0';
                            value = (char *)malloc(sizeof(char) * (counter + 1));
                            strcpy(value, tempValue);
                            insertBack(tempP->values, value);
                            counter = 0;
                            startIndex = i + 1;
                        }
                    }
                    if (startIndex < strlen(valueString)) {
                        counter = 0;
                        for (int j = startIndex; j < strlen(valueString); j++) {
                            counter++;
                        }
                        char tempValue [counter + 1];
                        for (int j = startIndex, k = 0; j < strlen(valueString); j++, k++) {
                            tempValue[k] = valueString[j];
                        }
                        tempValue[counter] = '\0';
                        value = (char *)malloc(sizeof(char) * (counter + 1));
                        strcpy(value, tempValue);
                        insertBack(tempP->values, value);
                    }
                    insertBack((*obj)->optionalProperties, tempP);
                }
                else {
                    deleteProperty(tempP);
                    deleteCard(*obj);
                    fclose(independentFile);
                    return INV_PROP;
                }
            }
            else {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_PROP;
            }
        }
        else if ((strlen(nameString) == 4 && strCaseIgnoreCmp(nameString, "BDAY") == 0) && (*obj)->birthday == NULL) {
            // Allocate space for birthday and it's related strings
            (*obj)->birthday = (DateTime *)malloc(sizeof(DateTime));
            (*obj)->birthday->date = (char *)malloc(sizeof(char));
            (*obj)->birthday->date[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->birthday->time = (char *)malloc(sizeof(char));
            (*obj)->birthday->time[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->birthday->text = (char *)malloc(sizeof(char));
            (*obj)->birthday->text[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->birthday->isText = false; // Initialize to false

            // Handle UTC
            if (valueString == NULL) {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_DT;
            }
            else {
                // Check if we have a Z in our valueString
                if (strlen(valueString) > 0 && valueString[strlen(valueString)-1] == 'Z') {
                    (*obj)->birthday->UTC = true; // Set the UTC to true
                }
                else {
                    (*obj)->birthday->UTC = false; // Set the UTC to false
                }
            }

            // Handle parameters
            if (parameterString != NULL) {
                tokenString = parameterString;
                while ((tokenString = strtok(tokenString, ";")) != NULL) {
                    counter = 0;
                    sizeOfName = 0;
                    sizeOfValue = 0;
                    equalFound = 0;

                    // Calculate the size of the name and value parts
                    for (int i = 0; i < strlen(tokenString); i++) {
                        if (tokenString[i] == '=') {
                            equalFound = 1;
                            break;
                        }
                        sizeOfName++;
                    }
                    
                    if (equalFound == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_DT;  // Invalid file, no '=' in the parameter
                    }

                    sizeOfValue = strlen(tokenString) - sizeOfName - 1; 
                    
                    // Make sure that both name and value sizes are valid
                    if (sizeOfName == 0 || sizeOfValue == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_DT;  // Invalid file, either name or value is missing
                    }

                    char nameStr [sizeOfName + 1];
                    char valueStr [sizeOfValue + 1];

                    for (int i = 0; i < sizeOfName; i++) {
                        nameStr[i] = tokenString[i];
                    }
                    nameStr[sizeOfName] = '\0';

                    for (int i = sizeOfName + 1, j = 0; i < strlen(tokenString); i++, j++) {
                        valueStr[j] = tokenString[i];
                    }
                    valueStr[sizeOfValue] = '\0';

                    if (strCaseIgnoreCmp(nameStr, "VALUE") == 0 && strCaseIgnoreCmp(valueStr, "text") == 0) {
                        (*obj)->birthday->isText = true;
                    }

                    tokenString = NULL;
                }
            }

            // Handle our valueString and insert value into the correct string (date, time, text)
            if ((*obj)->birthday->isText == true) {
                (*obj)->birthday->text = (char *)realloc((*obj)->birthday->text, sizeof(char) * (strlen(valueString) + 1));
                strcpy((*obj)->birthday->text, valueString);
            }
            else {
                // Handle time alone
                if (valueString > 0 && valueString[0] == 'T') {
                    char * time = valueString;
                    time++;
                    time = strtok(time, "Z");

                    (*obj)->birthday->time = (char *)realloc((*obj)->birthday->time, sizeof(char) * (strlen(time) + 1));
                    strcpy((*obj)->birthday->time, time);
                }
                // Handle date-time
                else if (valueString > 0 && strpbrk(valueString, "T") != NULL && valueString[0] != 'T') {
                    char * date = strtok(valueString, "T");
                    char * time = strtok(NULL, "Z");

                    (*obj)->birthday->date = (char *)realloc((*obj)->birthday->date, sizeof(char) * (strlen(date) + 1));
                    strcpy((*obj)->birthday->date, date);
                    (*obj)->birthday->time = (char *)realloc((*obj)->birthday->time, sizeof(char) * (strlen(time) + 1));
                    strcpy((*obj)->birthday->time, time);
                }
                // Handle date alone
                else {
                    (*obj)->birthday->date = (char *)realloc((*obj)->birthday->date, sizeof(char) * (strlen(valueString) + 1));
                    strcpy((*obj)->birthday->date, valueString);
                }
            }

            deleteProperty(tempP);
            fclose(independentFile);
            return OK;
        }
        else if ((strlen(nameString) == 11 && strCaseIgnoreCmp(nameString, "ANNIVERSARY") == 0) && (*obj)->anniversary == NULL) {
            // Allocate space for anniversary and it's related strings
            (*obj)->anniversary = (DateTime *)malloc(sizeof(DateTime));
            (*obj)->anniversary->date = (char *)malloc(sizeof(char));
            (*obj)->anniversary->date[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->anniversary->time = (char *)malloc(sizeof(char));
            (*obj)->anniversary->time[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->anniversary->text = (char *)malloc(sizeof(char));
            (*obj)->anniversary->text[0] = '\0';  // Initialize the first character to null terminator
            (*obj)->anniversary->isText = false; // Initialize to false

            // Handle UTC
            if (valueString == NULL) {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_DT;
            }
            else {
                // Check if we have a Z in our valueString
                if (strlen(valueString) > 0 && valueString[strlen(valueString)-1] == 'Z') {
                    (*obj)->anniversary->UTC = true; // Set the UTC to true
                }
                else {
                    (*obj)->anniversary->UTC = false; // Set the UTC to false
                }
            }

            // Handle parameters
            if (parameterString != NULL) {
                tokenString = parameterString;
                while ((tokenString = strtok(tokenString, ";")) != NULL) {
                    counter = 0;
                    sizeOfName = 0;
                    sizeOfValue = 0;
                    equalFound = 0;

                    // Calculate the size of the name and value parts
                    for (int i = 0; i < strlen(tokenString); i++) {
                        if (tokenString[i] == '=') {
                            equalFound = 1;
                            break;
                        }
                        sizeOfName++;
                    }
                    
                    if (equalFound == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_DT;  // Invalid file, no '=' in the parameter
                    }

                    sizeOfValue = strlen(tokenString) - sizeOfName - 1; 
                    
                    // Make sure that both name and value sizes are valid
                    if (sizeOfName == 0 || sizeOfValue == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_DT;  // Invalid file, either name or value is missing
                    }

                    char nameStr [sizeOfName + 1];
                    char valueStr [sizeOfValue + 1];

                    for (int i = 0; i < sizeOfName; i++) {
                        nameStr[i] = tokenString[i];
                    }
                    nameStr[sizeOfName] = '\0';

                    for (int i = sizeOfName + 1, j = 0; i < strlen(tokenString); i++, j++) {
                        valueStr[j] = tokenString[i];
                    }
                    valueStr[sizeOfValue] = '\0';

                    if (strCaseIgnoreCmp(nameStr, "VALUE") == 0 && strCaseIgnoreCmp(valueStr, "text") == 0) {
                        (*obj)->anniversary->isText = true;
                    }

                    tokenString = NULL;
                }
            }

            // Handle our valueString and insert value into the correct string (date, time, text)
            if ((*obj)->anniversary->isText == true) {
                (*obj)->anniversary->text = (char *)realloc((*obj)->anniversary->text, sizeof(char) * (strlen(valueString) + 1));
                strcpy((*obj)->anniversary->text, valueString);
            }
            else {
                // Handle time alone
                if (valueString > 0 && valueString[0] == 'T') {
                    char * time = valueString;
                    time++;
                    time = strtok(time, "Z");

                    (*obj)->anniversary->time = (char *)realloc((*obj)->anniversary->time, sizeof(char) * (strlen(time) + 1));
                    strcpy((*obj)->anniversary->time, time);
                }
                // Handle date-time
                else if (valueString > 0 && strpbrk(valueString, "T") != NULL && valueString[0] != 'T') {
                    char * date = strtok(valueString, "T");
                    char * time = strtok(NULL, "Z");

                    (*obj)->anniversary->date = (char *)realloc((*obj)->anniversary->date, sizeof(char) * (strlen(date) + 1));
                    strcpy((*obj)->anniversary->date, date);
                    (*obj)->anniversary->time = (char *)realloc((*obj)->anniversary->time, sizeof(char) * (strlen(time) + 1));
                    strcpy((*obj)->anniversary->time, time);
                }
                // Handle date alone
                else {
                    (*obj)->anniversary->date = (char *)realloc((*obj)->anniversary->date, sizeof(char) * (strlen(valueString) + 1));
                    strcpy((*obj)->anniversary->date, valueString);
                }
            }

            deleteProperty(tempP);
            fclose(independentFile);
            return OK;
        }
        else {
            // Handle name
            if (nameString != NULL) {
                tempP->name = (char *)realloc(tempP->name, sizeof(char) * (strlen(nameString) + 1));
                strcpy(tempP->name,nameString);
            }
            else {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_PROP;
            }

            // Handle group
            if (groupString != NULL) {
                tempP->group = (char *)realloc(tempP->group, sizeof(char) * (strlen(groupString) + 1));
                strcpy(tempP->group,groupString);
            }

            // Handle parameters
            if (parameterString != NULL) {
                tokenString = parameterString;
                while ((tokenString = strtok(tokenString, ";")) != NULL) {
                    counter = 0;
                    sizeOfName = 0;
                    sizeOfValue = 0;
                    equalFound = 0;

                    // Calculate the size of the name and value parts
                    for (int i = 0; i < strlen(tokenString); i++) {
                        if (tokenString[i] == '=') {
                            equalFound = 1;
                            break;
                        }
                        sizeOfName++;
                    }
                    
                    if (equalFound == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_PROP;  // Invalid file, no '=' in the parameter
                    }

                    sizeOfValue = strlen(tokenString) - sizeOfName - 1; 
                    
                    // Make sure that both name and value sizes are valid
                    if (sizeOfName == 0 || sizeOfValue == 0) {
                        deleteProperty(tempP);
                        deleteCard(*obj);
                        fclose(independentFile);
                        return INV_PROP;  // Invalid file, either name or value is missing
                    }

                    char nameStr [sizeOfName + 1];
                    char valueStr [sizeOfValue + 1];

                    for (int i = 0; i < sizeOfName; i++) {
                        nameStr[i] = tokenString[i];
                    }
                    nameStr[sizeOfName] = '\0';

                    for (int i = sizeOfName + 1, j = 0; i < strlen(tokenString); i++, j++) {
                        valueStr[j] = tokenString[i];
                    }
                    valueStr[sizeOfValue] = '\0';

                    tempParameter = (Parameter *)malloc(sizeof(Parameter));

                    tempParameter->name = (char *)malloc(sizeof(char) * (strlen(nameStr) + 1));
                    tempParameter->value = (char *)malloc(sizeof(char) * (strlen(valueStr) + 1));

                    strcpy(tempParameter->name, nameStr);
                    strcpy(tempParameter->value, valueStr);

                    insertBack(tempP->parameters, tempParameter);

                    tokenString = NULL;
                }
            }

            if (valueString == NULL) {
                // Declare and initialize some temp variables
                char lineFoldingString [1000];
                char * cleanFStr;
                char anotherLFS [1000] = "";

                while (fgets(lineFoldingString, sizeof(lineFoldingString), independentFile) != NULL) {
                    if (strCaseIgnoreCmp(lineFoldingString, "END:VCARD\r\n") == 0) {
                        *endFound = 1;
                        break;
                    }
                    else {
                        if (lineFoldingString > 0 && (lineFoldingString[0] == ' ' || lineFoldingString[0] == '\t')) {
                            cleanFStr = strtok(lineFoldingString, "\r\n");
                            cleanFStr++;

                            if (cleanFStr != NULL) {
                                strcat(anotherLFS, cleanFStr);
                            }
                        }
                        else {
                            break;
                        }
                    }
                }

                if (strlen(anotherLFS) > 0) {
                    valueString = anotherLFS;
                    lineUnfolded = true;
                }
            }

            // Handle values
            if (valueString != NULL) {
                // Declare and initialize some temp variables
                char lineFoldingString [1000];
                char * cleanFStr;

                if (lineUnfolded == false) {
                    while (fgets(lineFoldingString, sizeof(lineFoldingString), independentFile) != NULL) {
                        if (strCaseIgnoreCmp(lineFoldingString, "END:VCARD\r\n") == 0) {
                            *endFound = 1;
                            break;
                        }
                        else {
                            if (lineFoldingString > 0 && (lineFoldingString[0] == ' ' || lineFoldingString[0] == '\t')) {
                                cleanFStr = strtok(lineFoldingString, "\r\n");
                                cleanFStr++;
    
                                if (cleanFStr != NULL) {
                                    strcat(valueString, cleanFStr);
                                }
                            }
                            else {
                                break;
                            }
                        }
                    }
                }

                for (int i = 0; i < strlen(valueString); i++) {
                    if (valueString[i] == ';') {
                        for (int j = startIndex; j < i; j++) {
                            counter++;
                        }
                        char tempValue [counter + 1];
                        for (int j = startIndex, k = 0; j < i; j++, k++) {
                            tempValue[k] = valueString[j];
                        }
                        tempValue[counter] = '\0';
                        value = (char *)malloc(sizeof(char) * (counter + 1));
                        strcpy(value, tempValue);
                        insertBack(tempP->values, value);
                        counter = 0;
                        startIndex = i + 1;
                    }
                }
                if (startIndex < strlen(valueString)) {
                    counter = 0;
                    for (int j = startIndex; j < strlen(valueString); j++) {
                        counter++;
                    }
                    char tempValue [counter + 1];
                    for (int j = startIndex, k = 0; j < strlen(valueString); j++, k++) {
                        tempValue[k] = valueString[j];
                    }
                    tempValue[counter] = '\0';
                    value = (char *)malloc(sizeof(char) * (counter + 1));
                    strcpy(value, tempValue);
                    insertBack(tempP->values, value);
                }
                insertBack((*obj)->optionalProperties, tempP);
            }
            else {
                deleteProperty(tempP);
                deleteCard(*obj);
                fclose(independentFile);
                return INV_PROP;
            }
        }
    }   
    else {
        deleteProperty(tempP);
        deleteCard(*obj);
        fclose(independentFile);
        return INV_PROP;
    }

    fclose(independentFile);

    return OK;
}

// A function to count the occurrences of a character in a string
int countCharOccurrences(char * string, char character) {
    // Check if string is NULL
    if (string == NULL) {
        return 0;
    }
    
    // Declare and initialize all needed variables
    int numCount = 0;
    int lengthOfString = strlen(string);

    // Count the occurrences of the character in the string
    for (int i = 0; i < lengthOfString; i++) {
        if (string[i] == character) {
            numCount++;
        }
    }

    return numCount; // Return the count
}

VCardErrorCode validateProperty(const Property * prop) {
    // Make sure property is not NULL
    if (prop == NULL) {
        return INV_CARD; // Replace by actual return value
    }
    
    // Check to make sure that name is not empty or NULL
    if (prop->name == NULL || strcmp(prop->name, "") == 0) {
        return INV_CARD; // Replace by actual return value
    }

    // Check to make sure that group is not null
    if (prop->group == NULL) {
        return INV_CARD;
    }

    // Check to make sure that parameter list is not null
    if (prop->parameters == NULL) {
        return INV_CARD;
    }

    // Check to make sure that values list is not null and not empty
    if (prop->values == NULL && prop->values->length == 0) {
        return INV_CARD;
    }

    return OK;
}

VCardErrorCode validatePropertyName (const Property * prop) {
    // 6.1.1, BEGIN
    if (strCaseIgnoreCmp(prop->name, "BEGIN") == 0) {
        return OK;
    }

    // 6.1.2, END
    if (strCaseIgnoreCmp(prop->name, "END") == 0) {
        return OK;
    }
    
    // 6.1.3, SOURCE
    if (strCaseIgnoreCmp(prop->name, "SOURCE") == 0) {
        return OK;
    }

    // 6.1.4, KIND
    if (strCaseIgnoreCmp(prop->name, "KIND") == 0) {
        return OK;
    }

    // 6.1.5, XML
    if (strCaseIgnoreCmp(prop->name, "XML") == 0) {
        return OK;
    }

    // 6.2.1, FN
    if (strCaseIgnoreCmp(prop->name, "FN") == 0) {
        return OK;
    }

    // 6.2.2, N
    if (strCaseIgnoreCmp(prop->name, "N") == 0) {
        return OK;
    }

    // 6.2.3, NICKNAME
    if (strCaseIgnoreCmp(prop->name, "NICKNAME") == 0) {
        return OK;
    }

    // 6.2.4, PHOTO
    if (strCaseIgnoreCmp(prop->name, "PHOTO") == 0) {
        return OK;
    }

    // 6.2.5, BDAY
    if (strCaseIgnoreCmp(prop->name, "BDAY") == 0) {
        return OK;
    }

    // 6.2.6, ANNIVERSARY
    if (strCaseIgnoreCmp(prop->name, "ANNIVERSARY") == 0) {
        return OK;
    }

    // 6.2.7, GENDER
    if (strCaseIgnoreCmp(prop->name, "GENDER") == 0) {
        return OK;
    }

    // 6.3.1, ADR
    if (strCaseIgnoreCmp(prop->name, "ADR") == 0) {
        return OK;
    }

    // 6.4.1, TEL
    if (strCaseIgnoreCmp(prop->name, "TEL") == 0) {
        return OK;
    }

    // 6.4.2, EMAIL
    if (strCaseIgnoreCmp(prop->name, "EMAIL") == 0) {
        return OK;
    }

    // 6.4.3, IMPP
    if (strCaseIgnoreCmp(prop->name, "IMPP") == 0) {
        return OK;
    }

    // 6.4.4, LANG
    if (strCaseIgnoreCmp(prop->name, "LANG") == 0) {
        return OK;
    }

    // 6.5.1, TZ
    if (strCaseIgnoreCmp(prop->name, "TZ") == 0) {
        return OK;
    }

    // 6.5.2, GEO
    if (strCaseIgnoreCmp(prop->name, "GEO") == 0) {
        return OK;
    }

    // 6.6.1, TITLE
    if (strCaseIgnoreCmp(prop->name, "TITLE") == 0) {
        return OK;
    }

    // 6.6.2, ROLE
    if (strCaseIgnoreCmp(prop->name, "ROLE") == 0) {
        return OK;
    }

    // 6.6.3, LOGO
    if (strCaseIgnoreCmp(prop->name, "LOGO") == 0) {
        return OK;
    }

    // 6.6.4, ORG
    if (strCaseIgnoreCmp(prop->name, "ORG") == 0) {
        return OK;
    }

    // 6.6.5, MEMBER
    if (strCaseIgnoreCmp(prop->name, "MEMBER") == 0) {
        return OK;
    }

    // 6.6.6, RELATED
    if (strCaseIgnoreCmp(prop->name, "RELATED") == 0) {
        return OK;
    }

    // 6.7.1, CATEGORIES
    if (strCaseIgnoreCmp(prop->name, "CATEGORIES") == 0) {
        return OK;
    }

    // 6.7.2, NOTE
    if (strCaseIgnoreCmp(prop->name, "NOTE") == 0) {
        return OK;
    }

    // 6.7.3, PRODID
    if (strCaseIgnoreCmp(prop->name, "PRODID") == 0) {
        return OK;
    }

    // 6.7.4, REV
    if (strCaseIgnoreCmp(prop->name, "REV") == 0) {
        return OK;
    }

    // 6.7.5, SOUND
    if (strCaseIgnoreCmp(prop->name, "SOUND") == 0) {
        return OK;
    }

    // 6.7.6, UID
    if (strCaseIgnoreCmp(prop->name, "UID") == 0) {
        return OK;
    }

    // 6.7.7, CLIENTPIDMAP
    if (strCaseIgnoreCmp(prop->name, "CLIENTPIDMAP") == 0) {
        return OK;
    }

    // 6.7.8, URL
    if (strCaseIgnoreCmp(prop->name, "URL") == 0) {
        return OK;
    }

    // 6.7.9, VERSION
    if (strCaseIgnoreCmp(prop->name, "VERSION") == 0) {
        return OK;
    }

    // 6.8.1, KEY
    if (strCaseIgnoreCmp(prop->name, "KEY") == 0) {
        return OK;
    }

    // 6.9.1, FBURL
    if (strCaseIgnoreCmp(prop->name, "FBURL") == 0) {
        return OK;
    }

    // 6.9.2, CALADRURI
    if (strCaseIgnoreCmp(prop->name, "CALADRURI") == 0) {
        return OK;
    }

    // 6.9.3, CALURI
    if (strCaseIgnoreCmp(prop->name, "CALURI") == 0) {
        return OK;
    }
    
    return INV_PROP;
}

VCardErrorCode validatePropertyValues (const List * values) {
    // Declare and initialize all needed variables
    Node * currentNode = values->head;

    // Continue to loop while there is another entry in the list
    while (currentNode != NULL) {
        if (currentNode->data == NULL) {
            return INV_PROP;
        }

        currentNode = currentNode->next;
    }

    return OK; // No issues
}

VCardErrorCode validateParameter (const Property * prop) {
    // Declare and initialize all needed variables
    ListIterator tempIterator = createIterator(prop->parameters);
    void * elem;
    Parameter * tempPara;

    // Continue to loop while there is another entry in the list
    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempPara = (Parameter *)elem; // Case to Parameter *

        // If name or value is NULL, return INV_PROP
        if (tempPara->name == NULL || tempPara->value == NULL) {
            return INV_PROP;
        }

        // If name or value is empty string, then return INV_PROP
        if (strcmp(tempPara->name, "") == 0 || strcmp(tempPara->value, "") == 0) {
            return INV_PROP;
        }
    }

    return OK; // No issues
}