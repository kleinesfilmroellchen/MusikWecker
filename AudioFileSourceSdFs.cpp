#include "AudioFileSourceSdFs.h"

AudioFileSourceSdFs::AudioFileSourceSdFs(SdFs& card)
    : card(card)
{
}

AudioFileSourceSdFs::AudioFileSourceSdFs(SdFs& card, const char* filename)
    : card(card)
{
    open(filename);
}

bool AudioFileSourceSdFs::open(const char* filename)
{
    f = card.open(filename, O_RDONLY);
    return f;
}

AudioFileSourceSdFs::~AudioFileSourceSdFs()
{
    if (f)
        f.close();
}

uint32_t AudioFileSourceSdFs::read(void* data, uint32_t len)
{
    if (!f)
        return 0;

    return f.read(static_cast<uint8_t*>(data), len);
}

bool AudioFileSourceSdFs::seek(int32_t pos, int dir)
{
    if (!f)
        return false;
    if (dir == SEEK_SET)
        return f.seek(pos);
    else if (dir == SEEK_CUR)
        return f.seek(f.position() + pos);
    else if (dir == SEEK_END)
        return f.seek(f.size() + pos);
    return false;
}

bool AudioFileSourceSdFs::close()
{
    if (!f)
        return false;

    f.close();
    return true;
}

bool AudioFileSourceSdFs::isOpen()
{
    return f ? true : false;
}

uint32_t AudioFileSourceSdFs::getSize()
{
    if (!f)
        return 0;

    return f.size();
}

uint32_t AudioFileSourceSdFs::getPos()
{
    if (!f)
        return 0;

    return f.position();
}
