#include "audiere.h"
#include "internal.h"
#include "repeatable.h"
#include "utility.h"


namespace audiere {

  ADR_EXPORT(OutputStream*, AdrOpenSound)(
    AudioDevice* device,
    SampleSource* source)
  {
    if (!device || !source) {
      return 0;
    }

    // if the stream is not seekable, we cannot know how big of a buffer
    // to allocate, so try to stream it
    if (!source->isSeekable()) {
      return device->openStream(source);
    }

    int stream_length = source->getLength();
    int channel_count, sample_rate;
    SampleFormat sample_format;
    source->getFormat(channel_count, sample_rate, sample_format);

    int stream_length_bytes =
      stream_length * channel_count * GetSampleSize(sample_format);
    u8* buffer = new u8[stream_length_bytes];
    source->setPosition(0);  // in case the source has been read from already
    source->read(stream_length, buffer);

    OutputStream* stream = device->openBuffer(
      buffer, stream_length,
      channel_count, sample_rate, sample_format);

    delete[] buffer;
    return stream;
  }

}
