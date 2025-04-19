/* 
* Name: Gurbaaz Singh Cheema
* Date: March 27 2025
*/

#include "VCParser.h"
#include "VCHelpers.h"

int main () {
    // Declare and initialize all needed variables
    Card * obj = NULL;
    char * cardString;
    char * errorString1;
    char * errorString2;
    char * errorStringFile1;
    char * errorStringFile2;
    char * errorStringValidate1;
    char * errorStringValidate2;
    VCardErrorCode returnCode1;
    VCardErrorCode returnCode2;
    VCardErrorCode returnCodeFile1;
    VCardErrorCode returnCodeFile2;
    VCardErrorCode returnCodeValidate1;
    VCardErrorCode returnCodeValidate2;
    char * testing;

    // // Call the needed functions
    // returnCode1 = createCard("bin/cards/testCardInvEndings.vcf.vcf", &obj); 
    // errorString1 = errorToString(returnCode1);
    // cardString = cardToString(obj);
    // returnCodeFile1 = writeCard("beforeCard.vcard", obj);
    // errorStringFile1 = errorToString(returnCodeFile1);
    // returnCodeValidate1 = validateCard(obj);
    // errorStringValidate1 = errorToString(returnCodeValidate1);
    // testing = dateToString(obj->anniversary);

    // // Free the card or display a message as to why no card was created
    // if (returnCode1 == OK) {
    //     deleteCard(obj);
    //     printf("Error 1: %s\n\n", errorString1);
    //     printf("%s\n\n", errorStringFile1);
    //     printf("Validation Status: %s\n\n", errorStringValidate1);
    //     printf("%s\n\n", testing);
    //     free(testing);
    // }
    // else {
    //     printf("%s\n\n", errorString1);
    // }

    // returnCode2 = createCard("beforeCard.vcard", &obj);
    // errorString2 = errorToString(returnCode2);
    // returnCodeFile2 = writeCard("afterCard.vcard", obj);
    // errorStringFile2 = errorToString(returnCodeFile2);
    // testing = dateToString(obj->anniversary);
    // returnCodeValidate2 = validateCard(obj);
    // errorStringValidate2 = errorToString(returnCodeValidate2);

    // // Print the string
    // printf("\n%s\n\n", cardString);

    // // Free the card or display a message as to why no card was created
    // if (returnCode2 == OK) {
    //     deleteCard(obj);
    //     printf("Error 2: %s\n\n", errorString2);
    //     printf("%s\n\n", errorStringFile2);
    //     printf("Validation Status: %s\n\n", errorStringValidate2);
    //     printf("%s\n\n", testing);
    // }
    // else {
    //     printf("%s\n\n", errorString2);
    // }
    
    // // Free the strings
    // free(cardString);
    // free(errorString1);
    // free(errorString2);
    // free(errorStringFile1);
    // free(errorStringFile2);
    // free(errorStringValidate1);
    // free(errorStringValidate2);
    // free(testing);

    // Call the createFn function for testing
    printf(createValidateStr("bin/cards/testCardNoEnd.vcf"));

    return 0; // Return 0
}
