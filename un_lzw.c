
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define max(x,y) (x ^ ((x ^ y) & -(x < y)))


#define DICO_SIZE 4096
#define DICO_INIT_END 256
#define DICO_STRING_INIT 256

#define OUTPUT_BUFFER 40960


FILE *f_out;
uint8_t buff_out[OUTPUT_BUFFER];
uint16_t buff_pointer_out = 0;

uint8_t* dictionary[DICO_SIZE];
uint32_t dico_alloc[DICO_SIZE];
uint32_t dico_data_size[DICO_SIZE];
uint16_t dico_i = DICO_INIT_END;


void add_to_dico(uint16_t last_string_index, uint16_t x) {

    if (dico_i >= DICO_SIZE)
       dico_i = DICO_INIT_END;

    // new_string = last_string & 1 char of current_string
    int new_string_len = dico_data_size[last_string_index] + 1;

    if (new_string_len > dico_alloc[dico_i]) {
        dico_alloc[dico_i] = max(dico_alloc[dico_i] * 2, new_string_len);
                    
        free(dictionary[dico_i]);
        dictionary[dico_i] = malloc(dico_alloc[dico_i]);
    }

    // add new_string to the dictionary
    memcpy(dictionary[dico_i], dictionary[last_string_index], new_string_len-1);
    dictionary[dico_i][new_string_len-1] = dictionary[x][0];
    dico_data_size[dico_i] = new_string_len;

    dico_i++;
}

void output(uint8_t* data, uint32_t data_size) {

    if (data_size + buff_pointer_out > OUTPUT_BUFFER) {
        fwrite(buff_out, 1, buff_pointer_out, f_out);
        buff_pointer_out = 0;
    }

    memcpy(&buff_out[buff_pointer_out], data, data_size);
    buff_pointer_out += data_size;
}



int main(int argc, char** argv) {

    FILE *fileptr;
    char *buffer;
    long filelen;
    

    if (argc < 2) {
        printf("error, no input file given\n");
        return 1;
    }

    uint16_t i = 0;

    // Init dictionary
    for(i = 0; i < DICO_INIT_END; i++) {
        dictionary[i] = malloc(1);
        dictionary[i][0] = i;
        dico_data_size[i] = 1;
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

    f_out = fopen("out_v2", "wb");


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

    bool first_run = true;
    uint16_t last_string_index = 0;
    
    char* data = buffer;
    while(data < (buffer + filelen)) {
        uint8_t b1 = (uint8_t)*data;
        uint8_t b2 = (uint8_t)*(data+1);
        uint16_t x;
        
        // If there is at least 3 bytes left
        if (filelen - (data - buffer) >= 3) {
            uint8_t b3 = (uint8_t)*(data+2);
            
            x = ( b1 << 4) | ( b2 >> 4 ) ;
            uint16_t y = ( (b2 & 0x0F) << 8) | b3 ;
            
            // Not first iteration
            if (!first_run) {
                add_to_dico(last_string_index, x);
            }
            first_run = false;

            // Print the decompressed value for the current symbole
            output(dictionary[x], dico_data_size[x]);
            
            // Prepare for the next iteration
            last_string_index = x;
            x = y;

            // Consumed 3 bytes
            data += 3;
                        
        } else {            
            x = (b1 << 8) | b2 ;

            // Consumed 2 bytes
            data += 2;
        }

        add_to_dico(last_string_index, x);

        // Print the decompressed value for the current symbole
        output(dictionary[x], dico_data_size[x]);
            
        // Prepare for the next iteration
        last_string_index = x;                    
    }


    fwrite(buff_out, 1, buff_pointer_out, f_out);
    fclose(f_out); // Close the file

    for(i = 0; i < DICO_SIZE; i++) {
        free(dictionary[i]);
    }
    free(buffer);

    return 0;
    
}
