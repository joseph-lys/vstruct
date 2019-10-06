/* indexbase.h
 * 
 */
#ifndef VSTRUCT_INDEX_BASE_H_
#define VSTRUCT_INDEX_BASE_H_


#include <stdint.h>
#include "vstructbase.h"

namespace vstruct
{
  // Base class for Item and Array types
  class _IndexBase
  {
  protected:
    uint8_t* &_buf;
    uint16_t const _b;
    uint16_t const _sz;
    public:
    /* No Default Constuctor*/
    _IndexBase() = delete;
    virtual ~_IndexBase(){}

    /* Attach accessor to buffer */
    _IndexBase(uint8_t* &buf, uint16_t position_in_bits, const uint16_t size_in_bits);

    /* Attach as next Member in VStruct*/
    _IndexBase(_VStructBase& s, const uint16_t size_in_bits);
    
    /* Returns the bit total of all previous members and this member*/
    uint16_t totalBits();
  };

} /*namespace vstruct*/




#endif /* VSTRUCT_INDEX_BASE_H_ */