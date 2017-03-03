
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define DICO_SIZE 4096
#define DICO_INIT_END 256
#define DICO_STRING_INIT 4


int main(int argc, char** argv) {

    FILE *fileptr;
    char *buffer;
    long filelen;

    char* dictionary[DICO_SIZE];
    uint32_t dico_alloc[DICO_SIZE];
    
        if (argc < 2) {
        printf("error, no input file given\n");
        return 1;
    }

    uint16_t i = 0;

    // Init dictionary
    for(i = 0; i < DICO_INIT_END; i++) {
        dictionary[i] = malloc(2);
        dictionary[i][0] = i;
        dictionary[i][1] = 0;
    }

    for(i = DICO_INIT_END; i < DICO_SIZE; i++) {
        dico_alloc[i] = DICO_STRING_INIT;
        dictionary[i] = malloc(DICO_STRING_INIT);
    }
    
    //printf("Input file : %s\n", argv[1]);

    fileptr = fopen(argv[1], "rb");       // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    buffer = (char *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
    fread(buffer, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file
    
    FILE *output = fopen("out", "w");

    uint16_t dico_i = 256;

    char* last_string = 0;

    // data is comming as follow (x and y beeing 2 different data from 3 input bytes)
    // xxxx xxxx  xxxx yyyy  yyyy yyyy
    // ---------  ---------  ---------
    //    1          2          3
    //
    // 3 migth be omited at the end, then it will be padded as follow
    // 0000 xxxx  xxxx xxxx
    // ---------  ---------
    //    1          2
    //

    
    char* data = buffer;
    do {
        uint8_t b1 = (uint8_t)*data;
        uint8_t b2 = (uint8_t)*(data+1);
        uint8_t b3 = 0;
        uint16_t x, y;

        bool three_symboles = false;
        
        // If there is at least 3 bytes left
        if (filelen - (data - buffer) >= 3) {
            b3 = (uint8_t)*(data+2);
            x = ( b1 << 4) | ( b2 >> 4 ) ;
            y = ( (b2 & 0x0F) << 8) | b3 ;
            three_symboles = true;
        } else {            
            x = (b1 << 8) | b2 ;
            y = 0;
        }

        // If there is at least 3 bytes left, loop twice
        // Otherwise just exec this once
        for(i = 0; i < (three_symboles ? 2 : 1); i++) {
                
            // Not first iteration
            if (last_string != 0) {

                if (dico_i >= DICO_SIZE)
                   dico_i = DICO_INIT_END;

                // new_string = last_string & 1 char of current_string
                int new_string_len = strlen(last_string) + 2;

                if (new_string_len > dico_alloc[dico_i]) {
                    do {
                        dico_alloc[dico_i] = dico_alloc[dico_i] * 2;
                    } while(new_string_len > dico_alloc[dico_i]);
                        
                    free(dictionary[dico_i]);
                    dictionary[dico_i] = malloc(dico_alloc[dico_i]);
                }

                // Dodgy fix for the end of the big file
                if (x > dico_i) {
                    printf("Warning, b1:0x%x b2:0x%x b3:0x%x dico_i:%d x:%d\nx >> 4\n", b1, b2, b3, dico_i, x);
                    x = x >> 4;
                }

                // add new_string to the dictionary
                strcpy(dictionary[dico_i], last_string);
                dictionary[dico_i][new_string_len-2] = dictionary[x][0];
                dictionary[dico_i][new_string_len-1] = '\0';

                dico_i++;
            }

            // Print the decompressed value for the current symbole
            fprintf(output, "%s", dictionary[x]);
            
            // Prepare for the next iteration
            last_string = dictionary[x];

            // In case we loop twice, the next symbole will be in x ready to be processed
            x = y;
        }
        
        // Consumed 2 or 3 bytes depending on three_symboles
        data += (three_symboles ? 3 : 2);
                    
    } while(data < (buffer + filelen));

    for(i = 0; i < DICO_SIZE; i++) {
        free(dictionary[i]);
    }
    free(buffer);

    return 0;
    
}
