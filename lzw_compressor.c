//
// Author : Jordan Penard (jordan@penard.fr)
// MIT License
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#define INPUT_BUFFER 40960
#define OUTPUT_BUFFER 13650*3    // Its size has to be multiple of 3

#define DICO_SIZE 4096
#define DICO_INIT_END 256


// The dictionary will be stored in a tree using these nodes
typedef struct node {
    struct node *next[DICO_INIT_END];
    uint16_t index;
} node_t;

// Used to buffer the output
uint8_t buff_out[OUTPUT_BUFFER];
uint16_t buff_pointer_out = 0;
bool mid_byte_write = false;


// Write 12 bits data as follow
// [0] : x11 -> x4
// [1] : x3 x2 x1 x0 y11 y10 y9 y8
// [2] : y7 -> y0
void output(uint16_t data) {

    // If buffer too full to add data, flush and reset
    if (buff_pointer_out >= OUTPUT_BUFFER) {
        write(STDOUT_FILENO, buff_out, buff_pointer_out);
        buff_pointer_out = 0;
    }

    // Add data to the buffer
    if(mid_byte_write) {
        buff_out[buff_pointer_out++] |= (data >> 8) & 0x000F;
        buff_out[buff_pointer_out++] = data & 0x00FF;
    } else {
        buff_out[buff_pointer_out++] = (data >> 4) & 0x00FF; 
        buff_out[buff_pointer_out] = (data << 4) & 0x00F0; 
    }

    mid_byte_write = !mid_byte_write;
}


void reset_dictionary(node_t *root_node) {

    uint16_t i = 0;

    memset(&root_node[1], 0, sizeof(node_t) * DICO_SIZE);

    for(i = 0; i < DICO_INIT_END; i++) {
        root_node[i+1].index = i;
    }
}


int main(int argc, char** argv) {    

    uint16_t i = 0;

    // Are building a tree here, so we need nb items + 1 for the root
    node_t *root_node = malloc(sizeof(node_t) * (DICO_SIZE+1)); 
    node_t *current_node = root_node;
    for(i = 0; i < DICO_INIT_END; i++) {
        root_node[0].next[i] = &root_node[i+1];
    }    
    reset_dictionary(current_node);

    // Next index to be writen
    uint16_t next_index = DICO_INIT_END+1;
     
    // Input buffer
    uint8_t buff_in[INPUT_BUFFER];

    // This is possitive to enter the while for the 1st tine
    uint16_t data_len = 1;

    
    while(data_len) {

        data_len = read(STDIN_FILENO, buff_in, sizeof(buff_in));
        
        uint8_t* data = buff_in;
        while((data - buff_in) < data_len) {

            // If we reached the longest existing string in the dictionary
            if(current_node->next[*data] == 0) {

                // Print the current index
                output(current_node->index);

                // Add current char to the end of found string
                current_node->next[*data] = &root_node[next_index];
                root_node[next_index].index = next_index - 1;
                next_index++;

                // If dictionary full, reset
                if (next_index > DICO_SIZE) {
                    reset_dictionary(root_node);
                    next_index = DICO_INIT_END+1;
                }

                // Re-explore tree from the top
                current_node = root_node;
                
            }
            // Progress through the tree as we found a matching string
            current_node = current_node->next[*data];

            data++;
        }
    }

    // Print the last char
    output(current_node->index);

    // If we are not alligned on a full byte from the previous write, pad as follow
    // [0] : 0 0 0 0 x11 x10 x9 x8
    // [1] : x7 x6 x5 x4 x3 x2 x1 x0
    if (mid_byte_write) {
        buff_out[buff_pointer_out] = (buff_out[buff_pointer_out-1] << 4)
                                     | (buff_out[buff_pointer_out] >> 4); 
        buff_out[buff_pointer_out-1] = buff_out[buff_pointer_out-1] >> 4;
        buff_pointer_out++;
    }

    // Write the content of the buff to the output
    write(STDOUT_FILENO, buff_out, buff_pointer_out);

    return 0;
}
