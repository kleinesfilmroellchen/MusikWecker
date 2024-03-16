/*
 * Adjusted version of AudioFileSourceSD that works with SdFs.
 */

#include <AudioFileSource.h>
#include <SdFat.h>

class AudioFileSourceSdFs : public AudioFileSource {
public:
	AudioFileSourceSdFs(SdFs& card);
	AudioFileSourceSdFs(SdFs& card, const char* filename);
	virtual ~AudioFileSourceSdFs() override;

	virtual bool open(const char* filename) override;
	virtual uint32_t read(void* data, uint32_t len) override;
	virtual bool seek(int32_t pos, int dir) override;
	virtual bool close() override;
	virtual bool isOpen() override;
	virtual uint32_t getSize() override;
	virtual uint32_t getPos() override;

	String file_name() const;

private:
	SdFs& card;
	FsFile f;
};
