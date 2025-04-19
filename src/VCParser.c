/* 
* Name: Gurbaaz Singh Cheema
* Date: March 27 2025
*/

#include "VCParser.h"
#include "VCHelpers.h"

VCardErrorCode createCard(char* fileName, Card** obj) {
    // Check if fileName is NULL
    if (fileName == NULL) {
        return INV_FILE;
    }

    // Check if fileName given has .vcf or .vcard extension
    if (strcmp(fileName + strlen(fileName) - 4, ".vcf") != 0 && strcmp(fileName + strlen(fileName) - 6, ".vcard") != 0) {
        return INV_FILE;
    }

    // Allocate and initialize needed variables
    FILE * fileToRead;
    char tempString [1000];
    char anotherString [1000];
    int counter = 0;
    int propertyFNFound = 0;
    int endFound = 0;
    *obj = (Card *)malloc(sizeof(Card));
    (*obj)->fn = (Property *)malloc(sizeof(Property));
    (*obj)->fn->name = (char *)malloc(sizeof(char));
    (*obj)->fn->group = (char *)malloc(sizeof(char));
    (*obj)->fn->group[0] = '\0';  // Initialize the first character to null terminator
    (*obj)->fn->values = initializeList(&valueToString, &deleteValue, &compareValues);
    (*obj)->fn->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    (*obj)->optionalProperties = initializeList(&propertyToString, &deleteProperty, &compareProperties);
    (*obj)->birthday = NULL;
    (*obj)->anniversary = NULL;
    char * tokenString;
    char * value;
    VCardErrorCode returnCode;
    char * valueString;
    char * groupString;
    char * nameString;
    char * parameterString;
    int equalFound = 0;
    int sizeOfName = 0;
    int sizeOfValue = 0;
    Parameter * tempParameter;

    // Open the file in read mode
    fileToRead = fopen(fileName, "r");

    // Check if fileToRead is NULL
    if (fileToRead == NULL) {
        deleteCard(*obj);
        return INV_FILE;
    }

    // Continue to loop while fgets doesn't return NULL
    while (fgets(tempString, sizeof(tempString), fileToRead) != NULL) {
        // Check if the we are reading the first line of the file and if that line is BEGIN:VCARD
        if (counter == 0) {
            // Check if tempString is BEGIN:VCARD (case insensitive)
            if (strCaseIgnoreCmp(tempString, "BEGIN:VCARD\r\n") != 0) {
                deleteCard(*obj);
                fclose(fileToRead);
                return INV_CARD;
            }
        }
        else if (counter == 1) {
            // Check if tempString is BEGIN:VCARD (case insensitive)
            if (strCaseIgnoreCmp(tempString, "VERSION:4.0\r\n") != 0) {
                deleteCard(*obj);
                fclose(fileToRead);
                return INV_CARD;
            }
        }
        // Read proporties and stuff, if we read FN, set FN found to 1 and if we read END:VCARD set end to 1, else both will be false
        // Check for END:VCARD
        else if (strCaseIgnoreCmp(tempString, "END:VCARD\r\n") == 0) {
            endFound = 1;
            break;
        }
        else {
            strcpy(anotherString, tempString); // Make another copy of the string

            if (tempString > 0 && (tempString[0] == ' ' || tempString[0] == '\t')) {
                continue;
            }
            else if (strpbrk(anotherString, ":") != NULL && checkForCRLF(anotherString) == 1) {
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

                // Check if we have found FN property and if we encounter it the first time
                if (strlen(nameString) == 2 && strCaseIgnoreCmp(nameString, "FN") == 0 && propertyFNFound != 1) {
                    if (nameString != NULL) {
                        (*obj)->fn->name = (char *)realloc((*obj)->fn->name, sizeof(char) * (strlen(nameString) + 1));
                        strcpy((*obj)->fn->name,nameString);
                    }
                    else {
                        deleteCard(*obj);
                        fclose(fileToRead);
                        return INV_PROP;
                    }

                    // Handle group
                    if (groupString != NULL) {
                        (*obj)->fn->group = (char *)realloc((*obj)->fn->group, sizeof(char) * (strlen(groupString) + 1));
                        strcpy((*obj)->fn->group, groupString);
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
                                deleteCard(*obj);
                                fclose(fileToRead);
                                return INV_PROP;  // Invalid file, no '=' in the parameter
                            }

                            sizeOfValue = strlen(tokenString) - sizeOfName - 1; 
                            
                            // Make sure that both name and value sizes are valid
                            if (sizeOfName == 0 || sizeOfValue == 0) {
                                deleteCard(*obj);
                                fclose(fileToRead);
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

                            insertBack((*obj)->fn->parameters, tempParameter);

                            tokenString = NULL;
                        }
                    }

                    if (valueString != NULL) {
                        value = (char *)malloc(sizeof(char) * (strlen(valueString) + 1));
                        strcpy(value, valueString);

                        // Declare and initialize some temp variables
                        char lineFoldingString [1000];
                        char * cleanFStr;

                        while (fgets(lineFoldingString, sizeof(lineFoldingString), fileToRead) != NULL) {
                            if (strCaseIgnoreCmp(lineFoldingString, "END:VCARD\r\n") == 0) {
                                endFound = 1;
                                break;
                            }
                            else {
                                if (lineFoldingString > 0 && (lineFoldingString[0] == ' ' || lineFoldingString[0] == '\t')) {
                                    cleanFStr = strtok(lineFoldingString, "\r\n");
                                    cleanFStr++;;

                                    if (cleanFStr != NULL) {
                                        value = (char *)realloc(value, sizeof(char) * (strlen(value) + strlen(cleanFStr) + 1));
                                        strcat(value, cleanFStr);
                                    }
                                }
                                else {
                                    // Call the optionalProperty function here to do the rest of the processing
                                    returnCode = optionalPropertiesHandler (lineFoldingString, obj, fileToRead, fileName, &endFound);
                                    if (returnCode != OK) {
                                        free(value);
                                        fclose(fileToRead);
                                        return returnCode;
                                    }
                                    break;
                                }
                            }
                        }
                        insertBack((*obj)->fn->values, value);
                    }
                    else {
                        deleteCard(*obj);
                        fclose(fileToRead);
                        return INV_PROP;
                    }
                    propertyFNFound = 1;
                }
                else {
                    // Call the optionalProperty function here to do the rest of the processing
                    returnCode = optionalPropertiesHandler (tempString, obj, fileToRead, fileName, &endFound);
                    if (returnCode != OK) {
                        fclose(fileToRead);
                        return returnCode;
                    }
                }
            }
            else {
                deleteCard(*obj);
                fclose(fileToRead);
                return INV_PROP;
            }
        }
        counter++; // Add one to counter
    }

    // Check if fn and end:vcard was found, else free the allocated stuff
    if (propertyFNFound != 1 || endFound != 1) {
        deleteCard(*obj);
        fclose(fileToRead);
        return INV_CARD;
    }

    fclose(fileToRead); // Close the file

    return OK;
}

