#include "streamBuffer.h"

size_t bufferedStream::fread(void *dst, int siz, int C)
{
    // TODO implement this method
    // for now stubbed so that compilation is possible
    return 0;
}

bufferedStream::~bufferedStream()
{
    if(buffer != nullptr)
        delete[] buffer;
}

int bufferedStream::appendDataToBuffer( char * buffer, std::size_t bufSize)
{
    return -1;
}
char * bufferedStream::getNextDataPointer( std::size_t requiredSize)
{
    return buffer;
}
