
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define max(x,y) (x ^ ((x ^ y) & -(x < y)))


#define DICO_SIZE 4096
#define DICO_INIT_END 256
#define DICO_STRING_INIT 256

#define OUTPUT_BUFFER 40960
#define INPUT_BUFFER 13650*3    // Its size has to be multiple of 3


uint8_t buff_out[OUTPUT_BUFFER];
uint16_t buff_pointer_out = 0;

uint8_t* dictionary[DICO_SIZE];
uint32_t dico_alloc[DICO_SIZE];
uint32_t dico_data_size[DICO_SIZE];
uint16_t dico_i = DICO_INIT_END;


void add_to_dico(uint16_t last_string_index, uint16_t x) {

    // If dico full, reset it
    if (dico_i >= DICO_SIZE)
       dico_i = DICO_INIT_END;

    // new_string = last_string & 1 char of current_string
    int new_string_len = dico_data_size[last_string_index] + 1;

    // If not enought size to store new_string, alloc more memory
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

    // If buffer too full to add data, flush and reset
    if (data_size + buff_pointer_out > OUTPUT_BUFFER) {
        write(STDOUT_FILENO, buff_out, buff_pointer_out);
        buff_pointer_out = 0;
    }

    // Add data to the buffer
    memcpy(&buff_out[buff_pointer_out], data, data_size);
    buff_pointer_out += data_size;
}



int main(int argc, char** argv) {    

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
    

    // data is comming as follow (x and y beeing 2 symboles coded over 3 input bytes)
    // xxxx xxxx  xxxx yyyy  yyyy yyyy
    // ---------  ---------  ---------
    //    b1         b2         b3
    //
    // b3 migth be omited at the end, then it will contain only once symbole and be padded as follow
    // 0000 xxxx  xxxx xxxx
    // ---------  ---------
    //    b1         b2
    //

    bool first_run = true;
    uint16_t last_string_index = 0;

    // Its size has to be multiple of 3
    uint8_t buffer[INPUT_BUFFER];

    // This is possitive to enter the while for the 1st tine
    uint16_t data_len = 1;
    
    while(data_len) {

        data_len = read(STDIN_FILENO, buffer, sizeof(buffer));
        
        uint8_t* data = buffer;
        while((data - buffer) < data_len) {
            uint8_t b1 = (uint8_t)*data;
            uint8_t b2 = (uint8_t)*(data+1);
            uint16_t x;
        
            // If there is at least 3 bytes left, extract 2 symboles from them 
            if ((data - buffer) <= data_len - 3) {
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
    }

    write(STDOUT_FILENO, buff_out, buff_pointer_out);

    for(i = 0; i < DICO_SIZE; i++) {
        free(dictionary[i]);
    }
    
    return 0;
    
}