void deleteCard(Card* obj) {
    // Check if given card is NULL
    if (obj == NULL) {
        return;
    }

    // A valid card will always a have a property fn, so we will always delete
    deleteProperty(obj->fn);

    // Next we free the optional parameters
    freeList(obj->optionalProperties);

    // Check if birthday and anniversary need to be deleted
    if (obj->birthday != NULL) {
        deleteDate(obj->birthday);
    }
    
    if (obj->anniversary != NULL) {
        deleteDate(obj->anniversary);
    }

    free(obj); // Free the card itself
}

char* cardToString(const Card* obj) {
    // Declare and initialize all needed variables
    char * returnStr;
    char msg [] = "CardToString Return String";

    // Create string based on the msg to return
    returnStr = (char *)malloc(sizeof(char) * (strlen(msg) + 1));
    strcpy(returnStr, msg);
    
    return returnStr; // Return the string
}

char* errorToString(VCardErrorCode err) {
    // Declare and initialize all needed variables
    char * returnStr;
    char * msg;

    // Create a msg according to the error code
    if (err == OK) {
        msg = "No Errors!";
    }
    else if (err == INV_FILE) {
        msg = "Invalid File!";
    }
    else if (err == INV_CARD) {
        msg = "Invalid VCard!";
    }
    else if (err == INV_PROP) {
        msg = "Invalid Property";
    }
    else if (err == INV_DT) {
        msg = "Invalid DateTime";
    }
    else if (err == WRITE_ERROR) {
        msg = "Write Error!";
    }
    else {
        msg = "Unknown Error!";
    }

    // Create string based on the msg to return
    returnStr = (char *)malloc(sizeof(char) * (strlen(msg) + 1));
    strcpy(returnStr, msg);

    return returnStr; // Return the string
}

void deleteProperty(void* toBeDeleted) {
    // Declare and initialize all needed variables
    Property * toDelete;

    // Check if toBeDeleted is NULL, if so return
    if (toBeDeleted == NULL) {
        return;
    }

    // Cast to Property *
    toDelete = (Property *)toBeDeleted;

    // Free the allocated memory
    free(toDelete->name);
    free(toDelete->group);
    freeList(toDelete->values);
    freeList(toDelete->parameters);
    free(toDelete);
}

