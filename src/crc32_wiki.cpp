#include "crc32.hpp" 

//Fill the lookup table -- table = the lookup table base address
// void crc32_fill(std::uint32_t *table){
//         uint8_t index=0,z;
//         do{
//                 table[index]=index;
//                 for(z=8;z;z--) table[index]=(table[index]&1)?(table[index]>>1)^0xEDB88320:table[index]>>1;
//         }while(++index);
// }