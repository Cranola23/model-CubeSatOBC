#include "cobs.h"
#include <stdint.h>
#include <stdio.h>

size_t cobs_encode(const uint8_t *input, size_t length, uint8_t *output){
    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    uint8_t code = 1;

    while(read_index<length){
        if(input[read_index] == 0){
            output[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        } 
        else{
            output[write_index++] = input[read_index++];
            code++;
            if(code==0xFF){
                output[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }
    output[code_index] = code;
    return write_index; //total encoded length
}


size_t cobs_decode(const uint8_t *input, size_t length, uint8_t *output){
    size_t read_index = 0;
    size_t write_index = 0;

    while(read_index < length){
        uint8_t code = input[read_index];
        if(read_index + code > length && code != 1)
            return 0; //error
        read_index++;

        for (uint8_t i=1;i<code;i++){
            output[write_index++] = input[read_index++];
        }

        if(code!=0xFF && read_index<length){
            output[write_index++] = 0;
        }
    }
    return write_index; //total decoded length
}