int compareProperties(const void* first,const void* second) {
    return 0; // Will implement later, for now return 0
}

char* propertyToString(void* prop) {
    // Declare and initialize all needed variables
    char * returnStr;
    char * tempName;
    char * tempGroup;
    char tempString [1000] = "";
    int listLen = 0;
    int counter = 0;
    Property * temp;

    // Check if prop is null, if so return a string with 'NULL'
    if (prop == NULL) {
        tempName = "NULL";
        returnStr = (char *)malloc(sizeof(char) * (strlen(tempName) + 1));
        strcpy(returnStr, tempName);
        return returnStr;
    }

    // Cast prop to Property *
    temp = (Property *)prop;

    // Call valueToString to convert the name and group components of prop to strings
    tempName = valueToString(temp->name);
    tempGroup = valueToString(temp->group);

    // Cat the group and name together
    if (strcmp(tempGroup, "") != 0) {
        strcat(tempString, tempGroup);
        strcat(tempString, ".");
    } 
    strcat(tempString, tempName);

    // Store the parameterArrLen
    listLen = temp->parameters->length;

    // Check if we have any parameters
    if (listLen > 0) {
        // Add the ;
        strcat(tempString, ";");

        // Traverse the parameter list, adding the parameters to the tempString separated by ;
        ListIterator tempIterator = createIterator(temp->parameters);

        void * elem;

        while ((elem = nextElement(&tempIterator)) != NULL) {
            char * currElement = temp->parameters->printData(elem);
            strcat(tempString, currElement);

            if (counter != listLen - 1) {
                strcat(tempString, ";");
            }

            counter++;
            
            free(currElement);
        }
    }

    // Store the value list len and reset counter
    listLen = temp->values->length;
    counter = 0;

    // Add the ;
    strcat(tempString, ":");

    // Check if we have any values
    if (listLen > 0) {
        // Traverse the values list, adding the values to the tempString separated by ;
        ListIterator tempIterator = createIterator(temp->values);

        void * elem;

        while ((elem = nextElement(&tempIterator)) != NULL) {
            char * currElement = temp->values->printData(elem);
            strcat(tempString, currElement);

            if (counter != listLen - 1) {
                strcat(tempString, ";");
            }
            else {
                strcat(tempString, "\r\n");
            }

            counter++;
            
            free(currElement);
        }
    }

    // Allocate space for returnStr based on tempString
    returnStr = (char *)malloc(sizeof(char) * (strlen(tempString) + 1));

    // Copy the tempString into returnStr
    strcpy(returnStr, tempString);

    // Free tempName and tempGroup
    free(tempName);
    free(tempGroup);
    
    return returnStr; // Return the string
}

void deleteParameter(void* toBeDeleted) {
    // Declare and initialize all needed variables
    Parameter * toDelete;

    // Check if toBeDeleted is null, if so return
    if (toBeDeleted == NULL) {
        return;
    }

    // Cast toDelete to Parameter *
    toDelete = (Parameter *)toBeDeleted;

    // Free the allocated memory
    free(toDelete->name);
    free(toDelete->value);
    free(toDelete);
}

int compareParameters(const void* first,const void* second) {
    return 0; // Return 0 for now
}

char* parameterToString(void* param) {
    // Declare and initialize all needed variables
    char * returnStr;
    char * tempName;
    char * tempValue;
    Parameter * temp;

    // Check if param is null, if so return a string with 'NULL'
    if (param == NULL) {
        tempName = "NULL";
        returnStr = (char *)malloc(sizeof(char) * (strlen(tempName) + 1));
        strcpy(returnStr, tempName);
        return returnStr;
    }

    // Cast to Parameter
    temp = (Parameter *)param;

    // Call valueToString to convert the components of Parameter into strings
    tempName = valueToString(temp->name);
    tempValue = valueToString(temp->value);

    // Allocate space for returnStr
    returnStr = (char *)malloc(sizeof(char) * (strlen(tempName) + strlen(tempValue) + 2));

    // Build the string
    strcpy(returnStr, "");
    strcat(returnStr, tempName);
    strcat(returnStr, "=");
    strcat(returnStr, tempValue);

    // Free tempName and tempValue
    free(tempName);
    free(tempValue);
    
    return returnStr; // Return the string
}

void deleteValue(void* toBeDeleted) {
    // Declare and initialize all needed variables
    char * toDelete;

    // Make sure toBeDeleted is not NULL
    if (toBeDeleted == NULL) {
        return;
    }

    // Case to char *
    toDelete = (char *)toBeDeleted;

    free(toDelete); // Free the allocated memory
}

int compareValues(const void* first,const void* second) {
    return 0; // Return 0 for now
}

char* valueToString(void* val) {
    // Declare and initialize all needed variables
    char * returnStr;
    char * tempString;

    // Check if val is null, if so return a string with 'NULL'
    if (val == NULL) {
        tempString = "NULL";
        returnStr = (char *)malloc(sizeof(char) * (strlen(tempString) + 1));
        strcpy(returnStr, tempString);
        return returnStr;
    }

    // Cast val to char and store in tempString
    tempString = (char *)val;

    // Allocate space
    returnStr = (char *)malloc(sizeof(char) * (strlen(tempString) + 1));

    // Copy the tempString into returnStr
    strcpy(returnStr, tempString);
    
    return returnStr; // Return the string
}

void deleteDate(void* toBeDeleted) {
    // Declare and initialize all needed variables
    DateTime * toDelete;

    // Make sure toBeDeleted is not NULL
    if (toBeDeleted == NULL) {
        return;
    }

    // Cast to DateTime *
    toDelete = (DateTime *)toBeDeleted;

    // Free the allocated memory
    free(toDelete->date);
    free(toDelete->time);
    free(toDelete->text);
    free(toDelete);
}

int compareDates(const void* first,const void* second) {
    return 0; // Return 0 for now
}

char* dateToString(void* date) {
    // Declare and initialize all needed variables
    char * returnStr;
    char * dateTemp;
    char * timeTemp;
    DateTime * temp;

    // Check if date is null, if so return a string with 'NULL'
    if (date == NULL) {
        dateTemp = "NULL";
        returnStr = (char *)malloc(sizeof(char) * (strlen(dateTemp) + 1));
        strcpy(returnStr, dateTemp);
        return returnStr;
    }

    // Cast to date
    temp = (DateTime *)date;

    // Check if date is text
    if (temp->isText == true) {
        dateTemp = valueToString(temp->text); // Save the date

        // Allocate space for returnStr
        returnStr = (char *)malloc(sizeof(char) * (strlen(dateTemp) + 3));

        // Copy the date into returnStr and free date
        strcpy(returnStr, dateTemp);
        strcat(returnStr, "\r\n");
        free(dateTemp);
    }
    else {
        // Check if date only
        if (strcmp(temp->date, "") != 0 && strcmp(temp->time, "") == 0) {
            dateTemp = valueToString(temp->date); // Save the date

            // Allocate space for returnStr
            returnStr = (char *)malloc(sizeof(char) * (strlen(dateTemp) + 3));

            // Copy the date into returnStr and free date
            strcpy(returnStr, dateTemp);
            strcat(returnStr, "\r\n");
            free(dateTemp);
        }
        else if (strcmp(temp->date, "") == 0 && strcmp(temp->time, "") != 0) {
            timeTemp = valueToString(temp->time); // Save the time

            // Allocate space for returnStr
            if (temp->UTC == true) {
                returnStr = (char *)malloc(sizeof(char) * (strlen(timeTemp) + 5));

                // Copy the time into returnStr, adding Z and T
                strcpy(returnStr, "");
                strcat(returnStr, "T");
                strcat(returnStr, timeTemp);
                strcat(returnStr, "Z");
                strcat(returnStr, "\r\n");
            }
            else {
                returnStr = (char *)malloc(sizeof(char) * (strlen(timeTemp) + 4));

                // Copy the time into returnStr, adding T
                strcpy(returnStr, "");
                strcat(returnStr, "T");
                strcat(returnStr, timeTemp);
                strcat(returnStr, "\r\n");
            } 

            free(timeTemp); // Free time
        }
        else {
            dateTemp = valueToString(temp->date); // Save the date
            timeTemp = valueToString(temp->time); // Save the time

            // Allocate space for returnStr
            if (temp->UTC == true) {
                returnStr = (char *)malloc(sizeof(char) * (strlen(timeTemp) + strlen(dateTemp) + 5));

                // Copy the date and time into returnStr, adding Z and T
                strcpy(returnStr, "");
                strcat(returnStr, dateTemp);
                strcat(returnStr, "T");
                strcat(returnStr, timeTemp);
                strcat(returnStr, "Z");
                strcat(returnStr, "\r\n");
            }
            else {
                returnStr = (char *)malloc(sizeof(char) * (strlen(timeTemp) + strlen(dateTemp) + 4));

                // Copy the date and time into returnStr, adding T 
                strcpy(returnStr, "");
                strcat(returnStr, dateTemp);
                strcat(returnStr, "T");
                strcat(returnStr, timeTemp);
                strcat(returnStr, "\r\n");
            } 

            free(dateTemp); // Free date
            free(timeTemp); // Free time
        }
    }
    
    return returnStr; // Return the string
}

VCardErrorCode writeCard(const char* fileName, const Card* obj) {
    // Check if fileName is null
    if (fileName == NULL) {
        return WRITE_ERROR;
    }

    // Check if obj is null
    if (obj == NULL) {
        return WRITE_ERROR;
    }

    // Declare and initialize all needed variables
    FILE * fileToWrite;
    char tempString [1000];
    char * anniversary;
    char * bday;
    char * fnProperty;
    int listLen = 0;

    // Open the file in write mode
    fileToWrite = fopen(fileName, "w");

    // Check if the files was created or opened correctly
    if (fileToWrite == NULL) {
        return WRITE_ERROR;
    }

    // Write the BEGIN and VERSION lines into the file (always a part of a valid VCard)
    fprintf(fileToWrite, "BEGIN:VCARD\r\n");
    fprintf(fileToWrite, "VERSION:4.0\r\n");

    // Write the FN property into the file
    fnProperty = propertyToString(obj->fn);
    fprintf(fileToWrite, "%s", fnProperty); // Print the string into the file
    free(fnProperty); // Free tempString

    // Store the length of the optional properties length in listLen
    listLen = obj->optionalProperties->length;

    // Write the optional properties into the file
    if (listLen > 0) {
        // Traverse the parameter list, adding the values to the tempString separated by ;
        ListIterator tempIterator = createIterator(obj->optionalProperties);

        void * elem;

        while ((elem = nextElement(&tempIterator)) != NULL) {
            char * currElement = obj->optionalProperties->printData(elem);
            
            fprintf(fileToWrite, "%s", currElement); // Print the string into the file
            
            free(currElement);
        }
    }

    // Write the Birthday if its part of the given VCard
    if (obj->birthday != NULL) {
        strcpy(tempString, "");
        if (obj->birthday->isText == true) {
            strcat(tempString, "BDAY;VALUE=text:");
        }
        else {
            strcat(tempString, "BDAY:");
        }
        bday = dateToString(obj->birthday);
        strcat(tempString, bday);
        fprintf(fileToWrite, "%s", tempString); // Print the string into the file
        free(bday); // Free tempString
    }

    // Write the Anniversary if its part of the given VCard
    if (obj->anniversary != NULL) {
        strcpy(tempString, "");
        if (obj->anniversary->isText == true) {
            strcat(tempString, "ANNIVERSARY;VALUE=text:");
        }
        else {
            strcat(tempString, "ANNIVERSARY:");
        }
        anniversary = dateToString(obj->anniversary);
        strcat(tempString, anniversary);
        fprintf(fileToWrite, "%s", tempString); // Print the string into the file
        free(anniversary); // Free tempString
    }

    // Write the END line into the file (always a part of a valid VCard)
    fprintf(fileToWrite, "END:VCARD\r\n");

    // Close the file
    fclose(fileToWrite);

    return OK;
}

VCardErrorCode validateCard(const Card* obj) {
    // Make sure obj is not NULL
    if (obj == NULL) {
        return INV_CARD;
    }

    // Declare and initialize all needed variables
    VCardErrorCode returnCode;
    void * elem;
    Property * tempProp;
    Parameter * tempPara;
    int kindCount = 0;
    int nCount = 0;
    int genderCount = 0;
    int prodidCount = 0;
    int revCount = 0;
    int uidCount = 0;

    // ********************Validating the specifications in VCParser.h*******************

    // Validate the FN property using validateProperty function
    returnCode = validateProperty(obj->fn);

    if (returnCode != OK) {
        return returnCode;
    }
    
    // Validate the optionalProperties list using validateProperty function
    if (obj->optionalProperties == NULL) {
        return INV_CARD;
    }

    ListIterator tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        returnCode = validateProperty(elem);

        if (returnCode != OK) {
            return returnCode;
        }
    }

    // Make sure that birthday and anniversary don't show up in optionalProperties
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        if (strCaseIgnoreCmp(tempProp->name, "BDAY") == 0) {
            return INV_DT;
        }

        if (strCaseIgnoreCmp(tempProp->name, "ANNIVERSARY") == 0) {
            return INV_DT;
        }
    }

    // Validate the birthday and anniversary if they exist
    if (obj->birthday != NULL) {
        // Make sure that date and time are empty if isText is true
        if (obj->birthday->isText == true) {
            if (strcmp(obj->birthday->date, "") != 0) {
                return INV_DT;
            }

            if (strcmp(obj->birthday->time, "") != 0) {
                return INV_DT;
            }
        }
        // Make sure that text is empty if isText is false
        else {
            if (strcmp(obj->birthday->text, "") != 0) {
                return INV_DT;
            }
        }

        // Make sure that text is empty if UTC is true
        if (obj->birthday->UTC == true) {
            if (strcmp(obj->birthday->text, "") != 0) {
                return INV_DT;
            }
        }
    }

    if (obj->anniversary != NULL) {
        // Make sure that date and time are empty if isText is true
        if (obj->anniversary->isText == true) {
            if (strcmp(obj->anniversary->date, "") != 0) {
                return INV_DT;
            }

            if (strcmp(obj->anniversary->time, "") != 0) {
                return INV_DT;
            }
        }
        // Make sure that text is empty if isText is false
        else {
            if (strcmp(obj->anniversary->text, "") != 0) {
                return INV_DT;
            }
        }
        
        // Make sure that text is empty if UTC is true
        if (obj->anniversary->UTC == true) {
            if (strcmp(obj->anniversary->text, "") != 0) {
                return INV_DT;
            }
        }
    }

    // ********************Validating the specifications in RCF 6350 (some of it)*******************
    
    // Make sure that all properties have valid names from Section 6.1 - 6.9.3 of RCF 6350
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        returnCode = validatePropertyName(tempProp);

        if (returnCode != OK) {
            return returnCode;
        }
    }

    // Make sure that otherProperties does not have BEGIN, VERSION, or END
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        if (strCaseIgnoreCmp(tempProp->name, "BEGIN") == 0) {
            return INV_CARD;
        }

        if (strCaseIgnoreCmp(tempProp->name, "VERSION") == 0) {
            return INV_CARD;
        }

        if (strCaseIgnoreCmp(tempProp->name, "END") == 0) {
            return INV_CARD;
        }
    }

    // Validate the cardinality for all properties in optionalProperties
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        // KIND
        if (strCaseIgnoreCmp(tempProp->name, "KIND") == 0) {
            if (kindCount > 0) {
                return INV_PROP;
            }
            kindCount++;
        }

        // N
        if (strCaseIgnoreCmp(tempProp->name, "N") == 0) {
            if (nCount > 0) {
                return INV_PROP;
            }
            nCount++;
        }

        // GENDER
        if (strCaseIgnoreCmp(tempProp->name, "GENDER") == 0) {
            if (genderCount > 0) {
                return INV_PROP;
            }
            genderCount++;
        }

        // PRODID
        if (strCaseIgnoreCmp(tempProp->name, "PRODID") == 0) {
            if (prodidCount > 0) {
                return INV_PROP;
            }
            prodidCount++;
        }

        // REV
        if (strCaseIgnoreCmp(tempProp->name, "REV") == 0) {
            if (revCount > 0) {
                return INV_PROP;
            }
            revCount++;
        }

        // UID
        if (strCaseIgnoreCmp(tempProp->name, "UID") == 0) {
            if (uidCount > 0) {
                return INV_PROP;
            }
            uidCount++;
        }
    }

    // Validate the cardinality of the values of each property
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;
        
        // Source
        if (strCaseIgnoreCmp(tempProp->name, "SOURCE") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // KIND
        if (strCaseIgnoreCmp(tempProp->name, "KIND") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // XML
        if (strCaseIgnoreCmp(tempProp->name, "XML") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // FN
        if (strCaseIgnoreCmp(tempProp->name, "FN") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // N
        if (strCaseIgnoreCmp(tempProp->name, "N") == 0) {
            if (tempProp->values->length != 5) {
                return INV_PROP;
            }
        }

        // NICKNAME
        if (strCaseIgnoreCmp(tempProp->name, "NICKNAME") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // PHOTO
        if (strCaseIgnoreCmp(tempProp->name, "PHOTO") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // GENDER
        if (strCaseIgnoreCmp(tempProp->name, "GENDER") == 0) {
            if (tempProp->values->length != 1 && tempProp->values->length != 2) {
                return INV_PROP;
            }
        }

        // ADR
        if (strCaseIgnoreCmp(tempProp->name, "ADR") == 0) {
            if (tempProp->values->length != 7) {
                return INV_PROP;
            }
        }

        // TEL
        if (strCaseIgnoreCmp(tempProp->name, "TEL") == 0) {
            if (tempProp->values->length != 1 && tempProp->values->length != 2) {
                return INV_PROP;
            }
        }

        // EMAIL
        if (strCaseIgnoreCmp(tempProp->name, "EMAIL") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // IMPP
        if (strCaseIgnoreCmp(tempProp->name, "IMPP") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // LANG
        if (strCaseIgnoreCmp(tempProp->name, "LANG") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // TZ
        if (strCaseIgnoreCmp(tempProp->name, "TZ") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // GEO
        if (strCaseIgnoreCmp(tempProp->name, "GEO") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // TITLE
        if (strCaseIgnoreCmp(tempProp->name, "TITLE") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // ROLE
        if (strCaseIgnoreCmp(tempProp->name, "ROLE") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // LOGO
        if (strCaseIgnoreCmp(tempProp->name, "LOGO") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // ORG
        if (strCaseIgnoreCmp(tempProp->name, "ORG") == 0) {
            if (tempProp->values->length == 0) {
                return INV_PROP;
            }
        }

        // MEMBER
        if (strCaseIgnoreCmp(tempProp->name, "MEMBER") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // RELATED
        if (strCaseIgnoreCmp(tempProp->name, "RELATED") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // CATEGORIES
        if (strCaseIgnoreCmp(tempProp->name, "CATEGORIES") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // NOTE
        if (strCaseIgnoreCmp(tempProp->name, "NOTE") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // PRODID
        if (strCaseIgnoreCmp(tempProp->name, "PRODID") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // REV
        if (strCaseIgnoreCmp(tempProp->name, "REV") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // SOUND
        if (strCaseIgnoreCmp(tempProp->name, "SOUND") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // UID
        if (strCaseIgnoreCmp(tempProp->name, "UID") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // CLIENTPIDMAP
        if (strCaseIgnoreCmp(tempProp->name, "CLIENTPIDMAP") == 0) {
            if (tempProp->values->length != 2) {
                return INV_PROP;
            }
        }

        // URI
        if (strCaseIgnoreCmp(tempProp->name, "URI") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // KEY
        if (strCaseIgnoreCmp(tempProp->name, "KEY") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // FBURL
        if (strCaseIgnoreCmp(tempProp->name, "FBURL") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // CALADRURI
        if (strCaseIgnoreCmp(tempProp->name, "CALADRURI") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }

        // CALURI
        if (strCaseIgnoreCmp(tempProp->name, "CALURI") == 0) {
            if (tempProp->values->length != 1) {
                return INV_PROP;
            }
        }
    }

    // Validate each element of the values list (part of the property struct) to make sure non of the values are NULL
    returnCode = validatePropertyValues(obj->fn->values);

    if (returnCode != OK) {
        return returnCode;
    }
    
    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        returnCode = validatePropertyValues(tempProp->values);

        if (returnCode != OK) {
            return returnCode;
        }
    }

    // Validate that each parameter's name and value are not empty strings (CHECK THIS OVER AGAIN)
    tempIterator = createIterator(obj->fn->parameters);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempPara = (Parameter *)elem;

        if (tempPara->name == NULL || tempPara->value == NULL) {
            return INV_PROP;
        }

        if (strcmp(tempPara->name, "") == 0 || strcmp(tempPara->value, "") == 0) {
            return INV_PROP;
        }
    }

    tempIterator = createIterator(obj->optionalProperties);

    while ((elem = nextElement(&tempIterator)) != NULL) {
        tempProp = (Property *)elem;

        returnCode = validateParameter(tempProp);

        if (returnCode != OK) {
            return returnCode;
        }
    }

    return OK;
}

int createAndValidate (char * fileName) {
    // Declare and initialize all needed variables
    VCardErrorCode createReturnCode;
    VCardErrorCode validateReturnCode;
    Card * obj = NULL;

    // Call createCard with given file and temp obj
    createReturnCode = createCard(fileName, &obj);

    // If the returnCode is OK, call validateCard
    if (createReturnCode == OK) {
        validateReturnCode = validateCard(obj);

        // If validateReturnCode is OK, then return 1, else return 0
        if (validateReturnCode == OK) {
            deleteCard(obj); // Free the memory
            return 1;
        }
    }

    deleteCard(obj); // Free the memory
    return 0; // Return 0 in any other case
}

char * createValidateStr (char * fileName) {
    // Declare and initialize all needed variables
    VCardErrorCode createReturnCode;
    VCardErrorCode validateReturnCode;
    Card * obj = NULL;
    char * returnStr;
    char tempString [1000] = "";
    char * tempName;
    char * tempBdayTime;
    char * tempBdayDate;
    char * tempAnnTime;
    char * tempAnnDate;
    char tempLengthStr [100] = "";

    // Call createCard with given file and temp obj
    createReturnCode = createCard(fileName, &obj);

    // If the returnCode is OK, call validateCard
    if (createReturnCode == OK) {
        validateReturnCode = validateCard(obj);

        // If validateReturnCode is OK, then build the string to return
        if (validateReturnCode == OK) {
            tempName = toString(obj->fn->values);
            if (obj->birthday != NULL) {
                tempBdayTime = valueToString(obj->birthday->time);
                tempBdayDate = valueToString(obj->birthday->date);
            }
            else {
                tempBdayTime = "";
                tempBdayDate = "";
            }

            if (obj->anniversary != NULL) {
                tempAnnTime = valueToString(obj->anniversary->time);
                tempAnnDate = valueToString(obj->anniversary->date);
            }
            else {
                tempAnnTime = "";
                tempAnnDate = "";
            }

            strcat(tempString, tempName);
            strcat(tempString, ";");
            if (strcmp(tempBdayTime, "") != 0 && strcmp(tempBdayDate, "") != 0) {
                strcat(tempString, tempBdayDate);
                strcat(tempString, "T");
                strcat(tempString, tempBdayTime);
                if (obj->birthday->UTC == true) {
                    strcat(tempString, "Z");
                }
            }
            else {
                strcat(tempString, "");
            }
            strcat(tempString, ";");
            if (strcmp(tempAnnTime, "") != 0 && strcmp(tempAnnDate, "") != 0) {
                strcat(tempString, tempAnnDate);
                strcat(tempString, "T");
                strcat(tempString, tempAnnTime);
                if (obj->anniversary->UTC == true) {
                    strcat(tempString, "Z");
                }
            }
            else {
                strcat(tempString, "");
            }
            strcat(tempString, ";");
            sprintf(tempLengthStr, "%d", getLength(obj->optionalProperties));
            strcat(tempString, tempLengthStr);

            returnStr = (char *)malloc(sizeof(char) * (strlen(tempString) + 1));

            strcpy(returnStr, tempString);

            return returnStr;
        }
    }

    // Invalid file so return with a string 'NULL'
    tempName = "NULL";

    returnStr = (char *)malloc(sizeof(char) * (strlen(tempName) + 1));
    strcpy(returnStr, tempName);

    return returnStr;
}

int editFN (char * fileName, char * fn) {
    // Declare and initialize all needed variables
    VCardErrorCode createReturnCode;
    VCardErrorCode validateReturnCode;
    VCardErrorCode writeReturnCode;
    Card * obj = NULL;
    char * newFn;
    char * oldFn;

    // Call createCard with given file and temp obj
    createReturnCode = createCard(fileName, &obj);

    // If the returnCode is OK, call validateCard
    if (createReturnCode == OK) {
        validateReturnCode = validateCard(obj);

        // If validateReturnCode is OK, then change the FN property's value and save
        if (validateReturnCode == OK) {
            newFn = (char *)malloc(sizeof(char) * (strlen(fn) + 1));
            strcpy(newFn, fn);
            oldFn = obj->fn->values->head->data;
            obj->fn->values->head->data = newFn;

            validateReturnCode = validateCard(obj);

            if (validateReturnCode == OK) {
                free(oldFn);
                writeReturnCode = writeCard(fileName, obj);

                if (writeReturnCode == OK) {
                    deleteCard(obj);
                    return 1;
                }
            }
        }
    }

    deleteCard(obj);
    return 0;
}

int createFN (char * fileName, char * fn) {
    // Check if fileName is null
    if (fileName == NULL) {
        return 0;
    }

    // Check if fn is null
    if (fn == NULL) {
        return 0;
    }

    // Declare and initialize all needed variables
    FILE * fileToWrite;

    // Open the file in write mode
    fileToWrite = fopen(fileName, "w");

    // Check if the files was created or opened correctly
    if (fileToWrite == NULL) {
        return 0;
    }

    // Write the BEGIN and VERSION lines into the file (always a part of a valid VCard)
    fprintf(fileToWrite, "BEGIN:VCARD\r\n");
    fprintf(fileToWrite, "VERSION:4.0\r\n");

    // Write the fn property
    fprintf(fileToWrite, "FN:%s\r\n", fn);

    // Write the END line into the file (always a part of a valid VCard)
    fprintf(fileToWrite, "END:VCARD\r\n");

    // Close the file
    fclose(fileToWrite);

    return 1;
}
